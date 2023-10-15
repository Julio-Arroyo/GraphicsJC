#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include <unordered_map>
#include "Objects.hpp"
#include "Transformations.hpp"
#include "Parser.hpp"

class Scene {
public:
    Scene(const std::string& sceneDescriptionFname, size_t xres_, size_t yres_)
        : xres{xres_}, yres{yres_}
    {
        // Build Camera, base Objects, and Object-Copies (i.e. transformed Objects)
        assert(parseDescription(sceneDescriptionFname,
                                labelToObj,
                                objectCopies));
    }

    /** @brief Outputs to stdout a PPM of the image. */
    void wireframePPM() {
        const size_t ncols = xres;
        std::vector<std::vector<bool>> screenCoords{yres, std::vector<bool>(xres)};

        for (std::shared_ptr<Object> obj : objectCopies) {
            obj->fillScreenCoords(screenCoords, xres, yres);
        }

        std::string c1 = "0 0 0";  // "85 47 130";   // purple
        std::string c2 = "253 185 39";  // gold
        // output pixel grid to stdout as PPM
        std::cout << "P3" << std::endl;  // PPM header
        std::cout << yres << " " << xres << std::endl;
        std::cout << "255" << std::endl;
        for (int32_t col = xres; col >= 0; col--) {
            for (int32_t row = 0; row < yres; row++) {
                if (screenCoords[row][col]) {
                    std::cout << c2 << std::endl;
                } else {
                    std::cout << c1 << std::endl;
                }
            }
        }
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> labelToObj;
    std::vector<std::shared_ptr<Object>> objectCopies;
    // Eigen::Matrix4d worldToCameraProj;  // world to camera space
    // Eigen::Matrix4d perspectiveProj;    // camera space to homogeneous NDC
    // Camera camera;
    const size_t xres, yres;
};

#endif

