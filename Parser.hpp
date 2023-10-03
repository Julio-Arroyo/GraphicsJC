#ifndef PARSER_HPP
#define PARSER_HPP

#include <unordered_map>
#include "Objects.hpp"
#include "Transformations.hpp"

void parseFile(std::string& fname, std::vector<std::shared_ptr<Object>>& objects) {
    bool builderMode = true;
    std::ifstream file{fname};
    std::string line;
    std::getline(file, line);
    std::unordered_map<std::string, std::shared_ptr<Object>> labelToObj;

    while (file) {
        if (builderMode) {
            if (line.length() == 0) {
                builderMode = false;
            } else {
                size_t spaceIdx = line.find(" ");
                assert(spaceIdx != std::string::npos);

                std::string label = line.substr(0, spaceIdx);
                std::string objFilename = line.substr(spaceIdx + 1, line.length());
                std::shared_ptr<Object> obj = std::make_shared<Object>(objFilename, label, false);

                objects.push_back(obj);
                labelToObj.insert({label, obj});
            }
        } else {
            size_t spaceIdx = line.find(" ");
            if (line.length() == 0) {
                objects.back()->display();
            } else if (spaceIdx == std::string::npos) {  // define a new copy
                std::shared_ptr<Object> original = labelToObj.find(line)->second;
                std::shared_ptr<Object> copy = std::make_shared<Object>(*original);  // calls copy constructor
                objects.push_back(copy);
            } else {
                Eigen::Matrix4d transform;
                makeMatrix(transform, line);
                objects.back()->addTransformation(transform);
            }
        }
        std::getline(file, line);
    }
    if (objects.size() > 0) {
        objects.back()->display();
    }
}

#endif