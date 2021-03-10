#include <iostream>
//#include <string>
//#include <cstring>
//#include <opencv2/opencv.hpp>
#include <cmath>


#include "debug.h"
#include "path.h"
#include "RGBConvert.h"
#include "camera.hpp"
#include "port.hpp"
#include "threadpool.hpp"
#include "memorypool.hpp"
//#include "objectpool.hpp"

using namespace cv;
using namespace Dahua::GenICam;
using namespace Dahua::Infra;
using namespace Dahua::Memory;
using namespace girtcv;
using std::cout;
using std::endl;
using std::cin;
using std::thread;
using std::vector;

//实例化相机类
CSingleCamera* CSingleCamera::instance = new CSingleCamera();
//实例化串口类
CSerialPort CSerialPort::port = CSerialPort("/dev/ttyUSB0");
//Rect roi;

void PrintCameraInfo(ICameraPtr& cameraSptr)
{
	// 获取采图图像宽度和高度
	//getWidth(cameraSptr, nWidth);
    //getHeight(cameraSptr, nHeight);
	//printf("图像宽度和高度:%d && %d\n", nWidth, nHeight);
	// 获取传感器采样率
	long nWidth = 0, nHeight = 0;
	getResolution(cameraSptr, nWidth, nHeight);
	std::cout << "分辨率  ：width: " << nWidth
	<< " && height: " << nHeight << std::endl;
	// 获取最大分辨率
	getMaxResolution(cameraSptr, nWidth, nHeight);
	std::cout << "Max分辨 ：width: " << nWidth
	<< " && height: " << nHeight << std::endl;

	// 获取曝光时间
	double dExposureTime = 0;
    //setExposureTime(cameraSptr, 10000, false);
    getExposureTime(cameraSptr, dExposureTime);
	std::cout << "曝光时间：" << dExposureTime << std::endl;
	// 获取曝光范围
	double dMinExposure = 0, dMaxExposure = 0;
    getExposureTimeMinMaxValue(cameraSptr, dMinExposure, dMaxExposure);
	std::cout << "曝光范围：" << dMinExposure << " ~ " << dMaxExposure << std::endl;

	// 获取增益值
	double dGainRaw = 0;
    getGainRaw(cameraSptr, dGainRaw);
	std::cout << "增益值  ：" << dGainRaw << std::endl;
	// 获取最大增益值
    double dGainRawMin = 0;
    double dGainRawMax = 0;
    getGainRawMinMaxValue(cameraSptr, dGainRawMin, dGainRawMax);
	std::cout << "增益范围：" << dGainRawMin << " ~ " << dGainRawMax << std::endl;

	// 获取伽马值
	double dGamma = 0;
    getGamma(cameraSptr, dGamma);
	std::cout << "伽马值  ：" << dGamma << std::endl;
	// 获取最大伽马值
    double dGammaMin = 0;
    double dGammaMax = 0;
    getGammaMinMaxValue(cameraSptr, dGammaMin, dGammaMax);
	std::cout << "伽马范围：" << dGammaMin << " ~ " << dGammaMax << std::endl;
	// 获取白平衡值（有三个白平衡值）
	double dRedBalanceRatio = 0;
    double dGreenBalanceRatio = 0;
    double dBlueBalanceRatio = 0;
    getBalanceRatio(cameraSptr, dRedBalanceRatio, dGreenBalanceRatio, dBlueBalanceRatio);
	std::cout << "白平衡值：B:" << dBlueBalanceRatio << " G:" << dGreenBalanceRatio
	<< " R:" << dRedBalanceRatio << std::endl;
	// 获取白平衡值范围
    double dMinBalanceRatio = 0;
    double dMaxBalanceRatio = 0;
    getBalanceRatioMinMaxValue(cameraSptr, dMinBalanceRatio, dMaxBalanceRatio);
	std::cout << "平衡范围：" << dMinBalanceRatio << " ~ " << dMaxBalanceRatio << std::endl;
}

bool InitCamera(ICameraPtr& cameraSptr)
{
	// 设置传感器采样率（采集分辨率）
    setResolution(cameraSptr, 640, 480);
	// 设置曝光值(曝光、自动曝光/手动曝光)
    setExposureTime(cameraSptr, 3000, false);
	//setBinning(cameraSptr);
	// 设置增益值
    setGainRaw(cameraSptr, 1.2);
    // 设置伽马值
	setGamma(cameraSptr, 0.8);
	// 设置白平衡（RGB增益）
    setBalanceRatio(cameraSptr, 0.9, 1.0, 2.0);
    //setBalanceRatio(cameraSptr, 1.0, 1.0, 1.0);

	return true;
}

#if 0
bool GetFrameToMat(CFrame& frame, OUT Mat& img)
{
	CFrame frameClone = frame.clone();
	TSharedPtr<FrameBuffer> PtrFrameBuffer(new FrameBuffer(frameClone));
    if (!PtrFrameBuffer)
    {
        SHOW(("create PtrFrameBuffer failed!\n"));
        return false;
    }

	uint8_t *pSrcData = new (std::nothrow) uint8_t[frameClone.getImageSize()];
    if (!pSrcData) {
        SHOW(("new pSrcData failed!\n"));
        return false;
    }

	memcpy(pSrcData, frameClone.getImage(), frameClone.getImageSize());

	int dstDataSize = 0;
    IMGCNV_SOpenParam openParam;
    openParam.width = PtrFrameBuffer->Width();
    openParam.height = PtrFrameBuffer->Height();
    openParam.paddingX = PtrFrameBuffer->PaddingX();
    openParam.paddingY = PtrFrameBuffer->PaddingY();
    openParam.dataSize = PtrFrameBuffer->DataSize();
    openParam.pixelForamt = PtrFrameBuffer->PixelFormat();

    IMGCNV_EErr status = IMGCNV_ConvertToBGR24(pSrcData, &openParam, PtrFrameBuffer->bufPtr(), &dstDataSize);
    if (IMGCNV_SUCCESS != status)
    {
        delete[] pSrcData;
        return false;
    }

    delete[] pSrcData;

    //将读进来的帧数据转化为opencv中的Mat格式操作
    Size size;
    size.height = PtrFrameBuffer->Height();
    size.width = PtrFrameBuffer->Width();
    img = Mat(size, CV_8UC3, PtrFrameBuffer->bufPtr()).clone();
    frameClone.reset();
    return true;
}
#endif

void ProcessFrame(Mat& mask, Rect& rect)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
//	findContours(mask, contours, hireachy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0,0));
	findContours(mask, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,0));

	if(contours.size()>0) {
		//double maxArea = 0.0;
		for (size_t i = 0; i < contours.size(); ++i)
		{
			//double area = contourArea(contours[i]);
			//if(area>maxArea) {
			//	maxArea = area;
			//	rect = boundingRect(contours[i]);
			//}
			drawContours(mask, contours, i, Scalar(0,0,255),3,LINE_8,hireachy,1,Point(0,0));
		}
	} else {
		rect.x = rect.y = rect.width = rect.height = 0;
	}
}
int value = 127;

void TrackbarCallBack(int pos, void* userdata)
{
	cv::Mat canny;
	cv::Mat out = Mat::zeros(static_cast<cv::Mat*>(userdata)->size(), static_cast<cv::Mat*>(userdata)->type());

	vector<vector<Point>> contours;
	Canny(*static_cast<cv::Mat*>(userdata), canny, pos, value*2, 3, false);
	imshow("gray", canny);
	vector<Vec4i> hireachy;
	findContours(canny, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,0));
	RNG rng(12345);
	for (size_t i = 0; i < contours.size(); ++i)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(out, contours, i, color, 2, LINE_8, hireachy, 1, Point(0,0));
	}
	imshow("output", out);
}

void ThreadOfGetFrame(void* param = nullptr)
{
	// 获取流对象
	auto streamPtr = CSingleCamera::GetInstance()->GetStreamPtr();
	//开始采集图像
	bool isStartGrabbingSuccess = streamPtr->startGrabbing();
	if (!isStartGrabbingSuccess) {
		SHOW(("StartGrabbing  fail.\n"));
		return;
	}

	cv::Mat img;
	cv::Mat out;
	cv::Mat gray;
	cv::Mat bin;
	cv::Mat binary;
	cv::Mat kernel = getStructuringElement(MORPH_RECT, Size(3,3), Point(-1,-1));
	cv::Mat kernel1 = getStructuringElement(MORPH_RECT, Size(5,5), Point(-1,-1));
	//cv::Mat binary;
	Ptr<BackgroundSubtractor> p_MOG2 = createBackgroundSubtractorMOG2();
	Ptr<BackgroundSubtractor> p_KNN = createBackgroundSubtractorKNN();
	namedWindow("input", CV_WINDOW_AUTOSIZE);
	//namedWindow("output", CV_WINDOW_AUTOSIZE);
	//namedWindow("gray", CV_WINDOW_AUTOSIZE);
	//namedWindow("gray", CV_WINDOW_AUTOSIZE);
	//namedWindow("binary", CV_WINDOW_AUTOSIZE);
	//createTrackbar("bar", "gray", &value, 255, TrackbarCallBack, &gray);

	while (!(waitKey(30) == 27)) {
		CFrame frame;
		//主动采集图像
		streamPtr->getFrame(frame, 200);
		//判断帧的有效性
    	if (!frame.valid()){
    	    SHOW(("frame is invalid!\n"));
    	    continue;
    	}
		//MYDBG("imgSize", frame.getImageSize());
		FrameToBGRMat(frame, img);
		//GetFrameToMat(frame, img);
		//img.convertTo(out, CV_8UC3);
		cvtColor(img, gray, COLOR_BGR2GRAY);
		//二值化
		//adaptiveThreshold(gray, bin, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, 1.0);
		threshold(gray, binary, 0, 255, THRESH_BINARY|THRESH_OTSU);
		//bitwise_not(img, img);
		//p_MOG2->apply(img, out);
		//p_KNN->apply(img, gray);
		//inRange(img, Scalar(0,0,127), Scalar(125, 125, 255), out);
		//morphologyEx(out, out, MORPH_OPEN, kernel, Point(-1,-1));
		//dilate(out, out, kernel1, Point(-1,-1), 2);//膨胀
		//ProcessFrame(out, roi);//轮廓发现与位置标记
		//rectangle(img, roi, Scalar(0,255,0), 3, LINE_8);//绘制矩形
		//putText(img, "hello world", Point(11,33), CV_FONT_BLACK, 1.0, Scalar(12,255,200));

		//output image
		//MYDBG("type", img.type());
		//MYDBG("size", img.size());

		imshow("input", img);
		//imshow("output", out);
		//imshow("gray", binary);
		//imshow("binary", bin);
		// 等待60000 ms后窗口自动关闭
		//waitKey(60000);
	}

	//停止采集图像
	streamPtr->stopGrabbing();
	TAG();
}
extern void CallBack(void*);

int main(int argc, const char* argv[])
{
	ssize_t errCode = CSingleCamera::GetInstance()->GetErrCode();
	if(errCode) {//创建相机对象并且判断是否创建成功
	//	return errCode;
	}
	ICameraPtr cameraSptr = CSingleCamera::GetInstance()->GetCameraPtr();
	//初始化相机设备
	InitCamera(cameraSptr);
#ifdef _DBG
	//PrintCameraInfo(cameraSptr);
#endif

	errCode = CSerialPort::GetSerialPort().GetErrCode();
	if(errCode) {//创建串口对象并且判断是否初始化成功
	//	return errCode;
	}

	//初始化线程
	CThreadPool<CTask> pool(1);
	for (size_t i = 0; i < 100; i++)
	{
	//	pool.push_task(new CTask(&CallBack, (void*)i));
	}
	//pool.push_task(new CTask(&CallBack, (void*)0xCCCCCCCC));
	//pool.push_task(std::make_shared<CTask>(&CallBack, (void*)0x12345678));

	//ThreadOfGetFrame();

	TAG();
    return 0;
}