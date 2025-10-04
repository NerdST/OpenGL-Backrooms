#include "OcclusionCuller.h"
#include "MazeGenerator.h"
#include <glad/glad.h>
#include <algorithm>
#include <iostream>

OcclusionCuller::OcclusionCuller()
    : nextQueryID(1), frameCount(0), occludedCells(0)
{
}

OcclusionCuller::~OcclusionCuller()
{
  cleanup();
}

void OcclusionCuller::initialize()
{
  // Check if occlusion queries are supported
  if (!GLAD_GL_VERSION_1_5)
  {
    std::cout << "Warning: Occlusion queries not supported on this system" << std::endl;
    return;
  }

  // Pre-allocate query objects
  queryPool.reserve(MAX_QUERIES);

  std::cout << "Occlusion culling initialized with " << MAX_QUERIES << " queries" << std::endl;
}

void OcclusionCuller::beginFrame()
{
  frameCount++;
  occludedCells = 0;
  // Simplified - no more query processing or cache management
}

bool OcclusionCuller::shouldRenderCell(const glm::ivec2 &cellPos, const glm::vec3 &worldPos,
                                       const glm::vec3 &cameraPos, const MazeGenerator &maze)
{
  // Much simpler and more reliable approach
  float distance = glm::length(worldPos - cameraPos);

  // Only consider occlusion culling for very close cells to minimize visual errors
  if (distance > 8.0f || distance < 3.0f)
  {
    return true; // Don't occlude if too far or too close
  }

  // Very simple direction-based occlusion test
  // Only cull if there's a wall directly between camera and target
  glm::vec3 direction = glm::normalize(worldPos - cameraPos);

  const float CELL_SIZE = 2.0f;

  // Check just a few points along the line - be very conservative
  int checkPoints = 3;
  for (int i = 1; i <= checkPoints; i++)
  {
    float t = (distance / checkPoints) * i;
    glm::vec3 testPos = cameraPos + direction * t;

    int gridX = static_cast<int>(testPos.x / CELL_SIZE);
    int gridZ = static_cast<int>(testPos.z / CELL_SIZE);

    // If ANY point along the path is not a wall, don't occlude
    if (!maze.isWall(gridX, gridZ))
    {
      return true; // Clear path, render the cell
    }
  }

  // Only occlude if ALL check points hit walls
  occludedCells++;
  return false;
}

void OcclusionCuller::endFrame()
{
  // Nothing specific needed here - queries are processed in beginFrame
}

void OcclusionCuller::cleanup()
{
  // Simple cleanup
  queryPool.clear();
  activeQueries.clear();
  visibilityCache.clear();
}

int OcclusionCuller::getCellKey(const glm::ivec2 &cellPos) const
{
  // Simple hash function for 2D coordinates
  return cellPos.x * 73856093 + cellPos.y * 19349663;
}

bool OcclusionCuller::isLikelyOccluded(const glm::vec3 &cellPos, const glm::vec3 &cameraPos,
                                       const MazeGenerator &maze) const
{
  // Removed complex ray-marching - this simple version handles it in shouldRenderCell
  return false; // Not used anymore
}

void OcclusionCuller::renderOcclusionProxy(const glm::vec3 &position)
{
  // Simplified approach for now - just return without rendering
  // In a full implementation, you'd render proper geometry using modern OpenGL
  // with vertex buffers and shaders

  // For now, we'll rely on the heuristic-based occlusion testing
  // which doesn't require rendering proxy geometry
}

void OcclusionCuller::updateQueryResults()
{
  // Simplified version - no OpenGL queries for now
}

void OcclusionCuller::cleanupOldQueries()
{
  // Simplified cleanup - just clear cache if it gets too large
  if (visibilityCache.size() > MAX_QUERIES * 2)
  {
    visibilityCache.clear();
  }
}