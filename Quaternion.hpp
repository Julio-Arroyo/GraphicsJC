#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include <cmath>
#include "Eigen"

struct Float3 {
    float x;
    float y;
    float z;
};

class Quaternion {
public:
    Quaternion() {
        s = 1;
        v = {0, 0, 0};
    }

    Quaternion(float s_, Float3 v_) {
        s = s_;
        v = {v_.x, v_.y, v_.z};
    }
    
    Quaternion(float x_current, float y_current, float x_start, float y_start) {
        float z_current;
        float temp_current = x_current*x_current + y_current*y_current;
        if (temp_current <= 1) {
            z_current = std::sqrt(1 - temp_current);
        } else {
            z_current = 0;
        }

        float z_start;
        float temp_start = x_start*x_start + y_start*y_start;
        if (temp_start <= 1) {
            z_start = std::sqrt(1 - temp_start);
        } else {
            z_start = 0;
        }

        Eigen::Vector3f p_start;
        p_start << x_start, y_start , z_start;
        Eigen::Vector3f p_curr;
        p_curr << x_current, y_current, z_current;

        float num = p_start.dot(p_curr);
        float denom = (p_start.norm())*(p_curr.norm());
        float theta = std::acos(std::min((float) 1.0, num/denom));

        Eigen::Vector3f u = p_start.cross(p_curr);
        u.normalize();

        s = std::cos(theta/2);
        v.x = std::sin(theta/2) * u(0);
        v.y = std::sin(theta/2) * u(1);
        v.z = std::sin(theta/2) * u(2);
    }

    Quaternion operator*(const Quaternion& other) {
        Quaternion ans;
        ans.s = s*other.s - (v.x*other.v.x + v.y*other.v.y + v.z*other.v.z);
        ans.v.x = s*other.v.x + other.s*v.x + (v.y*other.v.z - v.z*other.v.y);
        ans.v.y = s*other.v.y + other.s*v.y - (v.x*other.v.z - v.z*other.v.x);
        ans.v.z = s*other.v.z + other.s*v.z + (v.x*other.v.y - v.y*other.v.x);
        return ans;
    }

    float* toMatrix() {
        float qs = s;
        float qx = v.x;
        float qy= v.y;
        float qz = v.z;
        float qx_sq = v.x*v.x;
        float qy_sq = v.y*v.y;
        float qz_sq = v.z*v.z;

        static float mat[16] = {1 - 2*qy_sq - 2*qz_sq, 2*(qx*qy - qz*qs)    , 2*(qx*qz+qy*qs)    , 0,
                         2*(qx*qy + qz*qs)    , 1 - 2*qx_sq - 2*qz_sq, 2*(qy*qz - qx*qs)  , 0,
                         2*(qx*qz - qy*qs)    , 2*(qy*qz + qx*qs)    , 1-2*qx_sq - 2*qy_sq, 0,
                         0                    , 0                    , 0                  , 1};
        return mat;
    }

private:
    float s;
    Float3 v;
};

#endif

