#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/dog.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) { //遍历model中的每一个face
        std::vector<int> face = model->face(i);
        if (face.size() < 3)continue;
        for (int j=0; j<3; j++) {//将每一个面的3个顶点构成的3条线段进行绘制
            Vec3f v0 = model->vert(face[j]);//Vec3f 是一种存储三维浮点型数据的结构
            Vec3f v1 = model->vert(face[(j+1)%3]);//每个face内存储3个坐标，范围为-1~1且以远点为中心坐标，需进行位移
            int x0 = (v0.x+0.8)*width/1.5;
            int y0 = (v0.y+0.1)*height/1.5;
            int x1 = (v1.x+0.8)*width/ 1.5;
            int y1 = (v1.y+ 0.1)*height/ 1.5;
            line(x0, y0, x1, y1, image, white);
        }
    }

    image.flip_vertically(); 
    image.write_tga_file("dogdog.tga");
    delete model;
    return 0;
}

/*
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = y0 * (1. - t) + y1 * t;
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; j++) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j + 1) % 3]);
            int x0 = (v0.x + 1.) * width / 2.;
            int y0 = (v0.y + 1.) * height / 2.;
            int x1 = (v1.x + 1.) * width / 2.;
            int y1 = (v1.y + 1.) * height / 2.;
            line(x0, y0, x1, y1, image, white);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}


*/
/*
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
bool steep = false;
if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//绝对值如果高差大于宽差，调换图片
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
}
if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
}
int dx = x1 - x0;
int dy = y1 - y0;
float derror2 = std::abs(dy / float(dx));//浮动点：除以dx和循环体中的.5进行比较。可以通过将错误变量替换为另一个误差来拜托浮动点
float error2 = 0;
int y = y0;
for (int x = x0; x <= x1; x++) {

    if (steep) {
        image.set(y, x, color);
    }
    else {
        image.set(x, y, color);
    }
    error2 += derror2;
    if (error2 > .5) {
        y += (y1 > y0 ? 1 : -1);//减少误差
        error2 -= dx*2;//为啥是减1？
    }

}






void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
bool steep = false;
if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//绝对值如果高差大于宽差，调换图片
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
}
if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
}
int dx = x1 - x0;
int dy = y1 - y0;
float derror = std::abs(dy / float(dx));
float error = 0;
int y = y0;
for (int x = x0; x <= x1; x++) {

    if (steep) {
        image.set(y, x, color);
    }
    else {
        image.set(x, y, color);
    }
    error += derror;
    if (error > .5) {
        y += (y1 > y0 ? 1 : -1);//减少误差
        error -= 1.;//为啥是减1？
    }

}


void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
bool steep = false;
if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//绝对值如果高差大于宽差，调换图片
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
}
if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
}
for (int x = x0; x <= x1; x++) {
    float t = (x - x0) / (float)(x1 - x0);
    int y = y0 * (1. - t) + y1 * t;
    if (steep) {
        image.set(y, x, color);
    }
    else {
        image.set(x, y, color);
    }

}

*/

/*
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
for (int x = x0; x <= x1; x++) {
    float t = (x - x0) / (float)(x1 - x0);
    int y = y0 * (1. - t) + y1 * t;
    image.set(x, y, color);
}

for (float t=0.; t<1.; t+=.1) {
    int x = x0*(1.-t) + x1*t;
    int y = y0*(1.-t) + y1*t;
    image.set(x, y, color);
}

}

int main(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);
    line(13, 20, 80, 40, image, white);
    line(20, 13, 40, 80, image, red);
    line(80, 40, 13, 20, image, red);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}

 */