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
	float r, g, b;
} color_t;

typedef struct {
	int width;
	int height;
} device_t;
