#ifndef TYPES_HPP
#define TYPES_HPP

enum Type : char {
    VERTEX = 'v',
    FACE = 'f',
    TRANSLATION_MAT = 't',
    ROTATION_MAT = 'r',
    SCALING_MAT = 's'
};

struct Vertex {
    double x;
    double y;
    double z;
};

struct Face {
    struct IdxTriple {
        int i1, i2, i3;
    };
    IdxTriple v;
    IdxTriple n;
};

struct VertexHomoNDC {
    Vertex v;
    bool inCamera;
};

struct Color {
    double r;
    double g;
    double b;
};

struct Position {
    double x, y, z;
};

struct Orientation {
    double x, y, z, theta;
};

struct Camera {
    Position pos;
    Orientation orientation;
    double near, far, left, right, top, bottom;
};

struct PointLight {
    Position pos;
    Color color;
    float attenuation;
};

#endif

