#include <vector>
#include <iostream>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f      eye(1, 1, 3);// eye(0,-1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct Shader : public IShader {//用抽象的类IShader作为着色器和光栅器之间的一个中间件。

    mat<2, 3, float> varying_uv;//创建一个2x3矩阵。2行代表u和v，3列（每个顶点一个）。存储当前三角形三个顶点的像素点坐标
    mat<4, 4, float> uniform_M;// Projection*ModelView
    mat<4, 4, float> uniform_MIT;// (Projection*ModelView).invert_transpose()
    mat<3, 3, float> varying_nrm;
    mat<3, 3, float> ndc_tri;//in normalized device coordinates标准设备坐标的三角形triangle 

   // Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    //顶点着色器的主要目标是转换顶点的坐标。次要目标是为片段着色器准备数据。
    virtual Vec4f vertex(int iface, int nthvert) {//顶点着色器的主要目标是转换顶点的坐标。第二个目标是为片段着色器准备数据。例如：光照信息。
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));//加入纹理-获取顶点的像素坐标映射i=iface; i--; rows[i][nthvert]=v[i]

        varying_nrm.set_col(nthvert, proj<3>(uniform_MIT * embed<4>(model->normal(iface, nthvert), 0.f)));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); //从obj中读取顶点，embed<4>进行升维
      
       gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     //MVP过程，P点（当前点）由模型中的世界坐标转为屏幕坐标
       // varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); //得到漫反射光的光照强度：此点的法线归一化*光照方向
       ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
    }
    //片段着色器的主要目标是确定当前像素的颜色，第二个目标是可以通过返回true来丢弃当前像素。
    virtual bool fragment(Vec3f bar, TGAColor &color) {
      //  float intensity = varying_intensity*bar;   //当前像素的光照强度*重心坐标用于varying光照数据的插值。
      //，在一个不同的坐标空间中进行光照，这个坐标空间里，法线贴图向量总是指向这个坐标空间的正z方向；所有的光照向量都相对与这个正z方向进行变换。这样我们就能始终使用同样的法线贴图，不管朝向问题。这个坐标空间叫做切线空间（tangent space）。
      // 用一个三角形的顶点和纹理坐标（因为纹理坐标和切线向量在同一空间中）计算出切线和副切线
            //向量信息在模型空间进行，到使用3D引擎时候还需经过MVP甚至其他特异矩阵变换。
        //顶点坐标随意变换还是那个顶点。法向量不同，随便一个非等比例缩放变换甚至更奇怪的异型变换，就可能导致法向量错误
         //总结一下就是顶点经过一系列矩阵变换后，其法向量需要经过顶点变换矩阵的转置逆矩阵（或者逆转置矩阵）同步处理，就能得到新的正确的法向量。
        //切线坐标系即TBN 坐标系建立在模型表面之上，N表示模型表面的法线，T则是与该法线垂直的切线 B是与T、B都垂直的副法线
        //（理论上可选任一与N垂直的切线，，但模型一般会给定该顶点的一个tangent，这个tangent方向一般是使用和纹理坐标方向相同的那条tangent（T））
        Vec2f uv = varying_uv * bar;//加入纹理， bar存储重心坐标
            //简单来说，全局坐标法线贴图存的是世界坐标轴的数值，切线坐标法线贴图存储的是该顶点的相对空间坐标轴数值，
        Vec3f bn = (varying_nrm * bar).normalize();
        
        mat<3, 3,float> A;//定义 3x3矩阵A，乘以未知的矢量 x=（A、B、C），给出向量 b = （f1-f0，f2-f0，0）。当我们以 b 乘以 A 时， 未知的矢量 x 就会为人所知。
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);// (p1 - p0)对应(p0p1)向量 // A[0][0]/A[0][1]/A[0][2]对应x、y、z三个分量
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;// bn表示的是原始法线向量n binormal
        mat<3, 3, float>AI = A.invert();  // AI是A的逆矩阵
        //二维法线方法计算：求u和v两个方向的切线，转换成法线，归一化。
        //Darboux 基础是三重向量（i，j，n），其中 n - 是原始的正常向量，i，j 可以计算如下：i=AI*((u1-u0),(u2-u0),0),于是我们有了所有的切线基础
        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0); // (varying_uv[0][1] - varying_uv[0][0]) 表示(u1 - u0)，(u2 - u0)同理
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);// (varying_uv[1][1] - varying_uv[1][0]) 表示(v1 - v0)，(v2 - v0)同理
        mat < 3, 3, float> B;// 改变为Darboux坐标系的基准
        B[0] = i.normalize();
        B[1] = j.normalize();
        B[2] = bn;
        B.invert_transpose();
        Vec3f n = (B * model->normal(uv)).normalize();// 新的法线向量n(Darboux框架)，即将纹理的法线转换为切线空间
        

       // Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();//获取法线  
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();//光向量    l=saturate(dot(N,normalize(lightVec.xyz))) * lightcolor
        Vec3f r = (n * (n * l * 2.f) - l).normalize();//反射光   *2-1转换为范围[-1,1]      
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));//高光贴图中拿取(uv[0], uv[1])[0]/1.f;求反射光强度
        float diff = std::max(0.f, n * l);//计算漫反射
        color =  model->diffuse(uv);//贴材质
        //根据Phong模型，相机观察某一个物体表面像素点，一共能观察到镜面反射、漫反射、环境光照三种光照。
        for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + color[i] * (diff + .6 * spec), 255);//对环境分量取5，漫射分量取1，镜面分量取0.6。 std::min<float>使值不超过255

       // float intensity = std::max(0.f, n * l);
      // color = model->diffuse(uv) * intensity;//加入纹理
     //  color = TGAColor(255, 155, 0)*intensity; //算色
        return false;                              // bool，所以返回false。（不丢弃当前像素）
    }
};

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);//读取模型
    } else {
        model = new Model("obj/african_head.obj");
    }

    lookat(eye, center, up);//初始化相机位置
    viewport(width/8, height/8, width*3/4, height*3/4);//初始化视窗
    projection(-1.f/(eye-center).norm());//根据相机位置初始化投影变换矩阵
    light_dir.normalize();//归一化光照向量

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

   // GouraudShader shader;
    Shader shader;//Uniform是GLSL中的一个保留关键字，它允许向着色器传递常量。这里我递了矩阵 Projection*ModelView 和它的反转置来变换法向量
    shader.uniform_M = Projection * ModelView;//传递光向量
    shader.uniform_MIT = (Projection * ModelView).invert_transpose();//法向量
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
       
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);//进入顶点着色器

        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.  write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
/*
* 

//加入法线
struct Shader : public IShader {//用抽象的类IShader作为着色器和光栅器之间的一个中间件。

    mat<2, 3, float> varying_uv;//创建一个2x3矩阵。2行代表u和v，3列（每个顶点一个）。存储当前三角形三个顶点的像素点坐标
    mat<4, 4, float> uniform_M;
    mat<4, 4, float> uniform_MIT;

    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    //顶点着色器的主要目标是转换顶点的坐标。次要目标是为片段着色器准备数据。
    virtual Vec4f vertex(int iface, int nthvert) {//顶点着色器的主要目标是转换顶点的坐标。第二个目标是为片段着色器准备数据。例如：光照信息。
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));//加入纹理-取uv值 i=iface; i--; rows[i][nthvert]=v[i]
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); //从obj中读取顶点，embed<4>进行升维
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     //MVP过程，P点（当前点）由模型中的世界坐标转为屏幕坐标
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); //得到漫反射光的光照强度：此点的法线归一化*光照方向

        return gl_Vertex;
    }
    //片段着色器的主要目标是确定当前像素的颜色，第二个目标是可以通过返回true来丢弃当前像素。
    virtual bool fragment(Vec3f bar, TGAColor &color) {
      //  float intensity = varying_intensity*bar;   //当前像素的光照强度*重心坐标用于varying光照数据的插值。

        Vec2f uv = varying_uv * bar;//加入纹理
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();//法向量
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();//光向量
        float intensity = std::max(0.f, n * l);
       color = model->diffuse(uv) * intensity;//加入纹理
     //  color = TGAColor(255, 155, 0)*intensity; //算色
        return false;                              // bool，所以返回false。（不丢弃当前像素）
    }
};

* 
* 
* 
#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f      eye(1, 1, 3);// eye(0,-1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct GouraudShader : public IShader {//用抽象的类IShader作为着色器和光栅器之间的一个中间件。


    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    //顶点着色器的主要目标是转换顶点的坐标。次要目标是为片段着色器准备数据。
    virtual Vec4f vertex(int iface, int nthvert) {//顶点着色器的主要目标是转换顶点的坐标。第二个目标是为片段着色器准备数据。例如：光照信息。
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); //从obj中读取顶点，embed<4>进行升维
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     //MVP过程，P点（当前点）由模型中的世界坐标转为屏幕坐标
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); //得到漫反射光的光照强度：此点的法线归一化*光照方向
        return gl_Vertex;
    }
    //片段着色器的主要目标是确定当前像素的颜色，第二个目标是可以通过返回true来丢弃当前像素。
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   //当前像素的光照强度*重心坐标用于varying光照数据的插值。
       // 把强度改成6个阶段：
    if (intensity > .85) intensity = .80;
    else if (intensity > .60) intensity = .60;
    else if (intensity > .45) intensity = .45;
    else if (intensity > .30) intensity = .30;
    else intensity = 0;
       //
           color = TGAColor(255, 155, 0) * intensity; //算色
       return false;                              // bool，所以返回false。（不丢弃当前像素）
    }
};

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);//读取模型
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    lookat(eye, center, up);//初始化相机位置
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);//初始化视窗
    projection(-1.f / (eye - center).norm());//根据相机位置初始化投影变换矩阵
    light_dir.normalize();//归一化光照向量

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    GouraudShader shader;
    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(i, j);//进入顶点着色器
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}


*/