/**
 * @file win32_handmade.cpp
 * @brief HandmadeHero
 * @author Ahmed Abd El-Aal <eng.ahmedhussein89@gmail.com>
 * @version 0.1.0.
 * @date 2019-12-26
 */
#include <windows.h>

#define internal static
#define local_presist static
#define global_variable static

// TODO(Hussein): This is global for now
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

// DIB: Device Indepent Bitmap
internal void
Win32ResizeDIBSection(int width, int height) {
  // TODO(Hussein): Bulletproof this.
  // Maybe don't free first, free after, then free first if that fails.

  if(BitmapHandle) {
    DeleteObject(BitmapHandle);
  }

  if(!BitmapDeviceContext) {
    // TODO(Hussein): Should we recreate these under certian special circumstances
    BitmapDeviceContext = CreateCompatibleDC(0);
  }

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth  = width;
  BitmapInfo.bmiHeader.biHeight = height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  BitmapHandle = CreateDIBSection(
      BitmapDeviceContext, &BitmapInfo,
      DIB_RGB_COLORS,
      &BitmapMemory,
      0, 0);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height) {
  StretchDIBits(DeviceContext,
                X, Y, Width, Height,
                X, Y, Width, Height,
                BitmapMemory,
                &BitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND   hWindow,
                        UINT   uMessage,
                        WPARAM wParam,
                        LPARAM lParam) {
    LRESULT Result = 0;

    switch(uMessage) {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(hWindow, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom = ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;
        case WM_CLOSE:
        {
            // TODO(Hussein): Handle this with a message to the user?
            Running = false;
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WH_ACTIVATEAPP\n");
        } break;
        case WM_DESTROY:
        {
            // TODO(Hussein): Handle this an error - recreate window?
            Running = false;
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(hWindow, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
            EndPaint(hWindow, &Paint);
        } break;
        default:
        {
            Result = DefWindowProc(hWindow, uMessage, wParam, lParam);
        } break;
    }

    return Result;
}

int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow) {
    WNDCLASS WindowClass = {};

    // TODO(Hussein): Check if HREDDRAW/VREDRAW/OWNDC still matter
    WindowClass.style       = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance   = hInstance;
    // WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if(RegisterClass(&WindowClass)) {
        HWND WindowHandle = CreateWindowEx(
                0,
                WindowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                hInstance,
                0);
        if(WindowHandle) {
            Running = true;
            while(Running) {
                MSG Message;
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if(MessageResult > 0) {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } else {
                    break;
                }
            }
        } else {
            // TODO(Hussein): Logging
        }
    } else {
        // TODO(Hussein): Logging
    }
  return 0;
}
