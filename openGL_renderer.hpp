#ifndef OPENGL_RENDERER_HPP
#define OPENGL_RENDERER_HPP

#include <cmath>
#include <iostream>
#include <vector>

#include "GL/glew.h"
#include "GL/glut.h"
#include "Scene.hpp"
#include "Quaternion.hpp"

/*
struct PointLight {
    float position[4];  // (x,y,z,w)
    float color[3];
    float attenuation;
};


struct Orientation {
    float x;
    float y;
    float z;
    float theta;
};

struct Camera {
    Float3 position;
    Orientation orientation;
    float n;
    float f;
    float l;
    float r;
    float t;
    float b;
};

struct Transforms
{
    * For each array below,
     * Index 0 has the x-component
     * Index 1 has the y-component
     * Index 2 has the z-component
    float translation[3];
    float rotation[3];
    float scaling[3];
    
    * Angle in degrees.
    float rotation_angle;
};
*/


/* 'y_view_angle' and 'x_view_angle' are parameters for our mouse user
 * interface. They keep track of how much the user wants to rotate the
 * camera from its default, specified orientation. See the 'mouse_moved'
 * function for more details.
 */
// float x_view_angle{0};
// float y_view_angle{0};
Quaternion lastRotation;
Quaternion currentRotation;

// Camera parameters
float cam_orientation_angle{0};
float cam_orientation_axis[3]{0, 0, 0};
float cam_position[3]{0, 0, 0};
float cam_left{0};
float cam_right{0};
float cam_bottom{0};
float cam_top{0};
float cam_near{0};
float cam_far{0};

float mouse_scale_x, mouse_scale_y;
int mouse_x, mouse_y;
bool is_pressed{false};

const float x_view_step = 90.0, y_view_step = 90.0;

std::vector<PointLight> lights;
std::vector<std::shared_ptr<Object>> objects;

void extract_parameters(Scene& scene);
void init();
void init_lights();
void display();
void reshape(int width, int height);
void mouse_pressed(int button, int state, int x, int y);
void mouse_moved(int x, int y);
// void key_pressed();

#endif

