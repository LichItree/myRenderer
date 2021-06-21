#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "pipeLine.h"
#include <iostream>
Model *model = NULL;
float *shadowbuffer = NULL;

const int width  = 800;
const int height = 800;
Vec3f light_dir(1,1,1);
Vec3f       eye(0,0,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);
float angle = 0.0;

struct DepthShader : public IShader {
    mat<3,3,float> varying_tri;

    DepthShader() : varying_tri() {}

    virtual Vec3f vertex(int iface, int nthvert) {
        Vec3f gl_Vertex =  v4tov3(viewport*projection*view*modelTras*embed<4>(model->vert(iface, nthvert)));          // transform it to screen coordinates
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f p = varying_tri*bar;
        color = TGAColor(255, 255, 255)*(p.z/depth);
        return false;
    }
};
struct Shader : public IShader {
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()
    mat<4,4,float> uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
    mat<2,3,float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<3,3,float> varying_tri; // triangle coordinates before Viewport transform, written by VS, read by FS

    Shader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}

    virtual Vec3f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec3f gl_Vertex =  v4tov3(viewport*projection*view*modelTras*embed<4>(model->vert(iface, nthvert)));          // transform it to screen coordinates
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f sb_p =v4tov3(uniform_Mshadow*embed<4>(varying_tri*bar)); // corresponding point in the shadow buffer
        float shadow = .3+.7*(shadowbuffer[int(sb_p[0]) + int(sb_p[1])*width]<sb_p[2]); //  avoid z-fighting
        
        Vec2f uv = varying_uv*bar;
        Vec3f n = v4tov3(uniform_MIT*embed<4>(model->normal(uv))).normalize();
        Vec3f l = v4tov3(uniform_M  *embed<4>(light_dir        )).normalize();
        Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);
        TGAColor c = model->diffuse(uv);

        for (int i=0; i<3; i++) color[i] = std::min<float>(20 + c[i]*shadow*(1.2*diff + .6*spec), 255);
        return false;
    }
};

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    float *zbuffer = new float[width*height];
    shadowbuffer   = new float[width*height];
    for (int i=width*height; --i; ) {
        zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
    }
    light_dir.normalize();

    // rendering the shadow buffer
    TGAImage depth(width, height, TGAImage::RGB);
    set_view(light_dir, center, up);
    set_projection(0);
    set_viewport(width/8, height/8, width*3/4, height*3/4);
    DepthShader depthshader;
    Vec3f screen_coords[3];
    for (int i=0; i<model->nfaces(); i++) {
        for (int j=0; j<3; j++) {
            screen_coords[j] = depthshader.vertex(i, j);
        }
        triangle(screen_coords, depthshader, depth, shadowbuffer);
    }
    depth.flip_vertically(); // to place the origin in the bottom left corner of the image
    depth.write_tga_file("depth.tga");

    Matrix M = viewport*projection*view*modelTras;

    // rendering the frame buffer
    TGAImage frame(width, height, TGAImage::RGB);
    set_view(eye, center, up);
    set_projection(-1.f/(eye-center).norm());
    set_viewport(width/8, height/8, width*3/4, height*3/4);

    Shader shader(view*modelTras, (projection*view*modelTras).invert_transpose(), M*(viewport*projection*view*modelTras).invert());
    for (int i=0; i<model->nfaces(); i++) {
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, frame, zbuffer);
    }
    frame.flip_vertically(); // to place the origin in the bottom left corner of the image
    frame.write_tga_file("framebuffer.tga");

    delete model;
    delete [] zbuffer;
    delete [] shadowbuffer;
    return 0;
}