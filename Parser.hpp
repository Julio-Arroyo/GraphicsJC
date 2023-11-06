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
                      std::vector<std::shared_ptr<Object>>& objectCopies,
                      std::vector<PointLight>& lights, Camera& camera,
                      Eigen::Matrix4d& worldToHomoNDC)
{
    ParsingStage stage{ParsingStage::CAMERA};
    std::ifstream file{fname};
    std::string line;
    
    Eigen::Matrix4d normalTrans;
    Color ambient, diffuse, specular;
    double shininess;

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

                    // handle case where scene description has no lights
                    if (firstCommaIdx == std::string::npos) {
                        stage = ParsingStage::OBJECTS;
                        continue;
                    }
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
                    // std::cout << "light pos: " << light.pos.x << ' ' << light.pos.y << ' ' << light.pos.z << ' ' << std::endl;
                    
                    // parse light's color
                    std::string light_color_str = "color";
                    light_color_str += line.substr(firstCommaIdx+1,
                                                   secondCommaIdx-firstCommaIdx-2);
                    no_params = parse_parameter_str(light_color_str,
                                                    paramHeader,
                                                    buffer,
                                                    3);
                    assert(no_params == 3);
                    assert(paramHeader == "color");
                    light.color = {buffer[0], buffer[1], buffer[2]};
                    // std::cout << "light color: " << light.color.r << ' ' << light.color.g << ' ' << light.color.b << ' ' << std::endl;

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

                    lights.push_back(light);
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
                    std::string objFilename = line.substr(spaceIdx + 1, std::string::npos);
                    std::shared_ptr<Object> obj =
                        std::make_shared<Object>(objFilename, label, false);

                    labelToObj.insert({label, obj});
                }
                break;
            }
            case ParsingStage::OBJECT_COPIES: {
                size_t spaceIdx = line.find(" ");
                if (line.length() == 0) {  // finished copy, no more transformations
                    objectCopies.back()->setMaterialProperties(ambient, diffuse,
                                                               specular, shininess);
                    // objectCopies.back()->applyTransformation();
                    // objectCopies.back()->applyNormalTransformation();
                } else if (spaceIdx == std::string::npos) {  // create a new copy
                    std::shared_ptr<Object> original = labelToObj.find(line)->second;
                    std::shared_ptr<Object> copy
                        = std::make_shared<Object>(*original);
                    objectCopies.push_back(copy);
                } else if (spaceIdx == 1) {  // new transformation matrix for the current copy
                    Eigen::Matrix4d transform;
                    float transParams[4];

                    Type tt = makeMatrix(transform, line, transParams);
                    if (tt == Type::ROTATION_MAT ||
                        tt == Type::SCALING_MAT) {
                        objectCopies.back()->addNormalTransformation(transform);
                    }
                    objectCopies.back()->addTransformation(transform);
                    objectCopies.back()->recordTransformation(tt, transParams);
                } else {
                    std::string paramHdr;
                    double buffer[3];
                    int no_params = parse_parameter_str(line,
                                                        paramHdr,
                                                        buffer,
                                                        3);
                    if (paramHdr == "ambient") {
                        assert(no_params == 3);
                        ambient = {buffer[0], buffer[1], buffer[2]};
                    } else if (paramHdr == "diffuse") {
                        assert(no_params == 3);
                        diffuse = {buffer[0], buffer[1], buffer[2]}; 
                    } else if (paramHdr == "specular") {
                        assert(no_params == 3);
                        specular = {buffer[0], buffer[1], buffer[2]}; 
                    } else if (paramHdr == "shininess") {
                        assert(no_params == 1);
                        shininess = buffer[0];
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
        objectCopies.back()->setMaterialProperties(ambient, diffuse,
                                                   specular, shininess);
        // objectCopies.back()->applyTransformation();
        // objectCopies.back()->applyNormalTransformation();
    }

    return true;
}

#endif

