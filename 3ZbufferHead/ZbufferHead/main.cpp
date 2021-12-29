#include<vector>
#include<iostream>
#include<cmath>
#include<cstdlib>
#include<limits>
#include "tgaimage.h"
#include"geometry.h"
#include"model.h"
using std::min;
using std::max;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 4000;
const int height = 4000;
Model *model = NULL;



Vec3f cross(Vec3f v1, Vec3f v2) {
	return { v1.y * v2.z - v1.z * v2.y,v1.z * v2.x - v1.x * v2.z,v1.x * v2.y - v1.y * v2.x };//�����ʽ
}
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2) 
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); 
}


void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
			if (zbuffer[int(P.x + P.y * width)] < P.z) {
				zbuffer[int(P.x + P.y * width)] = P.z;
		     	image.set(P.x, P.y, color);
			}
		}
	}
}

Vec3f world2screen(Vec3f v) {
	return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}


int main(int argc, char** argv) {
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head.obj");
	}
	TGAImage image(width, height, TGAImage::RGB);
	Vec3f light_dir(0, 0, -1);//������Ǵ�ֱ��Ļ��
	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());


	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec3f pts[3];
		Vec3f world_coords[3];
		for (int i = 0; i < 3; i++) {
			Vec3f v = model->vert(face[i]);
			world_coords[i] = v;
			pts[i] = world2screen(model->vert(face[i]));
		}
		Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
		n.normalize();
		float intensity = n * light_dir;
		triangle(pts, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
	}

	image.flip_vertically(); 
	image.write_tga_file("african_head.tga");
	delete model;
	return 0;
}
/*
//��άz-bufferʵ��
#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	for (int i=2; i--; ) {
		s[i][0] = C[i]-A[i];
		s[i][1] = B[i]-A[i];
		s[i][2] = A[i]-P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
	return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
	Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width()-1, image.get_height()-1);
	for (int i=0; i<3; i++) {
		for (int j=0; j<2; j++) {
			bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
		for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
			Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
			if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
			P.z = 0;
			for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
			if (zbuffer[int(P.x+P.y*width)]<P.z) {
				zbuffer[int(P.x+P.y*width)] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}

Vec3f world2screen(Vec3f v) {
	return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}


int main(int argc, char** argv) {
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/dog.obj");
	}

	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());
	Vec3f light_dir(0, 0, -1);//������Ǵ�ֱ��Ļ��
	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec3f pts[3];
		Vec3f world_coords[3];
		for (int i = 0; i < 3; i++) {
			Vec3f v = model->vert(face[i]);
			world_coords[i] = v;
			pts[i] = world2screen(model->vert(face[i]));
		}
		Vec3f n = cross((world_coords[2] - world_coords[0]) , (world_coords[1] - world_coords[0]));
		n.normalize();
		float intensity = n * light_dir;
		triangle(pts, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("dog.tga");
	delete model;
	return 0;
}


//��άz-bufferʵ��
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800;
const int height = 500;

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color) {
	bool steep = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}
	if (p0.x > p1.x) {
		std::swap(p0, p1);
	}
	for (int x = p0.x; x <= p1.x; x++) {
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y * (1.-t) + p1.y*t + .5 ;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}
void rasterize(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color, int ybuffer[]) {//
	if (p0.x > p1.x) {
		std::swap(p0, p1);
	}
	for (int x = p0.x; x <= p1.x; x++) {//ͨ����p0.x��p1.x֮�������x����������ţ�����öε���Ӧy����
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y * (1. - t) + p1.y * t + .5;//������һ�ֲ�ֵ���������귽��������y������
		if (ybuffer[x] < y) {//�����ǰ��yֵ��ybuffer�е�ֵ���ӽ��������ô����Ļ�ϻ�������������ybuffer��
			ybuffer[x] = y;
			image.set(x, 0, color);
		}
	}
}

int main(int argc, char** argv) {
	{
		TGAImage image(width, height, TGAImage::RGB);
		line(Vec2i(20, 34), Vec2i(744, 400), image, red);
		line(Vec2i(120, 434), Vec2i(444, 400), image, green);
		line(Vec2i(330, 463), Vec2i(594, 200), image, blue);
		line(Vec2i(10, 10), Vec2i(790, 10), image, red);

		image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		image.write_tga_file("image.tga");

	}
	{
		TGAImage render(width, 16, TGAImage::RGB);//����16���ظߵ�ͼ��

		int ybuffer[width];
		for (int i = 0; i < width; i++) {
			ybuffer[i] = std::numeric_limits<int>::min(); //numeric_limits��ֵ����;int����Сֵ;min���ؿ�ȡ����Сֵ���淶����
		}
		rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
		rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
		rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);

		for (int i = 0; i < width; i++) {//1���ؿ����壬������������
			for (int j = 1; j < 16; j++) {
				render.set(i, j, render.get(i, 0));
			}
		}
		render.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		render.write_tga_file("render.tga");

	}
	return 0;
}



*/