#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec2f>  uvs_;//"vt "//纹理坐标
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > faceuvs_;//fuv;//纹理UV索引坐标
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f uv(int i);
	std::vector<int> face(int idx);
	std::vector<int> faceuv(int idx);
};

#endif //__MODEL_H__
