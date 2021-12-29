#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);


void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//Ϊ�޸�ȱʧ�ĵ㡰�пס���ʹ��if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//���߶��ͣ�����˵yֵ��Թ���ʱ������x,y�ᡣ�൱�����45����з�ת
		std::swap(x0, y0);//����ֵ����߲���ڿ�����ͼƬ
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//Ϊ��֤����һ���ԣ����ж����������С��ͨ��������֤x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	//�������Ż�line�㷨
	int dx = x1 - x0;
	int dy = y1 - y0;
	//float derror = std::abs(dy / float(dx));//
	//float error = 0;
	float derror2 = std::abs(dy)*2;////�����㣺����dx��ѭ�����е�.5���бȽϡ�����ͨ������������滻Ϊ��һ����������Ѹ�����
	float error2 = 0;
	int y = y0;
	
	for(int x=x0;x<=x1;x++){//Ҫ����N���㣬��x1-x��ΪҪ���Ƶĵ��������
	//	float t = (x - x0) / (float)(x1 - x0); //
	//  int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//����ʱ�轫��ת����߶��ٴη�ת��������֤�����������
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
	//TGAImage�๹�캯���������image���洢�˶�����ƻ������Դ�С����ȣ����ڴ洢���ص���Ϣ��
	//RGBΪ����3����image�е����ݻ�����100*100*3��char����������data�����С�
	//image.set(52, 41, white);//image����set����������λ��x,y����RGB��Ϣ
	line(20, 13, 40, 80, image, red);//��Ϊ����x++Ϊ�׶Σ����Ը�����y�ķֲ���������ɡ��пס�����
	line(10, 20, 60, 30, image, white);//y�ı仯̫С���Թ����ܼ�����һ����
	line(60, 30, 10, 20, image, red);
	image.flip_vertically(); // ��ֱ��ת��������ˮƽ�ᷭת�ġ�������תy�ᣬ���ϽǶ������ڵ��������½�
	image.write_tga_file("output.tga");//write_tga_file ��data���������tga�ļ�
	return 0;
}

/*
* 
//����γ���
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//Ϊ�޸�ȱʧ�ĵ㡰�пס���ʹ��if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//���߶��ͣ�����˵yֵ��Թ���ʱ������x,y�ᡣ�൱�����45����з�ת
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//Ϊ��֤����һ���ԣ����ж����������С��ͨ��������֤x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	//�������Ż�line�㷨
	int dx = x1 - x0;
	int dy = y1 - y0;
	//float derror = std::abs(dy / float(dx));//
	//float error = 0;
	float derror2 = std::abs(dy)*2;//ͨ������������������һ�������������и�����
	float error2 = 0;
	int y = y0;

	for(int x=x0;x<=x1;x++){//Ҫ����N���㣬��x1-x��ΪҪ���Ƶĵ��������
	//	float t = (x - x0) / (float)(x1 - x0); //
	//  int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//����ʱ�轫��ת����߶��ٴη�ת��������֤�����������
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

//���Ĵγ���

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//Ϊ�޸�ȱʧ�ĵ㡰�пס���ʹ��if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//���߶��ͣ�����˵yֵ��Թ���ʱ������x,y�ᡣ�൱�����45����з�ת
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//Ϊ��֤����һ���ԣ����ж����������С��ͨ��������֤x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	//�������Ż�line�㷨
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy / float(dx));//
	float error = 0;
	int y = y0;

	for(int x=x0;x<=x1;x++){//Ҫ����N���㣬��x1-x��ΪҪ���Ƶĵ��������
	//	float t = (x - x0) / (float)(x1 - x0); //
	//  int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//����ʱ�轫��ת����߶��ٴη�ת��������֤�����������
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
//�����γ���
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//Ϊ�޸�ȱʧ�ĵ㡰�пס���ʹ��if (dx>dy) {for (int x)} else {for (int y)}
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {//���߶��ͣ�����˵yֵ��Թ���ʱ������x,y�ᡣ�൱�����45����з�ת
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;

	}
	if (x0 > x1) {//Ϊ��֤����һ���ԣ����ж����������С��ͨ��������֤x0<x1
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	for(int x=x0;x<=x1;x++){//Ҫ����N���㣬��x1-x��ΪҪ���Ƶĵ��������
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0*(1.-t) + y1*t;
		if (steep) {
			image.set(y, x, color);//����ʱ�轫��ת����߶��ٴη�ת��������֤�����������
		}
		else {
			image.set(x, y, color);
		}
	}
}





* //�ڶ��γ���
* void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for(int x=x0;x<=x1;x++){//Ҫ����N���㣬��x1-x��ΪҪ���Ƶĵ�����������ֱ���x���㷨ֻ�ܱ�֤x��������Բ��ܱ�֤y���
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0*(1.-t) + y1*t;
		image.set(x,y,color);
	}
}
int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	//TGAImage�๹�캯���������image���洢�˶�����ƻ������Դ�С����ȣ����ڴ洢���ص���Ϣ��
	//RGBΪ����3����image�е����ݻ�����100*100*3��char����������data�����С�
	//image.set(52, 41, white);//image����set����������λ��x,y����RGB��Ϣ
	line(20, 13, 40, 80, image, red);//��Ϊ����x++Ϊ�׶Σ����Ը�����y�ķֲ���������ɡ��пס�����
	line(10, 20, 60, 30, image, white);//y�ı仯̫С���Թ����ܼ�����һ����
	line(60, 30, 10, 20, image, red);
	image.flip_vertically(); // ��ֱ��ת��������ˮƽ�ᷭת�ġ�������תy�ᣬ���ϽǶ������ڵ��������½�
	image.write_tga_file("output.tga");//write_tga_file ��data���������tga�ļ�
	return 0;
}

* 
* //��һ�γ���
 void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for (float t = 0.; t < 1.; t += .1) {//���ó�ʼ������յ����꣬ʹ��t��Ϊ�������ڻ�������
		int x = x0 + (x1 - x0) * t;
		int y = y0 + (y1 - y0) * t;
		image.set(x,y,color);
	}
}
* 
//Bresenham's �㷨��ʲô��
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
	//TGAImage�๹�캯���������image���洢�˶�����ƻ������Դ�С����ȣ����ڴ洢���ص���Ϣ��
	//RGBΪ����3����image�е����ݻ�����100*100*3��char����������data�����С�
	image.set(52, 41, white);//image����set����������λ��x,y����RGB��Ϣ
	image.flip_vertically(); // ��ֱ��ת��������ˮƽ�ᷭת�ġ�������תy�ᣬ���ϽǶ������ڵ��������½�
	image.write_tga_file("output.tga");//write_tga_file ��data���������tga�ļ�
	return 0;
}
*/