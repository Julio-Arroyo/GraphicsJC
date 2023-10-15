#ifndef GEOMETRIC_TRANSFORMATIONS_HPP
#define GEOMETRIC_TRANSFORMATIONS_HPP

#include <cmath>
#include "Objects.hpp"
#include "Util.hpp"
#include "Eigen"
#include "Dense"

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

void makeMatrix(Eigen::Matrix4d& m, std::string& line) {
    std::string param_header;
    double buffer[4];

    int param_count = parse_parameter_str(line, param_header, buffer, 4);
    assert(-1 != param_count);
    assert(param_header.length() == 1);

    switch ((char) param_header[0]) {
        case Type::TRANSLATION_MAT:
            assert(param_count == 3);
            makeTranslationMat(m, buffer[0], buffer[1], buffer[2]);
            break;
        case Type::ROTATION_MAT:
            assert(param_count == 4);
            makeRotationMat(m, buffer[0], buffer[1], buffer[2], buffer[3]);
            break;
        case Type::SCALING_MAT:
            assert(param_count == 3);
            makeScalingMat(m, buffer[0], buffer[1], buffer[2]);
            break;
    }
}


/** @brief Converts between world and camera coordinates. */
void makeWorldToCameraProj(Eigen::Matrix4d& m, Camera& camera) {
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
void makePerspectiveProjection(Eigen::Matrix4d& perspectiveProj) {
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

#endif
