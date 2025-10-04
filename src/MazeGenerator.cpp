#include "MazeGenerator.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MazeGenerator::MazeGenerator(int width, int height, unsigned int seed)
    : width(width), height(height), rng(seed == 0 ? std::random_device{}() : seed)
{
  cells.resize(width * height);

  // Initialize all cells as walls
  for (int z = 0; z < height; ++z)
  {
    for (int x = 0; x < width; ++x)
    {
      int index = getIndex(x, z);
      cells[index].type = CellType::WALL;
      cells[index].position = glm::vec3(x * 2.0f, 0.0f, z * 2.0f);
      cells[index].visited = false;
    }
  }
}

void MazeGenerator::generateMaze()
{
  // Start carving from center, but ensure odd coordinates for proper maze generation
  int startX = (width / 2) | 1;  // Make odd
  int startZ = (height / 2) | 1; // Make odd

  carvePath(startX, startZ);

  // Add some randomness for backrooms feel
  std::uniform_real_distribution<float> chance(0.0f, 1.0f);
  for (int i = 0; i < (int)cells.size(); ++i)
  {
    if (cells[i].type == CellType::WALL && chance(rng) < 0.05f)
    {
      cells[i].type = CellType::FLOOR;
    }
  }
}

void MazeGenerator::generateBackroomsMaze()
{
  // Backrooms generator parameters (adjusted for dense authentic backrooms feel)
  const float MAZE_FILL_PERCENTAGE = 0.4f;                         // Much lower for denser wall structure
  const int NUM_MAZES = 20;                                        // Fewer overlapping mazes to maintain structure
  const float STOP_COLLISION_PROBABILITY = 0.8f;                   // Higher probability to create more walls
  const int NUM_ROOMS = 15;                                        // Many more rooms for authentic backrooms density
  const int ROOM_WIDTH_MIN = 3, ROOM_WIDTH_MAX = 8;                // Smaller rooms, more cramped
  const int ROOM_HEIGHT_MIN = 3, ROOM_HEIGHT_MAX = 8;              // Smaller rooms
  const int NUM_PILLAR_ROOMS = 8;                                  // More pillar rooms for obstacles
  const int PILLAR_ROOM_WIDTH_MIN = 4, PILLAR_ROOM_WIDTH_MAX = 12; // Reasonable pillar room sizes
  const int PILLAR_ROOM_HEIGHT_MIN = 4, PILLAR_ROOM_HEIGHT_MAX = 12;
  const int PILLAR_SPACING_MIN = 2, PILLAR_SPACING_MAX = 4;         // Closer pillar spacing
  const int NUM_CUSTOM_ROOMS = 5;                                   // More custom rooms
  const int MIN_NUM_SIDES = 3, MAX_NUM_SIDES = 6;                   // More reasonable polygon shapes
  const int MIN_CUSTOM_ROOM_RADIUS = 2, MAX_CUSTOM_ROOM_RADIUS = 6; // Smaller custom rooms

  // Initialize all cells as walls (visited = false means wall, visited = true means floor)
  for (auto &cell : cells)
  {
    cell.type = CellType::WALL;
    cell.visited = false;
  }

  std::set<std::pair<int, int>> visitedCells;
  int targetCells = static_cast<int>(width * height * MAZE_FILL_PERCENTAGE);

  std::uniform_int_distribution<int> xDist(0, width - 1);
  std::uniform_int_distribution<int> zDist(0, height - 1);
  std::uniform_real_distribution<float> randomFloat(0.0f, 1.0f);

  // Generate multiple overlapping mazes using Prim's algorithm (exact Python translation)
  for (int mazeNum = 0; mazeNum < NUM_MAZES; ++mazeNum)
  {
    if (static_cast<int>(visitedCells.size()) >= targetCells)
      break;

    int x = xDist(rng);
    int z = zDist(rng);

    visitedCells.insert({x, z});
    std::vector<std::pair<int, int>> frontier;
    frontier.push_back({x, z});

    while (static_cast<int>(visitedCells.size()) < targetCells)
    {
      if (frontier.empty())
        break;

      // Pick random cell from frontier
      int idx = std::uniform_int_distribution<int>(0, frontier.size() - 1)(rng);
      auto currentCell = frontier[idx];
      frontier.erase(frontier.begin() + idx);

      x = currentCell.first;
      z = currentCell.second;

      visitedCells.insert({x, z});
      cells[getIndex(x, z)].visited = true;
      cells[getIndex(x, z)].type = CellType::FLOOR;

      // Check neighbors exactly like Python
      std::vector<std::pair<int, int>> neighbors;
      if (x > 1 && visitedCells.find({x - 2, z}) == visitedCells.end())
        neighbors.push_back({x - 2, z});
      if (x < width - 2 && visitedCells.find({x + 2, z}) == visitedCells.end())
        neighbors.push_back({x + 2, z});
      if (z > 1 && visitedCells.find({x, z - 2}) == visitedCells.end())
        neighbors.push_back({x, z - 2});
      if (z < height - 2 && visitedCells.find({x, z + 2}) == visitedCells.end())
        neighbors.push_back({x, z + 2});

      if (!neighbors.empty())
      {
        auto nextCell = neighbors[std::uniform_int_distribution<int>(0, neighbors.size() - 1)(rng)];
        int nx = nextCell.first;
        int nz = nextCell.second;

        // Check collision with previous maze (exact Python logic)
        int betweenX = (x + nx) / 2;
        int betweenZ = (z + nz) / 2;

        if (randomFloat(rng) > STOP_COLLISION_PROBABILITY ||
            !cells[getIndex(betweenX, betweenZ)].visited)
        {
          frontier.push_back({nx, nz});
          cells[getIndex(betweenX, betweenZ)].visited = true;
          cells[getIndex(betweenX, betweenZ)].type = CellType::FLOOR;
        }
      }
    }
  }

  // Generate rooms (exact Python translation)
  generateRooms(NUM_ROOMS, ROOM_WIDTH_MIN, ROOM_WIDTH_MAX, ROOM_HEIGHT_MIN, ROOM_HEIGHT_MAX);

  // Generate pillar rooms (exact Python translation)
  generatePillarRooms(NUM_PILLAR_ROOMS, PILLAR_ROOM_WIDTH_MIN, PILLAR_ROOM_WIDTH_MAX,
                      PILLAR_ROOM_HEIGHT_MIN, PILLAR_ROOM_HEIGHT_MAX,
                      PILLAR_SPACING_MIN, PILLAR_SPACING_MAX);

  // Generate custom shaped rooms (exact Python translation)
  generateCustomRooms(NUM_CUSTOM_ROOMS, MIN_NUM_SIDES, MAX_NUM_SIDES,
                      MIN_CUSTOM_ROOM_RADIUS, MAX_CUSTOM_ROOM_RADIUS);

  std::cout << "Generated backrooms-style maze with " << width << "x" << height << " cells" << std::endl;
}

void MazeGenerator::generateChunk(int chunkX, int chunkZ)
{
  generateBackroomsLayout(chunkX, chunkZ);
}

void MazeGenerator::generateBackroomsLayout(int chunkX, int chunkZ)
{
  // Generate a backrooms-style layout for this chunk
  std::uniform_real_distribution<float> roomChance(0.0f, 1.0f);
  std::uniform_int_distribution<int> roomSize(3, 8);

  int startX = chunkX * CHUNK_SIZE;
  int startZ = chunkZ * CHUNK_SIZE;

  // Create a grid of rooms and corridors
  for (int z = 0; z < CHUNK_SIZE; z += 4)
  {
    for (int x = 0; x < CHUNK_SIZE; x += 4)
    {
      int worldX = startX + x;
      int worldZ = startZ + z;

      if (roomChance(rng) < 0.7f)
      { // 70% chance for room
        int roomW = std::min(roomSize(rng), CHUNK_SIZE - x);
        int roomH = std::min(roomSize(rng), CHUNK_SIZE - z);

        // Carve out room
        for (int rz = 0; rz < roomH; ++rz)
        {
          for (int rx = 0; rx < roomW; ++rx)
          {
            if (isValidCell(worldX + rx, worldZ + rz))
            {
              int index = getIndex(worldX + rx, worldZ + rz);
              cells[index].type = CellType::FLOOR;
            }
          }
        }
      }
    }
  }

  addCorridors(chunkX, chunkZ);
}

void MazeGenerator::addCorridors(int chunkX, int chunkZ)
{
  int startX = chunkX * CHUNK_SIZE;
  int startZ = chunkZ * CHUNK_SIZE;

  // Add horizontal corridors
  for (int z = 0; z < CHUNK_SIZE; z += 8)
  {
    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
      int worldX = startX + x;
      int worldZ = startZ + z;

      if (isValidCell(worldX, worldZ))
      {
        int index = getIndex(worldX, worldZ);
        cells[index].type = CellType::FLOOR;
      }
    }
  }

  // Add vertical corridors
  for (int x = 0; x < CHUNK_SIZE; x += 8)
  {
    for (int z = 0; z < CHUNK_SIZE; ++z)
    {
      int worldX = startX + x;
      int worldZ = startZ + z;

      if (isValidCell(worldX, worldZ))
      {
        int index = getIndex(worldX, worldZ);
        cells[index].type = CellType::FLOOR;
      }
    }
  }
}

void MazeGenerator::carvePath(int x, int z)
{
  if (!isValidCell(x, z))
    return;

  int index = getIndex(x, z);
  cells[index].type = CellType::FLOOR;
  cells[index].visited = true;

  auto neighbors = getNeighbors(x, z);
  std::shuffle(neighbors.begin(), neighbors.end(), rng);

  for (const auto &neighbor : neighbors)
  {
    int nx = neighbor.x;
    int nz = neighbor.y;

    if (isValidCell(nx, nz) && !cells[getIndex(nx, nz)].visited)
    {
      // Carve path between current cell and neighbor
      int betweenX = x + (nx - x) / 2;
      int betweenZ = z + (nz - z) / 2;

      if (isValidCell(betweenX, betweenZ))
      {
        cells[getIndex(betweenX, betweenZ)].type = CellType::FLOOR;
      }

      carvePath(nx, nz);
    }
  }
}

std::vector<glm::ivec2> MazeGenerator::getNeighbors(int x, int z)
{
  std::vector<glm::ivec2> neighbors;

  // Get neighbors 2 cells away (for maze generation)
  if (x >= 2)
    neighbors.push_back({x - 2, z});
  if (x < width - 2)
    neighbors.push_back({x + 2, z});
  if (z >= 2)
    neighbors.push_back({x, z - 2});
  if (z < height - 2)
    neighbors.push_back({x, z + 2});

  return neighbors;
}

CellType MazeGenerator::getCellType(int x, int z) const
{
  if (!isValidCell(x, z))
    return CellType::WALL;
  return cells[getIndex(x, z)].type;
}

bool MazeGenerator::isWall(int x, int z) const
{
  // Cells outside the maze bounds are considered walls
  if (!isValidCell(x, z))
  {
    return true;
  }
  return getCellType(x, z) == CellType::WALL;
}

bool MazeGenerator::isFloor(int x, int z) const
{
  return getCellType(x, z) == CellType::FLOOR;
}

bool MazeGenerator::isValidCell(int x, int z) const
{
  return x >= 0 && x < width && z >= 0 && z < height;
}

int MazeGenerator::getIndex(int x, int z) const
{
  return z * width + x;
}

std::vector<MazeCell> MazeGenerator::getChunk(int chunkX, int chunkZ) const
{
  std::vector<MazeCell> chunk;

  int startX = chunkX * CHUNK_SIZE;
  int startZ = chunkZ * CHUNK_SIZE;

  for (int z = 0; z < CHUNK_SIZE; ++z)
  {
    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
      int worldX = startX + x;
      int worldZ = startZ + z;

      if (isValidCell(worldX, worldZ))
      {
        chunk.push_back(cells[getIndex(worldX, worldZ)]);
      }
    }
  }

  return chunk;
}

// Exact translations of Python functions
void MazeGenerator::generateRooms(int numRooms, int widthMin, int widthMax, int heightMin, int heightMax)
{
  std::uniform_int_distribution<int> widthDist(widthMin, widthMax);
  std::uniform_int_distribution<int> heightDist(heightMin, heightMax);

  for (int i = 0; i < numRooms; ++i)
  {
    int roomWidth = widthDist(rng);
    int roomHeight = heightDist(rng);

    if (width <= roomWidth || height <= roomHeight)
      continue;

    int x = std::uniform_int_distribution<int>(0, width - roomWidth)(rng);
    int z = std::uniform_int_distribution<int>(0, height - roomHeight)(rng);

    // Carve out room (exact Python logic)
    for (int row = z; row < z + roomHeight; ++row)
    {
      for (int col = x; col < x + roomWidth; ++col)
      {
        if (isValidCell(col, row))
        {
          cells[getIndex(col, row)].visited = true;
          cells[getIndex(col, row)].type = CellType::FLOOR;
        }
      }
    }
  }
}

void MazeGenerator::generatePillarRooms(int numRooms, int widthMin, int widthMax,
                                        int heightMin, int heightMax,
                                        int spacingMin, int spacingMax)
{
  std::uniform_int_distribution<int> widthDist(widthMin, widthMax);
  std::uniform_int_distribution<int> heightDist(heightMin, heightMax);
  std::uniform_int_distribution<int> spacingDist(spacingMin, spacingMax);

  for (int i = 0; i < numRooms; ++i)
  {
    int roomWidth = widthDist(rng);
    int roomHeight = heightDist(rng);

    if (width <= roomWidth || height <= roomHeight)
      continue;

    int x = std::uniform_int_distribution<int>(0, width - roomWidth)(rng);
    int z = std::uniform_int_distribution<int>(0, height - roomHeight)(rng);

    // First carve out the room (exact Python logic)
    for (int row = z; row < z + roomHeight; ++row)
    {
      for (int col = x; col < x + roomWidth; ++col)
      {
        if (isValidCell(col, row))
        {
          cells[getIndex(col, row)].visited = true;
          cells[getIndex(col, row)].type = CellType::FLOOR;
        }
      }
    }

    // Add pillars (exact Python logic)
    int pillarSpacing = spacingDist(rng);
    for (int row = z; row < z + roomHeight; row += pillarSpacing)
    {
      for (int col = x; col < x + roomWidth; col += pillarSpacing)
      {
        if (isValidCell(col, row))
        {
          cells[getIndex(col, row)].visited = false;
          cells[getIndex(col, row)].type = CellType::WALL;
        }
      }
    }
  }
}

void MazeGenerator::generateCustomRooms(int numRooms, int minSides, int maxSides, int minRadius, int maxRadius)
{
  std::uniform_int_distribution<int> sidesDist(minSides, maxSides);
  std::uniform_int_distribution<int> radiusDist(minRadius, maxRadius);

  for (int i = 0; i < numRooms; ++i)
  {
    int numSides = sidesDist(rng);
    int roomRadius = radiusDist(rng);

    if (width <= roomRadius * 4 || height <= roomRadius * 4)
      continue;

    int x = std::uniform_int_distribution<int>(roomRadius * 2, width - roomRadius * 2)(rng);
    int z = std::uniform_int_distribution<int>(roomRadius * 2, height - roomRadius * 2)(rng);

    // Generate vertices (exact Python logic)
    std::vector<glm::ivec2> vertices;
    float angleStep = 2.0f * M_PI / numSides;
    for (int j = 0; j < numSides; ++j)
    {
      float angle = j * angleStep;
      int vertexX = static_cast<int>(x + roomRadius * cos(angle));
      int vertexZ = static_cast<int>(z + roomRadius * sin(angle));
      vertices.push_back({vertexX, vertexZ});
    }

    // Carve out the custom-shaped room (exact Python logic)
    for (int row = z - roomRadius; row <= z + roomRadius; ++row)
    {
      for (int col = x - roomRadius; col <= x + roomRadius; ++col)
      {
        if (isValidCell(col, row) && isInsideCustomRoom(col, row, vertices))
        {
          cells[getIndex(col, row)].visited = true;
          cells[getIndex(col, row)].type = CellType::FLOOR;
        }
      }
    }
  }
}

// Exact Python point-in-polygon algorithm
bool MazeGenerator::isInsideCustomRoom(int x, int z, const std::vector<glm::ivec2> &vertices)
{
  int numVertices = vertices.size();
  bool inside = false;

  for (int i = 0; i < numVertices; ++i)
  {
    int j = (i + 1) % numVertices;

    glm::ivec2 leftVertex, rightVertex;
    if (vertices[i].x < vertices[j].x)
    {
      leftVertex = vertices[i];
      rightVertex = vertices[j];
    }
    else
    {
      leftVertex = vertices[j];
      rightVertex = vertices[i];
    }

    if ((vertices[i].y > z) != (vertices[j].y > z) &&
        x < (rightVertex.x - leftVertex.x) * (z - leftVertex.y) / (rightVertex.y - leftVertex.y) + leftVertex.x)
    {
      inside = !inside;
    }
  }

  return inside;
}

void MazeGenerator::generateBackroomCorridors()
{
  std::uniform_int_distribution<int> corridorSpacing(4, 8);
  std::uniform_int_distribution<int> corridorWidth(1, 2);
  std::uniform_real_distribution<float> skipChance(0.0f, 1.0f);

  // Generate horizontal corridors
  for (int z = corridorSpacing(rng); z < height; z += corridorSpacing(rng))
  {
    if (skipChance(rng) > 0.3f) // 70% chance to create corridor
    {
      int width_corridor = corridorWidth(rng);
      for (int x = 0; x < width; ++x)
      {
        for (int w = 0; w < width_corridor && z + w < height; ++w)
        {
          if (isValidCell(x, z + w))
          {
            cells[getIndex(x, z + w)].visited = true;
            cells[getIndex(x, z + w)].type = CellType::FLOOR;
          }
        }
      }
    }
  }

  // Generate vertical corridors
  for (int x = corridorSpacing(rng); x < width; x += corridorSpacing(rng))
  {
    if (skipChance(rng) > 0.3f) // 70% chance to create corridor
    {
      int width_corridor = corridorWidth(rng);
      for (int z = 0; z < height; ++z)
      {
        for (int w = 0; w < width_corridor && x + w < width; ++w)
        {
          if (isValidCell(x + w, z))
          {
            cells[getIndex(x + w, z)].visited = true;
            cells[getIndex(x + w, z)].type = CellType::FLOOR;
          }
        }
      }
    }
  }
}