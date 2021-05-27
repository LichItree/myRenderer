#include "geometry.h"
class mvpTransform{
  public:
    mvpTransform();
    void set_model(float rotation_angle);
    void set_view(vec3 eye_pos);
    void set_projection(float eye_fov, float aspect_ratio,float zNear, float zFar);
    mat<4,4> get_mvp_Mat();
  private:
    mat<4,4> model;
    mat<4,4> view;
    mat<4,4> projection;
    mat<4,4> mvp;
};