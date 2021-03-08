//#pragma once
#ifndef _PORT_HPP_
#define _PORT_HPP_

#include <termios.h>
#include <unistd.h>
#include "debug.h"

#ifndef BUFSIZE //定义缓冲区大小
#define BUFSIZE 128
#endif

class CSerialPort
{
public:
    ///@brief 获取串口类实例化的单例对象
    static CSerialPort& GetSerialPort(){
        return port;
    }

    ssize_t Read();
    ssize_t Send();

    ///@brief 获取串口接收缓冲区readBuf首地址指针
    inline unsigned char* GetReadBufPtr() {
        return static_cast<unsigned char*>(readBuf);
    }

    ///@brief 获取串口发送缓冲区sendBuf首地址指针
    inline unsigned char* GetSendBufPtr() {
        return static_cast<unsigned char*>(sendBuf);
    }

    ///@brief 设置要接收的数据长度
    inline void SetrDataLen(size_t rDataLen) {
        this->rdataLen = rDataLen;
    }

    ///@brief 获取要接收的数据长度
    inline size_t GetrDataLen(void) const {
        return rdataLen;
    }

    ///@brief 设置要发送的数据长度
    inline void SetwDataLen(size_t wDataLen) {
        this->wdataLen = wDataLen;
    }

    ///@brief 获取要发送的数据长度
    inline size_t GetwDataLen(void) const {
        return wdataLen;
    }

    ///@brief 获取错误码 0：正确返回
    //1：打开串口失败 2：串口号fd不可用
    //3：数据位设置错误 4: 校验位设置错误
    //5：停止位设置错误 6：激活配置失败
    inline int GetErrCode(void) const {
        return errCode;
    }

private:
    CSerialPort(const char *);
    CSerialPort(CSerialPort&&) = default;
    CSerialPort(const CSerialPort&) = delete;
    CSerialPort& operator=(CSerialPort&&) = delete;
    CSerialPort& operator=(const CSerialPort&) = delete;

    ~CSerialPort(){
        if (fd >= 0) {
            SHOW(("fd close succeed\n"));
            close(fd);
        }
    }

    bool InitSerialPort();

    //串口的单例对象
    static CSerialPort port;
    //串口号
    int fd;
    //串口传输速度(即波特率 默认115200)
    speed_t speed;
    //数据位(默认8位)
    unsigned int databits;
    //停止位(默认1位)
    unsigned int stopbits;
    //校验类型(默认不校验)
    unsigned int parity;
    //错误码
    int errCode;
    //要接收的数据长度
    size_t rdataLen;
    //要发送的数据长度
    size_t wdataLen;
    //数据接收缓冲区
    unsigned char readBuf[BUFSIZE];
    //数据发送缓冲区
    unsigned char sendBuf[BUFSIZE];
};
#endif