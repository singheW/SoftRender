#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), vns_(),vts_(), faceuvs_(){
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
            Vec3f v;//v: ��ʾ���㣬�����ͼ�ĵ㣬����������8�����㣬ÿ��������x,y,z����ֵ��
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            vns_.push_back(n);
        }
        //��"vt u v����ʼ�ģ����������������顣"f x/x/x x/x/x x/x/x"�м��x����������ε��������꣬��������������ڣ���������ͼ��Ŀ�Ⱥ͸߶ȣ����õ�����Ҫ�ŵ���Ⱦ���е���ɫ��
        else if(!line.compare(0,3,"vt ")) {
            iss >> trash >> trash;
            Vec2f v;
            for (int i = 0; i < 2; i++) iss >> v[i];
            vts_.push_back(v);
        }
        //f 1/2/3 11/22/33 111/222/333 ��1/2/3Ϊ����1��Ӧ����������2����������uv������3�Ƿ�����������Ҳ����˵���������������ǵ�1,11,111����Ӧ��x��y��z��
        else if (!line.compare(0, 2, "f ")) {//��ʾһ���棬������v/vt/vn��������ʽ��ɡ�
            std::vector<int> f;//����f 5/15/7 4/14/6 6/16/8 ����ʾ��546�������һ������,ƽ�������ɵ�15 14 16�����������γɣ����ƽ��ĳ����ǵ�768������ķ�������ƽ��ֵ��
            std::vector<int> fuv;//����UV��������
            std::vector<int> n;
            int itrash, idx, uvidx,nidx;
            iss >> trash;
            //�˴���ѭ����Ҫ��ȡ����x / x/ x �еĵ�һ�����֣�������ά���������ݣ�����ͨ������while��������ÿ�λ�ȡidx����
            while (iss >> idx >> trash >> uvidx >> trash >> nidx) {//ͨ���ı�һ����˳�򼴿ɻ�ȡ����λ�õ����֣�����ڶ���x����������Ҫ�Ĳ����ļ����ص�vt����
                idx--; // in wavefront obj all indices start at 1, not zero
                uvidx--;
                nidx--;
                f.push_back(idx);
                fuv.push_back(uvidx);
                n.push_back(nidx);
            }
            faces_.push_back(f); //��ʾ��546�������һ�����棬��435�������
            faceuvs_.push_back(fuv);
            vnfaces_.push_back(n);

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

Vec2f Model::vt(int i) {
    return vts_[i];
}

std::vector<int> Model::vnfaces(int idx) {//����normal��face����
    return vnfaces_[idx];
}

Vec3f Model::vn(int i) {
    return vns_[i];
}

