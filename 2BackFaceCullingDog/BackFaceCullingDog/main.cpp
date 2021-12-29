#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

  const int width = 200;
  const int height = 200;
  Model* model = NULL;

  Vec3f cross(Vec3f  v1, Vec3f  v2)//����������ά����
  {
	  return {v1.y* v2.z - v1.z * v2.y, v1.z* v2.x - v1.x * v2.z, v1.x* v2.y - v1.y * v2.x};//�����ʽ
  }

  
  Vec3f barycentric(Vec2i *pts, Vec2i G) {//����һ��2D�����κ�һ����G��Ŀ���Ǽ������������G
//Vec3 n=cross(Vec3(i),Vec3(j))������iΪ��ABx,ACx,PAx����jΪ��ABy,ACy,PAy); nΪ��ֱ��ij�������ķ�������n��ֵΪ��k��,k��,k),��=1-��-�ã���Gλ�ã�G=��A+��B+��C����ABCΪptsָ���3���㣩
	  Vec3f u = cross(Vec3f(pts[2].u - pts[0].u, pts[1].u - pts[0].u, pts[0].u - G.u), Vec3f(pts[2].v - pts[0].v, pts[1].v - pts[0].v, pts[0].v - G.v));
	 //result ��������Z֮ǰ����Zֵ����Ϊ������
	  if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);//Z��int�ͣ��������ֵС��1��˵��������0����ʱ���ش����ŵ��������ꡣ
	  return Vec3f(1.f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);//����Zֵ�Իָ�����Ҫ��n:��u(�£��ã�1)=(u.x/u.z,u.y/u.z,u.z/u.z).���=1-(u.x+u.y)/u.z,��return(��,��,��)
  }
//��Χ���㷨�����¶��ϣ�������
//�����Σ�A��B��C����������P(x,y)=��A+��B+��C����+��+��=1��
//��=��PBC/��ABC����=��PAC/��ABC����=��PAB/��ABC
//�����������ϵ�����¦ö���0����֤���õ����������ڲ���
//��Χ��˼�룺�Ƚ��������xy���꣬�ҳ���С��x,y������x,y��Ϊ���½Ǻ����Ͻ����꣬Ȼ����б������ص�
void triangle(Vec2i *pts,  TGAImage &image, TGAColor color) {
	//�����Χ��
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	//ͼƬ�ı߽�
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	//���Ұ�Χ�б߽�
	for (int i = 0; i < 3; i++) {//���������ε�3������,����ѭ���Ƚ��ҵ�С��x,y������x,y��Ϊ���½Ǻ����Ͻ�����
		bboxmin.u = std::min(bboxmin.u, pts[i].u);
		bboxmin.v = std::min(bboxmin.v, pts[i].v);
		bboxmax.u = std::max(bboxmax.u, pts[i].u);
		bboxmax.v = std::max(bboxmax.v, pts[i].v);
	}
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {//������Χ���е�ÿһ����
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)continue;//�������������ж���ķ���������������겻���������ڣ����,��,��������һ��Ϊ��
			image.set(P.x, P.y, color);//�˵���ɫ
		}
	}
}




int main(int argc, char** argv) {
	//���Ȼ�ȡ�����ɫ�������ģ�ͣ��������������귽��ʵ�֡�
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);

	//��һ��ģ����ն������ν�����ɫ
	Vec3f light_dir(0, 0, -1);//������Ǵ�ֱ��Ļ��

	for (int i = 0; i < model->nfaces(); i++) { //����model�е�ÿһ��face
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];//��Ļ���꣬����������ת��������
		Vec3f world_coord[3];
		for (int j = 0; j < 3; j++) {//��ÿһ�����3�����㹹�ɵ�3���߶ν��л���
			Vec3f v = model->vert(face[j]);
			//	Vec3f world_coords = model->vert(face[j]);//Vec3f ��һ�ִ洢��ά���������ݵĽṹ
			screen_coords[j] = Vec2i((v.x + 0.6) * width / 1.2, (v.y + 0.05) * height / 1.2);  // ͶӰΪ����ͶӰ���ҽ���������ת��Ϊ��Ļ����
			world_coord[j] = v;
		}
		//��������������ĳ�������εķ��ߣ����� = ��������������������ˣ������������������ζ������������������
		Vec3f n = (world_coord[2] - world_coord[0]) ^ (world_coord[1] - world_coord[0]);
		n.normalize();// �� n ����һ������
		// �����η��ߺ͹��շ�������ˣ����ֵ���� 0��˵�����߷���͹��շ�����ͬһ�ֵ࣬Խ��˵��Խ��Ĺ����䵽�������ϣ���ɫԽ��
		float intensity = n * light_dir;
		//Back-face�޳�
		if (intensity > 0) {
			triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//����һ��������
	//triangle(pts, image, TGAColor(255, 0, 0, 255));
	image.flip_vertically();
	image.write_tga_file("african_head.tga");
	return 0;

}




/*
* 
*

//��ģ��������Ӱ
int main(int argc, char** argv) {
	//���Ȼ�ȡ�����ɫ�������ģ�ͣ��������������귽��ʵ�֡�
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/dog.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);

	//��һ��ģ����ն������ν�����ɫ
	Vec3f light_dir(0, 0, -1);//������Ǵ�ֱ��Ļ��

	for (int i = 0; i < model->nfaces(); i++) { //����model�е�ÿһ��face
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];//��Ļ���꣬����������ת��������
		Vec3f world_coord[3];
		for (int j = 0; j < 3; j++) {//��ÿһ�����3�����㹹�ɵ�3���߶ν��л���
			Vec3f v = model->vert(face[j]);
			//	Vec3f world_coords = model->vert(face[j]);//Vec3f ��һ�ִ洢��ά���������ݵĽṹ
			screen_coords[j] = Vec2i((v.x + 0.6) * width / 1.2, (v.y + 0.05) * height / 1.2);  // ͶӰΪ����ͶӰ���ҽ���������ת��Ϊ��Ļ����
			world_coord[j] = v;
		}
		//��������������ĳ�������εķ��ߣ����� = ��������������������ˣ������������������ζ������������������
		Vec3f n = (world_coord[2] - world_coord[0]) ^ (world_coord[1] - world_coord[0]);
		n.normalize();// �� n ����һ������
		// �����η��ߺ͹��շ�������ˣ����ֵ���� 0��˵�����߷���͹��շ�����ͬһ�ֵ࣬Խ��˵��Խ��Ĺ����䵽�������ϣ���ɫԽ��
		float intensity = n * light_dir;
		//Back-face�޳�
		if (intensity > 0) {
			triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//����һ��������
	//triangle(pts, image, TGAColor(255, 0, 0, 255));
	image.flip_vertically();
	image.write_tga_file("frame.tga");
	return 0;

}





//��model����������ɫ

int main(int argc, char** argv) {
	//���Ȼ�ȡ�����ɫ�������ģ�ͣ��������������귽��ʵ�֡�
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/dog.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model->nfaces(); i++) { //����model�е�ÿһ��face
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		for (int j = 0; j < 3; j++) {//��ÿһ�����3�����㹹�ɵ�3���߶ν��л���
			Vec3f world_coords = model->vert(face[j]);//Vec3f ��һ�ִ洢��ά���������ݵĽṹ
			screen_coords[j] = Vec2i((world_coords.x + 0.6) * width / 1.2, (world_coords.y + 0.05) * height / 1.2);//���������ν����ض�������������ת��Ϊ��Ļ���겢����������
		}
		triangle(screen_coords, image, TGAColor(rand() % 255, rand() % 255, rand() % 255,  255));
	}
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//����һ��������
	//triangle(pts, image, TGAColor(255, 0, 0, 255));
	image.flip_vertically();
	image.write_tga_file("frame.tga");
	return 0;

}

* 
* 
* 
* 
//�������껭������
#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
  const int width = 200;
  const int height = 200;

  Vec3f cross(Vec3f  v1, Vec3f  v2)//����������ά����
  {
	  return {v1.y* v2.z - v1.z * v2.y, v1.z* v2.x - v1.x * v2.z, v1.x* v2.y - v1.y * v2.x};//�����ʽ
  }


  Vec3f barycentric(Vec2i *pts, Vec2i G) {//����һ��2D�����κ�һ����G��Ŀ���Ǽ������������G
//Vec3 n=cross(Vec3(i),Vec3(j))������iΪ��ABx,ACx,PAx����jΪ��ABy,ACy,PAy); nΪ��ֱ��ij�������ķ�������n��ֵΪ��k��,k��,k),��=1-��-�ã���Gλ�ã�G=��A+��B+��C����ABCΪptsָ���3���㣩
	  Vec3f u = cross(Vec3f(pts[2].u - pts[0].u, pts[1].u - pts[0].u, pts[0].u - G.u), Vec3f(pts[2].v - pts[0].v, pts[1].v - pts[0].v, pts[0].v - G.v));
	 //result ��������Z֮ǰ����Zֵ����Ϊ������
	  if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);//Z��int�ͣ��������ֵС��1��˵��������0����ʱ���ش����ŵ��������ꡣ
	  return Vec3f(1.f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);//����Zֵ�Իָ�����Ҫ��n:��u(�£��ã�1)=(u.x/u.z,u.y/u.z,u.z/u.z).���=1-(u.x+u.y)/u.z,��return(��,��,��)
  }
//��Χ���㷨�����¶��ϣ�������
//�����Σ�A��B��C����������P(x,y)=��A+��B+��C����+��+��=1��
//��=��PBC/��ABC����=��PAC/��ABC����=��PAB/��ABC
//�����������ϵ�����¦ö���0����֤���õ����������ڲ���
//��Χ��˼�룺�Ƚ��������xy���꣬�ҳ���С��x,y������x,y��Ϊ���½Ǻ����Ͻ����꣬Ȼ����б������ص�
void triangle(Vec2i *pts,  TGAImage &image, TGAColor color) {
	//�����Χ��
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	//ͼƬ�ı߽�
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	//���Ұ�Χ�б߽�
	for (int i = 0; i < 3; i++) {//���������ε�3������,����ѭ���Ƚ��ҵ�С��x,y������x,y��Ϊ���½Ǻ����Ͻ�����
		bboxmin.u = std::min(bboxmin.u, pts[i].u);
		bboxmin.v = std::min(bboxmin.v, pts[i].v);
		bboxmax.u = std::max(bboxmax.u, pts[i].u);
		bboxmax.v = std::max(bboxmax.v, pts[i].v);
	}
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {//������Χ���е�ÿһ����
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)continue;//�������������ж���ķ���������������겻���������ڣ����,��,��������һ��Ϊ��
			image.set(P.x, P.y, color);//�˵���ɫ
		}
	}
}

int main(int argc, char** argv) {
	TGAImage image(200, 200, TGAImage::RGB);
	Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };//����һ��������

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
	if (a0.y == a1.y && a0.y == a2.y) return;//�����ɲ������ɵ�������
//���������������㣺t0��t1��t2�����ǰ� y ���갴����˳������
//Ȼ�󣬱߽� A �� t0 �� t2 ֮�䣬�߽� B �� t0 �� t1 ֮�䣬Ȼ���� t1 �� t2 ֮�䡣
	//ɨ�����㷨��һ�ֱȽ�ֱ�۵�����㷨����Ҫ˼���ǽ���������ˮƽ���򻮷ֳɸ߶�Ϊһ�����ص�����
	//�������ϻ�����Щ�����Ӷ���������������Ρ�
	//Ҫ��ʵ�ֻ��ƣ���Ҫ֪������߶ε������յ㣬Ҳ�����߶ε���ߺ��ұ�
	//Ϊ���б����ң�������Ҫ��¼�����ζ���ĸ߶�˳��
	//�������ĵ�һ����ͨ�������������꣬�������ε��������㰴�մ�С�����˳��洢��
	if (a0.y > a1.y) std::swap(a0, a1);
	if (a0.y > a2.y) std::swap(a0, a2);
	if (a1.y > a2.y) std::swap(a1, a2);
	//ͨ��ȷ������˳�����Ǿ���ȷ��λ��y���м�Ķ���a1��a1�������λ��ֳ������������֣�
	// ��a0a2����ߣ������������������ֻ����лᱻ�ָ�����ˡ�

	//�����ΰ�a1��y�����껮��Ϊ����������֮�����Ǿ��ܼ����total_height��segment_height
	//�ֱ��ʾ�ܸ߶Ⱥ��°벿�߶ȣ����ݵ�ǰy����͵�a0��y��߶Ȳ���ܸ߶ȡ��°�߶ȵı�����
	//���������η��������x�᷽���ϵ������a0��λ�������Ӷ��ܹ�׼ȷ�ҵ�ÿһ��dy���������˵㡣

	//line(a0, a1, image, green);
	//line(a1, a2, image, green);
	//line(a2, a0, image, red);
int total_height = a2.y - a0.y;
//for (int y = a0.y; y <= a1.y; y++) {
for (int i = 0; i < total_height; i++) {
	bool second_half = i > a1.y - a0.y || a1.y == a0.y;//||�߼���
	//int segment_height = a1.y - a0.y + 1;
	int segment_height = second_half ? a2.y - a1.y : a1.y - a0.y;
	//float alpha = (float)(y - a0.y) / total_height;
	float alpha = (float)i / total_height;
	//float beta = (float)i / segment_height;
	float beta = (float)(i - (second_half ? a1.y - a0.y : 0)) / segment_height;//�Ƿ���a1��y�����棬�ǵĻ���ȥa1��y���ٳ���a1��a2�ľ���
	Vec2i A = a0 + (a2 - a0) * alpha;
	//Vec2i B = a0 + (a1 - a0) * beta;
	Vec2i B = second_half ? a1 + (a2 - a1) * beta : a0 + (a1 - a0) * beta;//������Ļ�����a2�����濿��a1��
	//image.set(A.x, y, red);//a2>a1>a0�� A.xԽ��Խ����a2.x�� yԽ��Խ����a1.y
	//image.set(B.x, y, green);//B.xԽ��Խ����a1.x�� yԽ��Խ����a1.y
	//�õ��������˵��λ�ú����һ��һ������ɨ��Ӷ����ͼ���ˣ��ϰ벿��Ҳ���編���ƣ�
	//��Ҫ�ı�segment_height�ļ��㷽�����Լ�ƫ��ԭ���Ϊx1��x0�����ս�����ͳһ
	if (A.x > B.x) std::swap(A, B);//ȷ��A.x<B.x�Ա��ڽ���forѭ��
	for (int j = A.x; j <= B.x; j++) {
		image.set(j, a0.y + i, color);//��a0.y��a2.y
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
	//TGAImage�๹�캯���������image���洢�˶�����ƻ������Դ�С����ȣ����ڴ洢���ص���Ϣ��
	//RGBΪ����3����image�е����ݻ�����100*100*3��char����������data�����С�

	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // ��ֱ��ת��������ˮƽ�ᷭת�ġ�������תy�ᣬ���ϽǶ������ڵ��������½�
	image.write_tga_file("triangle.tga");//write_tga_file ��data���������tga�ļ�

	return 0;
}

*/