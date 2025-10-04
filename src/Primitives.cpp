#include "Primitives.h"

Mesh Primitives::createCube(float size)
{
  auto vertices = getCubeVertices(size);
  auto indices = getCubeIndices();
  std::vector<Texture> textures; // Empty for now

  return Mesh(vertices, indices, textures);
}

Mesh Primitives::createPlane(float width, float height)
{
  auto vertices = getPlaneVertices(width, height);
  auto indices = getPlaneIndices();
  std::vector<Texture> textures;

  return Mesh(vertices, indices, textures);
}

Mesh Primitives::createWall(float width, float height)
{
  return createPlane(width, height);
}

Mesh Primitives::createFloor(float width, float depth)
{
  auto vertices = getPlaneVertices(width, depth);
  auto indices = getPlaneIndices();
  std::vector<Texture> textures;

  // Rotate to be horizontal (floor)
  for (auto &vertex : vertices)
  {
    float temp = vertex.Position.y;
    vertex.Position.y = vertex.Position.z;
    vertex.Position.z = -temp;

    temp = vertex.Normal.y;
    vertex.Normal.y = vertex.Normal.z;
    vertex.Normal.z = -temp;
  }

  return Mesh(vertices, indices, textures);
}

Mesh Primitives::createCeiling(float width, float depth)
{
  auto vertices = getPlaneVertices(width, depth);
  auto indices = getPlaneIndices();
  std::vector<Texture> textures;

  // Rotate to be horizontal (ceiling) and flip normal to point downward
  for (auto &vertex : vertices)
  {
    float temp = vertex.Position.y;
    vertex.Position.y = vertex.Position.z;
    vertex.Position.z = -temp;

    // Set normal to point downward (-Y)
    vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
  }

  // Flip winding order for ceiling (so it faces down)
  for (size_t i = 0; i < indices.size(); i += 3)
  {
    std::swap(indices[i], indices[i + 2]);
  }

  return Mesh(vertices, indices, textures);
}

std::vector<Vertex> Primitives::getCubeVertices(float size)
{
  float half = size * 0.5f;

  return {
      // Front face
      {{-half, -half, half}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{half, -half, half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      {{half, half, half}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-half, half, half}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

      // Back face
      {{half, -half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
      {{-half, -half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
      {{-half, half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{half, half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

      // Left face
      {{-half, -half, -half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{-half, -half, half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{-half, half, half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{-half, half, -half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

      // Right face
      {{half, -half, half}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{half, -half, -half}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{half, half, -half}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{half, half, half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

      // Top face
      {{-half, half, half}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
      {{half, half, half}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{half, half, -half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
      {{-half, half, -half}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

      // Bottom face
      {{-half, -half, -half}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
      {{half, -half, -half}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
      {{half, -half, half}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
      {{-half, -half, half}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}};
}

std::vector<unsigned int> Primitives::getCubeIndices()
{
  return {
      // Front face
      0, 1, 2, 2, 3, 0,
      // Back face
      4, 5, 6, 6, 7, 4,
      // Left face
      8, 9, 10, 10, 11, 8,
      // Right face
      12, 13, 14, 14, 15, 12,
      // Top face
      16, 17, 18, 18, 19, 16,
      // Bottom face
      20, 21, 22, 22, 23, 20};
}

std::vector<Vertex> Primitives::getPlaneVertices(float width, float height)
{
  float halfW = width * 0.5f;
  float halfH = height * 0.5f;

  return {
      // Front face (facing +Z)
      {{-halfW, -halfH, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{halfW, -halfH, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      {{halfW, halfH, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-halfW, halfH, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

      // Back face (facing -Z) - same vertices but with opposite normal and flipped winding
      {{-halfW, -halfH, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
      {{-halfW, halfH, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{halfW, halfH, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
      {{halfW, -halfH, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}};
}

std::vector<unsigned int> Primitives::getPlaneIndices()
{
  return {0, 1, 2, 2, 3, 0};
}