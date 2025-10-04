#ifndef FRUSTUM_CULLER_H
#define FRUSTUM_CULLER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

struct Plane
{
  glm::vec3 normal;
  float distance;

  // Distance from point to plane (positive if on normal side)
  float distanceToPoint(const glm::vec3 &point) const
  {
    return glm::dot(normal, point) + distance;
  }
};

struct AABB
{
  glm::vec3 min;
  glm::vec3 max;

  AABB() : min(0.0f), max(0.0f) {}
  AABB(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) {}

  glm::vec3 getCenter() const
  {
    return (min + max) * 0.5f;
  }

  glm::vec3 getExtents() const
  {
    return (max - min) * 0.5f;
  }
};

class FrustumCuller
{
public:
  FrustumCuller();

  // Update frustum from view-projection matrix
  void updateFrustum(const glm::mat4 &viewProjectionMatrix);

  // Test if AABB is inside or intersecting frustum
  bool isAABBVisible(const AABB &aabb) const;

  // Test if point is inside frustum
  bool isPointVisible(const glm::vec3 &point) const;

  // Test if sphere is inside or intersecting frustum
  bool isSphereVisible(const glm::vec3 &center, float radius) const;

private:
  // 6 planes: left, right, bottom, top, near, far
  std::array<Plane, 6> planes;

  void extractPlane(const glm::mat4 &matrix, int row, bool negate = false);
};

#endif