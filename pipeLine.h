#include "geometry.h"
#include "tgaimage.h"
extern Matrix modelTras;
extern Matrix view;
extern Matrix projection;
extern Matrix viewport;
const float depth =2000.0;
struct IShader {
    virtual ~IShader();
    virtual Vec3f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};
void set_model(float rotation_angle);
void set_view(Vec3f eye, Vec3f center, Vec3f up);
void set_projection(float coeff);
void set_viewport(int x, int y, int w, int h);
void triangle(Vec3f *pts, IShader &shader, TGAImage &image, float* zbuffer);
Vec3f barycentric(Vec3f * pts, Vec3f P);
Vec3f v4tov3(Vec4f v);