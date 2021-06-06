#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> text_coords_;
    std::vector<std::vector<Vec3i> > faces_; // attention, this Vec3i means vertex/uv/normal
	std::vector<Vec3f> norms_;
	TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f vert(int iface, int nthvert);
	Vec2f uv(int iface, int nthvert);
    TGAColor diffuse(Vec2f uv);
	std::vector<int> face(int idx);
	std::vector<int> faceUv(int iduv);
	void load_texture(std::string filename, const char *suffix, TGAImage &img);
	Vec3f normal(int iface, int nthvert);
	float specular(Vec2f uvf);
	Vec3f normal(Vec2f uvf);//get a normal information from a tgaimage
};

#endif //__MODEL_H__
