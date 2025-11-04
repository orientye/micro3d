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

inline void triangle(device_t* device, vec4_t* v1, vec4_t* v2, vec4_t* v3, unsigned int clr)
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

            float cp0 = cross_product_2d(v2, v3, &p);
            float cp1 = cross_product_2d(v3, v1, &p);
            float cp2 = cross_product_2d(v1, v2, &p);

            // 检查像素是否在三角形内
            if (cp0 >= 0 && cp1 >= 0 && cp2 >= 0) {
                pixel(device, x, y, clr);
            }
        }
    }
}

//线框模式
inline void triangle_wireframe(device_t* device, vec4_t* v1, vec4_t* v2, vec4_t* v3, unsigned int clr)
{
    line(device, (int)v1->x, (int)v1->y, (int)v2->x, (int)v2->y, clr);
    line(device, (int)v2->x, (int)v2->y, (int)v3->x, (int)v3->y, clr);
    line(device, (int)v3->x, (int)v3->y, (int)v1->x, (int)v1->y, clr);
}

inline void matrix_look_at(matrix_t* view, vec4_t* eye, vec4_t* target, vec4_t* up)
{
    // 计算前向向量 (forward = target - eye)
    vec4_t forward = {
        target->x - eye->x,
        target->y - eye->y,
        target->z - eye->z,
        0.0f
    };

    // 归一化前向向量
    float length = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (length > 0) {
        forward.x /= length;
        forward.y /= length;
        forward.z /= length;
    }

    // 计算右向量 (right = forward × up)
    vec4_t right = {
        forward.y * up->z - forward.z * up->y,
        forward.z * up->x - forward.x * up->z,
        forward.x * up->y - forward.y * up->x,
        0.0f
    };

    // 归一化右向量
    length = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
    if (length > 0) {
        right.x /= length;
        right.y /= length;
        right.z /= length;
    }

    // 计算上向量 (up = right × forward)
    vec4_t new_up = {
        right.y * forward.z - right.z * forward.y,
        right.z * forward.x - right.x * forward.z,
        right.x * forward.y - right.y * forward.x,
        0.0f
    };

    // 构建视图矩阵 (右手坐标系，相机看向 -z 方向)
    // 旋转部分
    view->m[0][0] = right.x;
    view->m[0][1] = new_up.x;
    view->m[0][2] = -forward.x;
    view->m[0][3] = 0.0f;

    view->m[1][0] = right.y;
    view->m[1][1] = new_up.y;
    view->m[1][2] = -forward.y;
    view->m[1][3] = 0.0f;

    view->m[2][0] = right.z;
    view->m[2][1] = new_up.z;
    view->m[2][2] = -forward.z;
    view->m[2][3] = 0.0f;

    // 平移部分 (将世界坐标转换到相机坐标)
    view->m[3][0] = -(right.x * eye->x + right.y * eye->y + right.z * eye->z);
    view->m[3][1] = -(new_up.x * eye->x + new_up.y * eye->y + new_up.z * eye->z);
    view->m[3][2] = forward.x * eye->x + forward.y * eye->y + forward.z * eye->z;
    view->m[3][3] = 1.0f;
}

inline void matrix_perspective_fov(matrix_t* projection, float fovY, float aspect, float zn, float zf)
{
    // 清除矩阵
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            projection->m[i][j] = 0.0f;
        }
    }

    // 计算投影参数
    float yScale = 1.0f / tanf(fovY * 0.5f); // cot(fovY/2)
    float xScale = yScale / aspect;

    //// 构建左手透视投影矩阵    左手系：+Z 指向屏幕外
    //projection->m[0][0] = xScale; // 缩放 x 坐标
    //projection->m[1][1] = yScale; // 缩放 y 坐标
    //projection->m[2][2] = zf / (zf - zn); // 深度映射
    //projection->m[2][3] = 1.0f; // 透视除法
    //projection->m[3][2] = -zn * zf / (zf - zn); // 深度平移

    // 构建右手透视投影矩阵    右手系：+Z 指向屏幕内
    projection->m[0][0] = xScale; // 缩放 x 坐标
    projection->m[1][1] = yScale; // 缩放 y 坐标
    projection->m[2][2] = zf / (zn - zf); // 深度映射 (右手系)
    projection->m[2][3] = -1.0f; // 透视除法 (右手系为负)
    projection->m[3][2] = zn * zf / (zn - zf); // 深度平移 (右手系)

	//// OpenGL风格的右手投影矩阵 (Z范围[-1,1])
 //   projection->m[0][0] = xScale;
 //   projection->m[1][1] = yScale;
 //   projection->m[2][2] = (zf + zn) / (zn - zf);     // 映射到[-1,1]
 //   projection->m[2][3] = -1.0f;
 //   projection->m[3][2] = (2.0f * zf * zn) / (zn - zf);
}

// 矩阵乘法：result = a * b
inline void matrix_multiply(matrix_t* result, const matrix_t* a, const matrix_t* b)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result->m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                result->m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
}

// 向量与矩阵乘法：result = v * m
inline void vector_transform(vec4_t* result, const vec4_t* v, const matrix_t* m)
{
    result->x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + v->w * m->m[3][0];
    result->y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + v->w * m->m[3][1];
    result->z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + v->w * m->m[3][2];
    result->w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + v->w * m->m[3][3];
}

// 透视除法：将齐次坐标转换为屏幕坐标
inline void perspective_divide(vec4_t* v)
{
    if (v->w != 0.0f) {
        v->x /= v->w;
        v->y /= v->w;
        v->z /= v->w;
    }
}

// 将标准化设备坐标转换为屏幕坐标
inline void viewport_transform(vec4_t* v, int screen_width, int screen_height)
{
    v->x = (v->x + 1.0f) * 0.5f * screen_width;
    v->y = (1.0f - v->y) * 0.5f * screen_height; // Y轴翻转
}

// 创建单位矩阵
inline void matrix_identity(matrix_t* m)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m->m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

// 创建平移矩阵
inline void matrix_translation(matrix_t* m, float tx, float ty, float tz)
{
    matrix_identity(m);
    m->m[3][0] = tx;
    m->m[3][1] = ty;
    m->m[3][2] = tz;
}

// 创建旋转矩阵（绕X轴）
inline void matrix_rotation_x(matrix_t* m, float angle)
{
    matrix_identity(m);
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    m->m[1][1] = cosA;
    m->m[1][2] = sinA;
    m->m[2][1] = -sinA;
    m->m[2][2] = cosA;
}

// 创建旋转矩阵（绕Y轴）
inline void matrix_rotation_y(matrix_t* m, float angle)
{
    matrix_identity(m);
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    m->m[0][0] = cosA;
    m->m[0][2] = -sinA;
    m->m[2][0] = sinA;
    m->m[2][2] = cosA;
}

// 创建缩放矩阵
inline void matrix_scaling(matrix_t* m, float sx, float sy, float sz)
{
    matrix_identity(m);
    m->m[0][0] = sx;
    m->m[1][1] = sy;
    m->m[2][2] = sz;
}

// 绘制长方体
inline void draw_cube(device_t* device, const transform_t* transform)
{
    // 长方体的8个顶点（局部坐标）
    vec4_t vertices[8] = {
        // 前面四个顶点
        { -0.5f, -0.5f,  0.5f, 1.0f }, // 左下前 0
        {  0.5f, -0.5f,  0.5f, 1.0f }, // 右下前 1
        {  0.5f,  0.5f,  0.5f, 1.0f }, // 右上前 2
        { -0.5f,  0.5f,  0.5f, 1.0f }, // 左上前 3
        
        // 后面四个顶点
        { -0.5f, -0.5f, -0.5f, 1.0f }, // 左下后 4
        {  0.5f, -0.5f, -0.5f, 1.0f }, // 右下后 5
        {  0.5f,  0.5f, -0.5f, 1.0f }, // 右上后 6
        { -0.5f,  0.5f, -0.5f, 1.0f }  // 左上后 7
    };

    // 定义6个面的12个三角形，每个面指定颜色
    struct Face {
        int indices[3];  // 三角形顶点索引
        unsigned int color; // 面的颜色
    };
    
    // 六个面的颜色（RGB格式）
    unsigned int colors[6] = {
        0xFF0000, // 前面 - 红色
        0x00FF00, // 后面 - 绿色  
        0x0000FF, // 上面 - 蓝色
        0xFFFF00, // 下面 - 黄色
        0xFF00FF, // 左面 - 紫色
        0x00FFFF  // 右面 - 青色
    };
    
    Face faces[12] = {
        // 前面 (红色)
        {{0, 1, 2}, colors[0]},
        {{0, 2, 3}, colors[0]},
        
        // 后面 (绿色)
        {{5, 4, 7}, colors[1]},
        {{5, 7, 6}, colors[1]},
        
        // 上面 (蓝色)
        {{3, 2, 6}, colors[2]},
        {{3, 6, 7}, colors[2]},
        
        // 下面 (黄色)
        {{1, 0, 4}, colors[3]},
        {{1, 4, 5}, colors[3]},
        
        // 左面 (紫色)
        {{4, 0, 3}, colors[4]},
        {{4, 3, 7}, colors[4]},
        
        // 右面 (青色)
        {{1, 5, 6}, colors[5]},
        {{1, 6, 2}, colors[5]}
    };

    // 变换后的顶点
    vec4_t transformed_vertices[8];
    
    // 计算世界视图投影矩阵
    matrix_t world_view;
    matrix_t wvp; // world * view * projection
    matrix_multiply(&world_view, &transform->world, &transform->view);
    matrix_multiply(&wvp, &world_view, &transform->projection);

    // 变换所有顶点
    for (int i = 0; i < 8; i++) {
        vector_transform(&transformed_vertices[i], &vertices[i], &wvp);
        perspective_divide(&transformed_vertices[i]);
        viewport_transform(&transformed_vertices[i], device->width, device->height);
    }

    // 绘制所有三角形面，使用各自的颜色
    for (int i = 0; i < 12; i++) {
        int idx1 = faces[i].indices[0];
        int idx2 = faces[i].indices[1];
        int idx3 = faces[i].indices[2];
        
        triangle(device, 
                 &transformed_vertices[idx1], 
                 &transformed_vertices[idx2], 
                 &transformed_vertices[idx3], 
                 faces[i].color);
    }
}

// 绘制长方体线框
inline void draw_cube_wireframe(device_t* device, const transform_t* transform)
{
    // 长方体的8个顶点（局部坐标）
    vec4_t vertices[8] = {
        // 前面四个顶点
        { -0.5f, -0.5f,  0.5f, 1.0f }, // 左下前 0
        {  0.5f, -0.5f,  0.5f, 1.0f }, // 右下前 1
        {  0.5f,  0.5f,  0.5f, 1.0f }, // 右上前 2
        { -0.5f,  0.5f,  0.5f, 1.0f }, // 左上前 3
        
        // 后面四个顶点
        { -0.5f, -0.5f, -0.5f, 1.0f }, // 左下后 4
        {  0.5f, -0.5f, -0.5f, 1.0f }, // 右下后 5
        {  0.5f,  0.5f, -0.5f, 1.0f }, // 右上后 6
        { -0.5f,  0.5f, -0.5f, 1.0f }  // 左上后 7
    };

    // 定义12条边，每条边连接两个顶点
    struct Edge {
        int start, end;      // 边的起点和终点索引
        unsigned int color;  // 边的颜色
    };
    
    // 六个面的颜色（RGB格式）
    unsigned int colors[6] = {
        0xFF0000, // 前面 - 红色
        0x00FF00, // 后面 - 绿色  
        0x0000FF, // 上面 - 蓝色
        0xFFFF00, // 下面 - 黄色
        0xFF00FF, // 左面 - 紫色
        0x00FFFF  // 右面 - 青色
    };
    
    Edge edges[12] = {
        // 前面四条边 (红色)
        {0, 1, colors[0]}, // 下边
        {1, 2, colors[0]}, // 右边
        {2, 3, colors[0]}, // 上边
        {3, 0, colors[0]}, // 左边
        
        // 后面四条边 (绿色)
        {4, 5, colors[1]}, // 下边
        {5, 6, colors[1]}, // 右边
        {6, 7, colors[1]}, // 上边
        {7, 4, colors[1]}, // 左边
        
        // 连接前后面的四条边
        {0, 4, colors[4]}, // 左下 (紫色)
        {1, 5, colors[5]}, // 右下 (青色)
        {2, 6, colors[2]}, // 右上 (蓝色)
        {3, 7, colors[3]}  // 左上 (黄色)
    };

    // 变换后的顶点
    vec4_t transformed_vertices[8];
    
    // 计算世界视图投影矩阵
    matrix_t world_view;
    matrix_t wvp; // world * view * projection
    matrix_multiply(&world_view, &transform->world, &transform->view);
    matrix_multiply(&wvp, &world_view, &transform->projection);

    // 变换所有顶点
    for (int i = 0; i < 8; i++) {
        vector_transform(&transformed_vertices[i], &vertices[i], &wvp);
        perspective_divide(&transformed_vertices[i]);
        viewport_transform(&transformed_vertices[i], device->width, device->height);
    }

    // 绘制所有边
    for (int i = 0; i < 12; i++) {
        int start_idx = edges[i].start;
        int end_idx = edges[i].end;
        
        line(device, 
             (int)transformed_vertices[start_idx].x, 
             (int)transformed_vertices[start_idx].y,
             (int)transformed_vertices[end_idx].x, 
             (int)transformed_vertices[end_idx].y,
             edges[i].color);
    }
}

inline void render3d(device_t* device)
{
    // pixel(device, 400, 100, 0xc00000);
    // pixel(device, 400, 200, 0xc00000);
    // pixel(device, 400, 300, 0xc00000);

    // line(device, 0, 0, 400, 300, 0xc00000);
    // line(device, 400, 0, 400, 150, 0xc00000);

    //vec4_t v1 = { 100, 100, 0, 1 };
    //vec4_t v2 = { 400, 100, 0, 1 };
    //vec4_t v3 = { 250, 400, 0, 1 };
    //triangle(device, &v1, &v2, &v3, 0xc00000);

	 // 设置变换
    transform_t transform;

	// 世界矩阵：让长方体稍微旋转
    float angle = 0.0f;
    angle += 0.01f;
    
    matrix_t rotation_y, translation, scaling;
    matrix_rotation_y(&rotation_y, angle);
    matrix_translation(&translation, 0.0f, 0.0f, 0.1f);
    matrix_scaling(&scaling, 1.0f, 0.5f, 0.8f); // 非立方体，更像长方体
    
    // 组合世界变换：先缩放，再旋转，最后平移
    matrix_t temp;
    matrix_multiply(&temp, &scaling, &rotation_y);
    matrix_multiply(&transform.world, &temp, &translation);
    
    // 视图矩阵：相机位置
    vec4_t eye = { 0.0f, 0.0f, -2.0f, 1.0f };
    vec4_t target = { 0.0f, 0.0f, 0.0f, 1.0f };
    vec4_t up = { 0.0f, 1.0f, 0.0f, 0.0f };
    matrix_look_at(&transform.view, &eye, &target, &up);
    
    // 投影矩阵
    float aspect = (float)device->width / (float)device->height;
    matrix_perspective_fov(&transform.projection, 3.1415926f / 3.0f, aspect, 0.1f, 100.0f);
    
    // 绘制长方体
    draw_cube_wireframe(device, &transform);
}