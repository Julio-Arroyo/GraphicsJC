#ifndef DISCRETE_DIFFERENTIAL_GEOMETRY_HPP
#define DISCRETE_DIFFERENTIAL_GEOMETRY_HPP

#include "KLiStructs.hpp"
#include "Halfedge.hpp"
#include "Types.hpp"
#include <cmath>

KLi::Vec3f calc_normal(KLi::HEF* face) {
  KLi::HE* edge = face->edge;
  KLi::HE* next = face->edge->next;

  KLi::Vec3f v1 = {(float) edge->vertex->x,
                   (float) edge->vertex->y,
                   (float) edge->vertex->z};
  KLi::Vec3f v2 = {(float) next->vertex->x,
                   (float) next->vertex->y,
                   (float) next->vertex->z};
  KLi::Vec3f v3 = {(float) next->next->vertex->x,
                   (float) next->next->vertex->y,
                   (float) next->next->vertex->z};

  KLi::Vec3f A = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};  // v2-v1
  KLi::Vec3f B = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};  // v3-v1

  KLi::Vec3f a_cross_b = {A.y*B.z - A.z*B.y,
                          -(A.x*B.z - A.z*B.x),
                          A.x*B.y - A.y*B.x};
  return a_cross_b;
}

double calc_area(KLi::Vec3f n) {
  return 0.5*std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
}

KLi::Vec3f calc_vertex_normal(KLi::HEV* vertex)
{
  KLi::Vec3f normal;
  normal.x = 0;
  normal.y = 0;
  normal.z = 0;

  KLi::HE* he = vertex->out; // get outgoing halfedge from given vertex
  do
  {
    // compute the normal of the plane of the face
    KLi::Vec3f face_normal = calc_normal(he->face);
    // compute the area of the triangular face
    double face_area = calc_area(face_normal);

    // accummulate onto our normal vector
    normal.x += face_normal.x * face_area;
    normal.y += face_normal.y * face_area;
    normal.z += face_normal.z * face_area;

    // gives us the halfedge to the next adjacent vertex
    he = he->flip->next;
  } while(he != vertex->out);

  return normal;
}

/* Uses the area-weighted algorithm to populate 'normals' from vertices and faces */
void computeVertexNormals(std::vector<Vertex>& normals,
                          std::vector<Vertex>& vertices,
                          std::vector<Face>& faces) {
  assert(normals.size() == 1);  // should only contain dummy normal

  // Put 'vertices', 'faces' into Mesh_Data
  KLi::Mesh_Data* mesh_data = new KLi::Mesh_Data;
  mesh_data->vertices = new std::vector<KLi::Vertex*>();
  mesh_data->faces = new std::vector<KLi::Face*>();
  for (int i = 0; i < vertices.size(); i++) {
    KLi::Vertex* v = new KLi::Vertex{(float) vertices[i].x,
                                     (float) vertices[i].y,
                                     (float) vertices[i].z};
    mesh_data->vertices->push_back(v);
  }
  for (int i = 0; i < faces.size(); i++) {
    KLi::Face* f = new KLi::Face{faces[i].v.i1,
                                 faces[i].v.i2,
                                 faces[i].v.i3};
    mesh_data->faces->push_back(f);
  }

  // Populate halfedge data structures
  std::vector<KLi::HEV*> *hevs = new std::vector<KLi::HEV*>();
  std::vector<KLi::HEF*> *hefs = new std::vector<KLi::HEF*>();
  KLi::build_HE(mesh_data, hevs, hefs);

  // Compute normals
  for (int i = 0; i < hefs->size(); i++) {  // for each face
    KLi::HE* he = hefs->at(i)->edge;
    int vertex_count = 0;
    do {
      KLi::Vec3f vn = calc_vertex_normal(he->vertex);
      normals.push_back({vn.x, vn.y, vn.z});
      vertex_count++;
      he = he->next;
    } while (he != hefs->at(i)->edge);
    assert(vertex_count == 3);
  }

  // delete allocated memory
  KLi::delete_HE(hevs, hefs);
  for (int i = 0; i < mesh_data->vertices->size(); i++) {
      delete mesh_data->vertices->at(i);
  }
  delete mesh_data->vertices;

  for (int i = 0; i < mesh_data->faces->size(); i++) {
      delete mesh_data->faces->at(i);
  }
  delete mesh_data->faces;
  delete mesh_data;
}
#endif

