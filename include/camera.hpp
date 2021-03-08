#pragma once
#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#ifndef __unix__
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include "GenICam/System.h"
#include "debug.h"

namespace girtcv {

using namespace::Dahua::GenICam;
using namespace::Dahua::Infra;

/* 4、设置相机采图模式（连续采图、触发采图） */
int32_t setGrabMode(ICameraPtr& cameraSptr, bool bContious);

/* 5、获取相机采图模式 */
int32_t getGrabMode(ICameraPtr& cameraSptr, bool &bContious);

/* 6、软件触发 */
int32_t triggerSoftware(ICameraPtr& cameraSptr);

/* 7、设置binning (Off X Y XY) */
int32_t setBinning(ICameraPtr& cameraSptr);

/* 9、设置传感器采样率（采集分辨率） */
int32_t setResolution(ICameraPtr& cameraSptr, int nWidth, int nHeight);

/* 10、获取传感器采样率 */
int32_t getResolution(ICameraPtr& cameraSptr, int64_t &nWidth, int64_t &nHeight);

/* 11、获取传感器最大分辩率 */
int32_t getMaxResolution(ICameraPtr& cameraSptr, int64_t &nWidthMax, int64_t &nHeightMax);

/* 12、设置图像ROI */
int32_t setROI(ICameraPtr& cameraSptr, int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight);

/* 13、获取图像ROI */
int32_t getROI(ICameraPtr& cameraSptr, int64_t &nX, int64_t &nY, int64_t &nWidth, int64_t &nHeight);

/* 14、获取采图图像宽度 */
int32_t getWidth(ICameraPtr& cameraSptr, int64_t &nWidth);

/* 15、获取采图图像高度 */
int32_t getHeight(ICameraPtr& cameraSptr, int64_t &nHeight);

/* 17、设置曝光值(曝光、自动曝光/手动曝光) */
int32_t setExposureTime(ICameraPtr& cameraSptr, double dExposureTime, bool bAutoExposure = false);

/* 18、获取曝光时间 */
int32_t getExposureTime(ICameraPtr& cameraSptr, double &dExposureTime);

/* 19、获取曝光范围 */
int32_t getExposureTimeMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue);

/* 20、设置增益值 */
int32_t setGainRaw(ICameraPtr& cameraSptr, double dGainRaw);

/* 21、获取增益值 */
int32_t getGainRaw(ICameraPtr& cameraSptr, double &dGainRaw);

/* 22、获取增益值范围 */
int32_t getGainRawMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue);

/* 23、设置伽马值 */
int32_t setGamma(ICameraPtr& cameraSptr, double dGamma);

/* 24、获取伽马值 */
int32_t getGamma(ICameraPtr& cameraSptr, double &dGamma);

/* 25、获取伽马值范围 */
int32_t getGammaMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue);

/* 26、设置白平衡值（有三个白平衡值） */
int32_t setBalanceRatio(ICameraPtr& cameraSptr, double dRedBalanceRatio, double dGreenBalanceRatio, double dBlueBalanceRatio);

/* 27、获取白平衡值（有三个白平衡值） */
int32_t getBalanceRatio(ICameraPtr& cameraSptr, double &dRedBalanceRatio, double &dGreenBalanceRatio, double &dBlueBalanceRatio);

/* 28、获取白平衡值范围 */
int32_t getBalanceRatioMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue);

/* 29、设置采图速度（秒帧数） */
int32_t setAcquisitionFrameRate(ICameraPtr& cameraSptr, double dFrameRate);

/* 30、获取采图速度（秒帧数） */
int32_t getAcquisitionFrameRate(ICameraPtr& cameraSptr, double &dFrameRate);

/* 31、保存参数 */
int32_t userSetSave(ICameraPtr& cameraSptr);

/* 32、加载参数 */
int32_t loadUserSet(ICameraPtr& cameraSptr);

/* 33、设置外触发延时时间 */
int32_t setTriggerDelay(ICameraPtr& cameraSptr, double dDelayTime);

/* 34、获取外触发延时时间 */
int32_t getTriggerDelay(ICameraPtr& cameraSptr, double &dDelayTime);

/* 35、设置外触发模式（上升沿触发、下降沿触发） */
int32_t setLineTriggerMode(ICameraPtr& cameraSptr, bool bRisingEdge);

/* 36、获取外触发模式（上升沿触发、下降沿触发） */
int32_t getLineTriggerMode(ICameraPtr& cameraSptr, bool &bRisingEdge);

/* 37、设置外触发信号滤波时间 */
int32_t setLineDebouncerTimeAbs(ICameraPtr& cameraSptr, double dLineDebouncerTimeAbs);

/* 38、获取外触发信号滤波时间 */
int32_t getLineDebouncerTimeAbs(ICameraPtr& cameraSptr, double &dLineDebouncerTimeAbs);

// 39、设置外触发脉冲宽度（不支持）
// 40、获取外触发脉冲宽度（不支持）
// 41、设置输出信号线（控制光源用）（面阵相机是Line0）
// 42、获取输出信号线（面阵相机是Line0）

/* 43、设置外部光源曝光时间（设置输出值为TRUE的时间） */
int32_t setOutputTime(ICameraPtr& cameraSptr, int nTimeMS);

// 44、获取外部光源曝光时间（输出信号的时间由软件侧控制）
/* 45、设置X轴翻转 */
int32_t setReverseX(ICameraPtr& cameraSptr, bool flag);

/* 46、设置Y轴翻转 */
int32_t setReverseY(ICameraPtr& cameraSptr, bool flag);

/* 50、修改曝光时间 （与相机连接之后调用）*/
void modifyCamralExposureTime(CSystem &systemObj, ICameraPtr& cameraSptr);

// 51、显示设备列表 (打印相机基本信息)
void displayDeviceInfo(TVector<ICameraPtr>& vCameraPtrList);

//相机单例类
class CSingleCamera {
public:
    ///@brief 获取相机实例
    static CSingleCamera* GetInstance(void) {
        return instance;
    }

    ///@brief 获取相机设备的指针 返回指针对象的引用
    inline const ICameraPtr& GetCameraPtr(void) const {
        return cameraSptr;
    }

    ///@brief 获取相机流对象
    inline const IStreamSourcePtr& GetStreamPtr() const {
        return streamPtr;
    }

    //@brief 获取错误码 0：正确返回 -2：相机不存在
    //-1：未找到相机驱动 -3：连接相机失败
    //-4: 创建流对象失败 -5：开始采集图像失败
    inline int GetErrCode(void) const {
        return errCode;
    }

private:
    class CSingleGC{
    public://此类用来回收相机对象和相机资源
        ~CSingleGC(){
            if (CSingleCamera::instance) {
                //断开相机连接(程序退出前一定要记得断开)
	            CSingleCamera::GetInstance()->GetCameraPtr()->disConnect();
                SHOW(("相机断开成功...\n"));

                //释放单例相机对象
                delete CSingleCamera::instance;
                CSingleCamera::instance = nullptr;
            }
        }
    };

    CSingleCamera() {
        //获取相机接口
        CSystem& systemObj = CSystem::getInstance();
        TVector<ICameraPtr> vCameraPtrList;
        //寻找相机设备接口
        bool isDiscoverySuccess = systemObj.discovery(vCameraPtrList);
        if (!isDiscoverySuccess) {
            SHOW(("discovery device fail.\n"));
            errCode = -1;
            return;
        }
        MYDBG("discovery device succeed...isDis", isDiscoverySuccess);
        //相机是否存在
        if (vCameraPtrList.size() == 0) {
            SHOW(("no camera devices find.\n"));
            errCode = -2;
            return;
        }
#ifdef _DBG
        // print camera info (index,Type,vendor name, model,serial number,DeviceUserID,IP Address)
	    // 打印相机基本信息（序号,类型,制造商信息,型号,序列号,用户自定义ID,IP地址）
	    displayDeviceInfo(vCameraPtrList);
#endif
	    //获取相机设备的指针
	    cameraSptr = vCameraPtrList[0];
        //连接相机设备
	    bool isConnect = cameraSptr->connect();
	    //判断是否连接成功
	    if (!isConnect) {
	    	SHOW(("Camera connect fail...\n"));
            errCode = -3;
	    	return;
	    }
	    SHOW(("Camera connect succeed...\n"));
        // 创建流对象
	    // create acquisitioncontrol object
	    streamPtr = systemObj.createStreamSource(this->cameraSptr);
	    if (NULL == streamPtr.get()) {
	    	SHOW(("create stream obj  fail.\n"));
            errCode = -4;
	    	return;
	    }
        errCode = 0;
        //此对象用来关闭相机和回收单例对象
        static CSingleGC gc;
    }

    CSingleCamera(CSingleCamera&&) = delete;
    CSingleCamera(const CSingleCamera&) = delete;
    CSingleCamera& operator=(CSingleCamera&&) = delete;
    CSingleCamera& operator=(const CSingleCamera&) = delete;
    //~CSingleCamera() {TAG();}

    //定义相机接口类对象
    //单例对象
    static CSingleCamera* instance;
    //相机指针
    ICameraPtr cameraSptr;
    //相机流对象
    IStreamSourcePtr streamPtr;
    //错误码
    int errCode;
};
}
#endif
