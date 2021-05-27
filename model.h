#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<Vec2f> text_coords_;
	std::vector<std::vector<int> > facesUv_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f text_coord(int i);
	std::vector<int> face(int idx);
	std::vector<int> faceUv(int iduv);
	void load_texture(std::string filename, TGAImage &img);
};

#endif //__MODEL_H__
