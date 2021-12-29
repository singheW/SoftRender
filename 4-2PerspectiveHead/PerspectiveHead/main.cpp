#include <vector>
#include <cmath>
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
Vec3f cross(Vec3f  v1, Vec3f  v2)//传入两个三维向量
{
	return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };//叉积公式
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

void triangle(Vec3i* pts, float* zbuffer, TGAImage& image, TGAColor color, TGAImage& diffuse, Vec2f* uv) {
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
			float indx = uv[0][0] * bc_screen.x + uv[1][0] * bc_screen.y + uv[2][0] * bc_screen.z;//xy与uv中的三维对应
			float indy = uv[0][1] * bc_screen.x + uv[1][1] * bc_screen.y + uv[2][1] * bc_screen.z;
			indy = 1 - indy;//翻转y轴
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];

			if (zbuffer[int(P.x + P.y * width)] < P.z) {
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y, diffuse.get(int(indx * 1024), int(indy * 1024)));
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
	Vec3f light_dir(0, 0, -1);//假设光是垂直屏幕的
	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	Matrix Projection = Matrix::identity(4);
	Matrix Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	Projection[3][2] = -1.f / camera.z;

	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		std::vector<int> fuv = model->faceuv(i);//faceuv//faceuvs_;//fuv;//UV索引坐标
		Vec3i pts[3];
		Vec2f uv_coords[3];
		Vec3f world_coords[3];
		for (int i = 0; i < 3; i++) {
			Vec3f v = model->vert(face[i]);
			world_coords[i] = v;
			pts[i] = m2v(Viewport * Projection * v2m(v));//world2screen(model->vert(face[i]));////世界坐标系：升维，投影变换，视窗变换，降维：视窗坐标系
		}
		for (int i = 0; i < 3; i++) {
			Vec2f v = model->uv(fuv[i]);
			uv_coords[i] = v;
		}
		Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
		n.normalize();
		float intensity = n * light_dir;

		triangle(pts, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255), diffuse, uv_coords);
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
//三维z-buffer实现
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
	Vec3f light_dir(0, 0, -1);//假设光是垂直屏幕的
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


//二维z-buffer实现
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
	for (int x = p0.x; x <= p1.x; x++) {//通过将p0.x和p1.x之间的所有x坐标进行重排，计算该段的相应y坐标
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y * (1. - t) + p1.y * t + .5;//采用了一种插值或重心坐标方法来遍历y轴坐标
		if (ybuffer[x] < y) {//如果当前的y值比ybuffer中的值更接近相机，那么在屏幕上绘制它并更更新ybuffer。
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
		TGAImage render(width, 16, TGAImage::RGB);//创建16像素高的图像

		int ybuffer[width];
		for (int i = 0; i < width; i++) {
			ybuffer[i] = std::numeric_limits<int>::min(); //numeric_limits数值极限;int的最小值;min返回可取的最小值（规范化）
		}
		rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
		rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
		rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);

		for (int i = 0; i < width; i++) {//1像素看不清，把它纵向拉满
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