#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

Model* model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;
Vec3f camera(0, 0, 3);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f light_dir(0, 0, 1);//������Ǵ�ֱ��Ļ��
Vec3f m2v(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

Vec3f cross(Vec3f  v1, Vec3f  v2)//����������ά����
{
	return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };//�����ʽ
}

Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity(4);
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up ,z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix res = Matrix::identity(4);
	for (int i = 0; i < 3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		res[i][3] = -center[i];
	}
	return res;
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

void triangle(Vec3i* pts, float* zbuffer, TGAImage& image, TGAImage& diffuse, Vec2f* uv,Vec3f* norms) {
	Vec2i bboxmin(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	Vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;

	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			float indx = uv[0][0] * bc_screen.x + uv[1][0] * bc_screen.y + uv[2][0] * bc_screen.z;//xy��uv�е���ά��Ӧ
			float indy = uv[0][1] * bc_screen.x + uv[1][1] * bc_screen.y + uv[2][1] * bc_screen.z;

			float norx = norms[0].x * bc_screen.x + norms[1].x * bc_screen.y + norms[2].x * bc_screen.z;
			float nory = norms[0].y * bc_screen.x + norms[1].y * bc_screen.y + norms[2].y * bc_screen.z;
			float norz = norms[0].z * bc_screen.x + norms[1].z * bc_screen.y + norms[2].z * bc_screen.z;

			Vec3f normal(norx, nory, norz);
			normal.normalize();
			float intensity = normal * light_dir;
			


			indy = 1 - indy;//��תy��
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];

			if (zbuffer[int(P.x + P.y * width)] < P.z) {
				zbuffer[int(P.x + P.y * width)] = P.z;
				//image.set(P.x, P.y, color);
				if (intensity < 0)continue;
				TGAColor vcolor = diffuse.get(int(indx * 1024), int(indy * 1024));
				vcolor.r *= intensity;
				vcolor.g *= intensity;
				vcolor.b *= intensity;
				vcolor.a *= 255;
				image.set(P.x, P.y, vcolor);
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
	TGAImage diffuse(1024, 1024, TGAImage::RGB);
	diffuse.read_tga_file("obj/african_head_diffuse.tga");

	TGAImage image(width, height, TGAImage::RGB);

	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
	Matrix Projection = Matrix::identity(4);
	Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	Projection[3][2] = -1.f / (eye - center).norm();// camera.z;



	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		std::vector<int> fuv = model->faceuv(i);//faceuv//faceuvs_;//fuv;//UV��������
		std::vector<int> norface = model->norface(i);
		Vec3i pts[3];
		Vec2f uv_coords[3];
		Vec3f world_coords[3];
		Vec3f norm_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			
			pts[j] = m2v(ViewPort * Projection* v2m(v));// *ModelView world2screen(model->vert(face[i]));////��������ϵ����ά��ͶӰ�任���Ӵ��任����ά���Ӵ�����ϵ
			world_coords[j] = v;

			Vec2f w = model->uv(fuv[j]);
			uv_coords[j] = w;

			Vec3f n = model->nor(norface[j]);
			norm_coords[j] = n;

		}
		//Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
		//n.normalize();
	//	float intensity = n * light_dir;
		
		triangle(pts, zbuffer, image, diffuse, uv_coords,norm_coords);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("african_head.tga");

	{ // dump z-buffer (debugging purposes only)
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("zbuffer.tga");
	}
	delete model;
	delete[]zbuffer;
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