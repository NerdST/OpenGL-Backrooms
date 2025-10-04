#ifndef OCCLUSION_CULLER_H
#define OCCLUSION_CULLER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class MazeGenerator; // Forward declaration

struct OcclusionQuery
{
  unsigned int queryID;
  glm::ivec2 cellPos;
  bool resultReady;
  bool wasVisible;
  int framesSinceQuery;

  OcclusionQuery() : queryID(0), cellPos(0), resultReady(false), wasVisible(true), framesSinceQuery(0) {}
};

class OcclusionCuller
{
public:
  OcclusionCuller();
  ~OcclusionCuller();

  // Initialize occlusion culling system
  void initialize();

  // Begin occlusion culling for this frame
  void beginFrame();

  // Check if a cell should be rendered based on occlusion
  bool shouldRenderCell(const glm::ivec2 &cellPos, const glm::vec3 &worldPos,
                        const glm::vec3 &cameraPos, const MazeGenerator &maze);

  // End occlusion culling and process queries
  void endFrame();

  // Cleanup
  void cleanup();

  // Get statistics
  int getQueriesActive() const { return activeQueries.size(); }
  int getCellsOccluded() const { return occludedCells; }

private:
  std::unordered_map<int, OcclusionQuery> queryPool;
  std::unordered_set<int> activeQueries;
  std::unordered_map<int, bool> visibilityCache;

  int nextQueryID;
  int frameCount;
  int occludedCells;

  // Configuration
  static const int MAX_QUERIES = 512;
  static const int QUERY_FREQUENCY = 3;              // Query every N frames for non-visible objects
  static constexpr float OCCLUSION_DISTANCE = 15.0f; // Max distance for occlusion queries

  // Helper functions
  int getCellKey(const glm::ivec2 &cellPos) const;
  bool isLikelyOccluded(const glm::vec3 &cellPos, const glm::vec3 &cameraPos,
                        const MazeGenerator &maze) const;
  void renderOcclusionProxy(const glm::vec3 &position);
  void updateQueryResults();
  void cleanupOldQueries();
};

#endif