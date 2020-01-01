/**
 * @file win32_handmade.cpp
 * @brief HandmadeHero
 * @author Ahmed Abd El-Aal <eng.ahmedhussein89@gmail.com>
 * @version 0.1.0.
 * @date 2019-12-26
 */
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <stdio.h>
#include <dsound.h>

// TODO(Hussein):  Implement sine ourselves
#include <math.h>

#define internal        static
#define local_presist   static
#define global_variable static

#define Pi32 3.14159265359F

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32   bool32;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

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
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// NOTE(Hussein): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
  return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(Hussein): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
  return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void
Win32LoadXInput(void) {
  // TODO(Hussein): Test this on Wondows 8
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
  if(!XInputLibrary) {
    // TODO(Hussein): Diagnostic
    XInputLibrary = LoadLibraryA("xinput1_3.dll");
  }

  if(XInputLibrary) {
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    if(!XInputGetState) { XInputGetState = XInputGetStateStub; }

    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    if(!XInputSetState) { XInputSetState = XInputSetStateStub; }

    // TODO(Hussein): Diagnostic

  } else {
    // TODO(Hussein): Diagnostic
  }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize) {
  // NOTE(Hussein): Load the library
  HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
  if(DSoundLibrary) {
    // Note(Hussein): Get a DirectSound object! - cooperative
    direct_sound_create *DirectSoundCreate = (direct_sound_create *)
      GetProcAddress(DSoundLibrary, "DirectSoundCreate");

    // TODO(Hussein): Double-check that this works on XP - DirectSound8 or 7??
    LPDIRECTSOUND DirectSound;
    if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {

      WAVEFORMATEX WaveFormat = {};
      WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
      WaveFormat.nChannels  = 2;
      WaveFormat.nSamplesPerSec = SamplesPerSecond;
      WaveFormat.wBitsPerSample = 16;
      WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
      WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
      WaveFormat.cbSize = 0;

      if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
        DSBUFFERDESC BufferDescription = {};
        BufferDescription.dwSize = sizeof(BufferDescription);
        BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

        // Note(Hussein): "Create" a primary buffer
        // TODO(Hussein): DSBCAPS_GLOBALFOCUS?
        LPDIRECTSOUNDBUFFER PrimaryBuffer;
        if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {
          HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
          if(SUCCEEDED(Error)) {
            // TODO(Hussein): We have finally set the format
            OutputDebugStringA("Primary buffer format was set.\n");
          } else {
            // TODO(Hussein): Diagnostic
          }
        } else {
            // TODO(Hussein): Diagnostic
        }
      } else {
        // TODO(Hussein): Diagnostic
      }

      // TODO(Hussein): DSBCAPS_GETCURRENTPOSITION2
      DSBUFFERDESC BufferDescription = {};
      BufferDescription.dwSize = sizeof(BufferDescription);
      BufferDescription.dwFlags = 0;
      BufferDescription.dwBufferBytes = BufferSize;
      BufferDescription.lpwfxFormat = &WaveFormat;
      HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
      if(SUCCEEDED(Error)) {
        OutputDebugStringA("Secondary buffer created successfully.\n");
      } else {
        exit(0);
      }
    } else {
      // TODO(Hussein): Diagnostic
    }
  } else {
    // TODO(Hussein): Diagnostic
  }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window) {
  win32_window_dimension Result;

  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width  = ClientRect.right  - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;

  return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset) {
  // TODO(Hussein): Let's see what the optimizer does
  uint8 *Row = (uint8 *)Buffer->Memory;
  for(int Y = 0;
      Y < Buffer->Height;
      ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = 0;
        X < Buffer->Width;
        ++X) {
      uint8 Blue = (uint8)(X + BlueOffset);
      uint8 Green = (uint8)(Y + GreenOffset);

      *Pixel++ = ((Green << 8) | Blue);
    }
    Row += Buffer->Pitch;
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
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
                           HDC DeviceContext,
                           int WindowWidth, int WindowHeight) {
  // TODO(Hussein): Aspect ratio correction
  StretchDIBits(DeviceContext,
                /*
                X, Y, Width, Height,
                X, Y, Width, Height,
                */
                0, 0, WindowWidth, WindowHeight,
                0, 0, Buffer->Width, Buffer->Height,
                Buffer->Memory,
                &Buffer->Info,
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
            GlobalRunning = false;
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WH_ACTIVATEAPP\n");
        } break;
        case WM_DESTROY:
        {
            // TODO(Hussein): Handle this an error - recreate window?
            GlobalRunning = false;
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
          uint32 VKCode = wParam;
          bool WasDown = ((lParam & (1 << 30)) != 0);
          bool IsDown  = ((lParam & (1 << 31)) == 0);

          if(WasDown != IsDown) {
            if(VKCode == 'W') {
            } else if(VKCode == 'A') {
            } else if(VKCode == 'S') {
            } else if(VKCode == 'D') {
            } else if(VKCode == 'Q') {
            } else if(VKCode == 'E') {
            } else if(VKCode == VK_UP) {
            } else if(VKCode == VK_LEFT) {
            } else if(VKCode == VK_DOWN) {
            } else if(VKCode == VK_RIGHT) {
            } else if(VKCode == VK_ESCAPE) {
              OutputDebugStringA("Escape: ");
              if(IsDown) {
                OutputDebugStringA("IsDown ");
              }
              if(WasDown) {
                OutputDebugStringA("WasDown ");
              }
              OutputDebugStringA("\n");
            } else if(VKCode == VK_SPACE) {
            }
          }

          bool32 AltKeyWasDown = (lParam & (1 << 29));
          if((VKCode == VK_F4) && AltKeyWasDown) {
            GlobalRunning = false;
          }
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(hWindow, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(hWindow);
            Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext,
                                       Dimension.Width, Dimension.Height);
            EndPaint(hWindow, &Paint);
        } break;
        default:
        {
            Result = DefWindowProc(hWindow, uMessage, wParam, lParam);
        } break;
    }

    return Result;
}

struct win32_sound_output {
  // NOTE(Hussein): Sound test
  int    SamplesPerSecond;
  int    ToneHz;
  int16  ToneVolume;
  uint32 RunningSampleIndex;
  int    WavePeriod;
  int    BytesPerSample;
  int    SecondaryBufferSize;
};

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite) {
  // TODO(Hussein): More strenuous test
  // TODO(Hussein): Switch to a sine wave
  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;
  if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                           &Region1, &Region1Size,
                                           &Region2, &Region2Size,
                                           0))) {
    // TODO(Hussein): assert that Region1Size/Region2Size is valid

    // TODO(Hussein): Collapse these two loops
    DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
    int16 *SampleOut = (int16 *)Region1;
    for(DWORD SampleIndex = 0;
        SampleIndex < Region1SampleCount;
        ++SampleIndex) {
      // TODO(Hussein): Draw this out for people
      real32 t = 2.0F * Pi32 * (real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod;
      real32 SineValue = sinf(t);
      int16  SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;

      ++SoundOutput->RunningSampleIndex;
    }

    DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
    SampleOut = (int16 *)Region2;
    for(DWORD SampleIndex = 0;
        SampleIndex < Region2SampleCount;
        ++SampleIndex) {
      real32 t = 2.0F * Pi32 * (real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod;
      real32 SineValue = sinf(t);
      int16  SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;

      ++SoundOutput->RunningSampleIndex;
    }
  }

  GlobalSecondaryBuffer->Unlock(Region1, Region1Size,
                                Region2, Region2Size);
}

int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow) {
  Win32LoadXInput();

  WNDCLASSA WindowClass = {};

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
      // NOTE(Hussein): Since we specified CS_OWNDC, we can just
      // get one device context and use it forever because we
      // are not sharing it with anyone.
      HDC DeviceContext = GetDC(Window);

      // NOTE(Hussein): Graphics test
      int XOffset = 0;
      int YOffset = 0;

      win32_sound_output SoundOutput = {};

      SoundOutput.SamplesPerSecond    = 48000;
      SoundOutput.ToneHz              = 256;
      SoundOutput.ToneVolume          = 3000;
      SoundOutput.WavePeriod          = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
      SoundOutput.BytesPerSample      = sizeof(int16)*2;
      SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
      Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
      Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.SecondaryBufferSize);
      GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

      GlobalRunning = true;
      while(GlobalRunning) {
        MSG Message;

        while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
          if(Message.message == WM_QUIT) {
            GlobalRunning = false;
          }

          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }

        // TODO(Hussein): Should we poll this more frequently?
        for(DWORD ControllerIndex = 0;
            ControllerIndex < XUSER_MAX_COUNT;
            ++ControllerIndex) {
          XINPUT_STATE ControllerState;
          if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
            // NOTE(Hussein): This controller is plugged in
            // TODO(Hussein): See if ControllerState.dwPacketNumber increments too rapidly
            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

            bool Up            = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down          = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left          = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right         = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool Start         = (Pad->wButtons & XINPUT_GAMEPAD_START);
            bool Back          = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool LeftShoulder  = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool AButton       = (Pad->wButtons & XINPUT_GAMEPAD_A);
            bool BButton       = (Pad->wButtons & XINPUT_GAMEPAD_B);
            bool XButton       = (Pad->wButtons & XINPUT_GAMEPAD_X);
            bool YButton       = (Pad->wButtons & XINPUT_GAMEPAD_Y);

            int16 StickX = Pad->sThumbLX;
            int16 StickY = Pad->sThumbLY;

            XOffset += StickX >> 12;
            YOffset += StickX >> 12;
          } else {
            // NOTE(Hussein): This controller is not available
          }
        }

        RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

        // NOTE(Hussein): DirectSound output test
        DWORD PlayerCursor;
        DWORD WriteCursor;
        if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayerCursor, &WriteCursor))) {
          DWORD ByteToLock = (SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;

          DWORD BytesToWrite;
          // TODO(Hussein): Change this to using a lower latency offset from the playcursor
          // when we actually start having sound effects.
          if(ByteToLock == PlayerCursor) {
            BytesToWrite = 0;
          } else if(ByteToLock > PlayerCursor) {
            BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
            BytesToWrite += PlayerCursor;
          } else {
            BytesToWrite = PlayerCursor - ByteToLock;
          }
          Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);
        }

        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
      }

    } else {
        // TODO(Hussein): Logging
    }
  } else {
      // TODO(Hussein): Logging
  }
  return 0;
}
