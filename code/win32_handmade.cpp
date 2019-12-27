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

struct win32_offscreen_buffer {
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct win32_window_dimension {
  int Width;
  int Height;
};

// TODO(Hussein): This is global for now
global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

win32_window_dimension
Win32GetWindowDimension(HWND Window) {
  win32_window_dimension Result;

  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width  = ClientRect.right  - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;

  return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset) {
  // TODO(Hussein): Let's see what the optimizer does
  uint8 *Row = (uint8 *)Buffer.Memory;
  for(int Y = 0;
      Y < Buffer.Height;
      ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = 0;
        X < Buffer.Width;
        ++X) {
      uint8 Blue = (uint8)(X + BlueOffset);
      uint8 Green = (uint8)(Y + GreenOffset);

      *Pixel++ = ((Green << 8) | Blue);
    }
    Row += Buffer.Pitch;
  }
}

// DIB: Device Indepent Bitmap
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height) {
  // TODO(Hussein): Bulletproof this.
  // Maybe don't free first, free after, then free first if that fails.

  if(Buffer->Memory) {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }

  Buffer->Width         = Width;
  Buffer->Height        = Height;
  Buffer->BytesPerPixel = 4;

  Buffer->Info.bmiHeader.biSize        = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth       = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight      = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes      = 1;
  Buffer->Info.bmiHeader.biBitCount    = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  // NOTE(Hussein): Thanks you to Chris Hecker fo Spy Party frame
  // for clarifying the deal with StretchDIBits and BitBlt!
  // No more DC for us
  int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
  // TODO(Hussein): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
                           int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer Buffer,
                           int X, int Y, int Width, int Height) {
  // TODO(Hussein): Aspect ratio correction
  StretchDIBits(DeviceContext,
                /*
                X, Y, Width, Height,
                X, Y, Width, Height,
                */
                0, 0, WindowWidth, WindowHeight,
                0, 0, Buffer.Width, Buffer.Height,
                Buffer.Memory,
                &Buffer.Info,
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

            win32_window_dimension Dimension = Win32GetWindowDimension(hWindow);
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                       GlobalBackBuffer, X, Y, Width, Height);

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

    Win32ResizeDIBSection(&GlobalBackBuffer, 1288, 728);

    WindowClass.style       = CS_HREDRAW|CS_VREDRAW;
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

              RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

              HDC DeviceContext = GetDC(Window);
              win32_window_dimension Dimension = Win32GetWindowDimension(Window);
              Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                         GlobalBackBuffer,
                                         0, 0, Dimension.Width, Dimension.Height);
              ReleaseDC(Window, DeviceContext);

              ++XOffset;
              ++YOffset;
            }

        } else {
            // TODO(Hussein): Logging
        }
    } else {
        // TODO(Hussein): Logging
    }
  return 0;
}
