#pragma once
#ifndef _RGBCONVERT_H_
#define _RGBCONVERT_H_

#include <opencv2/opencv.hpp>
#include "GenICam/Frame.h"
#include "Memory/Block.h"
#include "Media/ImageConvert.h"
#include "GenICam/PixelType.h"

class FrameBuffer
{
private:
    uint8_t* Buffer_;

    int Width_;

    int Height_;

    int PaddingX_;

    int PaddingY_;

    int DataSize_;

    int PixelFormat_;

    uint64_t TimeStamp_;

public:
    FrameBuffer(Dahua::GenICam::CFrame const& frame)
    {
        if (frame.getImageSize() > 0)
        {
            if (frame.getImagePixelFormat() == Dahua::GenICam::gvspPixelMono8)
            {
                Buffer_ = new(std::nothrow) uint8_t[frame.getImageSize()];
            }
            else
            {
                Buffer_ = new(std::nothrow) uint8_t[frame.getImageWidth() * frame.getImageHeight() * 3];
            }
            if (Buffer_)
            {
                Width_ = frame.getImageWidth();
                Height_ = frame.getImageHeight();
                PaddingX_ = frame.getImagePadddingX();
                PaddingY_ = frame.getImagePadddingY();
                DataSize_ = frame.getImageSize();
                PixelFormat_ = frame.getImagePixelFormat();
            }
        }
    }

    ~FrameBuffer()
    {
        if (Buffer_ != NULL)
        {
            delete[] Buffer_;
            Buffer_ = NULL;
        }
    }

    bool Valid()
    {
        if (NULL != Buffer_)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    int Width()
    {
        return Width_;
    }

    int Height()
    {
        return Height_;
    }

    int PaddingX()
    {
        return PaddingX_;
    }

    int PaddingY()
    {
        return PaddingY_;
    }

    int DataSize()
    {
        return DataSize_;
    }

    uint64_t PixelFormat()
    {
        return PixelFormat_;
    }

    uint64_t TimeStamp()
    {
        return TimeStamp_;
    }

    void setWidth(uint32_t iWidth)
    {
        Width_ = iWidth;
    }

    void setPaddingX(uint32_t iPaddingX)
    {
        PaddingX_ = iPaddingX;
    }

    void setPaddingY(uint32_t iPaddingX)
    {
        PaddingY_ = iPaddingX;
    }

    void setHeight(uint32_t iHeight)
    {
        Height_ = iHeight;
    }

    void setDataSize(int dataSize)
    {
        DataSize_ = dataSize;
    }

    void setPixelFormat(uint32_t pixelFormat)
    {
        PixelFormat_ = pixelFormat;
    }

    void setTimeStamp(uint64_t timeStamp)
    {
        TimeStamp_ = timeStamp;
    }

    uint8_t* bufPtr()
    {
        return Buffer_;
    }

};
typedef Dahua::Memory::TSharedPtr<FrameBuffer> FrameBufferSPtr;

///@brief 将相机捕获到的图像转化为opencv的Mat对象
///@param frame [in]  要转换的图像
///@param image [out] 转换后的Mat对象
///@return 成功return true;失败return false
extern bool FrameToBGRMat(const Dahua::GenICam::CFrame& frame, cv::Mat& img);

#endif
