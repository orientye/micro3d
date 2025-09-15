#include <Windows.h>
#include <chrono>

// 声明窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 渲染函数
void Render(HDC hdc, int width, int height);

// 全局变量用于动画
int g_windowWidth = 800;
int g_windowHeight = 600;

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    // 注册窗口类
    const wchar_t CLASS_NAME[] = L"CustomRenderWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // 创建窗口
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"自定义渲染 - 点和线",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_windowWidth, g_windowHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 主循环
    MSG msg = { };
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (true)
    {
        // 处理消息
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // 计算帧时间
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

            // 约60FPS
            if (deltaTime > 16)
            {
                lastTime = currentTime;

                // 获取设备上下文
                HDC hdc = GetDC(hwnd);

                // 创建内存DC用于双缓冲
                HDC memDC = CreateCompatibleDC(hdc);
                HBITMAP memBitmap = CreateCompatibleBitmap(hdc, g_windowWidth, g_windowHeight);
                HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

                // 清空背景
                RECT rect = { 0, 0, g_windowWidth, g_windowHeight };
                HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));
                FillRect(memDC, &rect, bgBrush);
                DeleteObject(bgBrush);

                // 自定义渲染
                Render(memDC, g_windowWidth, g_windowHeight);

                // 将内存DC内容复制到屏幕DC
                BitBlt(hdc, 0, 0, g_windowWidth, g_windowHeight, memDC, 0, 0, SRCCOPY);

                // 清理资源
                SelectObject(memDC, oldBitmap);
                DeleteObject(memBitmap);
                DeleteDC(memDC);
                ReleaseDC(hwnd, hdc);
            }
        }
    }

    return (int)msg.wParam;
}

// 自定义渲染函数
void Render(HDC hdc, int width, int height)
{
    // 1. 绘制一个红色的点（使用多个像素使其更明显）
    COLORREF redColor = RGB(255, 0, 0);
    int pointSize = 3; // 点的大小（像素）
    int pointX = 100;
    int pointY = 100;

    for (int i = -pointSize/2; i <= pointSize/2; i++)
    {
        for (int j = -pointSize/2; j <= pointSize/2; j++)
        {
            SetPixel(hdc, pointX + i, pointY + j, redColor);
        }
    }

    // 2. 绘制一条蓝色的线（使用自定义的Bresenham画线算法）
    COLORREF blueColor = RGB(0, 0, 255);
    int startX = 200;
    int startY = 150;
    int endX = 400;
    int endY = 300;

    // 简单的Bresenham画线算法实现
    int dx = abs(endX - startX);
    int dy = abs(endY - startY);
    int sx = (startX < endX) ? 1 : -1;
    int sy = (startY < endY) ? 1 : -1;
    int err = dx - dy;

    int x = startX;
    int y = startY;

    while (true)
    {
        // 绘制线上的点（可以绘制多个像素使线更粗）
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                SetPixel(hdc, x + i, y + j, blueColor);
            }
        }

        if (x == endX && y == endY)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }

    // 3. 绘制坐标信息
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);

    wchar_t info[100];
    swprintf_s(info, L"点: (%d, %d)  线: (%d, %d) 到 (%d, %d)", 
               pointX, pointY, startX, startY, endX, endY);
    TextOut(hdc, 10, height - 30, info, wcslen(info));

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        // 更新窗口尺寸
        g_windowWidth = LOWORD(lParam);
        g_windowHeight = HIWORD(lParam);
        return 0;

    case WM_ERASEBKGND:
        // 阻止系统擦除背景，由我们自己处理
        return 1;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}