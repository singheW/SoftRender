#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#define M_PI       3.14159265358979323846   // pi

const TGAColor white  = TGAColor(255, 255, 255, 255);
const TGAColor red    = TGAColor(255, 0,   0,   255);
const TGAColor green  = TGAColor(0,   255, 0,   255);
const TGAColor blue   = TGAColor(0,   0,   255, 255);
const TGAColor yellow = TGAColor(255, 255, 0,   255);

Model *model = NULL;
const int width  = 1000;
const int height = 1000;
const int depth  = 255;

void line(Vec3i p0, Vec3i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }

    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t+.5;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

Vec3f m2v(Matrix m) {//��ά����ά
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

Matrix v2m(Vec3f v) {//��ά����ά
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Matrix viewport(int x, int y, int w, int h) {//�ӿ����ź�ƫ��
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    
    return m;
}

Matrix translation(Vec3f v) {//ƫ��
    Matrix Tr = Matrix::identity(4);
    Tr[0][3] = v.x;
    Tr[1][3] = v.y;
    Tr[2][3] = v.z;
    return Tr;
}

Matrix zoom(float factor) {//����
    Matrix Z = Matrix::identity(4);
    Z[0][0] = Z[1][1] = Z[2][2] = factor;
    return Z;
}

Matrix rotation_x(float cosangle, float sinangle) {//x����ת
    Matrix R = Matrix::identity(4);
    R[1][1] = R[2][2] = cosangle;
    R[1][2] = -sinangle;
    R[2][1] =  sinangle;
    return R;
}

Matrix rotation_y(float cosangle, float sinangle) {//y����ת
    Matrix R = Matrix::identity(4);
    R[0][0] = R[2][2] = cosangle;
    R[0][2] =  sinangle;
    R[2][0] = -sinangle;
    return R;
}

Matrix rotation_z(float cosangle, float sinangle) {//z����ת
    Matrix R = Matrix::identity(4);
    R[0][0] = R[1][1] = cosangle;
    R[0][1] = -sinangle;
    R[1][0] =  sinangle;
    return R;
}



int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/cube.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    Matrix VP = viewport(width/4, width/4, width/2, height/2);

    { // draw the axes����w/2h����oΪ����xy�ϵ��߶�
        Vec3f x(1.f, 0.f, 0.f), y(0.f, 1.f, 0.f), o(0.f, 0.f, 0.f),cz(0.f,-2.f,0.f), czy(0.f, 2.f, 0.f);
        o = m2v(VP*v2m(o));
        x = m2v(VP*v2m(x));
        y = m2v(VP*v2m(y));
        cz = m2v(VP * v2m(cz));
        czy = m2v(VP * v2m(czy));
        line(o, x, image, red);//����w/2h����oΪ����xy�ϵĺ�ɫ�߶�
        line(o, y, image, green);//����w/2h����oΪ����xy�ϵ���ɫ�߶�
        line(czy, cz, image,white);
    }


    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j=0; j<(int)face.size(); j++) {
            Vec3f wp0 = model->vert(face[j]);
            Vec3f wp1 = model->vert(face[(j+1) % face.size()]);

            Vec3f C(5, 0, 0);
            { // draw the original model
                Matrix T = zoom(0.3);
                Vec3f sp0 = m2v(VP*T*v2m(wp0));//һ����������������ӿ��ϵ�λ��
                Vec3f sp1 = m2v(VP*T*v2m(wp1));
                Vec3f c1 = m2v(VP *T* v2m(C));
               // line(sp0, sp1, image, white);
                line(sp0, c1, image, yellow);
            }
            /*
            { // draw the deformed���� model
 //               Matrix T = zoom(1.5);
                  Matrix T = Matrix::identity(4);
                  T[0][1] = 0.333;//��x�����1/3
//                Matrix T = translation(Vec3f(.33, .5, 0))*rotation_z(cos(10.*M_PI/180.), sin(10.*M_PI/180.));
                Vec3f sp0 = m2v(VP*T*v2m(wp0));
                Vec3f sp1 = m2v(VP*T*v2m(wp1));
                line(sp0, sp1, image, yellow);//�Ŵ�1.5��
            }*/
            { // draw the deformed���� model
              Matrix T = zoom(0.3);
 //               Matrix T = Matrix::identity(4);
               Matrix D = Matrix::identity(4);
//                T[0][1] = 0.333;//��x�����1/3
//                Matrix T = translation(Vec3f(.33, .5, 0))*rotation_z(cos(10.*M_PI/180.), sin(10.*M_PI/180.));
               D[0][0] = 1;
               D[1][1] = 1;
               D[2][2] = 1;
               D[3][0] = -0.2;
                Vec3f sp0 = m2v(VP * D *T* v2m(wp0));
                Vec3f sp1 = m2v(VP * D *T* v2m(wp1));
                line(sp0, sp1, image, red);
            }

        }
        break;
    }


    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("CubeProjection.tga");
    delete model;
    return 0;
}

