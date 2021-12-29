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
        std::istringstream iss(line.c_str());//使用输入控制流来控制读取，C_str提取首字母地址
        char trash;
        if (!line.compare(0, 2, "v ")) {//compare函数比较从0开始的2个字符，若等于之后字符串则返回0，故表达式表示该行字符串以V开头即为true
            iss >> trash;//读取的是字符，此处将V内容跳过，读取后续信息，对不同类型数据有不同的重载
            Vec3f v;//v: 表示顶点，即组成图的点，如立方体有8个顶点，每个顶点有x,y,z三个值。
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            vns_.push_back(n);
        }
        //以"vt u v”开始的，这是纹理坐标数组。"f x/x/x x/x/x x/x/x"中间的x是这个三角形的纹理坐标，将其插入三角形内，乘以纹理图像的宽度和高度，你会得到你需要放到渲染器中的颜色。
        else if(!line.compare(0,3,"vt ")) {
            iss >> trash >> trash;
            Vec2f v;
            for (int i = 0; i < 2; i++) iss >> v[i];
            vts_.push_back(v);
        }
        //f 1/2/3 11/22/33 111/222/333 以1/2/3为例，1对应顶点索引，2是纹理坐标uv索引，3是法向量索引。也就是说这个面的三个顶点是第1,11,111所对应的x，y，z。
        else if (!line.compare(0, 2, "f ")) {//表示一个面，由三个v/vt/vn的索引形式组成。
            std::vector<int> f;//比如f 5/15/7 4/14/6 6/16/8 ，表示第546顶点组成一三角面,平面纹理由第15 14 16三纹理坐标形成，这个平面的朝向是第768个三点的法向量求平均值。
            std::vector<int> fuv;//纹理UV索引坐标
            std::vector<int> n;
            int itrash, idx, uvidx,nidx;
            iss >> trash;
            //此处的循环需要读取的是x / x/ x 中的第一个数字，代表三维坐标轴数据，所以通过下述while条件即可每次获取idx内容
            while (iss >> idx >> trash >> uvidx >> trash >> nidx) {//通过改变一定的顺序即可获取其他位置的数字，比如第二个x就是我们需要的材质文件像素点vt坐标
                idx--; // in wavefront obj all indices start at 1, not zero
                uvidx--;
                nidx--;
                f.push_back(idx);
                fuv.push_back(uvidx);
                n.push_back(nidx);
            }
            faces_.push_back(f); //表示第546顶点组成一三角面，即435顶点组成
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

std::vector<int> Model::vnfaces(int idx) {//加入normal的face数组
    return vnfaces_[idx];
}

Vec3f Model::vn(int i) {
    return vns_[i];
}

