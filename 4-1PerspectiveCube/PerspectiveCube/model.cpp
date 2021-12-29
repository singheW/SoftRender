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
        if (!line.compare(0, 2, "v ")) {//v: ��ʾ���㣬�����ͼ�ĵ㣬����������8�����㣬ÿ��������x,y,z����ֵ��
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {//��ʾһ���棬������v/vt/vn��������ʽ��ɡ�
            std::vector<int> f;//����f 5/15/7 4/14/6 6/16/8 ����ʾ��546�������һ������,ƽ�������ɵ�15 14 16�����������γɣ����ƽ��ĳ����ǵ�768������ķ�������ƽ��ֵ��
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero 
                f.push_back(idx);
            }
            faces_.push_back(f); //��ʾ��546�������һ�����棬��435�������
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

