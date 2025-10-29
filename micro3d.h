#pragma once

#include <cmath>

typedef struct {
    float x, y, z, w;
} vec4_t;

typedef struct {
    float m[4][4];
} matrix_t;

typedef struct {
    matrix_t world;
    matrix_t view;
    matrix_t projection;
} transform_t;

typedef struct {
    int width;
    int height;
    unsigned int* buffer;
} device_t;

inline void pixel(device_t* device, int x, int y, unsigned int clr)
{
    if (x >= 0 && x < device->width && y >= 0 && y < device->height) {
        device->buffer[x + y * device->width] = clr;
    }
}

inline void line(device_t* device, int x1, int y1, int x2, int y2, unsigned int clr)
{
    int dx = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    int dy = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) >> 1;
    int err2;
    while (1) {
        pixel(device, x1, y1, clr);
        if (x1 == x2 && y1 == y2)
            break;
        err2 = err;
        if (err2 > -dx) {
            err -= dy;
            x1 += sx;
        }
        if (err2 < dy) {
            err += dx;
            y1 += sy;
        }
    }
}

// 向量 AB 和向量 AC 的二维叉积
// 公式：(Bx - Ax)*(Cy - Ay) - (By - Ay)*(Cx - Ax)
// 正值：AB 正旋转(即逆时针)到 AC 小于 180 度
// 负值：AB 正旋转(即逆时针)到 AC 大于 180 度
// 零：AB 与 AC 平行
inline float cross_product_2d(const vec4_t* a, const vec4_t* b, const vec4_t* c)
{
    return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
}

// 边界框三角形填充
inline void triangle_bbox(device_t* device, vec4_t* v1, vec4_t* v2, vec4_t* v3, unsigned int clr)
{
    // 计算三角形的边界框
    int min_x = (int)fminf(fminf(v1->x, v2->x), v3->x);
    int max_x = (int)fmaxf(fmaxf(v1->x, v2->x), v3->x);
    int min_y = (int)fminf(fminf(v1->y, v2->y), v3->y);
    int max_y = (int)fmaxf(fmaxf(v1->y, v2->y), v3->y);

    // 限制在屏幕范围内
    min_x = fmax(min_x, 0);
    max_x = fmin(max_x, device->width - 1);
    min_y = fmax(min_y, 0);
    max_y = fmin(max_y, device->height - 1);

    // 计算整个三角形的有向面积
    float area = cross_product_2d(v1, v2, v3);

    // 如果面积为0，说明是退化三角形，不绘制
    if (fabsf(area) < 1e-8f) {
        return;
    }

    // 遍历边界框内的每个像素
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            // 当前像素位置（使用像素中心）
            vec4_t p = { (float)x + 0.5f, (float)y + 0.5f, 0, 1 };

            float rd0 = cross_product_2d(v2, v3, &p);
            float rd1 = cross_product_2d(v3, v1, &p);
            float rd2 = cross_product_2d(v1, v2, &p);

            // 检查像素是否在三角形内
            if (rd0 >= 0 && rd1 >= 0 && rd2 >= 0) {
                pixel(device, x, y, clr);
            }
        }
    }
}

// 主三角形函数 - 根据需求选择不同的实现
inline void triangle(device_t* device, vec4_t* v1, vec4_t* v2, vec4_t* v3, unsigned int clr)
{
    triangle_bbox(device, v1, v2, v3, clr);
}

inline void lookAt(matrix_t* view, vec4_t* eye, vec4_t* target, vec4_t* up)
{
}

inline void render3d(device_t* device)
{
    // pixel(device, 400, 100, 0xc00000);
    // pixel(device, 400, 200, 0xc00000);
    // pixel(device, 400, 300, 0xc00000);
    // line(device, 0, 0, 400, 300, 0xc00000);
    // line(device, 400, 0, 400, 150, 0xc00000);

    // 定义三角形顶点
    vec4_t v1 = { 100, 100, 0, 1 };
    vec4_t v2 = { 400, 100, 0, 1 };
    vec4_t v3 = { 250, 400, 0, 1 };
    // 绘制三角形
    triangle(device, &v1, &v2, &v3, 0xc00000);
}