# GraphicsJC
A header-only graphics library for C++.

## Dependencies
- [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page) for linear algebra operations.

## Code layout
- `Parser.hpp` reads a file that contains the data for objects and transformations.
- `Objects.hpp` implements an object made up by vertices and faces, as well as auxiliary structures and enums.
- `Transformations.hpp` implements translations, rotations, and scaling operations.
