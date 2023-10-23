#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include <vector>
#include "Eigen"
#include "Types.hpp"

enum ShadingAlgo {
    NONE,
    GOURAUD,  // interpolates color directly
    PHONG     // interpolates NDC coords and normals, to get color
};

/*
 * @param p - point in world space to color
 * @param n - normal vector at 'p'
 * @param cd - diffuse material property 
 * @param ca - ambient material property
 * @param cs - specular material property
 * @param shininess
 * @param lights
 * @param e - camera position
 */
Color LightingModel(Vertex point, Vertex normal,
                    Color diffuse, Color ambient,
                    Color specular, double shininess,
                    std::vector<PointLight>& lights,
                    Vertex cameraPos) {
    Eigen::Vector3d cd, ca, cs;
    cd << diffuse.r, diffuse.g, diffuse.b;
    // std::cout << "cd " << cd << std::endl;
    ca << ambient.r, ambient.g, ambient.b;
    // std::cout << "ca " << ca << std::endl;
    cs << specular.r, specular.g, specular.b;
    // std::cout << "cs " << cs << std::endl;
    // std::cout << "SHININESS" << shininess << std::endl;

    Eigen::Vector3d P, e, n;
    P << point.x, point.y, point.z;
    e << cameraPos.x, cameraPos.y, cameraPos.z;
    n << normal.x, normal.y, normal.z;
    n.normalize();
    
    Eigen::Vector3d diffuse_sum, specular_sum;
    diffuse_sum << 0, 0, 0;
    specular_sum << 0, 0, 0;

    Eigen::Vector3d e_dir = (e - P).normalized();
    
    for (PointLight light : lights) {
        Eigen::Vector3d lp, lc;
        lp << light.pos.x, light.pos.y, light.pos.z;
        lc << light.color.r, light.color.g, light.color.b;
        
        // Compute attenuation
        double k = light.attenuation;
        Eigen::Vector3d l = lp - P;
        double d_sq = l(0)*l(0) + l(1)*l(1) + l(2)*l(2);
        lc = lc/(1+k*d_sq);

        Eigen::Vector3d l_direction = l.normalized();

        // std::cout << "lc " << lc << std::endl;
        // std::cout << "n dot l_diretion: " << n.dot(l_direction) << std::endl;
        Eigen::Vector3d l_diffuse = (std::max(0.0, n.dot(l_direction)))*lc;
        diffuse_sum += l_diffuse;

        Eigen::Vector3d h_hat = e_dir + l_direction;
        Eigen::Vector3d h = h_hat.normalized();
        double temp = n.dot(h);
        Eigen::Vector3d l_specular = (std::pow(std::max(0.0, temp), shininess))*lc;
        specular_sum += l_specular;
    }
    
    Eigen::Vector3d color = ca + diffuse_sum.cwiseProduct(cd) + specular_sum.cwiseProduct(cs);
    // Eigen::Vector3d color = ca + cd.cwiseProduct(diffuse_sum) + cs.cwiseProduct(specular_sum);

    Color c = {std::min(1.0, color(0)),
               std::min(1.0, color(1)),
               std::min(1.0, color(2))};

    return c;
}

#endif

