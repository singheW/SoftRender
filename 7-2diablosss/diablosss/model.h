#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vec3f> verts_;//v
    std::vector<std::vector<Vec3i> > faces_; //表示一个面，由三个v/vt/vn的索引形式组成。
    std::vector<Vec3f> norms_;//vn
    std::vector<Vec2f> uv_;//vt
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    TGAImage glowmap_;
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f normal(int iface, int nthvert);
    Vec3f normal(Vec2f uv);
    Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);
    Vec2f uv(int iface, int nthvert);
    TGAColor diffuse(Vec2f uv);
    float specular(Vec2f uv);
    float grow(Vec2f uv);
    std::vector<int> face(int idx);
};
#endif //__MODEL_H__

