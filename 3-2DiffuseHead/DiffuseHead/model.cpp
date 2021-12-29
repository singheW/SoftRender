#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), uvs_(), faceuvs_(){
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());//ʹ����������������ƶ�ȡ��C_str��ȡ����ĸ��ַ
        char trash;
        if (!line.compare(0, 2, "v ")) {//compare�����Ƚϴ�0��ʼ��2���ַ���������֮���ַ����򷵻�0���ʱ��ʽ��ʾ�����ַ�����V��ͷ��Ϊtrue
            iss >> trash;//��ȡ�����ַ����˴���V������������ȡ������Ϣ���Բ�ͬ���������в�ͬ������
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if(!line.compare(0,3,"vt ")) {//��������
            iss >> trash >> trash;
            Vec2f v;
            for (int i = 0; i < 2; i++) iss >> v[i];
            uvs_.push_back(v);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;//
            std::vector<int> fuv;//����UV��������
            int itrash, idx, uvidx;
            iss >> trash;
            //�˴���ѭ����Ҫ��ȡ����x / x/ x �еĵ�һ�����֣�������ά���������ݣ�����ͨ������while��������ÿ�λ�ȡidx����
            while (iss >> idx >> trash >> uvidx >> trash >> itrash) {//ͨ���ı�һ����˳�򼴿ɻ�ȡ����λ�õ����֣�����ڶ���x����������Ҫ�Ĳ����ļ����ص�vt����
                idx--; // in wavefront obj all indices start at 1, not zero
                uvidx--;
                f.push_back(idx);
                fuv.push_back(uvidx);
            }
            faces_.push_back(f);
            faceuvs_.push_back(fuv);
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

std::vector<int> Model::faceuv(int idx) {
    return faceuvs_[idx];
}

Vec2f Model::uv(int i) {
    return uvs_[i];
}
