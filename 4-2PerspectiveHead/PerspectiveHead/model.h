#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec2f>  uvs_;//"vt "//��������
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> norms_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > faceuvs_;//fuv;//����UV��������
	std::vector<std::vector<int> > norfaces_;
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f uv(int i);
	Vec3f nor(int i);//����normal
	std::vector<int> face(int idx);
	std::vector<int> faceuv(int idx);
	std::vector<int> norface(int idx);//����normal����
};

#endif //__MODEL_H__
