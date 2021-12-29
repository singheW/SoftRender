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
//齐次坐标可以区分向量和点。如果一个程序员写了vec2(x,y)，那么它到底是向量还是点呢？很难说。结论就是：在齐次坐标中，所有z=0的东西都是向量，其余的都是点。
//Vec3f camera(0, 0, 3);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f light_dir(0, 0, 1);//用一个模拟光照对三角形进行着色//假设光是垂直屏幕的
//Vec3f light_dir = Vec3f(1, -1, 1).normalize();//Gouraud
Vec3f m2v(Matrix m) {//四维变三维
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {//三维变四维
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

Vec3f cross(Vec3f  v1, Vec3f  v2)//传入两个三维向量
{
	return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };//叉积公式
}

Matrix viewport(int x, int y, int w, int h) {//视口缩放和偏移 取代screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
	//点v，属于[-1,1][-1,1]。想画在（width，height）中。值(v.x+1)在0和2之间变化，(v.x+1)/2在0和1之间变化，(v.x+1)*width/2也就是在0~width之间变化。
	Matrix m = Matrix::identity(4);
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {//三维空间中从一个基准坐标系到另一个基准坐标系,在新坐标系中进行render
	Vec3f z = (eye - center).normalize();//通过eye - center来固定z轴方向，
	Vec3f x = cross(up, z).normalize();//up与z求叉乘得到x轴方向
	Vec3f y = cross(z, x).normalize();//利用x轴和z轴叉乘得到y轴方向，新的坐标系方向就计算出来了。
	Matrix res = Matrix::identity(4);
	for (int i = 0; i < 3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		res[i][3] = -center[i];
	}
	return res;
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {//给定一个三角形和一个点G，目标是计算重心坐标点G
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	//Vec3 n=cross(Vec3(i),Vec3(j))。其中i为（ABx,ACx,PAx），j为（ABy,ACy,PAy); n为垂直于ij两向量的法向量，n的值为（kβ,kγ,k),α=1-β-γ；点G位置：G=αA+βB+γC。（ABC为pts指向的3个点）
	Vec3f u = cross(s[0], s[1]);
	//result 除以自身Z之前，其Z值可能为负数。
	if (std::abs(u[2]) > 1e-2)
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);//除以Z值以恢复我们要的n:即u(β，γ，1)=(u.x/u.z,u.y/u.z,u.z/u.z).则α=1-(u.x+u.y)/u.z,即return(α,β,γ)
	return Vec3f(-1, 1, 1);//u.z即u[2]。Z是int型，如果绝对值小于1，说明它等于0，此时返回带负号的重心坐标。
}
//包围盒算法，自下而上，自左到右
//三角形（A，B，C）重心坐标P(x,y)=αA+βB+γC；α+β+γ=1；
//α=▲PBC/▲ABC；β=▲PAC/▲ABC；γ=▲PAB/▲ABC
//若计算的三个系数αβγ都≥0，则证明该点再三角形内部。
//包围盒思想：首先，它计算边界盒，它被两个顶点描述：左下角和右上角。为了找到这些角，我们迭代了三角形的所有顶点并且找到最小/最大的坐标。
void triangle(Vec3i* pts, float* zbuffer, TGAImage &image,TGAImage& diffuse, Vec2f* vt, Vec3f* vn) {
	Vec2i bboxmin(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());	//定义包围盒
	Vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);//图片的边界，给屏幕加入了边界框剪裁，减少CPU计算时间（去掉屏幕外的三角形）
	//查找包围盒边界
	for (int i = 0; i < 3; i++) {//遍历三角形的3个顶点,
		for (int j = 0; j < 2; j++) {//进行循环比较找到小的x,y和最大的x,y作为左下角和右上角坐标
			bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {//遍历包围盒中的每一个点
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);//bc_screen是计算出的重心坐标(α,β,γ)
			//（1 - t, t）是点（x，y）相对于线段p0，p1的重心坐标：（x，y）= p0*（1 - t）+ p1 * t。
			//采用三角形光栅化的重心坐标，对于我们要绘制的每一个像素，只需要将其重心坐标乘以我们光栅化的三角形顶点xyz值即可。
			//通过三角形三个顶点的像素位置，通过插值方法得到p点在纹理中的像素位置，从而直接去纹理中找到对应点像素颜色，
			float indx = vt[0][0] * bc_screen.x + vt[1][0] * bc_screen.y + vt[2][0] * bc_screen.z;//xy与uv中的三维对应
			float indy = vt[0][1] * bc_screen.x + vt[1][1] * bc_screen.y + vt[2][1] * bc_screen.z;
			//三角形的纹理坐标，将其插入三角形内，乘以纹理图像的宽度和高度，你会得到你需要放到渲染器中的颜色。
		
		
			float norx = vn[0].x * bc_screen.x + vn[1].x * bc_screen.y + vn[2].x * bc_screen.z;
			float nory = vn[0].y * bc_screen.x + vn[1].y * bc_screen.y + vn[2].y * bc_screen.z;
			float norz = vn[0].z * bc_screen.x + vn[1].z * bc_screen.y + vn[2].z * bc_screen.z;

			Vec3f normal(norx, nory, norz);
			normal.normalize();//光照强度intensity数值需要进行归一化
			float intensity = normal * light_dir;// 光照强度等于光向量和给定三角形的法线的标量乘积。三角形的法线可以简单地计算为其两边的交叉乘积。

			indy = 1 - indy;//翻转y轴
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;//回忆重心坐标判定点的方法。如果重心坐标不在三角形内，则α,β,γ至少有一个为负
			//我们只知道三角形的边，而不知道三角形内部点的 z 的大小。// pts[i][2] 是 z 的值。// bc_screen[i] 分别是(α,β,γ)//计算后就可以得到 z 的值.而z 大的点应该遮住 z 小的点。
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
	//3D转2D最简单算法：1.假设图片大小是 width × height。2.用 zbuffer[width × height] 存储每一个要画像素的 z 值，初始化为无穷小
		//3.输入三角形的三条边，遍历三角形内部的所有点。如果内部点的 z 大于zbuffer 的值更新 zbuffer 并画点。达到在上面的像素覆盖之前像素的目的。4.重复上述操作，直到遍历完所有的三角形
			if (zbuffer[int(P.x + P.y * width)] < P.z) {//我从遍历p.x中的p.y到boxmax.y之间开始迭代并且计算线段对应的z坐标。然后我用当前的(P.x + P.y * width)索引检查我们在数组ybuffer中得到了什么。
				zbuffer[int(P.x + P.y * width)] = P.z;//如果当前的y值比ybuffer中的值更接近相机，那么在屏幕上绘制它并更更新ybuffer。
				//image.set(P.x, P.y, color);//此点着色
				if (intensity < 0)continue;//点积可以是负数。这意味着光可能从多边形后面照过来。如果场景建模很好我们可以丢弃这个三角形。这会快速的剃除看不到的三角，这叫背面剪裁。
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
	//TGAImage类构造函数构造对象image，存储了对象绘制画框属性大小和深度，用于存储像素点信息，
	//RGB为常量3，则image中的数据缓存在100*100*3个char类型数据在data数组中。
	//image.set(52, 41, white);//image调用set方法在像素位置x,y存入RGB信息
	float* zbuffer = new float[width * height]; //为了绘制在2D屏幕上，我们需要使用一个二维数组：用 zbuffer[width × height] 存储每一个要画像素的 z 值，
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());//初始化为无穷小。numeric_limits数值极限;int的最小值;min返回可取的最小值（规范化）

	Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
	Matrix Projection = Matrix::identity(4);
	Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	//Projection[3][2] = -1.f/camera.z;于z轴上的摄像头（重要！）距离原点c，获得相机坐标和投影屏幕的距离c,取倒数和负值作为齐次坐标矩阵的第四行第三列即可作为该变换的透视投影变换矩阵。
	Projection[3][2] = -1.f / (eye - center).norm();//位于z轴上的摄像头时 Projection[3][2] = -1/c //摄像头距离原点c   这里的C是(eye - center).norm() 

	for (int i = 0; i < model->nfaces(); i++) { //文件中获取模型信息装入model后，nfaces表示面的总数，遍历model中的每一个face
		std::vector<int> face = model->face(i);
		std::vector<int> fuv = model->faceuv(i);//faceuv//faceuvs_;//fuv;//UV索引坐标
		std::vector<int> vnfaces = model->vnfaces(i);
		Vec3i pts[3];//这是一个三角形
		Vec2f vt_coords[3];
		Vec3f world_coords[3];
		Vec3f vn_coords[3];
		for (int j = 0; j < 3; j++) {//将每一个面的3个顶点构成的3条线段进行绘制，
			Vec3f v = model->vert(face[j]);//Vec3f 是一种存储三维浮点型数据的结构。
			//Vec3f v1 = model->vert(face[(j + 1) % 3]);每一个face内存储有3个坐标vert分别为x,y,z,范围为-1到1并且因为是以原点为中心坐标，所以需要进行位移将图像摆放到image中间位置。
			//对于三角形的每一个顶点v，都要执行MVP变换，第一步将模型转为世界M，第二步根据相机位置变换世界坐标系V，第三步根据屏幕位置进行投影变换P，从而将世界坐标系转换为屏幕坐标系，再执行着色
			//ModelView/lookat/第一步将模型坐标转换为世界坐标/矩阵模型转为相机（眼睛坐标）//只需在模型帧中得到坐标为(x,y,z,1)的任意点，乘以矩阵ModelView，就可以得到相机坐标系中的坐标。
			//ViewPort/第三步根据屏幕位置进行投影变换，从而将世界坐标系转换为屏幕坐标系/视口缩放和偏移 取代screen_coords[j] /将剪贴坐标转化为画面坐标
			//Projection/第二步根据相机位置变换世界坐标系/(-1/c)对场景进行变形，产生透视变形，这个矩阵将场景变换成所谓的剪辑坐标
			pts[j] = m2v(ViewPort * Projection * v2m(v));//一个面的三个顶点在视口上的位置 world2screen(model->vert(face[i]));////世界坐标系：升维，投影变换，视窗变换，降维：视窗坐标系
			world_coords[j] = v;

			Vec2f w = model->vt(fuv[j]);
			vt_coords[j] = w;

			Vec3f n = model->vn(vnfaces[j]);
			vn_coords[j] = n;

		}
		//Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));//计算世界坐标中某个三角形的法线（法线 = 三角形任意两条边做叉乘）即法向量需用三角形顶点的世界坐标来计算
		//n.normalize();// 对 n 做归一化处理
	//	float intensity = n * light_dir;	// 三角形法线和光照方向做点乘，点乘值大于 0，说明法线方向和光照方向在同一侧，值越大，说明越多的光照射到三角形上，颜色越白

		triangle(pts, zbuffer,image, diffuse, vt_coords, vn_coords);
	}

	image.flip_vertically(); //垂直翻转。沿着中水平轴翻转的，即反转y轴，左上角顶点现在调动到左下角
	image.write_tga_file("african_head.tga");//write_tga_file 将data数组输出到tga文件

	{ // dump z-buffer (debugging purposes only)
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {//行
			for (int j = 0; j < height; j++) {//列
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));// zbuffer[i + j * width]，自左到右，自下到上，一行行扫上去，zbuffer是一维数组
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("zbuffer.tga");
	}
	delete model;
	delete[]zbuffer;
	return 0;
}