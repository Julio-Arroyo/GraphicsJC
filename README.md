# GraphicsJC
A graphics library for C++.

![Wireframe rendering pipeline](https://github.com/Julio-Arroyo/GraphicsJC/blob/main/img/scene_bunny1_high_res.png)

## Dependencies
- [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page) for linear algebra operations.

## Code layout
- `Parser.hpp` reads a file that contains the data for objects and transformations.
- `Objects.hpp` implements an object made up by vertices and faces, as well as auxiliary structures and enums.
- `Transformations.hpp` implements translations, rotations, and scaling operations.
