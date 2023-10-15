#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include "Eigen"

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
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class Object {
public:
    Object(std::string& fname, bool printObj = true) {
        vertices.emplace_back();  // dummy vertex for 1-indexing

        std::ifstream file{fname};
        std::string currLine;

        if (printObj) {
            // print header
            std::cout << fname.substr(0, fname.find(".")) << ":"
                      << std::endl << std::endl;
        }

        std::getline(file, currLine);

        while (file) {
            if (printObj) {
                std::cout << currLine << std::endl;
            }

            std::istringstream currStream{currLine};
            std::string token;

            currStream >> token;
            assert(token.length() == 1);  // first token should be header
            char objType = token[0];

            /* Read 3 numbers (which may be either vertex coordinates
                or vertex indices) into buffer */
            int count = 0;
            double buffer[3];
            currStream >> token;
            while (currStream) {
                assert(count < 3);
                buffer[count++] = std::stof(token);
                currStream >> token;
            }
            assert(count == 3);

            // Construct either Face or Vertex
            switch (objType) {
                case Type::FACE: {
                    Face f = {(int) buffer[0],
                              (int) buffer[1],
                              (int) buffer[2]};
                    faces.push_back(f);
                    break;
                }
                case Type::VERTEX: {
                    Vertex v = {buffer[0], buffer[1], buffer[2]};
                    vertices.push_back(v);
                    break;
                }
            }
            std::getline(file, currLine);
        }
        if (printObj) {
            std::cout << std::endl;
        }
    }

    Object(std::string& fname, std::string& label_, bool printObj = true)
        : Object(fname, printObj) 
    {
        label = label_;
    }

    Object(Object& other) {
        vertices = other.vertices;
        faces = other.faces;
        label = other.label + "_copy" + std::to_string(other.getAndIncNumCopies());
    }

    /**
     * TODO
    */
    void fillScreenCoords(std::vector<std::vector<bool>>& screenCoords,
                          size_t xres, size_t yres) {
        for (Face face : faces) {
            Vertex v1 = vertices[face.v1Idx];
            int32_t v1_x = (int32_t) (((v1.x - (-1)) / (1 - (-1))) * xres);
            int32_t v1_y = (int32_t) (((v1.y - (-1)) / (1 - (-1))) * yres);
            v1_x = std::min(std::max(0, v1_x), (int32_t) xres-1);
            v1_y = std::min(std::max(0, v1_y), (int32_t) yres-1);
            
            Vertex v2 = vertices[face.v2Idx];
            int32_t v2_x = (int32_t) (((v2.x - (-1)) / (1 - (-1))) * xres);
            int32_t v2_y = (int32_t) (((v2.y - (-1)) / (1 - (-1))) * yres);
            v2_x = std::min(std::max(0, v2_x), (int32_t) xres-1);
            v2_y = std::min(std::max(0, v2_y), (int32_t) yres-1);

            Vertex v3 = vertices[face.v3Idx];
            int32_t v3_x = (int32_t) (((v3.x - (-1)) / (1 - (-1))) * xres);
            int32_t v3_y = (int32_t) (((v3.y - (-1)) / (1 - (-1))) * yres);
            v3_x = std::min(std::max(0, v3_x), (int32_t) xres-1);
            v3_y = std::min(std::max(0, v3_y), (int32_t) yres-1);

            drawLine(v1_y, v1_x, v2_y, v2_x, screenCoords);
            // std::cout << "RIP: <<" << std::endl;
            drawLine(v1_y, v1_x, v3_y, v3_x, screenCoords);
            drawLine(v2_y, v2_x, v3_y, v3_x, screenCoords);
        }
    }

    void addTransformation(Eigen::Matrix4d& t) {
        transformation = t*transformation;
    }

    /**
     * Applies `transformation` to every vertex in `vertices`, in-place
    */
    void applyTransformation() {
        for (size_t i = 1; i < vertices.size(); i++) {
            // World Space
            Eigen::Vector4d V_ws;
            V_ws << vertices[i].x, vertices[i].y, vertices[i].z, 1;

            // World space --> homogeneous NDC
            Eigen::Vector4d V_hndc = transformation*V_ws;
            double x_hndc = V_hndc(0);
            double y_hndc = V_hndc(1);
            double z_hndc = V_hndc(2);
            double w_hndc = V_hndc(3);

            // homogeneous NDC --> cartesian NDC
            Eigen::Vector4d V_cndc;
            V_cndc << x_hndc/w_hndc, y_hndc/w_hndc, z_hndc/w_hndc, 1;

            vertices[i] = {V_cndc(0), V_cndc(1), V_cndc(2)};
        }
    }

    size_t getAndIncNumCopies() {
        return ++numCopies;
    }

private:
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                  std::vector<std::vector<bool>>& screenCoords) {
        int32_t delta_x = x1 - x0;
        int32_t delta_y = y1 - y0;

        int32_t i_left, j_left, i_right, j_right;  // implementation: i_left <= i_right

        // if slope > 1 ==> flip x and y to make 0<=slope<=1
        bool flip_x_and_y = std::abs(delta_x) < std::abs(delta_y);
        if (flip_x_and_y) {
            if (delta_y >= 0) {
                i_left = y0;
                j_left = x0;
                i_right = y1;
                j_right = x1;
            } else {
                i_left = y1;
                j_left = x1;
                i_right = y0;
                j_right = x0;
            }
        } else {
            if (delta_x >= 0) {
                i_left = x0;
                j_left = y0;
                i_right = x1;
                j_right = y1;
            } else {
                i_left = x1;
                j_left = y1;
                i_right = x0;
                j_right = y0;
            }
        }

        if (j_left <= j_right) {
            rasterizeLinePosSlope(i_left, j_left, i_right, j_right, screenCoords, flip_x_and_y);
        } else {
            rasterizeLineNegSlope(i_left, j_left, i_right, j_right, screenCoords, flip_x_and_y);
        }

    }

    void rasterizeLineNegSlope(int32_t i0, int32_t j0, int32_t i1, int32_t j1,
                               std::vector<std::vector<bool>>& screenCoords,
                               bool flipped_x_and_y) {
        int32_t eps = 0;
        int32_t y = j0;
        int32_t neg_delta_x = i0 - i1;
        assert(neg_delta_x <= 0);
        int32_t delta_y = j1 - j0;
        assert(delta_y < 0);

        // special case
        if (i0 == i1) {
            for (int32_t y = j0; y <= j1; y++) {
                // TODO: flip y to y_max - y
                if (flipped_x_and_y) {
                    screenCoords[i0][y] = true;
                } else {
                    screenCoords[y][i0] = true;
                }
            }
            return;
        }

        for (int32_t x = i0; x <= i1; x++) {
            if (flipped_x_and_y) {
                screenCoords[x][y] = true;
            } else {
                screenCoords[y][x] = true;
            }

            int32_t val = (eps + delta_y) << 1;
            if (val < neg_delta_x) {
                eps += delta_y - neg_delta_x;  // delta_y + delta_x
                y--;
            } else {
                eps += delta_y;
            }
        }
    }

    void rasterizeLinePosSlope(int32_t i0, int32_t j0, int32_t i1, int32_t j1,
                               std::vector<std::vector<bool>>& screenCoords,
                               bool flipped_x_and_y) {
        int32_t eps = 0;
        int32_t y = j0;
        int32_t delta_x = i1 - i0;
        assert(delta_x >= 0);
        int32_t delta_y = j1 - j0;
        assert(delta_y >= 0);  // positive slope

        // special case
        if (i0 == i1) {
            for (int32_t y = j0; y <= j1; y++) {
                // TODO: flip y to y_max - y
                if (flipped_x_and_y) {
                    screenCoords[i0][y] = true;
                } else {
                    screenCoords[y][i0] = true;
                }
            }
            return;
        }

        for (int32_t x = i0; x <= i1; x++) {
            if (flipped_x_and_y) {
                screenCoords[x][y] = true;
            } else {
                screenCoords[y][x] = true;
            }

            int32_t val = (eps + delta_y) << 1;
            if (val < delta_x) {
                eps += delta_y;
            } else {
                eps += (delta_y - delta_x);
                y++;
            }
        }
    }

    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    /** Product of all transformations:  geometric transformations, world-to-camera projection,
        perspective projection. */
    Eigen::Matrix4d transformation{Eigen::Matrix4d::Identity()};

    std::string label;
    size_t numCopies{0};
};

#endif
