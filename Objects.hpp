#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include "Eigen"
#include "Types.hpp"
#include "Lights.hpp"
#include "Util.hpp"
#include "Transformations.hpp"

class Object {
public:
    Object(std::string& fname, bool printObj = true) {
        vertices.emplace_back();  // dummy vertex for 1-indexing
        normals.emplace_back();

        std::ifstream file{fname};
        std::string currLine;

        if (printObj) {
            // print header
            std::cout << ". loc: " << fname.find(".") << std::endl;
            std::cout << fname.substr(0, fname.find(".")) << ":"
                      << std::endl << std::endl;
        }

        std::getline(file, currLine);

        while (file) {
            std::string hdr = currLine.substr(0, currLine.find(' '));
            if (hdr == "v") {
                double buff[3];
                std::string s;
                assert(3 == parse_parameter_str(currLine,
                                                s,
                                                buff,
                                                3));
                Vertex v = {buff[0], buff[1], buff[2]};
                vertices.push_back(v);
            } else if (hdr == "vn") {
                double buff[3];
                std::string s;
                assert(3 == parse_parameter_str(currLine,
                                                s,
                                                buff,
                                                3));
                Vertex n = {buff[0], buff[1], buff[2]};
                // std::cout << "vn " << n.x << ' ' << n.y << ' ' << n.z << std::endl;
                normals.push_back(n);
            } else if (hdr == "f") {
                 int vBuff[3];
                 int nBuff[3];
                parseStrTwoBuff(currLine, vBuff, nBuff);
                Face f = {{vBuff[0], vBuff[1], vBuff[2]},
                          {nBuff[0], nBuff[1], nBuff[2]}};
                // std::cout << "FACE: "
                //             << vBuff[0] << "//" << nBuff[0] << " "
                //             << vBuff[1] << "//" << nBuff[1] << " "
                //             << vBuff[2] << "//" << nBuff[2] << " " << std::endl;
                faces.push_back(f);
            } else {
                std::cout << "ERROR: parsing .obj unknown hdr "
                          << hdr << std::endl;
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
        normals = other.normals;
        faces = other.faces;
        label = other.label + "_copy" + std::to_string(other.getAndIncNumCopies());
    }

    /**
     * TODO rename to renderWireframe
    */
    void fillScreenCoords(std::vector<std::vector<bool>>& screenCoords,
                          size_t xres, size_t yres) {  // TODO OVERLOAD with argument RENDER_MODE, buffer_grid
        for (Face face : faces) {
            Vertex v1 = vertices[face.v.i1];
            int32_t v1_x = (int32_t) (((v1.x - (-1)) / (1 - (-1))) * xres);
            int32_t v1_y = (int32_t) (((v1.y - (-1)) / (1 - (-1))) * yres);
            v1_x = std::min(std::max(0, v1_x), (int32_t) xres-1);
            v1_y = std::min(std::max(0, v1_y), (int32_t) yres-1);

            Vertex v2 = vertices[face.v.i2];
            int32_t v2_x = (int32_t) (((v2.x - (-1)) / (1 - (-1))) * xres);
            int32_t v2_y = (int32_t) (((v2.y - (-1)) / (1 - (-1))) * yres);
            v2_x = std::min(std::max(0, v2_x), (int32_t) xres-1);
            v2_y = std::min(std::max(0, v2_y), (int32_t) yres-1);

            Vertex v3 = vertices[face.v.i3];
            int32_t v3_x = (int32_t) (((v3.x - (-1)) / (1 - (-1))) * xres);
            int32_t v3_y = (int32_t) (((v3.y - (-1)) / (1 - (-1))) * yres);
            v3_x = std::min(std::max(0, v3_x), (int32_t) xres-1);
            v3_y = std::min(std::max(0, v3_y), (int32_t) yres-1);

            drawLine(v1_y, v1_x, v2_y, v2_x, screenCoords);
            drawLine(v1_y, v1_x, v3_y, v3_x, screenCoords);
            drawLine(v2_y, v2_x, v3_y, v3_x, screenCoords);
        }
    }

    void addTransformation(Eigen::Matrix4d& t) {
        transformation = t*transformation;
    }

    void addNormalTransformation(Eigen::Matrix4d& t) {
        normalTrans = t*normalTrans;
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
            Eigen::Vector4d V_t = transformation*V_ws;
            /*
            double x_hndc = V_hndc(0);
            double y_hndc = V_hndc(1);
            double z_hndc = V_hndc(2);
            double w_hndc = V_hndc(3);
            */
            assert(V_t(3) == 1);

            // homogeneous NDC --> cartesian NDC
            // Eigen::Vector4d V_cndc;
            // V_cndc << x_hndc/w_hndc, y_hndc/w_hndc, z_hndc/w_hndc, 1;

            vertices[i] = {V_t(0), V_t(1), V_t(2)};
        }
    }

    void applyNormalTransformation() {
        Eigen::Matrix4d inv = normalTrans.inverse();
        Eigen::Matrix4d T = inv.transpose();

        for (size_t i = 1; i < normals.size(); i++) {
            Eigen::Vector4d N;
            N << normals[i].x, normals[i].y, normals[i].z, 1;

            Eigen::Vector4d N_t = T*N;
            // N_t.normalize();

            normals[i] = {N_t(0), N_t(1), N_t(2)};
        }
    }

    void setMaterialProperties(Color& a, Color& d,
                               Color& s, double sns) {
        ambient = a;
        diffuse = d;
        specular = s;
        shininess = sns;
    }

    size_t getAndIncNumCopies() {
        return ++numCopies;
    }

    void renderShadedObj(std::vector<std::vector<Color>>& screenGrid,
                         size_t xres, size_t yres, ShadingAlgo alg,
                         std::vector<PointLight>& lights, Vertex& cameraPos,
                         Eigen::Matrix4d& worldToHomoNDC,
                         std::vector<std::vector<double>>& minDepth) {
        for (Face f : faces) {
            Vertex v1 = vertices[f.v.i1];
            Vertex v2 = vertices[f.v.i2];
            Vertex v3 = vertices[f.v.i3];
            // std::cout << "normals" << normals.size() << std::endl;
            // std::cout << "f.n.ij " << f.n.i1 << ' ' << f.n.i2 << ' ' << f.n.i3 << std::endl;
            Vertex n1 = normals[f.n.i1];
            Vertex n2 = normals[f.n.i2];
            Vertex n3 = normals[f.n.i3];
            // std::cout << "not normals" << std::endl;

            // std::cout << "diffusemeee " << diffuse.r << std::endl;
            Color c1 = LightingModel(v1, n1, diffuse, ambient,
                                     specular, shininess, lights,
                                     cameraPos);
            Color c2 = LightingModel(v2, n2, diffuse, ambient,
                                     specular, shininess, lights,
                                     cameraPos);
            Color c3 = LightingModel(v3, n3, diffuse, ambient,
                                     specular, shininess, lights,
                                     cameraPos);
            // std::cout << "c1 (r,g,b): " << c1.r << " " << c1.g << " " << c1.b << std::endl;

            Vertex v1_ndc = worldToNDC(worldToHomoNDC, v1);
            Vertex v2_ndc = worldToNDC(worldToHomoNDC, v2);
            Vertex v3_ndc = worldToNDC(worldToHomoNDC, v3);
            // std::cout << "v1_ndc: " << v1_ndc.x << " " << v1_ndc.y << " " << v1_ndc.z << std::endl;
            // std::cout << "v2_ndc: " << v2_ndc.x << " " << v2_ndc.y << " " << v2_ndc.z << std::endl;
            // std::cout << "v3_ndc: " << v3_ndc.x << " " << v3_ndc.y << " " << v3_ndc.z << std::endl;

            if (isBackFacing(v1_ndc, v2_ndc, v3_ndc)) {
                // std::cout << "CABRON" << std::endl;
                continue;
            }
            // std::cout << "front-facing" << std::endl;

            std::pair<int, int> v1_sc = NDCtoScreen(v1_ndc, xres, yres);
            std::pair<int, int> v2_sc = NDCtoScreen(v2_ndc, xres, yres);
            std::pair<int, int> v3_sc = NDCtoScreen(v3_ndc, xres, yres);
            // std::cout << "v1_sc: " << v1_sc.first << " " << v1_sc.second << std::endl;

            size_t x_min = std::min({v1_sc.first, v2_sc.first, v3_sc.first});
            size_t y_min = std::min({v1_sc.second, v2_sc.second, v3_sc.second});
            size_t x_max = std::max({v1_sc.first, v2_sc.first, v3_sc.first});
            size_t y_max = std::max({v1_sc.second, v2_sc.second, v3_sc.second});
            // std::cout << "xmin " << x_min << std::endl;
            // std::cout << "xmax " << x_max << std::endl;
            // std::cout << "ymin " << y_min << std::endl;
            // std::cout << "ymax " << y_max << std::endl;

            for (size_t x = x_min; x <= x_max; x++) {
                for (size_t y = y_min; y <= y_max; y++) {
                    // DEBUG
                    // std::cout << "xperro " << x << std::endl;
                    // std::cout << "v1first " << v1_sc.first << std::endl;
                    // std::cout << "JU " <<  f_ij(x, y, v2_sc.first, v2_sc.second, v3_sc.first, v3_sc.second) << std::endl;
                    // std::cout << "LIO " << f_ij(v1_sc.first, v1_sc.second, v2_sc.first, v2_sc.second, v3_sc.first, v3_sc.second) << std::endl;
                    double alpha = f_ij(x, y,
                                        v2_sc.first, v2_sc.second,
                                        v3_sc.first, v3_sc.second) /
                                   f_ij(v1_sc.first, v1_sc.second,
                                        v2_sc.first, v2_sc.second,
                                        v3_sc.first, v3_sc.second);
                    double beta = f_ij(x, y,
                                       v1_sc.first, v1_sc.second,
                                       v3_sc.first, v3_sc.second) /
                                   f_ij(v2_sc.first, v2_sc.second,
                                        v1_sc.first, v1_sc.second,
                                        v3_sc.first, v3_sc.second);
                    double gamma = f_ij(x, y,
                                        v1_sc.first, v1_sc.second,
                                        v2_sc.first, v2_sc.second) /
                                   f_ij(v3_sc.first, v3_sc.second,
                                        v1_sc.first, v1_sc.second,
                                        v2_sc.first, v2_sc.second);

                    // std::cout << "alpha" << alpha << std::endl;  DEBUG
                    // std::cout << "beta" << beta << std::endl;
                    // std::cout << "gamma" << gamma << std::endl;
                    if (0 <= alpha && alpha <= 1 &&
                        0 <= beta && beta <= 1 &&
                        0 <= gamma && gamma <= 1) {
                        /*
                        std::cout << "v1_ndc: " << v1_ndc.x << " " << v1_ndc.y << " " << v1_ndc.z << std::endl;
                        std::cout << "v2_ndc: " << v2_ndc.x << " " << v2_ndc.y << " " << v2_ndc.z << std::endl;
                        std::cout << "v3_ndc: " << v3_ndc.x << " " << v3_ndc.y << " " << v3_ndc.z << std::endl;
                    std::cout << "alpha" << alpha << std::endl;
                    std::cout << "beta" << beta << std::endl;
                    std::cout << "gamma" << gamma << std::endl;
                    */
                        Vertex interp_ndc = {alpha*v1_ndc.x + beta*v2_ndc.x + gamma*v3_ndc.x,
                                             alpha*v1_ndc.y + beta*v2_ndc.y + gamma*v3_ndc.y,
                                             alpha*v1_ndc.z + beta*v2_ndc.z + gamma*v3_ndc.z};
                        // std::cout << "ndc_inter : " << interp_ndc.x << " " << interp_ndc.y << " " << interp_ndc.z << std::endl;
                        // std::cout << "second" << std::endl;
                        if (inNDCcube(interp_ndc) &&
                            (interp_ndc.z < minDepth[x][y])) {
                            minDepth[x][y] = interp_ndc.z;
                            switch (alg) {
                                case ShadingAlgo::GOURAUD: {
                                                             // std::cout << "gouraud" << std::endl;
                                    double r = alpha*c1.r + beta*c2.r + gamma*c3.r;
                                    double g = alpha*c1.g + beta*c2.g + gamma*c3.g;
                                    double b = alpha*c1.b + beta*c2.b + gamma*c3.b;
                                    screenGrid[x][y] = {r, g, b};
                                    break;
                                }
                                case ShadingAlgo::PHONG: {
                                                           // std::cout << "phong" << std::endl;
                                    double vx = alpha*v1.x + beta*v2.x + gamma*v3.x;
                                    double vy = alpha*v1.y + beta*v2.y + gamma*v3.y;
                                    double vz = alpha*v1.z + beta*v2.z + gamma*v3.z;
                                    Vertex v_interp = {vx, vy, vz};

                                    double nx = alpha*n1.x + beta*n2.x + gamma*n3.x;
                                    double ny = alpha*n1.y + beta*n2.y + gamma*n3.y;
                                    double nz = alpha*n1.z + beta*n2.z + gamma*n3.z;
                                    double norm = std::sqrt(nx*nx + ny*ny + nz*nz);

                                    Vertex n_interp = {nx/norm, ny/norm, nz/norm};
                                    screenGrid[x][y] = LightingModel(v_interp, n_interp,
                                                                     diffuse, ambient,
                                                                     specular, shininess,
                                                                     lights, cameraPos);
                                    break;
                                }
                                default:
                                    assert(false);
                            }
                        }
                    }
                }
            }
        }
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

    /** Product of all transformations:  geometric transformations, world-to-camera projection,
        perspective projection. */
    Eigen::Matrix4d transformation{Eigen::Matrix4d::Identity()};
    Eigen::Matrix4d normalTrans{Eigen::Matrix4d::Identity()};

    std::string label;
    size_t numCopies{0};
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Vertex> normals;
    Color ambient;
    Color diffuse;
    Color specular;
    double shininess;
};

#endif
