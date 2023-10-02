#ifndef GEOMETRIC_TRANSFORMATIONS_HPP
#define GEOMETRIC_TRANSFORMATIONS_HPP

#include <cmath>
#include <sstream>
#include "Objects.hpp"
#include "Eigen/Eigen"
#include "Eigen/Dense"

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
    std::istringstream ss{line};
    std::string token;
    ss >> token;
    assert(token.length() == 1);
    char matType = (char) token[0];

    double buffer[4];
    int count = 0;
    ss >> token;
    while (ss) {
        buffer[count++] = std::stof(token);
        ss >> token;
    }
    
    switch (matType) {
        case Type::TRANSLATION_MAT:
            assert(count == 3);
            makeTranslationMat(m, buffer[0], buffer[1], buffer[2]);
            break;
        case Type::ROTATION_MAT:
            assert(count == 4);
            makeRotationMat(m, buffer[0], buffer[1], buffer[2], buffer[3]);
            break;
        case Type::SCALING_MAT:
            assert(count == 3);
            makeScalingMat(m, buffer[0], buffer[1], buffer[2]);
            break;
    }
}

#endif
