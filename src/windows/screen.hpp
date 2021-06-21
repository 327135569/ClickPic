#pragma once
#include "Windows.h"
#include "base/screen.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/opencv.hpp"
#include "utils/logger.hpp"
#include <exception>
#include <optional>
#include <stdio.h>

class ScreenImpl : public Screen {

  private:
    int screenx;
    int screeny;
    int screenWidth;
    int screenHeight;
    HWND deskH;
    HDC desktopDC;

  public:
    void set_screenHeight(int v) { screenHeight = v; }
    void set_screenWidth(int v) { screenWidth = v; }
    int get_screenHeight() { return screenHeight; }
    int get_screenWidth() { return screenWidth; }

    ScreenImpl(int x, int y) {
        screenx = 0;
        screeny = 0;
        screenHeight = y;
        screenWidth = x;

        deskH = ::GetDesktopWindow();
        desktopDC = ::GetDC(deskH);
    }

    ScreenImpl() {
        screenx = GetSystemMetrics(SM_YVIRTUALSCREEN);
        screeny = GetSystemMetrics(SM_XVIRTUALSCREEN);

        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);

        printf("screenx %d\n", screenx);
        printf("screeny %d\n", screeny);

        printf("screenWidth %d\n", screenWidth);
        printf("screenHeight %d\n", screenHeight);

        deskH = ::GetDesktopWindow();
        desktopDC = ::GetDC(deskH);

        RECT rect;
        ::GetWindowRect(deskH, &rect);
        screenHeight = rect.bottom - rect.top;
        screenWidth = rect.right - rect.left;
        printf("screenWidth %d\n", screenWidth);
        printf("screenHeight %d\n", screenHeight);
    }

    ~ScreenImpl() { ReleaseDC(deskH, desktopDC); }

    std::optional<cv::Mat> fetch() {

        cv::Mat mat;

        HDC memDC;
        HBITMAP desktopBitmap;
        BITMAP bmpScreen;

        try {
            memDC = ::CreateCompatibleDC(desktopDC);
            if (!memDC) {
                printf("create cdc error");
                return std::nullopt;
            }
            SetStretchBltMode(memDC, COLORONCOLOR);

            desktopBitmap = ::CreateCompatibleBitmap(
                desktopDC, this->screenWidth, this->screenHeight);
            if (!desktopBitmap) {
                printf("create compatibl bitmap error");
                return std::nullopt;
            }
            mat.create(screenHeight, screenWidth, CV_8UC3);
            // printf("mat x%d\n", mat.cols);
            // printf("mat y%d\n", mat.rows);

            SelectObject(memDC, desktopBitmap);
            StretchBlt(memDC, 0, 0, screenWidth, screenHeight, desktopDC, 0, 0,
                       screenWidth, screenHeight, SRCCOPY);

            GetObject(desktopBitmap, sizeof(BITMAP), &bmpScreen);
            // printf("x %ld\n", bmpScreen.bmWidth);
            // printf("y %ld\n", bmpScreen.bmHeight);

            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(BITMAPINFOHEADER));
            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = bmpScreen.bmWidth;
            bi.biHeight = -bmpScreen.bmHeight;
            bi.biPlanes = 1;
            bi.biBitCount = 24;
            bi.biCompression = BI_RGB;
            bi.biSizeImage = 0;
            bi.biXPelsPerMeter = 0;
            bi.biYPelsPerMeter = 0;
            bi.biClrUsed = 0;
            bi.biClrImportant = 0;

            ::GetDIBits(memDC, desktopBitmap, 0, (UINT)bmpScreen.bmHeight,
                        mat.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

        } catch (...) {
            logd("unknow exception");
            std::terminate();
        }

        DeleteObject(desktopBitmap);
        DeleteDC(memDC);

        return mat;
    }
};