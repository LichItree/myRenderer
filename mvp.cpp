#include "mvp.h"
#include <limits>
mat<4,4> mvpTransform::get_mvp_Mat(){
  // std::cout<<view;
    //std::cout<<model;
    mvp =  model;
    //mvp = projection * view * model;
    //std::cout<<mvp;
    return mvp;
}
mvpTransform::mvpTransform(){
    for (int i=4; i--; )
        for (int j=4;j--; mvp[i][j]=(i==j));
   view = mat<4,4>::identity();
   model = mat<4,4>::identity();
   projection = mat<4,4>::identity();
}

constexpr double MY_PI = 3.1415926;
void mvpTransform::set_view(vec3 eye_pos){
    mat<4,4> translate = mat<4,4>::identity();
    vec4 eye = {-eye_pos[0],-eye_pos[1],-eye_pos[2],1};
    translate.set_col(3,eye);
    view = translate * view;
}
void mvpTransform:: set_model(float rotation_angle){
     float randian = rotation_angle/180.0*acos(-1);
     mat<4,4> model_trans = mat<4,4>::identity();
     model_trans.set_row(0,{float(cos(randian)),float(-sin(randian)),0,0});
     model_trans.set_row(1,{float(sin(randian)),float(cos(randian)),0,0});
     model=model_trans*model;
}

void mvpTransform:: set_projection(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar){
    mat<4,4> trans_proj = mat<4,4>::identity();
    trans_proj[0][0] = zNear;
    trans_proj[1][1] = zNear;
    trans_proj.set_row(2,{ 0,0,zNear+zFar,-zFar*zNear});
    trans_proj.set_row(3,{ 0,0,1,0});

    mat<4,4> trans_orth = mat<4,4>::identity();;
    float t =1/(tan(eye_fov/2)*fabsf(zNear));
    float r = float(1.0/t*aspect_ratio);
    trans_orth[0][0] = r;
    trans_orth[1][1] = t;
    trans_orth.set_row(2,{0,0,2/(zNear-zFar),0});

    projection =trans_orth*trans_proj*projection;
}