#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "mvp.h"
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }
    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

Vec3f barycentric(Vec3f* pts,Vec3i P){
    Vec3f u = Vec3f(pts[2][0]-pts[0][0], pts[1][0]-pts[0][0], pts[0][0]-P[0])^Vec3f(pts[2][1]-pts[0][1], pts[1][1]-pts[0][1], pts[0][1]-P[1]);
    if (std::abs(u[2])<1) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void triangle(const mat<4,4>& MVP,Vec2f* text_coords,float* zbuffer,Vec3f pts[3], TGAImage &image,TGAImage &textrueImage, TGAColor color) { 
    mat<4,4> mvp = MVP;
    mat<4,4> beforeMVP = mat<4,4>::identity();
    mat<4,4> afterMVP = mat<4,4>::identity();
    beforeMVP.set_row(0,{pts[0][0],pts[0][1],pts[0][2],1});
    beforeMVP.set_row(1,{pts[1][0],pts[1][1],pts[1][2],1});
    beforeMVP.set_row(2,{pts[2][0],pts[2][1],pts[2][2],1});
    beforeMVP.set_row(3,{1,1,1,1});
    afterMVP = mvp*beforeMVP;
    std::cout<<afterMVP;
    //pts[0] = {1,1,1};
    //pts[1] = {mvp[1][0],mvp[1][1],mvp[1][2]};
    //std::cout<< pts[1];
    //pts[2] = {mvp[2][0],mvp[2][1],mvp[2][2]};
   // std::cout<<pts[0];
   
    //pts[0][0]=afterMVP[0][0]/afterMVP[0][3];
    //pts[0][1]=afterMVP[0][1];
    //pts[0][2]=afterMVP[0][0]/afterMVP[0][3];

    //afterMVP[0][1]/afterMVP[0][3],afterMVP[0][2]/afterMVP[0][3]};
    //pts[1] = {afterMVP[1][0]/afterMVP[1][3],afterMVP[1][1]/afterMVP[1][3],afterMVP[1][2]/afterMVP[1][3]};
    //pts[2] = {afterMVP[2][0]/afterMVP[2][3],afterMVP[2][1]/afterMVP[2][3],afterMVP[2][2]/afterMVP[2][3]};
    
    int min_X = std::floor(std::min(pts[0][0],std::min(pts[1][0],pts[2][0])));
    int min_Y = std::floor(std::min(pts[0][1],std::min(pts[1][1],pts[2][1])));
    int max_X = std::ceil(std::max(pts[0][0],std::max(pts[1][0],pts[2][0])));
    int max_Y = std::ceil(std::max(pts[0][1],std::max(pts[1][1],pts[2][1])));
    Vec3i P; 
    Vec3f uv; 
    for(int x = min_X; x<max_X+1;x++){
       for(int y = min_Y; y<max_Y+1;y++){
            P.x = x;
            P.y = y;
            Vec3f bcentric  = barycentric(pts, P); 
            if (bcentric.x<0 || bcentric.y<0 || bcentric.z<0) continue; 
            P.z = bcentric.x*pts[0].z+bcentric.y*pts[1].z+bcentric.z*pts[2].z;
            uv.x = bcentric.x*text_coords[0].x+bcentric.y*text_coords[1].x+bcentric.z*text_coords[2].x;
            uv.y = bcentric.x*text_coords[0].y+bcentric.y*text_coords[1].y+bcentric.z*text_coords[2].y;
            
            if(zbuffer[int(P.x+P.y*width)]<P.z){
                zbuffer[int(P.x+P.y*width)]=P.z;
                color = textrueImage.get(uv.x*1024,uv.y*1024);
                image.set(P.x, P.y, color); 
            }
        } 
    } 
} 

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage textrueImage(1024, 1024, TGAImage::RGB);;
    model->load_texture("obj/african_head_diffuse.tga",textrueImage);
    Vec3f light_dir(0,0,-1);
    float zbuffer[width*height];
    for (int i=width*height;i>0; i--){
        zbuffer[i] = -std::numeric_limits<float>::max();
    };
    vec3 eye_pos = {0, 0, 5};
    float angle = 30.0;
    mvpTransform transformMVP;
    transformMVP.set_model(angle);
    transformMVP.set_view(eye_pos);
    transformMVP.set_projection(45, 1, 0.1, 50);
    mat<4,4> mvp = transformMVP.get_mvp_Mat();
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> faceUv = model->faceUv(i);
        Vec3f screen_coords[3];
        Vec2f text_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            Vec2f uv = model->text_coord(faceUv[j]);
            screen_coords[j] = Vec3f((v.x+1.)*width/2., (v.y+1.)*height/2.,v.z);
            text_coords[j] = Vec2f(uv.x, uv.y);
            world_coords[j]  = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            triangle(mvp,text_coords,zbuffer,screen_coords,image,textrueImage, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}