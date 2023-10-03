#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include <unordered_map>
#include "Objects.hpp"
#include "Transformations.hpp"

struct Camera {
    struct Position {
        int x, y, z;
    };
    struct Orientation {
        int x, y, z, theta;
    };

    Position pos;
    Orientation orientation;
    int near, far, left, right, top, bottom;
};

class Scene {
public:
    Scene(const std::string& sceneDescriptionFname) {
        assert(parseDescription(sceneDescriptionFname, true));

        makeWorldToCameraProj();
        makePerspectiveProjection();

        Eigen::Matrix4d worldToHomoNDC = perspectiveProj*worldToCameraProj;
        for (std::shared_ptr<Object> obj_copy : objectCopies) {
            obj_copy->addTransformation(worldToCameraProj);
        }
    }

    void wireframeToPPM(const std::string& outputFilename) {}

private:
    /** @brief Converts between world and camera coordinates. */
    inline void makeWorldToCameraProj() {
        Eigen::Matrix4d Tc, Rc;  // camera position, rotation transformations respectively
        makeTranslationMat(Tc, camera.pos.x, camera.pos.y, camera.pos.z);
        makeRotationMat(Rc, camera.orientation.x,
                            camera.orientation.y,
                            camera.orientation.z,
                            camera.orientation.theta);

        Eigen::Matrix4d prod = Tc*Rc;
        worldToCameraProj = prod.inverse();
    }

    /** @brief Makes the projection matrix from camera space to homogeneous
     *         Normalized Device Coordinates (NDC). */
    inline void makePerspectiveProjection() {
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

    bool parseDescription(const std::string& fname, bool enable_logging = false) {
        ParsingStage stage{ParsingStage::CAMERA};
        std::ifstream file{fname};
        std::string line;

        std::getline(file, line);
        assert(line == "camera:");  // expected first line in file

        std::getline(file, line);
        while (file) {
            switch (stage) {
                case ParsingStage::CAMERA: {
                    if (line.length() == 0) {
                        stage = ParsingStage::OBJECTS;
                    } else {
                        std::string param_header;
                        int buffer[4];
                        int no_params = parse_parameter_str(line, param_header, buffer, 4);

                        if (param_header == "position") {
                            assert(no_params == 3);
                            camera.pos = (Camera::Position) {buffer[0], buffer[1], buffer[2]};
                        } else if (param_header == "orientation") {
                            assert(no_params == 4);
                            camera.orientation = (Camera::Orientation) {buffer[0],
                                                                        buffer[1],
                                                                        buffer[2],
                                                                        buffer[3]};
                        } else if (param_header == "near") {
                            assert(no_params == 1);
                            camera.near = buffer[0];
                        } else if (param_header == "far") {
                            assert(no_params == 1);
                            camera.far = buffer[0];
                        } else if (param_header == "left") {
                            assert(no_params == 1);
                            camera.left = buffer[0];
                        } else if (param_header == "right") {
                            assert(no_params == 1);
                            camera.right = buffer[0];
                        } else if (param_header == "top") {
                            assert(no_params == 1);
                            camera.top = buffer[0];
                        } else if (param_header == "bottom") {
                            assert(no_params == 1);
                            camera.bottom = buffer[0];
                        } else {
                            std::cout << "Error: ParsingStage::CAMERA" << std::endl;
                            return false;
                        }
                    }
                    break;
                }
                case ParsingStage::OBJECTS: {
                    if (line.length() == 0) {
                        stage = ParsingStage::OBJECT_COPIES;
                    } else if (line == "objects:") {
                        
                    } else {
                        size_t spaceIdx = line.find(" ");
                        assert(spaceIdx != std::string::npos);

                        std::string label = line.substr(0, spaceIdx);
                        std::string objFilename = line.substr(spaceIdx + 1,
                                                              line.length());
                        std::shared_ptr<Object> obj =
                            std::make_shared<Object>(objFilename, label, false);

                        labelToObj.insert({label, obj});
                    }
                    break;
                }
                case ParsingStage::OBJECT_COPIES: {
                    size_t spaceIdx = line.find(" ");
                    if (line.length() == 0) {  // finished copy, no more transformations
                        if (enable_logging) {
                            objectCopies.back()->display();
                        }
                    } else if (spaceIdx == std::string::npos) {  // create a new copy
                        std::shared_ptr<Object> original = labelToObj.find(line)->second;
                        std::shared_ptr<Object> copy
                            = std::make_shared<Object>(*original);
                        objectCopies.push_back(copy);
                    } else {  // new transformation matrix for the current copy
                        Eigen::Matrix4d transform;
                        makeMatrix(transform, line);
                        objectCopies.back()->addTransformation(transform);
                    }
                    break;
                }
            }
            std::getline(file, line);
        }

        if (objectCopies.size() > 0 && enable_logging) {
            objectCopies.back()->display();
        }

        return true;
    }

    enum ParsingStage : size_t {
        CAMERA,
        OBJECTS,
        OBJECT_COPIES
    };

    std::unordered_map<std::string, std::shared_ptr<Object>> labelToObj;
    std::vector<std::shared_ptr<Object>> objectCopies;
    Eigen::Matrix4d worldToCameraProj;  // world to camera space
    Eigen::Matrix4d perspectiveProj;    // camera space to homogeneous NDC
    Camera camera;
};

#endif
