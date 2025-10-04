#include "FrustumCuller.h"
#include <algorithm>

FrustumCuller::FrustumCuller()
{
  // Initialize with identity frustum
  updateFrustum(glm::mat4(1.0f));
}

void FrustumCuller::updateFrustum(const glm::mat4 &viewProjectionMatrix)
{
  // Extract frustum planes from view-projection matrix
  // Using the method from "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix"

  // Left plane
  extractPlane(viewProjectionMatrix, 3, false); // w + x
  planes[0].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0];
  planes[0].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0];
  planes[0].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0];
  planes[0].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0];

  // Right plane
  planes[1].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0];
  planes[1].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0];
  planes[1].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0];
  planes[1].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0];

  // Bottom plane
  planes[2].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1];
  planes[2].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1];
  planes[2].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1];
  planes[2].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1];

  // Top plane
  planes[3].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1];
  planes[3].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1];
  planes[3].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1];
  planes[3].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1];

  // Near plane
  planes[4].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2];
  planes[4].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2];
  planes[4].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2];
  planes[4].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2];

  // Far plane
  planes[5].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2];
  planes[5].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2];
  planes[5].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2];
  planes[5].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2];

  // Normalize all planes
  for (auto &plane : planes)
  {
    float length = glm::length(plane.normal);
    if (length > 0.0f)
    {
      plane.normal /= length;
      plane.distance /= length;
    }
  }
}

bool FrustumCuller::isAABBVisible(const AABB &aabb) const
{
  // Test AABB against all 6 frustum planes
  for (const auto &plane : planes)
  {
    // Find the positive and negative vertices
    glm::vec3 positiveVertex = aabb.min;
    glm::vec3 negativeVertex = aabb.max;

    if (plane.normal.x >= 0)
    {
      positiveVertex.x = aabb.max.x;
      negativeVertex.x = aabb.min.x;
    }
    if (plane.normal.y >= 0)
    {
      positiveVertex.y = aabb.max.y;
      negativeVertex.y = aabb.min.y;
    }
    if (plane.normal.z >= 0)
    {
      positiveVertex.z = aabb.max.z;
      negativeVertex.z = aabb.min.z;
    }

    // If positive vertex is outside, AABB is completely outside
    if (plane.distanceToPoint(positiveVertex) < 0)
    {
      return false;
    }
  }

  return true; // AABB is inside or intersecting
}

bool FrustumCuller::isPointVisible(const glm::vec3 &point) const
{
  for (const auto &plane : planes)
  {
    if (plane.distanceToPoint(point) < 0)
    {
      return false;
    }
  }
  return true;
}

bool FrustumCuller::isSphereVisible(const glm::vec3 &center, float radius) const
{
  for (const auto &plane : planes)
  {
    if (plane.distanceToPoint(center) < -radius)
    {
      return false;
    }
  }
  return true;
}

void FrustumCuller::extractPlane(const glm::mat4 &matrix, int row, bool negate)
{
  // This method is kept for potential future use but the main extraction
  // is done directly in updateFrustum for efficiency
}