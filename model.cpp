#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> fuv;
            int itrash, idx,iduv;
            iss >> trash;
            while (iss >> idx >> trash >> iduv >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
                iduv--;
                fuv.push_back(iduv);
            }
            faces_.push_back(f);
            facesUv_.push_back(fuv);
        }else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;//why error when only one trash
            Vec2f vt;
            for (int i=0;i<2;i++) iss >> vt[i];
            text_coords_.push_back(vt);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}
Vec2f Model::text_coord(int i) {
    return text_coords_[i];
}

std::vector<int> Model::faceUv(int iduv) {
    return facesUv_[iduv];
}
void Model::load_texture(std::string filename,  TGAImage &img) {
    //std::cerr << "texture file " << filename << " loading " << (img.read_tga_file(filename.c_str()) ? "ok" : "failed") << std::endl;
    img.read_tga_file(filename.c_str());
    img.flip_vertically();
}