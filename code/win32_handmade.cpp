#include <windows.h>

LRESULT CALLBACK MainWindowCallback(HWND   hWindow,
                                    UINT   uMessage,
                                    WPARAM wParam,
                                    LPARAM lParam) {
    LRESULT Result = 0;

    switch(uMessage) {
        case WM_SIZE:
        {
            OutputDebugStringA("WH_SIZE\n");
        } break;
        case WM_DESTROY:
        {
            OutputDebugStringA("WH_DESTROY\n");
        } break;
        case WM_CLOSE:
        {
            OutputDebugStringA("WH_CLOSE\n");
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WH_ACTIVATEAPP\n");
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(hWindow, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            static DWORD Operation = WHITENESS;
            PatBlt(DeviceContext, X, Y, Width, Height, Operation);
            if(Operation == WHITENESS) {
                Operation = BLACKNESS;
            } else {
                Operation = WHITENESS;
            }
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
    WindowClass.lpfnWndProc = MainWindowCallback;
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
            for(;;) {
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
