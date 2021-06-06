#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "mvp.h"
#include <iostream>
Model *model = NULL;
const int width  = 800;
const int height = 800;
Vec3f light_dir(1,1,1);
Vec3f       eye(0,-1,3);
//Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);
float angle = 0.0;

struct Shader0: public IShader{
    Vec3f varying_intensity; 
    virtual Vec3f vertex(int iface, int nthvert) {
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); 
        Vec4f gl_Vertex = v3tov4(model->vert(iface, nthvert));
        return v4tov3(viewport*projection*view*modelTras*gl_Vertex);
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel    
        color = TGAColor(255,255,255)*intensity; 
        return false;                               
    }
};
struct Shader1 : public IShader {
    Vec3f          varying_intensity; // written by vertex shader, read by fragment shader
    mat<2,3,float> varying_uv;        // same as above
    virtual Vec3f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = v3tov4(model->vert(iface, nthvert));
        return v4tov3(viewport*projection*view*modelTras*gl_Vertex);
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        color = model->diffuse(uv)*intensity;      
        return false;                               
    }
};
struct Shader2 : public IShader {
    mat<2,3,float> varying_uv;  // same as above
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()
    virtual Vec3f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = v3tov4(model->vert(iface, nthvert));
        return v4tov3(viewport*projection*view*modelTras*gl_Vertex);
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar; 

        Vec3f n = v4tov3(uniform_MIT*v3tov4(model->normal(uv))).normalize();
        Vec3f l = v4tov3(uniform_M  *v3tov4(light_dir        )).normalize();
        float intensity = std::max(0.f, n*l);

        color = model->diffuse(uv)*intensity;      
        return false;                               
    }
};
struct Shader3 : public IShader {
    mat<2,3,float> varying_uv;  // same as above
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    virtual Vec3f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = v3tov4(model->vert(iface, nthvert));
        return v4tov3(viewport*projection*view*modelTras*gl_Vertex);
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar;
        Vec3f n = v4tov3(uniform_MIT*v3tov4(model->normal(uv))).normalize();
        Vec3f l = v4tov3(uniform_M  *v3tov4(light_dir        )).normalize();
        Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(5 + c[i]*(diff + .6*spec), 255);
        return false;
    }
};
struct Shader : public IShader {
    mat<2,3,float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<3,3,float> varying_nrm; // normal per vertex to be interpolated by FS
    mat<3,3,float> ndc_tri;     // triangle in normalized device coordinates

    virtual Vec3f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_nrm.set_col(nthvert, proj<3>((projection*view*modelTras).invert_transpose()*embed<4>(model->normal(iface, nthvert), 0.f)));//invert_transpose() zhuanzhi(T) and qiuni(-1)
        Vec4f gl_Vertex = projection*view*modelTras*v3tov4(model->vert(iface, nthvert));
        ndc_tri.set_col(nthvert, v4tov3(gl_Vertex));
        return v4tov3(viewport*gl_Vertex);
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f bn = (varying_nrm*bar).normalize();
        Vec2f uv = varying_uv*bar;

        mat<3,3,float> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;

        mat<3,3,float> AI = A.invert();//A(-1)

        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

        mat<3,3,float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

        Vec3f n = (B*model->normal(uv)).normalize();

        float diff = std::max(0.f, n*light_dir);
        color = model->diffuse(uv)*diff;

        return false;
    }
};


int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    set_model(angle);
    set_view(eye, center, up);
    set_projection(-1.f/(eye-center).norm());
    set_viewport(width/8, height/8, width*3/4, height*3/4);
    light_dir.normalize();
    Shader shader;
    //shader.uniform_M   =  projection*view*modelTras;
    //shader.uniform_MIT = (projection*view*modelTras).invert_transpose();
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i=0; i<model->nfaces(); i++) {
        Vec3f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i,j);
        }
        triangle(screen_coords, shader, image, zbuffer);
     }
    image.flip_vertically(); 
    zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write_tga_file("buffer.tga");
    delete model;
    return 0;
}