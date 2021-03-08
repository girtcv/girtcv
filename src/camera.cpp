/* motto: We don't produce code, we're just GitHub porters!
 * ============================================================================
 *
 *       Filename:  camera.cpp
 *
 *    Description:  相机的配置函数集合，比如曝光，分辨率
 *  这个文件都有对应的函数进行获取和修改
 *        Version:  1.0
 *        Created:  2020年12月03日 星期四 16时59分48秒
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

#include <cstdio>
//大华相机头文件库
#ifndef __unix__
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include "GenICam/System.h"
#include "GenICam/Camera.h"
#include "GenICam/StreamSource.h"
#include "GenICam/GigE/GigECamera.h"
#include "GenICam/GigE/GigEInterface.h"
#include "Infra/PrintLog.h"

#include "Memory/SharedPtr.h"
#include "Media/ImageConvert.h"

#include "camera.hpp"
using std::cout;
using std::endl;
using std::cin;

namespace girtcv {

/* 4、设置相机采图模式（连续采图、触发采图） */
int32_t setGrabMode(ICameraPtr& cameraSptr, bool bContious)
{
    int32_t bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (nullptr == sptrAcquisitionControl)
    {
        return -1;
    }

    CEnumNode enumNode = sptrAcquisitionControl->triggerSelector();
    bRet = enumNode.setValueBySymbol("FrameStart");
    if (false == bRet)
    {
        printf("set TriggerSelector fail.\n");
        return -1;
    }

    if (true == bContious)
    {
        enumNode = sptrAcquisitionControl->triggerMode();
        bRet = enumNode.setValueBySymbol("Off");
        if (false == bRet)
        {
            printf("set triggerMode fail.\n");
            return -1;
        }
    }
    else
    {
        enumNode = sptrAcquisitionControl->triggerMode();
        bRet = enumNode.setValueBySymbol("On");
        if (false == bRet)
        {
            printf("set triggerMode fail.\n");
            return -1;
        }

        /* 设置触发源为软触发（硬触发为Line1） */
        enumNode = sptrAcquisitionControl->triggerSource();
        bRet = enumNode.setValueBySymbol("Software");
        if (false == bRet)
        {
            printf("set triggerSource fail.\n");
            return -1;
        }
    }
    return 0;
}

/* 5、获取相机采图模式 */
int32_t getGrabMode(ICameraPtr& cameraSptr, bool &bContious)
{
    int32_t bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    CEnumNode enumNode = sptrAcquisitionControl->triggerSelector();
    bRet = enumNode.setValueBySymbol("FrameStart");
    if (false == bRet)
    {
        printf("set TriggerSelector fail.\n");
        return -1;
    }

    CString strValue;
    enumNode = sptrAcquisitionControl->triggerMode();
    bRet = enumNode.getValueSymbol(strValue);
    if (false == bRet)
    {
        printf("get triggerMode fail.\n");
        return -1;
    }

    if (strValue == "Off")
    {
        bContious = true;
    }
    else if (strValue == "On")
    {
        bContious = false;
    }
    else
    {
        printf("get triggerMode fail.\n");
        return -1;
    }
    return 0;
}

/* 6、软件触发 */
int32_t triggerSoftware(ICameraPtr& cameraSptr)
{
    int32_t bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    CCmdNode cmdNode = sptrAcquisitionControl->triggerSoftware();
    bRet = cmdNode.execute();
    if (false == bRet)
    {
        printf("triggerSoftware execute fail.\n");
        return -1;
    }
    return 0;
}

/* 9、设置传感器采样率（采集分辨率） */
int32_t setResolution(ICameraPtr& cameraSptr, int nWidth, int nHeight)
{
    int32_t bRet;
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.setValue(nWidth);
    if (false == bRet)
    {
        printf("set width fail.\n");
        return -1;
    }

    intNode = sptrImageFormatControl->height();
    bRet = intNode.setValue(nHeight);
    if (false == bRet)
    {
        printf("set height fail.\n");
        return -1;
    }
    return 0;
}

/* 10、获取传感器采样率 */
int32_t getResolution(ICameraPtr& cameraSptr, int64_t &nWidth, int64_t &nHeight)
{
    int32_t bRet;
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (nullptr == sptrImageFormatControl)
    {
        return -1;
    }

    CIntNode intNode = sptrImageFormatControl->width();
    //MYDBG("sizeof CIntNode", sizeof(CIntNode));
    bRet = intNode.getValue(nWidth);
    if (false == bRet)
    {
        printf("get width fail.\n");
        return -1;
    }
    //SHOW(("分辨率 width: %d ", nWidth));

    intNode = sptrImageFormatControl->height();
    bRet = intNode.getValue(nHeight);
    if (false == bRet)
    {
        printf("get height fail.\n");
        return -1;
    }
    //SHOW(("&& height: %d\n", nHeight));

    return 0;
}

/* 设置binning (Off X Y XY) */
int32_t setBinning(ICameraPtr& cameraSptr)
{
    CEnumNodePtr ptrParam(new CEnumNode(cameraSptr, "Binning"));
    if (ptrParam)
    {
        if (false == ptrParam->isReadable())
        {
            printf("binning not support.\n");
            return -1;
        }

        if (false == ptrParam->setValueBySymbol("XY"))
        {
            printf("set Binning XY fail.\n");
            return -1;
        }

        if (false == ptrParam->setValueBySymbol("Off"))
        {
            printf("set Binning Off fail.\n");
            return -1;
        }
    }
    return 0;
}

/* 11、获取传感器最大分辩率 */
int32_t getMaxResolution(ICameraPtr& cameraSptr, int64_t &nWidthMax, int64_t &nHeightMax)
{
    /* width */
    {
        CIntNodePtr ptrParam(new CIntNode(cameraSptr, "SensorWidth"));
        if (ptrParam)
        {
            if (false == ptrParam->getValue(nWidthMax))
            {
                printf("get WidthMax fail.\n");
                return -1;
            }
        }
    }

    /* height */
    {
        CIntNodePtr ptrParam(new CIntNode(cameraSptr, "SensorHeight"));
        if (ptrParam)
        {
            if (false == ptrParam->getValue(nWidthMax))
            {
                printf("get WidthMax fail.\n");
                return -1;
            }
        }
    }
    return 0;
}

/* 12、设置图像ROI */
int32_t setROI(ICameraPtr& cameraSptr, int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight)
{
    bool bRet;
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    /* width */
    CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.setValue(nWidth);
    if (!bRet)
    {
        printf("set width fail.\n");
    return -1;
    }

    /* height */
    intNode = sptrImageFormatControl->height();
    bRet = intNode.setValue(nHeight);
    if (!bRet)
    {
        printf("set height fail.\n");
        return -1;
    }

    /* OffsetX */
    intNode = sptrImageFormatControl->offsetX();
    bRet = intNode.setValue(nX);
    if (!bRet)
    {
        printf("set offsetX fail.\n");
    return -1;
    }

    /* OffsetY */
    intNode = sptrImageFormatControl->offsetY();
    bRet = intNode.setValue(nY);
    if (!bRet)
    {
        printf("set offsetY fail.\n");
    return -1;
    }

    return 0;
}

/* 13、获取图像ROI */
int32_t getROI(ICameraPtr& cameraSptr, int64_t &nX, int64_t &nY, int64_t &nWidth, int64_t &nHeight)
{
    bool bRet;
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    /* width */
    CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.getValue(nWidth);
    if (!bRet)
    {
        printf("get width fail.\n");
    }

    /* height */
    intNode = sptrImageFormatControl->height();
    bRet = intNode.getValue(nHeight);
    if (!bRet)
    {
        printf("get height fail.\n");
    }

    /* OffsetX */
    intNode = sptrImageFormatControl->offsetX();
    bRet = intNode.getValue(nX);
    if (!bRet)
    {
        printf("get offsetX fail.\n");
    }

    /* OffsetY */
    intNode = sptrImageFormatControl->offsetY();
    bRet = intNode.getValue(nY);
    if (!bRet)
    {
        printf("get offsetY fail.\n");
    }
    return 0;
}

/* 14、获取采图图像宽度 */
int32_t getWidth(ICameraPtr& cameraSptr, int64_t &nWidth)
{
    bool bRet;
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.getValue(nWidth);
    if (!bRet)
    {
        printf("get width fail.\n");
    }
    return 0;
}

/* 15、获取采图图像高度 */
int32_t getHeight(ICameraPtr& cameraSptr, int64_t &nHeight)
{
    bool bRet;
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    CIntNode intNode = sptrImageFormatControl->height();
    bRet = intNode.getValue(nHeight);
    if (!bRet)
    {
        printf("get height fail.\n");
        return -1;
    }
    return 0;
}

/* 17、设置曝光值(曝光、自动曝光/手动曝光) */
int32_t setExposureTime(ICameraPtr& cameraSptr, double dExposureTime, bool bAutoExposure)
{
    bool bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    if (bAutoExposure)
    {
        CEnumNode enumNode = sptrAcquisitionControl->exposureAuto();
        bRet = enumNode.setValueBySymbol("Continuous");
        if (false == bRet)
        {
            printf("set exposureAuto fail.\n");
            return -1;
        }
    }
    else
    {
        CEnumNode enumNode = sptrAcquisitionControl->exposureAuto();
        bRet = enumNode.setValueBySymbol("Off");
        if (false == bRet)
        {
            printf("set exposureAuto fail.\n");
            return -1;
        }

        CDoubleNode doubleNode = sptrAcquisitionControl->exposureTime();
        bRet = doubleNode.setValue(dExposureTime);
        if (false == bRet)
        {
            printf("set exposureTime fail.\n");
            return -1;
        }
    }
    return 0;
}

/* 18、获取曝光时间 */
int32_t getExposureTime(ICameraPtr& cameraSptr, double &dExposureTime)
{
    bool bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAcquisitionControl->exposureTime();
    bRet = doubleNode.getValue(dExposureTime);
    if (false == bRet)
    {
        printf("get exposureTime fail.\n");
        return -1;
    }
    return 0;
}

/* 19、获取曝光范围 */
int32_t getExposureTimeMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
{
    bool bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAcquisitionControl->exposureTime();
    bRet = doubleNode.getMinVal(dMinValue);
    if (false == bRet)
    {
        printf("get exposureTime minValue fail.\n");
        return -1;
    }

    bRet = doubleNode.getMaxVal(dMaxValue);
    if (false == bRet)
    {
        printf("get exposureTime maxValue fail.\n");
        return -1;
    }
    return 0;
}

/* 20、设置增益值 */
int32_t setGainRaw(ICameraPtr& cameraSptr, double dGainRaw)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->gainRaw();
    bRet = doubleNode.setValue(dGainRaw);
    if (false == bRet)
    {
        printf("set gainRaw fail.\n");
        return -1;
    }
    return 0;
}

/* 21、获取增益值 */
int32_t getGainRaw(ICameraPtr& cameraSptr, double &dGainRaw)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->gainRaw();
    bRet = doubleNode.getValue(dGainRaw);
    if (false == bRet)
    {
        printf("get gainRaw fail.\n");
        return -1;
    }
    return 0;
}

/* 22、获取增益值范围 */
int32_t getGainRawMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->gainRaw();
    bRet = doubleNode.getMinVal(dMinValue);
    if (false == bRet)
    {
        printf("get gainRaw minValue fail.\n");
        return -1;
    }

    bRet = doubleNode.getMaxVal(dMaxValue);
    if (false == bRet)
    {
        printf("get gainRaw maxValue fail.\n");
        return -1;
    }
    return 0;
}

/* 23、设置伽马值 */
int32_t setGamma(ICameraPtr& cameraSptr, double dGamma)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->gamma();
    bRet = doubleNode.setValue(dGamma);
    if (false == bRet)
    {
        printf("set gamma fail.\n");
        return -1;
    }
    return 0;
}

/* 24、获取伽马值 */
int32_t getGamma(ICameraPtr& cameraSptr, double &dGamma)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->gamma();
    bRet = doubleNode.getValue(dGamma);
    if (false == bRet)
    {
        printf("get gamma fail.\n");
        return -1;
    }
    return 0;
}

/* 25、获取伽马值范围 */
int32_t getGammaMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->gamma();
    bRet = doubleNode.getMinVal(dMinValue);
    if (false == bRet)
    {
        printf("get gamma minValue fail.\n");
        return -1;
    }

    bRet = doubleNode.getMaxVal(dMaxValue);
    if (false == bRet)
    {
        printf("get gamma maxValue fail.\n");
        return -1;
    }
    return 0;
}

/* 26、设置白平衡值（有三个白平衡值） */
int32_t setBalanceRatio(ICameraPtr& cameraSptr, double dRedBalanceRatio, double dGreenBalanceRatio, double dBlueBalanceRatio)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    /* 关闭自动白平衡 */
    CEnumNode enumNode = sptrAnalogControl->balanceWhiteAuto();
    if (false == enumNode.isReadable())
    {
        printf("balanceRatio not support.\n");
        return -1;
    }

    bRet = enumNode.setValueBySymbol("Off");
    if (false == bRet)
    {
        printf("set balanceWhiteAuto Off fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Red");
    if (false == bRet)
    {
        printf("set red balanceRatioSelector fail.\n");
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.setValue(dRedBalanceRatio);
    if (false == bRet)
    {
        printf("set red balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Green");
    if (false == bRet)
    {
        printf("set green balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.setValue(dGreenBalanceRatio);
    if (false == bRet)
    {
        printf("set green balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Blue");
    if (false == bRet)
    {
        printf("set blue balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.setValue(dBlueBalanceRatio);
    if (false == bRet)
    {
        printf("set blue balanceRatio fail.\n");
        return -1;
    }
    return 0;
}

/* 27、获取白平衡值（有三个白平衡值） */
int32_t getBalanceRatio(ICameraPtr& cameraSptr, double &dRedBalanceRatio, double &dGreenBalanceRatio, double &dBlueBalanceRatio)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CEnumNode enumNode = sptrAnalogControl->balanceRatioSelector();
    if (false == enumNode.isReadable())
    {
        printf("balanceRatio not support.\n");
        return -1;
    }

    bRet = enumNode.setValueBySymbol("Red");
    if (false == bRet)
    {
        printf("set red balanceRatioSelector fail.\n");
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.getValue(dRedBalanceRatio);
    if (false == bRet)
    {
        printf("get red balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Green");
    if (false == bRet)
    {
        printf("set green balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.getValue(dGreenBalanceRatio);
    if (false == bRet)
    {
        printf("get green balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Blue");
    if (false == bRet)
    {
        printf("set blue balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.getValue(dBlueBalanceRatio);
    if (false == bRet)
    {
        printf("get blue balanceRatio fail.\n");
        return -1;
    }
    return 0;
}

/* 28、获取白平衡值范围 */
int32_t getBalanceRatioMinMaxValue(ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
{
    bool bRet;
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptrAnalogControl->balanceRatio();
    if (false == doubleNode.isReadable())
    {
        printf("balanceRatio not support.\n");
        return -1;
    }

    bRet = doubleNode.getMinVal(dMinValue);
    if (false == bRet)
    {
        printf("get balanceRatio min value fail.\n");
        return -1;
    }

    bRet = doubleNode.getMaxVal(dMaxValue);
    if (false == bRet)
    {
        printf("get balanceRatio max value fail.\n");
        return -1;
    }

    return 0;
}

/* 29、设置采图速度（秒帧数） */
int32_t setAcquisitionFrameRate(ICameraPtr& cameraSptr, double dFrameRate)
{
    bool bRet;
    IAcquisitionControlPtr sptAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    CBoolNode booleanNode = sptAcquisitionControl->acquisitionFrameRateEnable();
    bRet = booleanNode.setValue(true);
    if (false == bRet)
    {
        printf("set acquisitionFrameRateEnable fail.\n");
        return -1;
    }

    CDoubleNode doubleNode = sptAcquisitionControl->acquisitionFrameRate();
    bRet = doubleNode.setValue(dFrameRate);
    if (false == bRet)
    {
        printf("set acquisitionFrameRate fail.\n");
        return -1;
    }
    return 0;
}

/* 30、获取采图速度（秒帧数） */
int32_t getAcquisitionFrameRate(ICameraPtr& cameraSptr, double &dFrameRate)
{
    bool bRet;
    IAcquisitionControlPtr sptAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptAcquisitionControl->acquisitionFrameRate();
    bRet = doubleNode.getValue(dFrameRate);
    if (false == bRet)
    {
        printf("get acquisitionFrameRate fail.\n");
        return -1;
    }
    return 0;
}

/* 31、保存参数 */
int32_t userSetSave(ICameraPtr& cameraSptr)
{
    bool bRet;
    IUserSetControlPtr sptUserSetControl = CSystem::getInstance().createUserSetControl(cameraSptr);
    if (NULL == sptUserSetControl)
    {
        return -1;
    }

    bRet = sptUserSetControl->saveUserSet(IUserSetControl::userSet1);
    if (false == bRet)
    {
        printf("saveUserSet fail.\n");
        return -1;
    }
    return 0;
}

/* 32、加载参数 */
int32_t loadUserSet(ICameraPtr& cameraSptr)
{
    bool bRet;
    IUserSetControlPtr sptUserSetControl = CSystem::getInstance().createUserSetControl(cameraSptr);
    if (NULL == sptUserSetControl)
    {
        return -1;
    }

    bRet = sptUserSetControl->setCurrentUserSet(IUserSetControl::userSet1);
    if (false == bRet)
    {
        printf("saveUserSet fail.\n");
        return -1;
    }
    return 0;
}

/* 33、设置外触发延时时间 */
int32_t setTriggerDelay(ICameraPtr& cameraSptr, double dDelayTime)
{
    bool bRet;
    IAcquisitionControlPtr sptAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptAcquisitionControl->triggerDelay();
    bRet = doubleNode.setValue(dDelayTime);
    if (false == bRet)
    {
        printf("set triggerDelay fail.\n");
        return -1;
    }
    return 0;
}

/* 34、获取外触发延时时间 */
int32_t getTriggerDelay(ICameraPtr& cameraSptr, double &dDelayTime)
{
    bool bRet;
    IAcquisitionControlPtr sptAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    CDoubleNode doubleNode = sptAcquisitionControl->triggerDelay();
    bRet = doubleNode.getValue(dDelayTime);
    if (false == bRet)
    {
        printf("set triggerDelay fail.\n");
        return -1;
    }
    return 0;
}

/* 35、设置外触发模式（上升沿触发、下降沿触发） */
int32_t setLineTriggerMode(ICameraPtr& cameraSptr, bool bRisingEdge)
{
    bool bRet;
    IAcquisitionControlPtr sptAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    CEnumNode enumNode = sptAcquisitionControl->triggerSelector();
    if (false == enumNode.setValueBySymbol("FrameStart"))
    {
        printf("set triggerSelector fail.\n");
        return -1;
    }

    enumNode = sptAcquisitionControl->triggerMode();
    if (false == enumNode.setValueBySymbol("On"))
    {
        printf("set triggerMode fail.\n");
        return -1;
    }

    enumNode = sptAcquisitionControl->triggerSource();
    if (false == enumNode.setValueBySymbol("Line1"))
    {
        printf("set triggerSource fail.\n");
        return -1;
    }

    enumNode = sptAcquisitionControl->triggerActivation();
    if (true == bRisingEdge)
    {
        bRet = enumNode.setValueBySymbol("RisingEdge");
    }
    else
    {
        bRet = enumNode.setValueBySymbol("FallingEdge");
    }

    if (false == bRet)
    {
        printf("set fail.\n");
        return -1;
    }
    return 0;
}

/* 36、获取外触发模式（上升沿触发、下降沿触发） */
int32_t getLineTriggerMode(ICameraPtr& cameraSptr, bool &bRisingEdge)
{
    bool bRet;
    IAcquisitionControlPtr sptAcquisitionControl = CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    CEnumNode enumNode = sptAcquisitionControl->triggerSelector();
    if (false == enumNode.setValueBySymbol("FrameStart"))
    {
        printf("set triggerSelector fail.\n");
        return -1;
    }

    CString strValue;
    enumNode = sptAcquisitionControl->triggerActivation();
    if (true == bRisingEdge)
    {
        bRet = enumNode.getValueSymbol(strValue);
    }
    else
    {
        bRet = enumNode.getValueSymbol(strValue);
    }

    if (false == bRet)
    {
        printf("get triggerActivation fail.\n");
        return -1;
    }

    if (strValue == "RisingEdge")
    {
        bRisingEdge = true;
    }
    else if (strValue == "FallingEdge")
    {
        bRisingEdge = false;
    }
    else
    {
        printf("get triggerActivation fail.\n");
        return -1;
    }
    return 0;
}

/* 37、设置外触发信号滤波时间 */
int32_t setLineDebouncerTimeAbs(ICameraPtr& cameraSptr, double dLineDebouncerTimeAbs)
{
    IDigitalIOControlPtr sptDigitalIOControl = CSystem::getInstance().createDigitalIOControl(cameraSptr);
    if (NULL == sptDigitalIOControl)
    {
        return -1;
    }

    CEnumNode enumNode = sptDigitalIOControl->lineSelector();
    if (false == enumNode.setValueBySymbol("Line1"))
    {
        printf("set lineSelector fail.\n");
        return -1;
    }

    CDoubleNode doubleNode = sptDigitalIOControl->lineDebouncerTimeAbs();
    if (false == doubleNode.setValue(dLineDebouncerTimeAbs))
    {
        printf("set lineDebouncerTimeAbs fail.\n");
        return -1;
    }
    return 0;
}

/* 38、获取外触发信号滤波时间 */
int32_t getLineDebouncerTimeAbs(ICameraPtr& cameraSptr, double &dLineDebouncerTimeAbs)
{
    IDigitalIOControlPtr sptDigitalIOControl = CSystem::getInstance().createDigitalIOControl(cameraSptr);
    if (NULL == sptDigitalIOControl)
    {
        return -1;
    }

    CEnumNode enumNode = sptDigitalIOControl->lineSelector();
    if (false == enumNode.setValueBySymbol("Line1"))
    {
        printf("set lineSelector fail.\n");
        return -1;
    }

    CDoubleNode doubleNode = sptDigitalIOControl->lineDebouncerTimeAbs();
    if (false == doubleNode.getValue(dLineDebouncerTimeAbs))
    {
        printf("get lineDebouncerTimeAbs fail.\n");
        return -1;
    }
    return 0;
}

/* 39、设置外触发脉冲宽度（不支持） */
/* 40、获取外触发脉冲宽度（不支持） */
/* 41、设置输出信号线（控制光源用）（面阵相机是Line0） */
/* 42、获取输出信号线（面阵相机是Line0） */

/* 43、设置外部光源曝光时间（设置输出值为TRUE的时间） */
int32_t setOutputTime(ICameraPtr& cameraSptr, int nTimeMS)
{
    IDigitalIOControlPtr sptDigitalIOControl = CSystem::getInstance().createDigitalIOControl(cameraSptr);
    if (NULL == sptDigitalIOControl)
    {
        return -1;
    }

    CEnumNode paramLineSource(cameraSptr, "LineSource");
    if (false == paramLineSource.setValueBySymbol("UserOutput1"))
    {
        printf("set LineSource fail.");
        return -1;
    }

    /* 将输出信号拉高然后拉低 */
    CBoolNode booleanNode = sptDigitalIOControl->userOutputValue();
    if (false == booleanNode.setValue(true))
    {
        printf("set userOutputValue fail.\n");
        return -1;
    }

    CThread::sleep(nTimeMS);

    if (false == booleanNode.setValue(false))
    {
        printf("set userOutputValue fail.\n");
        return -1;
    }

    return 0;
}

/* 44、获取外部光源曝光时间（输出信号的时间由软件侧控制） */

/* 45、设置X轴翻转 */
int32_t setReverseX(ICameraPtr& cameraSptr, bool flag)
{
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);

    CBoolNode boolNodeReverseX = sptrImageFormatControl->reverseX();
    if(!boolNodeReverseX.setValue(flag))
    {
    	printf("set reverseX fail.\n");
    	return -1;
    }

    return 0;
}

/* 46、设置Y轴翻转 */
int32_t setReverseY(ICameraPtr& cameraSptr, bool flag)
{
    IImageFormatControlPtr sptrImageFormatControl = CSystem::getInstance().createImageFormatControl(cameraSptr);

    CBoolNode boolNodeReverseY = sptrImageFormatControl->reverseY();
    if(!boolNodeReverseY.setValue(flag))
    {
    	printf("set reverseY fail.\n");
    	return -1;
    }

    return 0;
}

/* 50、修改曝光时间 （与相机连接之后调用）*/
void modifyCamralExposureTime(CSystem &systemObj, ICameraPtr& cameraSptr)
{
    IAcquisitionControlPtr sptrAcquisitionControl = systemObj.createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return;
    }

    double exposureTimeValue = 0.0;
    CDoubleNode exposureTime = sptrAcquisitionControl->exposureTime();

    exposureTime.getValue(exposureTimeValue);
    printf("before change ,exposureTime is %f. thread ID :%d\n", exposureTimeValue, CThread::getCurrentThreadID());

    exposureTime.setValue(exposureTimeValue + 2);
    exposureTime.getValue(exposureTimeValue);
    printf("after change ,exposureTime is %f. thread ID :%d\n", exposureTimeValue, CThread::getCurrentThreadID());
}

// 51、********************** 这部分处理与SDK操作相机无关，用于显示设备列表 begin*****************************
void displayDeviceInfo(TVector<ICameraPtr>& vCameraPtrList)
{
	ICameraPtr cameraSptr;
	/* 打印Title行 */
	printf("\nIdx Type Vendor     Model      S/N             DeviceUserID    IP Address    \n");
	printf("------------------------------------------------------------------------------\n");
	for (size_t cameraIndex = 0; cameraIndex < vCameraPtrList.size(); cameraIndex++)
	{
		cameraSptr = vCameraPtrList[cameraIndex];
		/* Idx 设备列表的相机索引 最大表示字数：3 */
		printf("%-3ld", cameraIndex + 1);

		/* Type 相机的设备类型（GigE，U3V，CL，PCIe）*/
		switch (cameraSptr->getType())
		{
		case ICamera::typeGige:
			printf(" GigE");
			break;
		case ICamera::typeU3v:
			printf(" U3V ");
			break;
		case ICamera::typeCL:
			printf(" CL  ");
			break;
		case ICamera::typePCIe:
			printf(" PCIe");
			break;
		default:
			printf("     ");
			break;
		}

		/* VendorName 制造商信息 最大表示字数：10 */
		const char* vendorName = cameraSptr->getVendorName();
		char vendorNameCat[18];
		if (strlen(vendorName) > 17)
		{
			strncpy(vendorNameCat, vendorName, 7);
			vendorNameCat[7] = '\0';
			strcat(vendorNameCat, "...");
			printf(" %-10.10s", vendorNameCat);
		}
		else
		{
			printf(" %-10.10s", vendorName);
		}

		/* ModeName 相机的型号信息 最大表示字数：10 */
		printf(" %-10.10s", cameraSptr->getModelName());

		/* Serial Number 相机的序列号 最大表示字数：15 */
		printf(" %-15.15s", cameraSptr->getSerialNumber());

		/* deviceUserID 自定义用户ID 最大表示字数：15 */
		const char* deviceUserID = cameraSptr->getName();
		char deviceUserIDCat[18] = {0};
		if (strlen(deviceUserID) > 17)
		{
			strncpy(deviceUserIDCat, deviceUserID, 12);
			deviceUserIDCat[12] = '\0';
			strcat(deviceUserIDCat, "...");
			printf(" %-15.15s", deviceUserIDCat);
		}
		else
		{
			//防止console显示乱码,UTF8转换成ANSI进行显示
			memcpy(deviceUserIDCat, deviceUserID, sizeof(deviceUserIDCat));
			printf(" %-15.15s", deviceUserIDCat);
		}

		/* IPAddress GigE相机时获取IP地址 */
		IGigECameraPtr gigeCameraPtr = IGigECamera::getInstance(cameraSptr);
		if (NULL != gigeCameraPtr.get())
		{
			CString ip = gigeCameraPtr->getIpAddress();
			printf(" %s", ip.c_str());
		}
		printf("\n");

	}
}

}
