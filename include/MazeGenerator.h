#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H

#include <vector>
#include <glm/glm.hpp>
#include <random>

enum class CellType
{
  WALL,
  FLOOR,
  CEILING,
  EMPTY
};

struct MazeCell
{
  CellType type;
  glm::vec3 position;
  bool visited = false;
};

class MazeGenerator
{
public:
  MazeGenerator(int width, int height, unsigned int seed = 0);

  void generateMaze();
  void generateBackroomsMaze(); // New backrooms-style generation
  void generateChunk(int chunkX, int chunkZ);

  std::vector<MazeCell> getCells() const { return cells; }
  std::vector<MazeCell> getChunk(int chunkX, int chunkZ) const;

  CellType getCellType(int x, int z) const;
  bool isWall(int x, int z) const;
  bool isFloor(int x, int z) const;
  bool isValidCell(int x, int z) const;

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  static const int CHUNK_SIZE = 16;

private:
  int width, height;
  std::vector<MazeCell> cells;
  std::mt19937 rng;

  void carvePath(int x, int z);
  std::vector<glm::ivec2> getNeighbors(int x, int z);
  int getIndex(int x, int z) const;

  // Original backrooms-style generation
  void generateBackroomsLayout(int chunkX, int chunkZ);
  void addRandomRooms(int chunkX, int chunkZ);
  void addCorridors(int chunkX, int chunkZ);

  // Exact Python translations
  void generateRooms(int numRooms, int widthMin, int widthMax, int heightMin, int heightMax);
  void generatePillarRooms(int numRooms, int widthMin, int widthMax, int heightMin, int heightMax, int spacingMin, int spacingMax);
  void generateCustomRooms(int numRooms, int minSides, int maxSides, int minRadius, int maxRadius);
  void generateBackroomCorridors();
  bool isInsideCustomRoom(int x, int z, const std::vector<glm::ivec2> &vertices);
};

#endif