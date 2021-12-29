#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

  const int width = 200;
  const int height = 200;
  Model* model = NULL;

  Vec3f cross(Vec3f  v1, Vec3f  v2)//传入两个三维向量
  {
	  return {v1.y* v2.z - v1.z * v2.y, v1.z* v2.x - v1.x * v2.z, v1.x* v2.y - v1.y * v2.x};//叉积公式
  }

  
  Vec3f barycentric(Vec2i *pts, Vec2i G) {//给定一个2D三角形和一个点G，目标是计算重心坐标点G
//Vec3 n=cross(Vec3(i),Vec3(j))。其中i为（ABx,ACx,PAx），j为（ABy,ACy,PAy); n为垂直于ij两向量的法向量，n的值为（kβ,kγ,k),α=1-β-γ；点G位置：G=αA+βB+γC。（ABC为pts指向的3个点）
	  Vec3f u = cross(Vec3f(pts[2].u - pts[0].u, pts[1].u - pts[0].u, pts[0].u - G.u), Vec3f(pts[2].v - pts[0].v, pts[1].v - pts[0].v, pts[0].v - G.v));
	 //result 除以自身Z之前，其Z值可能为负数。
	  if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);//Z是int型，如果绝对值小于1，说明它等于0，此时返回带负号的重心坐标。
	  return Vec3f(1.f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);//除以Z值以恢复我们要的n:即u(β，γ，1)=(u.x/u.z,u.y/u.z,u.z/u.z).则α=1-(u.x+u.y)/u.z,即return(α,β,γ)
  }
//包围盒算法，自下而上，自左到右
//三角形（A，B，C）重心坐标P(x,y)=αA+βB+γC；α+β+γ=1；
//α=▲PBC/▲ABC；β=▲PAC/▲ABC；γ=▲PAB/▲ABC
//若计算的三个系数αβγ都≥0，则证明该点再三角形内部。
//包围盒思想：比较三个点的xy坐标，找出最小的x,y和最大的x,y作为左下角和右上角坐标，然后盒中遍历像素点
void triangle(Vec2i *pts,  TGAImage &image, TGAColor color) {
	//定义包围盒
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	//图片的边界
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	//查找包围盒边界
	for (int i = 0; i < 3; i++) {//遍历三角形的3个顶点,进行循环比较找到小的x,y和最大的x,y作为左下角和右上角坐标
		bboxmin.u = std::min(bboxmin.u, pts[i].u);
		bboxmin.v = std::min(bboxmin.v, pts[i].v);
		bboxmax.u = std::max(bboxmax.u, pts[i].u);
		bboxmax.v = std::max(bboxmax.v, pts[i].v);
	}
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {//遍历包围盒中的每一个点
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)continue;//回忆重心坐标判定点的方法。如果重心坐标不在三角形内，则α,β,γ至少有一个为负
			image.set(P.x, P.y, color);//此点着色
		}
	}
}




int main(int argc, char** argv) {
	//首先获取随机颜色填充人脸模型，采用了重心坐标方法实现。
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);

	//用一个模拟光照对三角形进行着色
	Vec3f light_dir(0, 0, -1);//假设光是垂直屏幕的

	for (int i = 0; i < model->nfaces(); i++) { //遍历model中的每一个face
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];//屏幕坐标，由世界坐标转换而来、
		Vec3f world_coord[3];
		for (int j = 0; j < 3; j++) {//将每一个面的3个顶点构成的3条线段进行绘制
			Vec3f v = model->vert(face[j]);
			//	Vec3f world_coords = model->vert(face[j]);//Vec3f 是一种存储三维浮点型数据的结构
			screen_coords[j] = Vec2i((v.x + 0.6) * width / 1.2, (v.y + 0.05) * height / 1.2);  // 投影为正交投影，且将世界坐标转换为屏幕坐标
			world_coord[j] = v;
		}
		//计算世界坐标中某个三角形的法线（法线 = 三角形任意两条边做叉乘）即法向量需用三角形顶点的世界坐标来计算
		Vec3f n = (world_coord[2] - world_coord[0]) ^ (world_coord[1] - world_coord[0]);
		n.normalize();// 对 n 做归一化处理
		// 三角形法线和光照方向做点乘，点乘值大于 0，说明法线方向和光照方向在同一侧，值越大，说明越多的光照射到三角形上，颜色越白
		float intensity = n * light_dir;
		//Back-face剔除
		if (intensity > 0) {
			triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//这是一个三角形
	//triangle(pts, image, TGAColor(255, 0, 0, 255));
	image.flip_vertically();
	image.write_tga_file("african_head.tga");
	return 0;

}




/*
* 
*

//给模型增加阴影
int main(int argc, char** argv) {
	//首先获取随机颜色填充人脸模型，采用了重心坐标方法实现。
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/dog.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);

	//用一个模拟光照对三角形进行着色
	Vec3f light_dir(0, 0, -1);//假设光是垂直屏幕的

	for (int i = 0; i < model->nfaces(); i++) { //遍历model中的每一个face
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];//屏幕坐标，由世界坐标转换而来、
		Vec3f world_coord[3];
		for (int j = 0; j < 3; j++) {//将每一个面的3个顶点构成的3条线段进行绘制
			Vec3f v = model->vert(face[j]);
			//	Vec3f world_coords = model->vert(face[j]);//Vec3f 是一种存储三维浮点型数据的结构
			screen_coords[j] = Vec2i((v.x + 0.6) * width / 1.2, (v.y + 0.05) * height / 1.2);  // 投影为正交投影，且将世界坐标转换为屏幕坐标
			world_coord[j] = v;
		}
		//计算世界坐标中某个三角形的法线（法线 = 三角形任意两条边做叉乘）即法向量需用三角形顶点的世界坐标来计算
		Vec3f n = (world_coord[2] - world_coord[0]) ^ (world_coord[1] - world_coord[0]);
		n.normalize();// 对 n 做归一化处理
		// 三角形法线和光照方向做点乘，点乘值大于 0，说明法线方向和光照方向在同一侧，值越大，说明越多的光照射到三角形上，颜色越白
		float intensity = n * light_dir;
		//Back-face剔除
		if (intensity > 0) {
			triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//这是一个三角形
	//triangle(pts, image, TGAColor(255, 0, 0, 255));
	image.flip_vertically();
	image.write_tga_file("frame.tga");
	return 0;

}





//给model的三角形上色

int main(int argc, char** argv) {
	//首先获取随机颜色填充人脸模型，采用了重心坐标方法实现。
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/dog.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model->nfaces(); i++) { //遍历model中的每一个face
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		for (int j = 0; j < 3; j++) {//将每一个面的3个顶点构成的3条线段进行绘制
			Vec3f world_coords = model->vert(face[j]);//Vec3f 是一种存储三维浮点型数据的结构
			screen_coords[j] = Vec2i((world_coords.x + 0.6) * width / 1.2, (world_coords.y + 0.05) * height / 1.2);//所有三角形进行重定，将世界坐标转换为屏幕坐标并绘制三角形
		}
		triangle(screen_coords, image, TGAColor(rand() % 255, rand() % 255, rand() % 255,  255));
	}
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//这是一个三角形
	//triangle(pts, image, TGAColor(255, 0, 0, 255));
	image.flip_vertically();
	image.write_tga_file("frame.tga");
	return 0;

}

* 
* 
* 
* 
//重心坐标画三角形
#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
  const int width = 200;
  const int height = 200;

  Vec3f cross(Vec3f  v1, Vec3f  v2)//传入两个三维向量
  {
	  return {v1.y* v2.z - v1.z * v2.y, v1.z* v2.x - v1.x * v2.z, v1.x* v2.y - v1.y * v2.x};//叉积公式
  }


  Vec3f barycentric(Vec2i *pts, Vec2i G) {//给定一个2D三角形和一个点G，目标是计算重心坐标点G
//Vec3 n=cross(Vec3(i),Vec3(j))。其中i为（ABx,ACx,PAx），j为（ABy,ACy,PAy); n为垂直于ij两向量的法向量，n的值为（kβ,kγ,k),α=1-β-γ；点G位置：G=αA+βB+γC。（ABC为pts指向的3个点）
	  Vec3f u = cross(Vec3f(pts[2].u - pts[0].u, pts[1].u - pts[0].u, pts[0].u - G.u), Vec3f(pts[2].v - pts[0].v, pts[1].v - pts[0].v, pts[0].v - G.v));
	 //result 除以自身Z之前，其Z值可能为负数。
	  if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);//Z是int型，如果绝对值小于1，说明它等于0，此时返回带负号的重心坐标。
	  return Vec3f(1.f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);//除以Z值以恢复我们要的n:即u(β，γ，1)=(u.x/u.z,u.y/u.z,u.z/u.z).则α=1-(u.x+u.y)/u.z,即return(α,β,γ)
  }
//包围盒算法，自下而上，自左到右
//三角形（A，B，C）重心坐标P(x,y)=αA+βB+γC；α+β+γ=1；
//α=▲PBC/▲ABC；β=▲PAC/▲ABC；γ=▲PAB/▲ABC
//若计算的三个系数αβγ都≥0，则证明该点再三角形内部。
//包围盒思想：比较三个点的xy坐标，找出最小的x,y和最大的x,y作为左下角和右上角坐标，然后盒中遍历像素点
void triangle(Vec2i *pts,  TGAImage &image, TGAColor color) {
	//定义包围盒
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	//图片的边界
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	//查找包围盒边界
	for (int i = 0; i < 3; i++) {//遍历三角形的3个顶点,进行循环比较找到小的x,y和最大的x,y作为左下角和右上角坐标
		bboxmin.u = std::min(bboxmin.u, pts[i].u);
		bboxmin.v = std::min(bboxmin.v, pts[i].v);
		bboxmax.u = std::max(bboxmax.u, pts[i].u);
		bboxmax.v = std::max(bboxmax.v, pts[i].v);
	}
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {//遍历包围盒中的每一个点
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)continue;//回忆重心坐标判定点的方法。如果重心坐标不在三角形内，则α,β,γ至少有一个为负
			image.set(P.x, P.y, color);//此点着色
		}
	}
}

int main(int argc, char** argv) {
	TGAImage image(200, 200, TGAImage::RGB);
	Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//这是一个三角形

	triangle(pts, image, TGAColor(255, 0, 0, 255));
	image.flip_vertically();
	image.write_tga_file("frame.tga");
	return 0;

}

* 
* 
* 
* 
void triangle(Vec2i a0, Vec2i a1, Vec2i a2, TGAImage& image, TGAColor color) {
	if (a0.y == a1.y && a0.y == a2.y) return;//不生成不能生成的三角形
//假设三角形有三点：t0、t1、t2，它们按 y 坐标按上升顺序排序。
//然后，边界 A 在 t0 和 t2 之间，边界 B 在 t0 和 t1 之间，然后在 t1 和 t2 之间。
	//扫描线算法是一种比较直观的填充算法，主要思想是将三角形沿水平方向划分成高度为一个像素的线条
	//，并不断绘制这些线条从而填充满整个三角形。
	//要想实现绘制，需要知道填充线段的起点和终点，也就是线段的左边和右边
	//为了判别左右，我们需要记录三角形顶点的高度顺序，
	//所以做的第一步是通过交换顶点坐标，将三角形的三个顶点按照从小到大的顺序存储。
	if (a0.y > a1.y) std::swap(a0, a1);
	if (a0.y > a2.y) std::swap(a0, a2);
	if (a1.y > a2.y) std::swap(a1, a2);
	//通过确定顶点顺序，我们就能确定位于y轴中间的顶点a1，a1将三角形划分成了上下两部分，
	// 且a0a2是最长边，在三角形上下两部分划分中会被分割成两端。

	//三角形按a1的y轴坐标划分为上下两部分之后，我们就能计算出total_height和segment_height
	//分别表示总高度和下半部高度，根据当前y与最低点a0的y轴高度差，与总高度、下半高度的比例，
	//相似三角形方法计算出x轴方向上的相对于a0的位移量，从而能够准确找到每一段dy的左右两端点。

	//line(a0, a1, image, green);
	//line(a1, a2, image, green);
	//line(a2, a0, image, red);
int total_height = a2.y - a0.y;
//for (int y = a0.y; y <= a1.y; y++) {
for (int i = 0; i < total_height; i++) {
	bool second_half = i > a1.y - a0.y || a1.y == a0.y;//||逻辑或
	//int segment_height = a1.y - a0.y + 1;
	int segment_height = second_half ? a2.y - a1.y : a1.y - a0.y;
	//float alpha = (float)(y - a0.y) / total_height;
	float alpha = (float)i / total_height;
	//float beta = (float)i / segment_height;
	float beta = (float)(i - (second_half ? a1.y - a0.y : 0)) / segment_height;//是否在a1的y的上面，是的话减去a1的y，再除以a1到a2的距离
	Vec2i A = a0 + (a2 - a0) * alpha;
	//Vec2i B = a0 + (a1 - a0) * beta;
	Vec2i B = second_half ? a1 + (a2 - a1) * beta : a0 + (a1 - a0) * beta;//在上面的话靠近a2，下面靠近a1，
	//image.set(A.x, y, red);//a2>a1>a0。 A.x越来越靠近a2.x， y越来越靠近a1.y
	//image.set(B.x, y, green);//B.x越来越靠近a1.x， y越来越靠近a1.y
	//得到左右两端点的位置后就能一层一层向上扫描从而填充图像了，上半部分也能如法炮制，
	//需要改变segment_height的计算方法，以及偏移原点改为x1和x0。最终将代码统一
	if (A.x > B.x) std::swap(A, B);//确保A.x<B.x以便于进行for循环
	for (int j = A.x; j <= B.x; j++) {
		image.set(j, a0.y + i, color);//从a0.y到a2.y
	}
}
}

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);

	Vec2i t0[3] = { Vec2i(10,70), Vec2i(50,160), Vec2i(70,80) };
	Vec2i t1[3] = { Vec2i(180,50), Vec2i(150,1), Vec2i(70,180) };
	Vec2i t2[3] = { Vec2i(180,150), Vec2i(120,160), Vec2i(130,180) };

	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);


	//	image.set(52, 41, white);
	//	line(0, 0, 100, 100, image, red);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("main.tga");
	return 0;
}

//triangle
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 200;
const int height = 200;


void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++) {
			image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y
		}
	}
}
int main(int argc, char** argv) {

	TGAImage image(width, height, TGAImage::RGB);
	//TGAImage类构造函数构造对象image，存储了对象绘制画框属性大小和深度，用于存储像素点信息，
	//RGB为常量3，则image中的数据缓存在100*100*3个char类型数据在data数组中。

	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // 垂直翻转。沿着中水平轴翻转的。，即反转y轴，左上角顶点现在调动到左下角
	image.write_tga_file("triangle.tga");//write_tga_file 将data数组输出到tga文件

	return 0;
}

*/