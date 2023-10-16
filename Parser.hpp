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

/*
 * Parses 'fname' file and populates
 *  - worldToHomoNDC
 *  - camera
 *  - labelToObj
 *  - objectCopies
 */
bool parseDescription(const std::string& fname,
                      std::unordered_map<std::string, std::shared_ptr<Object>>& labelToObj,
                      std::vector<std::shared_ptr<Object>>& objectCopies)
{
    ParsingStage stage{ParsingStage::CAMERA};
    std::ifstream file{fname};
    std::string line;
    Eigen::Matrix4d worldToHomoNDC;
    Camera camera;

    std::getline(file, line);
    assert(line == "camera:");  // expected first line in file

    std::getline(file, line);
    while (file) {
        switch (stage) {
            case ParsingStage::CAMERA: {
                if (line.length() > 0) {
                    std::string param_header;
                    double buffer[4];
                    int no_params = parse_parameter_str(line,
                                                        param_header,
                                                        buffer,
                                                        4);

                    if (param_header == "position") {
                        assert(no_params == 3);
                        camera.pos = {buffer[0], buffer[1], buffer[2]};
                    } else if (param_header == "orientation") {
                        assert(no_params == 4);
                        camera.orientation = {buffer[0],
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
                    makeWorldToCameraProj(worldToCameraProj, camera);
                    makePerspectiveProjection(perspectiveProj, camera);
                    worldToHomoNDC = perspectiveProj*worldToCameraProj;
                    stage = ParsingStage::LIGHTS;
                }
                break;
            }
            case ParsingStage::LIGHTS: {
                if (line.length() > 0) {
                    double buffer[3];
                    std::string paramHeader;
                    int firstCommaIdx = line.find(',');
                    assert(firstCommaIdx != std::string::npos);
                    double secondCommaIdx = line.find(',', firstCommaIdx+1);
                    assert(secondCommaIdx != std::string::npos);

                    PointLight light;
                    // parse light's position
                    std::string light_pos_str = line.substr(0, firstCommaIdx);
                    int no_params = parse_parameter_str(light_pos_str,
                                                        paramHeader,
                                                        buffer,
                                                        3);
                    assert(no_params == 3);
                    light.pos = {buffer[0], buffer[1], buffer[2]};
                    
                    // parse light's color
                    std::string light_color_str = "color";
                    light_color_str += line.substr(firstCommaIdx+1,
                                                   secondCommaIdx-firstCommaIdx);
                    no_params = parse_parameter_str(light_color_str,
                                                    paramHeader,
                                                    buffer,
                                                    3);
                    assert(no_params == 3);
                    assert(paramHeader == "color");
                    light.color = {buffer[0], buffer[1], buffer[2]};

                    // get light's attenuation parameter
                    std::string atten_str = "attenuation";
                    atten_str += line.substr(secondCommaIdx+1,
                                             std::string::npos);
                    no_params = parse_parameter_str(atten_str,
                                                    paramHeader,
                                                    buffer,
                                                    1);
                    assert(no_params == 1);
                    assert(paramHeader == "attenuation");
                    light.attenuation = buffer[0];
                } else {
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
                } else if (spaceIdx == 1) {  // new transformation matrix for the current copy
                    Eigen::Matrix4d transform;
                    makeMatrix(transform, line);
                    objectCopies.back()->addTransformation(transform);
                } else {
                    std::string paramHdr;
                    double buffer[3];
                    int no_params = parse_parameter_str(line,
                                                        paramHdr,
                                                        buffer,
                                                        3);
                    if (paramHdr == "ambient") {
                        assert(no_params == 3);
                    } else if (paramHdr == "diffuse") {
                        assert(no_params == 3);
                    } else if (paramHdr == "specular") {
                        assert(no_params == 3);
                    } else if (paramHdr == "shininess") {
                        assert(no_params == 1);
                    } else {
                        std::cout << "ERROR: unexpected parameter" << std::endl;
                        assert(false);
                    }
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

