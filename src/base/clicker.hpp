#pragma once
#include "constant.hpp"
#include "driver_control.hpp"
#include "utils/logger.hpp"
#include <Windows.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

class Clicker {

  private:
    double scale = 1.25;
    double sx;
    double sy;
    driver_control *dr;

  public:
    Clicker(driver_control *dr, int sx, int sy) {
        this->sx = sx;
        this->sy = sy;
        this->dr = dr;
    }

    void paste(const std::string &text) {
        auto h = ::OpenClipboard(::GetDesktopWindow());
        ::EmptyClipboard();

        auto hglbCopy = GlobalAlloc(GMEM_MOVEABLE, text.length() + 1);
        if (hglbCopy == NULL) {
            CloseClipboard();
            return;
        }

        auto lptstrCopy = (char *)GlobalLock(hglbCopy);
        memcpy(lptstrCopy, text.data(), text.length());
        lptstrCopy[text.length()] = 0;
        GlobalUnlock(hglbCopy);

        ::SetClipboardData(CF_TEXT, hglbCopy);

        paste();
        ::CloseClipboard();
    }

    void paste() {

        // 1d ctrl
        // 2f v
        dr->keyboard_event(0, 0x1d, 0x0, 0, 0);
        dr->keyboard_event(0, 0x2f, 0x0, 0, 0);
        dr->keyboard_event(0, 0x2f, 0x1, 0, 0);
        dr->keyboard_event(0, 0x1d, 0x1, 0, 0);

        // INPUT inputs[4] = {};
        // ZeroMemory(inputs, sizeof(inputs));

        // inputs[0].type = INPUT_KEYBOARD;
        // inputs[0].ki.wVk = VK_LCONTROL;

        // inputs[1].type = INPUT_KEYBOARD;
        // inputs[1].ki.wVk = 'V';

        // inputs[2].type = INPUT_KEYBOARD;
        // inputs[2].ki.wVk = 'V';
        // inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        // inputs[3].type = INPUT_KEYBOARD;
        // inputs[3].ki.wVk = VK_LCONTROL;
        // inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        // UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        // if (uSent != ARRAYSIZE(inputs)) {
        //     printf("SendInput failed: 0x%x\n",
        //            HRESULT_FROM_WIN32(GetLastError()));
        // }
    }

    void input(std::wstring text) {
        int len = text.length();
        if (len == 0) {
            throw std::runtime_error("err len while input");
        }

        std::vector<INPUT> vec;

        logd("Input %ls", text.c_str());
        for (int i = 0; i < len; i++) {
            wchar_t c = text[i];
            INPUT input;
            memset(&input, 0, sizeof(INPUT));
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = KEYEVENTF_UNICODE;
            input.ki.wScan = c;
            vec.push_back(input);

            input.ki.dwFlags |= KEYEVENTF_KEYUP;
            vec.push_back(input);
        }
        SendInput(vec.size(), vec.data(), sizeof(INPUT));
        std::this_thread::sleep_for(UI_WAITTIME);
    }

    void setsy(int v) { sy = v; }

    void setsx(int v) { sx = v; }

    void toDesktop() {

        INPUT inputs[4] = {};
        ZeroMemory(inputs, sizeof(inputs));

        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_LWIN;

        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = 'D';

        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = 'D';
        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_LWIN;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        if (uSent != ARRAYSIZE(inputs)) {
            printf("SendInput failed: 0x%x\n",
                   HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    void click(HWND hwnd, int x, int y) {

        POINT point;
        point.x = x;
        point.y = y;

        // point.x = toPixelX(point.x);
        // point.y = toPixelX(point.y);

        // ::GetCursorPos(&point);
        // printf("after get cursor pos to client %d %d\n", point.x, point.y);

        ::ScreenToClient(hwnd, &point);
        printf("after screen to client %d %d\n", point.x, point.y);

        // ::MapWindowPoints(GetDesktopWindow(), hwnd, &point, 1);

        // point.x = x * scale;
        // point.y = y * scale;

        ::SendMessageA(hwnd, WM_PARENTNOTIFY, WM_LBUTTONDOWN,
                       MAKELPARAM(point.x, point.y));
        ::SendMessageA(hwnd, WM_MOUSEACTIVATE, (WPARAM)hwnd,
                       MAKELPARAM(1, WM_LBUTTONDOWN));
        // ::PostMessageA(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(point.x, point.y));
    }

    int toPixelX(int x) { return x * (65535.0 / sx); }

    int toPixelY(int y) { return y * (65535.0 / sy); }

    void dclick(int x, int y) {
        _click(x, y);
        std::this_thread::sleep_for(30ms);
        _click(x, y);
        std::this_thread::sleep_for(UI_WAITTIME);
    }

    void click(int x, int y) {
        _click(x, y);
        std::this_thread::sleep_for(UI_WAITTIME);
    }

    void _click(int x, int y) {

        dr->mouse_event(65535.0 / this->sx * x, 65535.0 / this->sy * y, 0x1, 0);
        std::this_thread::sleep_for(300ms);
        dr->mouse_event(65535.0 / this->sx * x, 65535.0 / this->sy * y, 0x2, 0);

        // using namespace std::chrono;

        // INPUT inputs[3] = {};
        // ZeroMemory(inputs, sizeof(inputs));

        // int i = 0;
        // inputs[i].type = INPUT_MOUSE;
        // inputs[i].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
        // inputs[i].mi.mouseData = 0;
        // inputs[i].mi.dx = x * (65536.0 / sx);
        // inputs[i].mi.dy = y * (65536.0 / sy);

        // i++;
        // inputs[i].type = INPUT_MOUSE;
        // inputs[i].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        // inputs[i].mi.mouseData = 0;
        // inputs[i].mi.dx = x * (65536.0 / sx);
        // inputs[i].mi.dy = y * (65536.0 / sy);

        // std::this_thread::sleep_for(30ms);

        // i++;
        // inputs[i].type = INPUT_MOUSE;
        // inputs[i].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        // inputs[i].mi.mouseData = 0;
        // inputs[i].mi.dx = x * (65536.0 / sx);
        // inputs[i].mi.dy = y * (65536.0 / sy);

        // SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    }
};