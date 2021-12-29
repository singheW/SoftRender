#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec2f>  vts_;//"vt "//��������
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> vns_;//������Ա�������Լ���cppʹ��
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > faceuvs_;//fuv;//����UV��������
	std::vector<std::vector<int> > vnfaces_;
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f vt(int i);
	Vec3f vn(int i);//����normal��main����
	std::vector<int> face(int idx);
	std::vector<int> faceuv(int idx);
	std::vector<int> vnfaces(int idx);//����normal����
};

#endif //__MODEL_H__
