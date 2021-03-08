#include <opencv2/opencv.hpp>
#include <vector>
#include "debug.h"

using namespace cv;
using namespace std;

Rect selection;
int smin = 30;
int vmin = 40;
int bins = 16;

ssize_t CAMShiftTrace(void)
{
    VideoCapture capture;
    capture.open("../bin/0.mp4");
    if (!capture.isOpened()) {
        SHOW(("video not open\n"));
        return false;
    }
    SHOW(("video opened\n"));
    bool firstRead = true;
    float hranges[] = {0, 180};
    const float* p_hranges = hranges;
    Mat frame, hsv, hue, hist, mask, backproject;
    Mat drawImg = Mat::zeros(300, 300, CV_8UC3);

    cv::namedWindow("CAM Tracking", CV_WINDOW_AUTOSIZE);
    cv::namedWindow("ROI Histogram", CV_WINDOW_AUTOSIZE);

    while(capture.read(frame)) {
        if(firstRead) {
            Rect2d first = selectROI("CAM Tracking", frame);
            selection.x = first.x;
            selection.y = first.y;
            selection.width = first.width;
            selection.height = first.height;
        }
        //convert to HSV 转换到HSV色彩空间
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        inRange(hsv, Scalar(0,smin,vmin), Scalar(180, 255, 255), mask);
        //获取单通道的值
        hue = Mat(hsv.size(), hsv.depth());
        int channels[] = {0,0};
        mixChannels(&hsv, 1, &hue, 1, channels, 1);
        //直方图计算
        if (firstRead) {
            Mat roi(hue, selection);
            Mat maskroi(mask, selection);
            calcHist(&roi, 1, 0, maskroi, hist, 1, &bins, &p_hranges);
            normalize(hist, hist, 0, 255, NORM_MINMAX);
            //show histogram 绘制直方图
            int binw = drawImg.cols/bins;
            Mat colorIndex = Mat(1, bins, CV_8UC3);
            for (size_t i = 0; i < bins; ++i) {
                colorIndex.at<Vec3b>(0,i) = Vec3b(saturate_cast<uchar>(i*180/bins), 255, 255);
            }
            cvtColor(colorIndex, colorIndex, COLOR_HSV2BGR);
            for (size_t i = 0; i < bins; ++i) {
                int val = saturate_cast<int>(hist.at<float>(i)*drawImg.rows/255);
                rectangle(drawImg, Point(i*binw,drawImg.rows), Point((i+1)*binw,drawImg.rows-val), Scalar(colorIndex.at<Vec3b>(0,i)),-1,8,0);
            }
           firstRead = false;
        }
        //back projection
        calcBackProject(&hue, 1, 0, hist, backproject, &p_hranges);
        //CAMShift tracking
        backproject &= mask;
        RotatedRect trackBox = CamShift(backproject, selection, TermCriteria((TermCriteria::COUNT|TermCriteria::EPS),10,1));

        //draw location on frame
        //ellipse(frame, trackBox, Scalar(0,0,255), 3, 8);
        rectangle(frame, trackBox.boundingRect(), Scalar(0,255,0), 3, LINE_8);//绘制矩形
        imshow("CAM Tracking", frame);
        imshow("ROI Histogram", drawImg);
        waitKey(20);
    }
    SHOW(("video end\n"));
    return true;
}

ssize_t Test()
{
    cv::Mat img, out, hsv, bin, canny;

    // 读入一张图片
    img = imread("../bin/1.png", IMREAD_COLOR);
    // 判断是否成功读入图片
    if(img.empty()) {
        SHOW(("could not open image...\n"));
        return false;
    }
    // 创建一个名为 "input" 窗口
    namedWindow("input", CV_WINDOW_AUTOSIZE);
    namedWindow("output", CV_WINDOW_AUTOSIZE);
	namedWindow("canny", CV_WINDOW_AUTOSIZE);

    // 在窗口中显示输入原图片
    imshow("input", img);
	out = Mat::zeros(img.size(), img.type());

    cvtColor(img, hsv, CV_BGR2HSV);

    //inRange(hsv, Scalar(0,43,46), Scalar(30, 255, 255), bin);
    inRange(img, Scalar(0,0,127), Scalar(125, 125, 255), bin);
	imshow("canny", bin);
    //直方图均衡化，提高对比度
    equalizeHist(bin, bin);
    //形态学操作
	cv::Mat kopen = getStructuringElement(MORPH_RECT, Size(3,3), Point(-1,-1));
	cv::Mat kclose = getStructuringElement(MORPH_RECT, Size(3,3), Point(-1,-1));
    morphologyEx(bin, bin, MORPH_CLOSE, kclose, Point(-1,-1));
    morphologyEx(bin, bin, MORPH_OPEN, kopen, Point(-1,-1));
    //高斯模糊降噪
    GaussianBlur(bin, bin, Size(3,3), 11, 11);
    //Canny边缘检测
	Canny(bin, canny, 180, 234, 3, false);
    //轮廓查找
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	findContours(canny, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,0));
	RNG rng(12345);
	for (size_t i = 0; i < contours.size(); ++i)
	{
        //面积过滤
        double area = contourArea(contours[i]);
        if(area < 100.0) {
           continue;
        }
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(out, contours, i, color, 2, LINE_8, hireachy, 1, Point(0,0));
	}
	imshow("output", out);
    // 等待 1 min 后窗口自动关闭
    waitKey(60000);
    return true;
}

void CallBack(void* param)
{
    SHOW(("CallBack%p\n", param));
    //CAMShiftTrace();
    //Test();
}