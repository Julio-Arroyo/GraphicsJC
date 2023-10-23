#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include <limits>
#include <unordered_map>
#include "Objects.hpp"
#include "Transformations.hpp"
#include "Parser.hpp"
#include "Lights.hpp"

class Scene {
public:
    Scene(const std::string& sceneDescriptionFname, size_t xres_, size_t yres_)
        : xres{xres_}, yres{yres_}
    {
        // Build Camera, base Objects, and Object-Copies (i.e. transformed Objects)
        assert(parseDescription(sceneDescriptionFname,
                                labelToObj,
                                objectCopies,
                                lights,
                                camera,
                                worldToHomoNDC));
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

    void renderShadedScene(ShadingAlgo shadingAlgo) {
        std::vector<std::vector<Color>> screen{yres, std::vector<Color>{xres}};

        std::vector<double> defaultRow;
        for (size_t i = 0; i < xres; i++) {
            defaultRow.push_back(std::numeric_limits<double>::max());
        }
        std::vector<std::vector<double>> minDepth{yres, defaultRow};

        for (std::shared_ptr<Object> obj : objectCopies) {
            obj->renderShadedObj(screen, xres, yres, shadingAlgo, lights,
                                 camera.pos, worldToHomoNDC, minDepth);
        }

        // output screen to stdout in PPM format
        std::cout << "P3" << std::endl;  // PPM header
        std::cout << yres << " " << xres << std::endl;
        std::cout << "255" << std::endl;
        for (int32_t col = xres; col >= 0; col--) {
            for (int32_t row = 0; row < yres; row++) {
                uint32_t r = 255*screen[row][col].r;
                uint32_t g = 255*screen[row][col].g;
                uint32_t b = 255*screen[row][col].b;
                std::cout << r << " " << g << " " << b << std::endl;
            }
        }
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> labelToObj;
    std::vector<std::shared_ptr<Object>> objectCopies;
    std::vector<PointLight> lights;
    Eigen::Matrix4d worldToHomoNDC;
    Camera camera;
    const size_t xres, yres;
    ShadingAlgo shadingAlgo{ShadingAlgo::NONE};
};

#endif

