#include <Windows.h>
#include <chrono>
#include <cmath>

// 声明窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 渲染函数
void Render();

// 自定义绘制函数
void DrawPixel(int x, int y, COLORREF color);
void DrawLine(int x1, int y1, int x2, int y2, COLORREF color);
void ClearScreen(COLORREF color);

// 全局变量
int g_windowWidth = 800;
int g_windowHeight = 600;

// DIB相关变量
HBITMAP g_hBitmap = nullptr;
void* g_pixels = nullptr;
HDC g_memDC = nullptr;
BITMAPINFO g_bmi = {};

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    // 注册窗口类
    const wchar_t CLASS_NAME[] = L"DIBSectionWindowClass";

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
        L"micro3d",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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

    // 初始化DIB Section
    HDC hdc = GetDC(hwnd);
    
    // 设置BITMAPINFO结构
    ZeroMemory(&g_bmi, sizeof(BITMAPINFO));
    g_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth = g_windowWidth;
    g_bmi.bmiHeader.biHeight = -g_windowHeight; // 负值表示从上到下的DIB
    g_bmi.bmiHeader.biPlanes = 1;
    g_bmi.bmiHeader.biBitCount = 32; // 32位ARGB
    g_bmi.bmiHeader.biCompression = BI_RGB;
    g_bmi.bmiHeader.biSizeImage = 0;

    // 创建DIB Section
    g_hBitmap = CreateDIBSection(hdc, &g_bmi, DIB_RGB_COLORS, &g_pixels, NULL, 0);
    
    // 创建内存DC
    g_memDC = CreateCompatibleDC(hdc);
    SelectObject(g_memDC, g_hBitmap);
    
    ReleaseDC(hwnd, hdc);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 主循环
    MSG msg = { };
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

            if (deltaTime > 16) // ~60 FPS
            {
                lastTime = currentTime;

                // 清屏
                ClearScreen(RGB(240, 240, 240));

                // 自定义渲染
                Render();

                // 更新到屏幕
                HDC hdc = GetDC(hwnd);
                BitBlt(hdc, 0, 0, g_windowWidth, g_windowHeight, g_memDC, 0, 0, SRCCOPY);
                ReleaseDC(hwnd, hdc);
            }
        }
    }

    // 清理资源
    if (g_memDC) DeleteDC(g_memDC);
    if (g_hBitmap) DeleteObject(g_hBitmap);

    return (int)msg.wParam;
}

// 绘制像素函数
void DrawPixel(int x, int y, COLORREF color)
{
    if (x < 0 || x >= g_windowWidth || y < 0 || y >= g_windowHeight)
        return;

    // 计算像素位置（32位，每个像素4字节）
    int offset = y * g_windowWidth + x;
    BYTE* pixel = (BYTE*)g_pixels + offset * 4;
    
    // 设置颜色（BGR格式）
    pixel[0] = GetBValue(color); // Blue
    pixel[1] = GetGValue(color); // Green
    pixel[2] = GetRValue(color); // Red
    pixel[3] = 0;               // Alpha (未使用)
}

// 绘制线函数（Bresenham算法）
void DrawLine(int x1, int y1, int x2, int y2, COLORREF color)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        // 绘制线宽为3的线
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                DrawPixel(x1 + i, y1 + j, color);
            }
        }

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

// 清屏函数
void ClearScreen(COLORREF color)
{
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);

    // 直接操作整个像素缓冲区
    BYTE* pixel = (BYTE*)g_pixels;
    int totalPixels = g_windowWidth * g_windowHeight;

    for (int i = 0; i < totalPixels; i++)
    {
        pixel[0] = b; // Blue
        pixel[1] = g; // Green
        pixel[2] = r; // Red
        pixel[3] = 0; // Alpha
        
        pixel += 4;
    }
}

// 自定义渲染函数
void Render()
{
    static int frameCount = 0;
    frameCount++;

    // 1. 绘制一个红色的点（会动）
    int pointX = 100 + 50 * sin(frameCount * 0.05f);
    int pointY = 100 + 50 * cos(frameCount * 0.05f);
    
    // 绘制大一点的点
    for (int i = -3; i <= 3; i++)
    {
        for (int j = -3; j <= 3; j++)
        {
            DrawPixel(pointX + i, pointY + j, RGB(255, 0, 0));
        }
    }

    // 2. 绘制一条蓝色的线（也会动）
    int startX = 200;
    int startY = 150;
    int endX = 400 + 50 * sin(frameCount * 0.03f);
    int endY = 300 + 50 * cos(frameCount * 0.03f);
    
    DrawLine(startX, startY, endX, endY, RGB(0, 0, 255));

    // 3. 绘制一些文字信息（使用GDI，因为文字渲染用DIB比较麻烦）
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

    HFONT hOldFont = (HFONT)SelectObject(g_memDC, hFont);
    SetTextColor(g_memDC, RGB(0, 0, 0));
    SetBkMode(g_memDC, TRANSPARENT);

    wchar_t info[256];
    swprintf_s(info, L"micro3d - 点: (%d, %d) 线: (%d, %d)到(%d, %d) FPS: 60", 
               pointX, pointY, startX, startY, endX, endY);
    TextOut(g_memDC, 10, g_windowHeight - 30, info, wcslen(info));

    SelectObject(g_memDC, hOldFont);
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
    {
        // 窗口大小改变时重新创建DIB Section
        g_windowWidth = LOWORD(lParam);
        g_windowHeight = HIWORD(lParam);

        // 清理旧资源
        if (g_memDC) DeleteDC(g_memDC);
        if (g_hBitmap) DeleteObject(g_hBitmap);

        // 创建新的DIB Section
        HDC hdc = GetDC(hwnd);
        
        g_bmi.bmiHeader.biWidth = g_windowWidth;
        g_bmi.bmiHeader.biHeight = -g_windowHeight;
        
        g_hBitmap = CreateDIBSection(hdc, &g_bmi, DIB_RGB_COLORS, &g_pixels, NULL, 0);
        g_memDC = CreateCompatibleDC(hdc);
        SelectObject(g_memDC, g_hBitmap);
        
        ReleaseDC(hwnd, hdc);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1; // 阻止系统擦除背景

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}