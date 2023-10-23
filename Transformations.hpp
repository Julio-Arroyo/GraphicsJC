#ifndef GEOMETRIC_TRANSFORMATIONS_HPP
#define GEOMETRIC_TRANSFORMATIONS_HPP

#include <cmath>
#include "Objects.hpp"
#include "Util.hpp"
#include "Eigen"
#include "Dense"

enum TransformationType {
    TRANSLATION,
    ROTATION,
    SCALING
};

void makeTranslationMat(Eigen::Matrix4d& m, double tx, double ty, double tz) {
    m << 1, 0, 0, tx,
         0, 1, 0, ty,
         0, 0, 1, tz,
         0, 0, 0, 1;
}

void makeRotationMat(Eigen::Matrix4d& m, double rx, double ry, double rz, double theta) {
    double rx_sq = rx*rx;
    double ry_sq = ry*ry;
    double rz_sq = rz*rz;
    double cos_th = std::cos(theta);
    double sin_th = std::sin(theta);

    m << rx_sq + (1-rx_sq)*cos_th    , rx*ry*(1-cos_th) - rz*sin_th, rx*rz*(1-cos_th) + ry*sin_th, 0,
         ry*rx*(1-cos_th) + rz*sin_th, ry_sq + (1-ry_sq)*cos_th    , ry*rz*(1-cos_th) - rx*sin_th, 0,
         rz*rx*(1-cos_th) - ry*sin_th, rz*ry*(1-cos_th) + rx*sin_th, rz_sq + (1-rz_sq)*cos_th    , 0,
         0                           , 0                           , 0                           , 1;
}

void makeScalingMat(Eigen::Matrix4d& m, double sx, double sy, double sz) {
    m << sx, 0,  0,  0,
         0,  sy, 0,  0,
         0,  0,  sz, 0,
         0,  0,  0,  1;
}

Type makeMatrix(Eigen::Matrix4d& m, std::string& line) {
    std::string param_header;
    double buffer[4];

    int param_count = parse_parameter_str(line, param_header, buffer, 4);
    assert(-1 != param_count);
    assert(param_header.length() == 1);

    switch ((char) param_header[0]) {
        case Type::TRANSLATION_MAT:
            assert(param_count == 3);
            makeTranslationMat(m, buffer[0], buffer[1], buffer[2]);
            return Type::TRANSLATION_MAT;
        case Type::ROTATION_MAT:
            assert(param_count == 4);
            makeRotationMat(m, buffer[0], buffer[1], buffer[2], buffer[3]);
            return Type::ROTATION_MAT;
        case Type::SCALING_MAT:
            assert(param_count == 3);
            makeScalingMat(m, buffer[0], buffer[1], buffer[2]);
            return Type::SCALING_MAT;
        default:
            assert(false);
    }
}


/** @brief Converts between world and camera coordinates. */
void makeWorldToCameraProj(Eigen::Matrix4d& m, const Camera& camera) {
    Eigen::Matrix4d Tc, Rc;  // camera position, rotation transformations respectively
    makeTranslationMat(Tc, camera.pos.x, camera.pos.y, camera.pos.z);
    makeRotationMat(Rc, camera.orientation.x,
                        camera.orientation.y,
                        camera.orientation.z,
                        camera.orientation.theta);

    Eigen::Matrix4d prod = Tc*Rc;
    m = prod.inverse();
}


/** @brief Makes the projection matrix from camera space to homogeneous
 *         Normalized Device Coordinates (NDC). */
void makePerspectiveProjection(Eigen::Matrix4d& perspectiveProj,
                               const Camera& camera)
{
    double m00, m11, m22, m02, m12, m23;

    m00 = 2.0*camera.near / (camera.right - camera.left);
    m02 = ((double) camera.right + camera.left) / (camera.right - camera.left);
    m11 = 2.0*camera.near / (camera.top - camera.bottom);
    m12 = ((double) camera.top + camera.bottom) / (camera.top - camera.bottom);
    m22 = ((double) - (camera.far + camera.near)) / (camera.far - camera.near);
    m23 = (-2.0 * camera.far * camera.near) / (camera.far - camera.near);

    perspectiveProj << m00,   0, m02,   0,
                         0, m11, m12,   0,
                         0,   0, m22, m23,
                         0,   0,   -1,  0;
}

inline Vertex worldToNDC(Eigen::Matrix4d& worldToHomoNDC, Vertex& v) {
    // World Space
    Eigen::Vector4d V_ws;
    V_ws << v.x, v.y, v.z, 1;

    // World space --> homogeneous NDC
    Eigen::Vector4d V_hndc = worldToHomoNDC*V_ws;
    // std::cout << "V_hndc " << V_hndc << std::endl;
    double x_hndc = V_hndc(0);
    double y_hndc = V_hndc(1);
    double z_hndc = V_hndc(2);
    double w_hndc = V_hndc(3);

    // homogeneous NDC --> cartesian NDC
    return {x_hndc/w_hndc, y_hndc/w_hndc, z_hndc/w_hndc};
}

inline std::pair<int, int> NDCtoScreen(Vertex& v1, size_t xres, size_t yres) {
    int v1_x = (int) (((v1.x - (-1)) / (1 - (-1))) * xres);
    int v1_y = (int) (((v1.y - (-1)) / (1 - (-1))) * yres);
    // if (std::abs(v1_x) > xres) {
    //     std::cout << "v1_x " << v1_x << std::endl;
    // }
    // if (std::abs(v1_y) > yres) {
    //     std::cout << "v1_y " << v1_y << std::endl;
    // }
    v1_x = std::min(std::max(0, v1_x), (int32_t) xres-1);
    v1_y = std::min(std::max(0, v1_y), (int32_t) yres-1);
    return std::make_pair(v1_x, v1_y);
}

bool isBackFacing(Vertex& v1, Vertex& v2, Vertex& v3) {
    double ux = v3.x - v1.x;
    double uy = v3.y - v1.y;
    double rx = v1.x - v2.x;
    double ry = v1.y - v2.y;
    return ux*ry - uy*rx < 0;
     Eigen::Vector3d V1, V2, V3;
     V1 << v1.x, v1.y, v1.z;
     V2 << v2.x, v2.y, v2.z;
     V3 << v3.x, v3.y, v3.z;

     Eigen::Vector3d cross = (V3-V1).cross(V1-V2);
     return cross(2) < 0;
}

double f_ij(double x, double y, double xi, double yi, double xj, double yj) {
    // DEBUG
    // std::cout << "x" << x << std::endl;
    // std::cout << "xj" << xj << std::endl;
    return (yi - yj)*x + (xj - xi)*y + xi*yj - xj*yi;
}

inline bool inNDCcube(Vertex& v) {
    return -1 <= v.x && v.x <= 1 &&
           -1 <= v.y && v.y <= 1 &&
           -1 <= v.z && v.z <= 1;
}

#endif
