#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);


void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//为修复缺失的点“有孔”，使用if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//当线陡峭，或者说y值相对过高时，交换x,y轴。相当与关于45°进行翻转
		std::swap(x0, y0);//绝对值如果高差大于宽差，调换图片
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//为保证绘制一致性，线判断两点坐标大小，通过交换保证x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	//减少误差，优化line算法
	int dx = x1 - x0;
	int dy = y1 - y0;
	//float derror = std::abs(dy / float(dx));//
	//float error = 0;
	float derror2 = std::abs(dy)*2;////浮动点：除以dx和循环体中的.5进行比较。可以通过将错误变量替换为另一个误差来摆脱浮动点
	float error2 = 0;
	int y = y0;
	
	for(int x=x0;x<=x1;x++){//要绘制N个点，以x1-x作为要绘制的点的数量。
	//	float t = (x - x0) / (float)(x1 - x0); //
	//  int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//绘制时需将翻转后的线段再次翻转回来，保证长轴的连续性
		}
		else {
			image.set(x, y, color);
		}
		//error += derror;
		error2 += derror2;
		//		if (error > .5) {
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			//error -= 1.;
			error2 -= dx*2;
		}
	}
}
int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	//TGAImage类构造函数构造对象image，存储了对象绘制画框属性大小和深度，用于存储像素点信息，
	//RGB为常量3，则image中的数据缓存在100*100*3个char类型数据在data数组中。
	//image.set(52, 41, white);//image调用set方法在像素位置x,y存入RGB信息
	line(20, 13, 40, 80, image, red);//因为是以x++为阶段，所以各点间的y的分布不均，造成“有孔”现象
	line(10, 20, 60, 30, image, white);//y的变化太小所以过于密集成了一条线
	line(60, 30, 10, 20, image, red);
	image.flip_vertically(); // 垂直翻转。沿着中水平轴翻转的。，即反转y轴，左上角顶点现在调动到左下角
	image.write_tga_file("output.tga");//write_tga_file 将data数组输出到tga文件
	return 0;
}

/*
* 
//第五次尝试
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//为修复缺失的点“有孔”，使用if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//当线陡峭，或者说y值相对过高时，交换x,y轴。相当与关于45°进行翻转
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//为保证绘制一致性，线判断两点坐标大小，通过交换保证x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	//减少误差，优化line算法
	int dx = x1 - x0;
	int dy = y1 - y0;
	//float derror = std::abs(dy / float(dx));//
	//float error = 0;
	float derror2 = std::abs(dy)*2;//通过将错误变量替代成另一个误差变量来拜托浮点数
	float error2 = 0;
	int y = y0;

	for(int x=x0;x<=x1;x++){//要绘制N个点，以x1-x作为要绘制的点的数量。
	//	float t = (x - x0) / (float)(x1 - x0); //
	//  int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//绘制时需将翻转后的线段再次翻转回来，保证长轴的连续性
		}
		else {
			image.set(x, y, color);
		}
		//error += derror;
		error2 += derror2;
		//		if (error > .5) {
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			//error -= 1.;
			error2 -= dx*2;
		}
	}
}

//第四次尝试

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//为修复缺失的点“有孔”，使用if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//当线陡峭，或者说y值相对过高时，交换x,y轴。相当与关于45°进行翻转
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//为保证绘制一致性，线判断两点坐标大小，通过交换保证x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	//减少误差，优化line算法
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy / float(dx));//
	float error = 0;
	int y = y0;

	for(int x=x0;x<=x1;x++){//要绘制N个点，以x1-x作为要绘制的点的数量。
	//	float t = (x - x0) / (float)(x1 - x0); //
	//  int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//绘制时需将翻转后的线段再次翻转回来，保证长轴的连续性
		}
		else {
			image.set(x, y, color);
		}
		error += derror;
		if (error > .5) {
			y += (y1 > y0 ? 1 : -1);
			error -= 1.;
		}
	}
}


* 
//第三次尝试
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//为修复缺失的点“有孔”，使用if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//当线陡峭，或者说y值相对过高时，交换x,y轴。相当与关于45°进行翻转
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//为保证绘制一致性，线判断两点坐标大小，通过交换保证x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	for(int x=x0;x<=x1;x++){//要绘制N个点，以x1-x作为要绘制的点的数量。
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//绘制时需将翻转后的线段再次翻转回来，保证长轴的连续性
		}
		else {
			image.set(x, y, color);
		}
	}
}





* //第二次尝试
* void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for(int x=x0;x<=x1;x++){//要绘制N个点，以x1-x作为要绘制的点的数量。这种遍历x的算法只能保证x轴的连续性不能保证y轴的
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0*(1.-t) + y1*t;
		image.set(x,y,color);
	}
}
int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	//TGAImage类构造函数构造对象image，存储了对象绘制画框属性大小和深度，用于存储像素点信息，
	//RGB为常量3，则image中的数据缓存在100*100*3个char类型数据在data数组中。
	//image.set(52, 41, white);//image调用set方法在像素位置x,y存入RGB信息
	line(20, 13, 40, 80, image, red);//因为是以x++为阶段，所以各点间的y的分布不均，造成“有孔”现象
	line(10, 20, 60, 30, image, white);//y的变化太小所以过于密集成了一条线
	line(60, 30, 10, 20, image, red);
	image.flip_vertically(); // 垂直翻转。沿着中水平轴翻转的。，即反转y轴，左上角顶点现在调动到左下角
	image.write_tga_file("output.tga");//write_tga_file 将data数组输出到tga文件
	return 0;
}

* 
* //第一次尝试
 void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for (float t = 0.; t < 1.; t += .1) {//设置初始坐标和终点坐标，使用t作为变量调节绘制坐标
		int x = x0 + (x1 - x0) * t;
		int y = y0 + (y1 - y0) * t;
		image.set(x,y,color);
	}
}
* 
//Bresenham's 算法是什么。
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	auto f = [&](int x, int y) {
		int dx = x0 - x1;
		int dy = y0 - y1;
		return x * dy - y * dx + y1 * dx - x1 * dy;
	};
	for (int x = x1, y = y1; x <= x0;x++) {
		if (abs(f(x, y)) - abs(f(x, y + 1)) < 0)
			image(x, y) = color;
		else image(x, ++y) = color;

	}
}
int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	//TGAImage类构造函数构造对象image，存储了对象绘制画框属性大小和深度，用于存储像素点信息，
	//RGB为常量3，则image中的数据缓存在100*100*3个char类型数据在data数组中。
	image.set(52, 41, white);//image调用set方法在像素位置x,y存入RGB信息
	image.flip_vertically(); // 垂直翻转。沿着中水平轴翻转的。，即反转y轴，左上角顶点现在调动到左下角
	image.write_tga_file("output.tga");//write_tga_file 将data数组输出到tga文件
	return 0;
}
*/