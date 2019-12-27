/**
 * @file win32_handmade.cpp
 * @brief HandmadeHero
 * @author Ahmed Abd El-Aal <eng.ahmedhussein89@gmail.com>
 * @version 0.1.0.
 * @date 2019-12-26
 */
#include <windows.h>
#include <stdint.h>

#define internal static
#define local_presist static
#define global_variable static

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// TODO(Hussein): This is global for now
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void
RenderWeirdGradient(int XOffset, int YOffset) {
  int Width  = BitmapWidth;
  int Height = BitmapHeight;

  int Pitch = Width*BytesPerPixel;
  uint8 *Row = (uint8 *)BitmapMemory;
  for(int Y = 0;
      Y < BitmapHeight;
      ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = 0;
        X < BitmapWidth;
        ++X) {
      uint8 Blue = (uint8)(X + XOffset);
      uint8 Green = (uint8)(Y + YOffset);

      *Pixel++ = ((Green << 8) | Blue);
    }
    Row += Pitch;
  }
}

// DIB: Device Indepent Bitmap
internal void
Win32ResizeDIBSection(int Width, int Height) {
  // TODO(Hussein): Bulletproof this.
  // Maybe don't free first, free after, then free first if that fails.

  if(BitmapMemory) {
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }

  BitmapWidth  = Width;
  BitmapHeight = Height;

  BitmapInfo.bmiHeader.biSize        = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth       = BitmapWidth;
  BitmapInfo.bmiHeader.biHeight      = -BitmapHeight;
  BitmapInfo.bmiHeader.biPlanes      = 1;
  BitmapInfo.bmiHeader.biBitCount    = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  // NOTE(Hussein): Thanks you to Chris Hecker fo Spy Party frame
  // for clarifying the deal with StretchDIBits and BitBlt!
  // No more DC for us
  int BytesPerPixel = 4;
  int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  // TODO(Hussein): Probably clear this to black
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height) {
  int WindowWidth  = ClientRect->right  - ClientRect->left;
  int WindowHeight = ClientRect->bottom - ClientRect->top;
  StretchDIBits(DeviceContext,
                /*
                X, Y, Width, Height,
                X, Y, Width, Height,
                */
                0, 0, BitmapWidth, BitmapHeight,
                0, 0, WindowWidth, WindowHeight,
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
            int Width  = ClientRect.right  - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
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
            int Width  = Paint.rcPaint.right  - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            RECT ClientRect;
            GetClientRect(hWindow, &ClientRect);
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
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
        HWND Window = CreateWindowEx(
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
        if(Window) {
            int XOffset = 0;
            int YOffset = 0;
            Running = true;
            while(Running) {
              MSG Message;
              while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                if(Message.message == WM_QUIT) {
                  Running = false;
                }
                TranslateMessage(&Message);
                DispatchMessage(&Message);
              }

              RenderWeirdGradient(XOffset, YOffset);

              HDC DeviceContext = GetDC(Window);
              RECT ClientRect;
              GetClientRect(Window, &ClientRect);
              int WindowWidth  = ClientRect.right  - ClientRect.left;
              int WindowHeight = ClientRect.bottom - ClientRect.top;
              Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
              ReleaseDC(Window, DeviceContext);

              ++XOffset;
            }

        } else {
            // TODO(Hussein): Logging
        }
    } else {
        // TODO(Hussein): Logging
    }
  return 0;
}
