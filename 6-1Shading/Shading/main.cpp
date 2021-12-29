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

struct Shader : public IShader {//�ó������IShader��Ϊ��ɫ���͹�դ��֮���һ���м����

    mat<2, 3, float> varying_uv;//����һ��2x3����2�д���u��v��3�У�ÿ������һ�������洢��ǰ������������������ص�����
    mat<4, 4, float> uniform_M;// Projection*ModelView
    mat<4, 4, float> uniform_MIT;// (Projection*ModelView).invert_transpose()

    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    //������ɫ������ҪĿ����ת����������ꡣ��ҪĿ����ΪƬ����ɫ��׼�����ݡ�
    virtual Vec4f vertex(int iface, int nthvert) {//������ɫ������ҪĿ����ת����������ꡣ�ڶ���Ŀ����ΪƬ����ɫ��׼�����ݡ����磺������Ϣ��
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));//��������-��ȡ�������������ӳ��i=iface; i--; rows[i][nthvert]=v[i]
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); //��obj�ж�ȡ���㣬embed<4>������ά
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     //MVP���̣�P�㣨��ǰ�㣩��ģ���е���������תΪ��Ļ����
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); //�õ��������Ĺ���ǿ�ȣ��˵�ķ��߹�һ��*���շ���

        return gl_Vertex;
    }
    //Ƭ����ɫ������ҪĿ����ȷ����ǰ���ص���ɫ���ڶ���Ŀ���ǿ���ͨ������true��������ǰ���ء�
    virtual bool fragment(Vec3f bar, TGAColor &color) {
      //  float intensity = varying_intensity*bar;   //��ǰ���صĹ���ǿ��*������������varying�������ݵĲ�ֵ��
            //������Ϣ��ģ�Ϳռ���У���ʹ��3D����ʱ���辭��MVP���������������任��
        //������������任�����Ǹ����㡣��������ͬ�����һ���ǵȱ������ű任��������ֵ����ͱ任���Ϳ��ܵ��·���������
         //�ܽ�һ�¾��Ƕ��㾭��һϵ�о���任���䷨������Ҫ��������任�����ת������󣨻�����ת�þ���ͬ���������ܵõ��µ���ȷ�ķ�������
        //��������ϵ��TBN ����ϵ������ģ�ͱ���֮�ϣ�N��ʾģ�ͱ���ķ��ߣ�T������÷��ߴ�ֱ������ B����T��B����ֱ�ĸ�����
        //�������Ͽ�ѡ��һ��N��ֱ�����ߣ�����ģ��һ�������ö����һ��tangent�����tangent����һ����ʹ�ú��������귽����ͬ������tangent��T����
        Vec2f uv = varying_uv * bar;//�������� bar�洢��������
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();//��ȡ����  
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();//������    
        Vec3f r = (n * (n * l * 2.f) - l).normalize();//�����   *2-1ת��Ϊ��Χ[-1,1]      
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));//�߹���ͼ����ȡ(uv[0], uv[1])[0]/1.f;�����ǿ��
        float diff = std::max(0.f, n * l);
        color =  model->diffuse(uv);//������
        //����Phongģ�ͣ�����۲�ĳһ������������ص㣬һ���ܹ۲쵽���淴�䡢�����䡢�����������ֹ��ա�
        for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + color[i] * (diff + .6 * spec), 255);//�Ի�������ȡ5���������ȡ1���������ȡ0.6�� std::min<float>ʹֵ������255

       // float intensity = std::max(0.f, n * l);
      // color = model->diffuse(uv) * intensity;//��������
     //  color = TGAColor(255, 155, 0)*intensity; //��ɫ
        return false;                              // bool�����Է���false������������ǰ���أ�
    }
};

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);//��ȡģ��
    } else {
        model = new Model("obj/african_head.obj");
    }

    lookat(eye, center, up);//��ʼ�����λ��
    viewport(width/8, height/8, width*3/4, height*3/4);//��ʼ���Ӵ�
    projection(-1.f/(eye-center).norm());//�������λ�ó�ʼ��ͶӰ�任����
    light_dir.normalize();//��һ����������

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

   // GouraudShader shader;
    Shader shader;//Uniform��GLSL�е�һ�������ؼ��֣�����������ɫ�����ݳ����������ҵ��˾��� Projection*ModelView �����ķ�ת�����任������
    shader.uniform_M = Projection * ModelView;//���ݹ�����
    shader.uniform_MIT = (Projection * ModelView).invert_transpose();//������
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
       
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);//���붥����ɫ��

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

//���뷨��
struct Shader : public IShader {//�ó������IShader��Ϊ��ɫ���͹�դ��֮���һ���м����

    mat<2, 3, float> varying_uv;//����һ��2x3����2�д���u��v��3�У�ÿ������һ�������洢��ǰ������������������ص�����
    mat<4, 4, float> uniform_M;
    mat<4, 4, float> uniform_MIT;

    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    //������ɫ������ҪĿ����ת����������ꡣ��ҪĿ����ΪƬ����ɫ��׼�����ݡ�
    virtual Vec4f vertex(int iface, int nthvert) {//������ɫ������ҪĿ����ת����������ꡣ�ڶ���Ŀ����ΪƬ����ɫ��׼�����ݡ����磺������Ϣ��
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));//��������-ȡuvֵ i=iface; i--; rows[i][nthvert]=v[i]
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); //��obj�ж�ȡ���㣬embed<4>������ά
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     //MVP���̣�P�㣨��ǰ�㣩��ģ���е���������תΪ��Ļ����
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); //�õ��������Ĺ���ǿ�ȣ��˵�ķ��߹�һ��*���շ���

        return gl_Vertex;
    }
    //Ƭ����ɫ������ҪĿ����ȷ����ǰ���ص���ɫ���ڶ���Ŀ���ǿ���ͨ������true��������ǰ���ء�
    virtual bool fragment(Vec3f bar, TGAColor &color) {
      //  float intensity = varying_intensity*bar;   //��ǰ���صĹ���ǿ��*������������varying�������ݵĲ�ֵ��

        Vec2f uv = varying_uv * bar;//��������
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();//������
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();//������
        float intensity = std::max(0.f, n * l);
       color = model->diffuse(uv) * intensity;//��������
     //  color = TGAColor(255, 155, 0)*intensity; //��ɫ
        return false;                              // bool�����Է���false������������ǰ���أ�
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

struct GouraudShader : public IShader {//�ó������IShader��Ϊ��ɫ���͹�դ��֮���һ���м����


    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    //������ɫ������ҪĿ����ת����������ꡣ��ҪĿ����ΪƬ����ɫ��׼�����ݡ�
    virtual Vec4f vertex(int iface, int nthvert) {//������ɫ������ҪĿ����ת����������ꡣ�ڶ���Ŀ����ΪƬ����ɫ��׼�����ݡ����磺������Ϣ��
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); //��obj�ж�ȡ���㣬embed<4>������ά
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     //MVP���̣�P�㣨��ǰ�㣩��ģ���е���������תΪ��Ļ����
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); //�õ��������Ĺ���ǿ�ȣ��˵�ķ��߹�һ��*���շ���
        return gl_Vertex;
    }
    //Ƭ����ɫ������ҪĿ����ȷ����ǰ���ص���ɫ���ڶ���Ŀ���ǿ���ͨ������true��������ǰ���ء�
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   //��ǰ���صĹ���ǿ��*������������varying�������ݵĲ�ֵ��
       // ��ǿ�ȸĳ�6���׶Σ�
    if (intensity > .85) intensity = .80;
    else if (intensity > .60) intensity = .60;
    else if (intensity > .45) intensity = .45;
    else if (intensity > .30) intensity = .30;
    else intensity = 0;
       //
           color = TGAColor(255, 155, 0) * intensity; //��ɫ
       return false;                              // bool�����Է���false������������ǰ���أ�
    }
};

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);//��ȡģ��
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    lookat(eye, center, up);//��ʼ�����λ��
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);//��ʼ���Ӵ�
    projection(-1.f / (eye - center).norm());//�������λ�ó�ʼ��ͶӰ�任����
    light_dir.normalize();//��һ����������

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    GouraudShader shader;
    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(i, j);//���붥����ɫ��
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