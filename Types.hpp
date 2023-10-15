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
    int v1Idx;
    int v2Idx;
    int v3Idx;
};

struct VertexHomoNDC {
    Vertex v;
    bool inCamera;
};

struct Color {
    float r;
    float g;
    float b;
};

struct Camera {
    struct Position {
        double x, y, z;
    };
    struct Orientation {
        double x, y, z, theta;
    };

    Position pos;
    Orientation orientation;
    double near, far, left, right, top, bottom;
};

#endif

