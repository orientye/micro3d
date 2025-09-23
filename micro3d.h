#pragma once

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
	union {
		struct {
			unsigned char r, g, b, a;
		};
		unsigned char raw[4];
		unsigned int val;
	};
} color_t;

typedef struct {
	int width;
	int height;
	unsigned int* buffer;
} device_t;

inline void pixel(device_t* device, int x, int y, unsigned int clr) {
	if (x >= 0 && x < device->width && y >= 0 && y < device->height) {
        device->buffer[x  + y * device->width] = clr;
    }
}

inline void line(device_t* device, int x1, int y1, int x2, int y2, unsigned int clr) {
    int dx = (x1 < x2) ? (x2 - x1) : (x1 - x2);
	int dy = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int err2;
    while (1) {
        pixel(device, x1, y1, clr);
        if (x1 == x2 && y1 == y2) break;
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

inline void triangle(device_t* device, vec4_t* v1, vec4_t* v2, vec4_t* v3) {

}

inline void lookAt(matrix_t* view, vec4_t* eye, vec4_t*target, vec4_t* up) {

}

inline void render3d(device_t* device) {
	pixel(device, 400, 100, 0xc00000);
	pixel(device, 400, 200, 0xc00000);
	pixel(device, 400, 300, 0xc00000);
	line(device, 0, 0, 400, 300, 0xc00000);
	line(device, 400, 0, 400, 150, 0xc00000);
}