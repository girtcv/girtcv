/* motto: We don't produce code, we're just GitHub porters!
 * ============================================================================
 *
 *       Filename:  port.cpp
 *
 *    Description:  串口类:实现了串口的设置、接收和发送等
 *
 *        Version:  1.0
 *        Created:  2021年01月14日 星期四 01时24分49秒
 *       Compiler:  GCC
 *
 *         Author:  shing
 *          Email:  girtcv@163.com
 *   Organization:  GIRT
 *
 *        License:  The GPL License
 *      CopyRight:  See Copyright Notice in LICENSE.
 *
 * ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "port.hpp"
#include "debug.h"

//波特率 115200
#define BAUDRATE 115200;
const speed_t speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
const speed_t name_arr[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};


///@brief 波特率 115200 数据位 8位 停止位 1位 校验类型 无校验
///@param portpath：串口路径
CSerialPort::CSerialPort(const char * portpath)
{
    errCode = 0;
    //O_RDWR:可读可写 O_NOCTTY:不要把设备用作控制终端
    //O_NONBLOCK:找不到设备立即返回-1
    fd = open(portpath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    SHOW(("open serial port:%d\n", fd));
    if(fd < 0) {
        errCode = 1;
        SHOW(("open serial port fail...\n"));
        return;
    }

    speed = BAUDRATE;
    databits = 8;
    stopbits = 1;
    parity = 'N';
    rdataLen = wdataLen = 0;
    for (size_t i = 0; i < sizeof(readBuf)/sizeof(unsigned char); ++i) {
        readBuf[i] = sendBuf[i] = 0; //初始化缓冲区
    }

    if(!InitSerialPort()) {//初始化串口
    //如果串口初始化失败
        close(fd);
        fd = -1;
    }
}


bool CSerialPort::InitSerialPort(void)
{
    struct termios options;

    //tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,
    //该函数还可以测试配置是否正确，串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    if(tcgetattr(fd,&options) != 0) {
        SHOW(("Setup serial port error\n"));
        errCode = 2;
        return false;
    }

    //设置串口输入波特率和输出波特率
    for (size_t i= 0;  i < sizeof(speed_arr) / sizeof(int); ++i) {
        if (speed == name_arr[i]) {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            break;
        }
    }

    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    //设置数据流控制
    switch(0) {
        case 0://不使用流控制
        default:
            options.c_cflag &= ~CRTSCTS;
            break;
        case 1://使用硬件流控制
            options.c_cflag |= CRTSCTS;
            break;
        case 2://使用软件流控制
            options.c_cflag |= IXON | IXOFF | IXANY;
            break;
    }

    //设置数据位
    //屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits) {
#if 0
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag |= CS6;
            break;
#endif
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            SHOW(("Unsupported data size\n"));
            errCode = 3;
            return false;
    }

    //设置校验位
    switch (parity) {
        case 'n':
        case 'N': //无奇偶校验位
                 options.c_cflag &= ~PARENB;
                 options.c_iflag &= ~INPCK;
                 break;
        case 'o':
        case 'O'://设置为奇校验(Odd)
                 options.c_cflag |= (PARODD | PARENB);
                 options.c_iflag |= INPCK;
                 break;
        case 'e':
        case 'E'://设置为偶校验(Even)
                 options.c_cflag |= PARENB;
                 options.c_cflag &= ~PARODD;
                 options.c_iflag |= INPCK;
                 break;
        case 's':
        case 'S': //设置为空格(Space)
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break;
        default:
                 SHOW(("Unsupported parity\n"));
                 errCode = 4;
                 return false;
    }

    // 设置停止位
    switch (stopbits) {
        case 1: options.c_cflag &= ~CSTOPB; break;
        case 2: options.c_cflag |= CSTOPB; break;
        default:
            SHOW(("Unsupported stop bits\n"));
            errCode = 5;
            return false;
    }

    //修改输出模式，原始数据输出
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //options.c_lflag &= ~(ISIG | ICANON);

    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; //读取一个字符等待1*(1/10)s
    options.c_cc[VMIN] = 1;  //读取字符的最少个数为1

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd,TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）TCSANOW:使配置立即生效
    if (tcsetattr(fd,TCSANOW,&options) != 0) {
        SHOW(("com set error!\n"));
        errCode = 6;
        return false;
    }

    return true;
}

/**
 * @brief 接收串口数据,将接收到的数据写入readBuf;
 * GetReadBufPtr()可以获取readBuf指针;
 * 调用本函数前需要提前设置接收数据长度
 * @param rdataLen 要接收数据的长度[通过SetrDataLen(rDataLen)设置]
 * @return 接收成功返回读取内容的长度，失败返回0
 */
ssize_t CSerialPort::Read()
{
    fd_set fs_read;   
    struct timeval timeout;    
       
    FD_ZERO(&fs_read);  // 清空串口接收端口集   
    FD_SET(fd,&fs_read);// 设置串口接收端口集    
       
    timeout.tv_sec = 10;//设置超时时间    
    timeout.tv_usec = 0;

    //使用select实现串口的多路通信    
    if(select(fd+1, &fs_read, NULL, NULL, &timeout) > 0) {
        for (size_t i = 0; i < rdataLen; ++i) {
            readBuf[i] = 0;//初始化缓冲区
        }
            
        size_t len = read(fd, readBuf, rdataLen);
        if (len == rdataLen) {
            //write(STDOUT_FILENO, readBuf, len);
            return len;
        } else {
            SHOW(("read data fail...\n"));
            tcflush(fd,TCIFLUSH);
            return 0;
        }    
    } else {
        SHOW(("nothing data...\n"));
        return false;    
    }
}

/**
 * @brief 串口发送数据(需要提前将发送内容写入sendBuf,GetSendBufPtr()可以获取sendBuf指针)
 * @param wdataLen 要发送数据的长度[通过SetwDataLen(wDataLen)设置]
 * @return 发送成功返回发送内容的长度，失败返回0
 */
ssize_t CSerialPort::Send()
{
    size_t len = write(fd, sendBuf, wdataLen);
    if (len == wdataLen) {
        SHOW(("send data succeed!\n"));
        return len;
    } else {
        SHOW(("send data fail...\n"));
        tcflush(fd,TCOFLUSH);
        return 0;
    }
}
