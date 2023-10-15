#ifndef PARSER_HPP
#define PARSER_HPP

#include <unordered_map>
#include "Objects.hpp"
#include "Transformations.hpp"

enum ParsingStage : size_t {
    CAMERA,
    LIGHTS,
    OBJECTS,
    OBJECT_COPIES
};

/* void parseFile(std::string& fname, std::vector<std::shared_ptr<Object>>& objects) {
    ParsingStage stage{ParsingStage::CAMERA};
    std::ifstream file{fname};
    std::string line;
    std::getline(file, line);
    std::unordered_map<std::string, std::shared_ptr<Object>> labelToObj;

    while (file) {
        switch (stage) {
            case ParsingStage::CAMERA:
                // TODO
                break;
            case ParsingStage::OBJECTS:
                if (line.length() == 0) {
                    stage = ParsingStage::OBJECT_COPIES;
                } else {
                    size_t spaceIdx = line.find(" ");
                    assert(spaceIdx != std::string::npos);

                    std::string label = line.substr(0, spaceIdx);
                    std::string objFilename = line.substr(spaceIdx + 1,
                                                        line.length());
                    std::shared_ptr<Object> obj =
                        std::make_shared<Object>(objFilename,label, false);

                    objects.push_back(obj);
                    labelToObj.insert({label, obj});
                }
                break;
            case ParsingStage::OBJECT_COPIES:
                size_t spaceIdx = line.find(" ");
                if (line.length() == 0) {  // finished creating the copy, no more transformations
                    objects.back()->display();
                } else if (spaceIdx == std::string::npos) {  // create a new copy
                    std::shared_ptr<Object> original = labelToObj.find(line)->second;
                    std::shared_ptr<Object> copy
                        = std::make_shared<Object>(*original);
                    objects.push_back(copy);
                } else {  // new transformation matrix for the current copy
                    Eigen::Matrix4d transform;
                    makeMatrix(transform, line);
                    objects.back()->addTransformation(transform);
                }
                break;
        }
        std::getline(file, line);
    }

    if (objects.size() > 0) {
        objects.back()->display();
    }
}*/

/*
 * Parses 'fname' file and populates
 *  - worldToHomoNDC
 *  - camera
 *  - labelToObj
 *  - objectCopies
 */
bool parseDescription(const std::string& fname,
                      Eigen::Matrix4d& worldToHomoNDC,
                      Camera& camera,
                      std::unordered_map<std::string, std::shared_ptr<Object>>& labelToObj,
                      std::vector<std::shared_ptr<Object>>& objectCopies)
{
    ParsingStage stage{ParsingStage::CAMERA};
    std::ifstream file{fname};
    std::string line;
    Eigen::Matrix4d worldToHomoNDC;

    std::getline(file, line);
    assert(line == "camera:");  // expected first line in file

    std::getline(file, line);
    while (file) {
        switch (stage) {
            case ParsingStage::CAMERA: {
                if (line.length() > 0) {
                    std::string param_header;
                    double buffer[4];
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
                } else {
                    assert(line.length() == 0);
                    Eigen::Matrix4d perspectiveProj, worldToCameraProj;
                    makeWorldToCameraProj(worldToCameraProj);
                    makePerspectiveProjection(perspectiveProj);
                    worldToHomoNDC = perspectiveProj*worldToCameraProj;
                    stage = ParsingStage::OBJECTS;
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
                    objectCopies.back()->addTransformation(worldToHomoNDC);
                    objectCopies.back()->applyTransformation();
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

    if (objectCopies.size() > 0) {
        objectCopies.back()->addTransformation(worldToHomoNDC);
        objectCopies.back()->applyTransformation();
    }

    return true;
}

#endif

