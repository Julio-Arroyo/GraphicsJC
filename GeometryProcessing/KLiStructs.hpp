#ifndef KLI_STRUCTS_H
#define KLI_STRUCTS_H

#include <vector>

namespace KLi {
  struct Vec3f
  {
    float x, y, z;
  };

  struct Vertex
  {
      float x, y, z;
  };

  struct Face
  {
      int idx1, idx2, idx3;
  };

  struct Mesh_Data
  {
      std::vector<Vertex*> *vertices;
      std::vector<Face*> *faces;
  };
}

#endif
