const int WIDTH = 800;
const int HEIGHT = 600;

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

struct {
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
	unsigned int buffer[WIDTH][HEIGHT];
} device_t;

void pixel(device_t* device, int x, int y, color_t clr) {
     if (x >= 0 && x < device->width && y >= 0 && y < device->height) {
        device->buffer[x][y] = clr;
    }
}

void line(device_t* device, int x1, int y1, int x2, int y2, color_t clr) {

}

void triangle(device_t* device, const vec4_t& v1, const vec4_t& v2, const vec4_t& v3) {

}

void lookAt(matrix_t* view, const vec4_t& eye, const vec4_t& target, const vec4_t& up) {

}

int main (void) {
    return 0;
}
