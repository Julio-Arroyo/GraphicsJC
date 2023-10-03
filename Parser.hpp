#ifndef PARSER_HPP
#define PARSER_HPP

#include <unordered_map>
#include "Objects.hpp"
#include "Transformations.hpp"

enum ParsingStage : size_t {
    CAMERA,
    OBJECTS,
    OBJECT_COPIES
};

void parseFile(std::string& fname, std::vector<std::shared_ptr<Object>>& objects) {
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
}

#endif
