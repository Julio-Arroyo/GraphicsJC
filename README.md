# GraphicsJC
A graphics library for C++.

![Wireframe rendering pipeline](https://github.com/Julio-Arroyo/GraphicsJC/blob/main/img/scene_bunny1_high_res.png?raw=true)

## Dependencies
- [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page) for linear algebra operations.

## Code layout
- `Parser.hpp` reads a file that contains the data for objects and transformations.
- `Objects.hpp` implements an object made up by vertices and faces, as well as auxiliary structures and enums.
- `Transformations.hpp` implements translations, rotations, and scaling operations.

## Available Graphics Pipelines
### Wireframe Rendering

### Shaded Rendering
Scene description file provides point light information, and each object copy now has material properties.
- Parse description file
  - Read in light information
  - After info for a given object copy is fully parsed, transform normals (in world space).

Gouraud Shading:
Parameters:
  - Face in NDC
    - gives you NDC coordinates of three vertices
    - gives you color of each vertex
  - screenCoords grid
