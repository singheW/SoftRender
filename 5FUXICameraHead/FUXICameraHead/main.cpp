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
//�������������������͵㡣���һ������Աд��vec2(x,y)����ô���������������ǵ��أ�����˵�����۾��ǣ�����������У�����z=0�Ķ�����������������Ķ��ǵ㡣
//Vec3f camera(0, 0, 3);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f light_dir(0, 0, 1);//��һ��ģ����ն������ν�����ɫ//������Ǵ�ֱ��Ļ��
//Vec3f light_dir = Vec3f(1, -1, 1).normalize();//Gouraud
Vec3f m2v(Matrix m) {//��ά����ά
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {//��ά����ά
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

Matrix viewport(int x, int y, int w, int h) {//�ӿ����ź�ƫ�� ȡ��screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
	//��v������[-1,1][-1,1]���뻭�ڣ�width��height���С�ֵ(v.x+1)��0��2֮��仯��(v.x+1)/2��0��1֮��仯��(v.x+1)*width/2Ҳ������0~width֮��仯��
	Matrix m = Matrix::identity(4);
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {//��ά�ռ��д�һ����׼����ϵ����һ����׼����ϵ,��������ϵ�н���render
	Vec3f z = (eye - center).normalize();//ͨ��eye - center���̶�z�᷽��
	Vec3f x = cross(up, z).normalize();//up��z���˵õ�x�᷽��
	Vec3f y = cross(z, x).normalize();//����x���z���˵õ�y�᷽���µ�����ϵ����ͼ�������ˡ�
	Matrix res = Matrix::identity(4);
	for (int i = 0; i < 3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		res[i][3] = -center[i];
	}
	return res;
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {//����һ�������κ�һ����G��Ŀ���Ǽ������������G
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	//Vec3 n=cross(Vec3(i),Vec3(j))������iΪ��ABx,ACx,PAx����jΪ��ABy,ACy,PAy); nΪ��ֱ��ij�������ķ�������n��ֵΪ��k��,k��,k),��=1-��-�ã���Gλ�ã�G=��A+��B+��C����ABCΪptsָ���3���㣩
	Vec3f u = cross(s[0], s[1]);
	//result ��������Z֮ǰ����Zֵ����Ϊ������
	if (std::abs(u[2]) > 1e-2)
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);//����Zֵ�Իָ�����Ҫ��n:��u(�£��ã�1)=(u.x/u.z,u.y/u.z,u.z/u.z).���=1-(u.x+u.y)/u.z,��return(��,��,��)
	return Vec3f(-1, 1, 1);//u.z��u[2]��Z��int�ͣ��������ֵС��1��˵��������0����ʱ���ش����ŵ��������ꡣ
}
//��Χ���㷨�����¶��ϣ�������
//�����Σ�A��B��C����������P(x,y)=��A+��B+��C����+��+��=1��
//��=��PBC/��ABC����=��PAC/��ABC����=��PAB/��ABC
//�����������ϵ�����¦ö���0����֤���õ����������ڲ���
//��Χ��˼�룺���ȣ�������߽�У����������������������½Ǻ����Ͻǡ�Ϊ���ҵ���Щ�ǣ����ǵ����������ε����ж��㲢���ҵ���С/�������ꡣ
void triangle(Vec3i* pts, float* zbuffer, TGAImage &image,TGAImage& diffuse, Vec2f* vt, Vec3f* vn) {
	Vec2i bboxmin(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());	//�����Χ��
	Vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);//ͼƬ�ı߽磬����Ļ�����˱߽����ã�����CPU����ʱ�䣨ȥ����Ļ��������Σ�
	//���Ұ�Χ�б߽�
	for (int i = 0; i < 3; i++) {//���������ε�3������,
		for (int j = 0; j < 2; j++) {//����ѭ���Ƚ��ҵ�С��x,y������x,y��Ϊ���½Ǻ����Ͻ�����
			bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {//������Χ���е�ÿһ����
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);//bc_screen�Ǽ��������������(��,��,��)
			//��1 - t, t���ǵ㣨x��y��������߶�p0��p1���������꣺��x��y��= p0*��1 - t��+ p1 * t��
			//���������ι�դ�����������꣬��������Ҫ���Ƶ�ÿһ�����أ�ֻ��Ҫ������������������ǹ�դ���������ζ���xyzֵ���ɡ�
			//ͨ���������������������λ�ã�ͨ����ֵ�����õ�p���������е�����λ�ã��Ӷ�ֱ��ȥ�������ҵ���Ӧ��������ɫ��
			float indx = vt[0][0] * bc_screen.x + vt[1][0] * bc_screen.y + vt[2][0] * bc_screen.z;//xy��uv�е���ά��Ӧ
			float indy = vt[0][1] * bc_screen.x + vt[1][1] * bc_screen.y + vt[2][1] * bc_screen.z;
			//�����ε��������꣬��������������ڣ���������ͼ��Ŀ�Ⱥ͸߶ȣ����õ�����Ҫ�ŵ���Ⱦ���е���ɫ��
		
		
			float norx = vn[0].x * bc_screen.x + vn[1].x * bc_screen.y + vn[2].x * bc_screen.z;
			float nory = vn[0].y * bc_screen.x + vn[1].y * bc_screen.y + vn[2].y * bc_screen.z;
			float norz = vn[0].z * bc_screen.x + vn[1].z * bc_screen.y + vn[2].z * bc_screen.z;

			Vec3f normal(norx, nory, norz);
			normal.normalize();//����ǿ��intensity��ֵ��Ҫ���й�һ��
			float intensity = normal * light_dir;// ����ǿ�ȵ��ڹ������͸��������εķ��ߵı����˻��������εķ��߿��Լ򵥵ؼ���Ϊ�����ߵĽ���˻���

			indy = 1 - indy;//��תy��
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;//�������������ж���ķ���������������겻���������ڣ����,��,��������һ��Ϊ��
			//����ֻ֪�������εıߣ�����֪���������ڲ���� z �Ĵ�С��// pts[i][2] �� z ��ֵ��// bc_screen[i] �ֱ���(��,��,��)//�����Ϳ��Եõ� z ��ֵ.��z ��ĵ�Ӧ����ס z С�ĵ㡣
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
	//3Dת2D����㷨��1.����ͼƬ��С�� width �� height��2.�� zbuffer[width �� height] �洢ÿһ��Ҫ�����ص� z ֵ����ʼ��Ϊ����С
		//3.���������ε������ߣ������������ڲ������е㡣����ڲ���� z ����zbuffer ��ֵ���� zbuffer �����㡣�ﵽ����������ظ���֮ǰ���ص�Ŀ�ġ�4.�ظ�����������ֱ�����������е�������
			if (zbuffer[int(P.x + P.y * width)] < P.z) {//�Ҵӱ���p.x�е�p.y��boxmax.y֮�俪ʼ�������Ҽ����߶ζ�Ӧ��z���ꡣȻ�����õ�ǰ��(P.x + P.y * width)�����������������ybuffer�еõ���ʲô��
				zbuffer[int(P.x + P.y * width)] = P.z;//�����ǰ��yֵ��ybuffer�е�ֵ���ӽ��������ô����Ļ�ϻ�������������ybuffer��
				//image.set(P.x, P.y, color);//�˵���ɫ
				if (intensity < 0)continue;//��������Ǹ���������ζ�Ź���ܴӶ���κ����չ��������������ģ�ܺ����ǿ��Զ�����������Ρ������ٵ���������������ǣ���б�����á�
				TGAColor vcolor = diffuse.get(int(indx * 1024), int(indy * 1024));
				//TGAColor vcolor(255, 255, 255, 255);//Gouraud
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
	//TGAImage�๹�캯���������image���洢�˶�����ƻ������Դ�С����ȣ����ڴ洢���ص���Ϣ��
	//RGBΪ����3����image�е����ݻ�����100*100*3��char����������data�����С�
	//image.set(52, 41, white);//image����set����������λ��x,y����RGB��Ϣ
	float* zbuffer = new float[width * height]; //Ϊ�˻�����2D��Ļ�ϣ�������Ҫʹ��һ����ά���飺�� zbuffer[width �� height] �洢ÿһ��Ҫ�����ص� z ֵ��
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());//��ʼ��Ϊ����С��numeric_limits��ֵ����;int����Сֵ;min���ؿ�ȡ����Сֵ���淶����

	Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
	Matrix Projection = Matrix::identity(4);
	Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	//Projection[3][2] = -1.f/camera.z;��z���ϵ�����ͷ����Ҫ��������ԭ��c�������������ͶӰ��Ļ�ľ���c,ȡ�����͸�ֵ��Ϊ����������ĵ����е����м�����Ϊ�ñ任��͸��ͶӰ�任����
	Projection[3][2] = -1.f / (eye - center).norm();//λ��z���ϵ�����ͷʱ Projection[3][2] = -1/c //����ͷ����ԭ��c   �����C��(eye - center).norm() 

	for (int i = 0; i < model->nfaces(); i++) { //�ļ��л�ȡģ����Ϣװ��model��nfaces��ʾ�������������model�е�ÿһ��face
		std::vector<int> face = model->face(i);
		std::vector<int> fuv = model->faceuv(i);//faceuv//faceuvs_;//fuv;//UV��������
		std::vector<int> vnfaces = model->vnfaces(i);
		Vec3i pts[3];//����һ��������
		Vec2f vt_coords[3];
		Vec3f world_coords[3];
		Vec3f vn_coords[3];
		for (int j = 0; j < 3; j++) {//��ÿһ�����3�����㹹�ɵ�3���߶ν��л��ƣ�
			Vec3f v = model->vert(face[j]);//Vec3f ��һ�ִ洢��ά���������ݵĽṹ��
			//Vec3f v1 = model->vert(face[(j + 1) % 3]);ÿһ��face�ڴ洢��3������vert�ֱ�Ϊx,y,z,��ΧΪ-1��1������Ϊ����ԭ��Ϊ�������꣬������Ҫ����λ�ƽ�ͼ��ڷŵ�image�м�λ�á�
			//���������ε�ÿһ������v����Ҫִ��MVP�任����һ����ģ��תΪ����M���ڶ����������λ�ñ任��������ϵV��������������Ļλ�ý���ͶӰ�任P���Ӷ�����������ϵת��Ϊ��Ļ����ϵ����ִ����ɫ
			//ModelView/lookat/��һ����ģ������ת��Ϊ��������/����ģ��תΪ������۾����꣩//ֻ����ģ��֡�еõ�����Ϊ(x,y,z,1)������㣬���Ծ���ModelView���Ϳ��Եõ��������ϵ�е����ꡣ
			//ViewPort/������������Ļλ�ý���ͶӰ�任���Ӷ�����������ϵת��Ϊ��Ļ����ϵ/�ӿ����ź�ƫ�� ȡ��screen_coords[j] /����������ת��Ϊ��������
			//Projection/�ڶ����������λ�ñ任��������ϵ/(-1/c)�Գ������б��Σ�����͸�ӱ��Σ�������󽫳����任����ν�ļ�������
			pts[j] = m2v(ViewPort * Projection * v2m(v));//һ����������������ӿ��ϵ�λ�� world2screen(model->vert(face[i]));////��������ϵ����ά��ͶӰ�任���Ӵ��任����ά���Ӵ�����ϵ
			world_coords[j] = v;

			Vec2f w = model->vt(fuv[j]);
			vt_coords[j] = w;

			Vec3f n = model->vn(vnfaces[j]);
			vn_coords[j] = n;

		}
		//Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));//��������������ĳ�������εķ��ߣ����� = ��������������������ˣ������������������ζ������������������
		//n.normalize();// �� n ����һ������
	//	float intensity = n * light_dir;	// �����η��ߺ͹��շ�������ˣ����ֵ���� 0��˵�����߷���͹��շ�����ͬһ�ֵ࣬Խ��˵��Խ��Ĺ����䵽�������ϣ���ɫԽ��

		triangle(pts, zbuffer,image, diffuse, vt_coords, vn_coords);
	}

	image.flip_vertically(); //��ֱ��ת��������ˮƽ�ᷭת�ģ�����תy�ᣬ���ϽǶ������ڵ��������½�
	image.write_tga_file("african_head.tga");//write_tga_file ��data���������tga�ļ�

	{ // dump z-buffer (debugging purposes only)
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {//��
			for (int j = 0; j < height; j++) {//��
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));// zbuffer[i + j * width]�������ң����µ��ϣ�һ����ɨ��ȥ��zbuffer��һά����
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("zbuffer.tga");
	}
	delete model;
	delete[]zbuffer;
	return 0;
}