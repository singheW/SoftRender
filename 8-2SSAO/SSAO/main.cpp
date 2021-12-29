#include <vector>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model* model = NULL;

const int width = 800;
const int height = 800;

Vec3f       eye(1.2, -.8, 3);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

struct ZShader : public IShader {
	mat<4, 3, float> varying_tri;

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, gl_Vertex);
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor& color) {
		color = TGAColor(0, 0, 0);//这种方法的着色器仅记录图形的z-buffer即可，不需要此时片段着色。
		return false;
	}
};
//对于每一条射线，都要计算足够远的像素距离，避免没发现的遮挡物。遍历该方向dir上的像素点cur，找出cur与出发点p线段的z轴最大斜率maxangle并返回，作为该方向的最大角度。
float max_elevation_angle(float* zbuffer, Vec2f p, Vec2f dir) {
	float maxangle = 0;
	for (float t = 0.; t < 1000.; t += 1.) {
		Vec2f cur = p + dir * t;
		if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0) return maxangle;

		float distance = (p - cur).norm();
		if (distance < 1.f) continue;
		float elevation = zbuffer[int(cur.x) + int(cur.y) * width] - zbuffer[int(p.x) + int(p.y) * width];
		maxangle = std::max(maxangle, atanf(elevation / distance));
	}
	return maxangle;
}
//屏幕空间环境光遮蔽算法会计算像素点周边的z轴坐标，计算从像素点射向周边的线段斜率，从而可以得知当前像素点相对于周围像素的z轴信息，从而叠加计算当前像素点环境光信息。
int main(int argc, char** argv) {
	if (2 == argc) {
		model = new Model(argv[1]);//读取模型
	}
	else {
		model = new Model("obj/diablo3_pose.obj");
	}
	float* zbuffer = new float[width * height];


	TGAImage frame(width, height, TGAImage::RGB);
	lookat(eye, center, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.f / (eye - center).norm());
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	ZShader zshader;
	for (int i = 0; i < model->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			zshader.vertex(i, j);
		}
		triangle(zshader.varying_tri, zshader, frame, zbuffer);
	}

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (zbuffer[x + y * width] < -1e5) continue;
			float total = 0;
			for (float a = 0; a < M_PI * 2 - 1e-4; a += M_PI / 4) {
				total += M_PI / 2 - max_elevation_angle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));//max_elevation_angle给出我们投射的每条射线遇到的最大斜率
			}
			total /= (M_PI / 2) * 8;//后处理：对于图像的每个像素，我会在像素周围的所有方向上发出大量光线（此处为8个）。
			total = 1 - pow(total, 100.f);
			frame.set(x, y, TGAColor(total * 255, total * 255, total * 255));
		}
	}





	frame.flip_vertically();
	frame.write_tga_file("framebuffer.tga");

	delete[] zbuffer;
	delete model;

	return 0;
}