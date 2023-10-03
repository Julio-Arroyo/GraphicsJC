#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
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

    void display() {  // TODO: turn into << overload
        std::cout << label << std::endl;
        for (size_t i = 1; i < vertices.size(); i++) {
            Eigen::Vector4d vec;
            Vertex v = vertices[i];
            vec << v.x, v.y, v.z, 1;
            vec = transformation*vec;
            std::cout << vec(0) << " " << vec(1) << " " << vec(2) << std::endl;
        }
    }

    void addTransformation(Eigen::Matrix4d& t) {
        transformation *= t;
    }

    size_t getAndIncNumCopies() {
        return ++numCopies;
    }

private:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    Eigen::Matrix4d transformation{Eigen::Matrix4d::Identity()};
    std::string label;
    size_t numCopies{0};
};

#endif
