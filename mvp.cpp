#include "mvp.h"
#include <limits>
Matrix modelTras = Matrix::identity();
Matrix view = Matrix::identity();
Matrix projection = Matrix::identity();
Matrix viewport = Matrix::identity();
IShader::~IShader() {}
constexpr double MY_PI = 3.1415926;
void set_model(float rotation_angle)
{
    float randian = rotation_angle / 180.0 * MY_PI;
    Matrix model_trans = Matrix::identity();
    model_trans[0][0]= cos(randian);
    model_trans[0][1]= -sin(randian);
    model_trans[1][0]= sin(randian);
    model_trans[1][1]= cos(randian);
    modelTras = model_trans * modelTras;
}
void set_projection(float coeff)
{
    projection[3][2] = coeff;
}

void set_view(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    for (int i = 0; i < 3; i++)
    {
        view[0][i] = x[i];
        view[1][i] = y[i];
        view[2][i] = z[i];
        view[i][3] = -center[i];
    }
}

void set_viewport(int x, int y, int w, int h)
{
    viewport[0][3] = x + w / 2.f;
    viewport[1][3] = y + h / 2.f;
    viewport[2][3] = 255.0 / 2.f;
    viewport[0][0] = w / 2.f;
    viewport[1][1] = h / 2.f;
    viewport[2][2] = 255.0 / 2.f;
}
Vec3f barycentric(Vec3f *pts, Vec3f P)
{
    Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]) , Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
    if (std::abs(u[2]) < 1)
        return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}
void triangle(Vec3f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer) {
    int min_X = std::floor(std::min(pts[0][0], std::min(pts[1][0], pts[2][0])));
    int min_Y = std::floor(std::min(pts[0][1], std::min(pts[1][1], pts[2][1])));
    int max_X = std::ceil(std::max(pts[0][0], std::max(pts[1][0], pts[2][0])));
    int max_Y = std::ceil(std::max(pts[0][1], std::max(pts[1][1], pts[2][1])));
    Vec3f P;
     for (int x = min_X; x < max_X + 1; x++)
    {
        for (int y = min_Y; y < max_Y + 1; y++)
        {   
            TGAColor color;
            P.x = x;
            P.y = y;
            Vec3f bcentric = barycentric(pts, P);
            if (bcentric.x < 0 || bcentric.y < 0 || bcentric.z < 0)
                continue;
            P.z = bcentric.x * pts[0].z + bcentric.y * pts[1].z + bcentric.z * pts[2].z;
            if (zbuffer.get(P.x, P.y)[0] > P.z)
                 continue;
            bool discard = shader.fragment(bcentric, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, TGAColor(P.z));
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec4f v3tov4(Vec3f v) {
    Vec4f m;
    m[0] = v.x;
    m[1] = v.y;
    m[2] = v.z;
    m[3] = 1.f;
    return m;
}
Vec3f v4tov3(Vec4f v) {
    Vec3f m;
    m[0] = v[0]/v[3];
    m[1] = v[1]/v[3];
    m[2] = v[2]/v[3];
    return m;
}