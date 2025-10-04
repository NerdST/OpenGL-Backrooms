#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include "MazeGenerator.h"

// Player movement states
enum PlayerMovement
{
  P_FORWARD,
  P_BACKWARD,
  P_LEFT,
  P_RIGHT,
  P_JUMP
};

// Player controller with physics and collision detection
class Player
{
public:
  // Player properties
  glm::vec3 position;
  glm::vec3 velocity;
  Camera *camera; // Associated camera

  // Player dimensions (hitbox)
  float height;
  float radius;
  float eyeHeight;

  // Physics properties
  float walkSpeed;
  float runSpeed;
  float jumpForce;
  float gravity;
  bool isGrounded;
  bool isRunning;

  // State
  bool godMode; // Toggle between player mode and fly cam

  // Constructor
  Player(Camera *cam, glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 0.0f));

  // Update player physics and position
  void Update(float deltaTime, const MazeGenerator &maze);

  // Process player input
  void ProcessMovement(PlayerMovement direction, float deltaTime, bool running = false);
  void ProcessCombinedMovement(bool forward, bool backward, bool left, bool right, bool jump, float deltaTime, bool running = false);

  // Toggle between player mode and god mode
  void ToggleGodMode();

  // Collision detection
  bool CheckCollision(const glm::vec3 &newPos, const MazeGenerator &maze);

  // Get the position where the camera should be (at eye level)
  glm::vec3 GetCameraPosition() const;

  // Set player position (with collision checking)
  void SetPosition(const glm::vec3 &newPos, const MazeGenerator &maze);

private:
  // Internal collision helpers
  bool CheckWallCollision(const glm::vec3 &pos, const MazeGenerator &maze);
  bool CheckGroundCollision(const glm::vec3 &pos, const MazeGenerator &maze);
  glm::vec3 ResolveCollision(const glm::vec3 &oldPos, const glm::vec3 &newPos, const MazeGenerator &maze);

  // Physics helpers
  void ApplyGravity(float deltaTime);
  void HandleJump();

  // Constants
  static constexpr float FLOOR_HEIGHT = 0.1f;
  static constexpr float CEILING_HEIGHT = 3.0f;
  static constexpr float CELL_SIZE = 2.0f;
};

#endif
