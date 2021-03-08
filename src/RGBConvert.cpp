#include "RGBConvert.h"
#include "GenICam/PixelType.h"
#include "Media/ImageConvert.h"
#include <assert.h>
#include "Infra/Time.h"
#include "debug.h"
using namespace Dahua::GenICam;

static uint32_t gFormatTransferTbl[] =
{
    // Mono Format
    gvspPixelMono1p,
    gvspPixelMono8,
    gvspPixelMono10,
    gvspPixelMono10Packed,
    gvspPixelMono12,
    gvspPixelMono12Packed,

    // Bayer Format
    gvspPixelBayRG8,
    gvspPixelBayGB8,
    gvspPixelBayBG8,
    gvspPixelBayRG10,
    gvspPixelBayGB10,
    gvspPixelBayBG10,
    gvspPixelBayRG12,
    gvspPixelBayGB12,
    gvspPixelBayBG12,
    gvspPixelBayRG10Packed,
    gvspPixelBayGB10Packed,
    gvspPixelBayBG10Packed,
    gvspPixelBayRG12Packed,
    gvspPixelBayGB12Packed,
    gvspPixelBayBG12Packed,
    gvspPixelBayRG16,
    gvspPixelBayGB16,
    gvspPixelBayBG16,
    gvspPixelBayRG10p,
    gvspPixelBayRG12p,

    gvspPixelMono1c,

    // RGB Format
    gvspPixelRGB8,
    gvspPixelBGR8,

    // YVR Format
    gvspPixelYUV411_8_UYYVYY,
    gvspPixelYUV422_8_UYVY,
    gvspPixelYUV422_8,
    gvspPixelYUV8_UYV,
};
#define gFormatTransferTblLen   sizeof(gFormatTransferTbl)/sizeof(gFormatTransferTbl[0])

struct ImplData
{
    int width;
    int height;
};

static int32_t findMatchCode(uint32_t code)
{
    for (size_t i = 0; i < gFormatTransferTblLen; ++i)
    {
        if (gFormatTransferTbl[i] == code)
        {
            return i;
        }
    }
    return -1;
}

#if 0
static void freeMem(uint8_t* mem)
{
    if (NULL != mem)
        ::free(mem);
}
#endif

/* 转码函数 */
bool ConvertImage(const Dahua::GenICam::CFrame& input, FrameBufferSPtr& output)
{
    int idx = findMatchCode((input.getImagePixelFormat()));
    if (idx < 0)
    {
        return false;
    }

    FrameBufferSPtr PtrFrameBuffer(new FrameBuffer(input));
    if (!PtrFrameBuffer)
    {
        SHOW(("PtrFrameBuffer is null.\n"));
        return false;
    }

     /* Mono8无需转换直接源数据显示 */
     if (PtrFrameBuffer->PixelFormat() == gvspPixelMono8)
     {
         memcpy(PtrFrameBuffer->bufPtr(), input.getImage(), input.getImageSize());
     }
     else
     {
        uint8_t* pSrcData = new(std::nothrow) uint8_t[input.getImageSize()];
        if (pSrcData)
        {
            memcpy(pSrcData, input.getImage(), input.getImageSize());
        }
        else
        {
            SHOW(("m_pSrcData is null.\n"));
            return false;
        }

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
            SHOW(("IMGCNV_open is failed.err=%d\n", status));
            return false;
        }

        delete[] pSrcData;
    }

    output = PtrFrameBuffer;
    return true;
}

//将相机捕获到的图像转化为opencv的Mat对象
bool FrameToBGRMat(const Dahua::GenICam::CFrame& frame, cv::Mat& img)
{
	Dahua::Memory::TSharedPtr<FrameBuffer> PtrFrameBuffer(new FrameBuffer(frame));
	//将采集到的图像转换成BGR24格式
	if (!ConvertImage(frame, PtrFrameBuffer)) {
        SHOW(("FrameToBGRMat failed!\n"));
        return false;
    }

	//将采集到的图像转化为Mat对象
	img = cv::Mat(
	frame.getImageHeight(),
	frame.getImageWidth(),
	CV_8UC3,
	PtrFrameBuffer->bufPtr()).clone();
	//必须使用clone进行深拷贝，因为函数执行完会销毁Buf内存
	return true;
}