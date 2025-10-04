#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "Mesh.h"
#include <vector>

class Primitives
{
public:
  static Mesh createCube(float size = 1.0f);
  static Mesh createPlane(float width = 1.0f, float height = 1.0f);
  static Mesh createWall(float width = 1.0f, float height = 1.0f);
  static Mesh createFloor(float width = 1.0f, float depth = 1.0f);
  static Mesh createCeiling(float width = 1.0f, float depth = 1.0f);

private:
  static std::vector<Vertex> getCubeVertices(float size);
  static std::vector<unsigned int> getCubeIndices();
  static std::vector<Vertex> getPlaneVertices(float width, float height);
  static std::vector<unsigned int> getPlaneIndices();
};

#endif