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
} device_t;

void pixel(device_t* device, int x, int y, color_t clr) {

}

void line(device_t* device, int x1, int y1, int x2, int y2, color_t clr) {

}
