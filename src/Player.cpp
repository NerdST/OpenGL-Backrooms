#include "Player.h"
#include <algorithm>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Player::Player(Camera *cam, glm::vec3 startPos)
    : position(startPos), velocity(0.0f), camera(cam), isGrounded(false),
      isRunning(false), godMode(true) // Start in god mode (current behavior)
{
  // Player dimensions
  height = 1.8f;    // Player is 1.8m tall
  radius = 0.3f;    // 30cm radius for collision
  eyeHeight = 1.6f; // Eyes are 1.6m from ground

  // Movement properties
  walkSpeed = 3.0f; // 3 m/s walking
  runSpeed = 6.0f;  // 6 m/s running
  jumpForce = 8.0f; // Jump velocity
  gravity = -20.0f; // Strong gravity for responsive feel

  // Set initial camera position
  if (camera)
  {
    camera->Position = GetCameraPosition();
  }
}

void Player::Update(float deltaTime, const MazeGenerator &maze)
{
  if (godMode)
  {
    // In god mode, camera handles its own movement
    // Just sync our position with camera
    position = camera->Position;
    position.y -= eyeHeight; // Adjust for eye height
    return;
  }

  // Player mode - apply physics
  ApplyGravity(deltaTime);

  // Apply velocity to position with collision detection
  glm::vec3 newPosition = position + velocity * deltaTime;

  // Check and resolve collisions
  newPosition = ResolveCollision(position, newPosition, maze);

  // Update position
  position = newPosition;

  // Update camera position to follow player
  if (camera)
  {
    camera->Position = GetCameraPosition();
  }

  // Check if we're on the ground
  isGrounded = CheckGroundCollision(position, maze);

  // Stop downward velocity if we hit the ground
  if (isGrounded && velocity.y <= 0)
  {
    velocity.y = 0.0f;
    position.y = FLOOR_HEIGHT; // Snap to floor
  }
}

void Player::ProcessMovement(PlayerMovement direction, float deltaTime, bool running)
{
  if (godMode)
  {
    // In god mode, use camera's movement system with proper speed
    float speed = running ? 8.0f : 5.0f; // Faster in god mode
    Camera_Movement camDir;
    switch (direction)
    {
    case P_FORWARD:
      camDir = FORWARD;
      break;
    case P_BACKWARD:
      camDir = BACKWARD;
      break;
    case P_LEFT:
      camDir = LEFT;
      break;
    case P_RIGHT:
      camDir = RIGHT;
      break;
    case P_JUMP:
      camDir = UP;
      break;
    default:
      return;
    }
    camera->ProcessKeyboard(camDir, deltaTime * speed);
    return;
  }

  // Player mode movement
  isRunning = running;
  float speed = isRunning ? runSpeed : walkSpeed;

  // Get movement direction based on camera's front and right vectors
  glm::vec3 front = glm::normalize(glm::vec3(camera->Front.x, 0.0f, camera->Front.z));
  glm::vec3 right = glm::normalize(glm::vec3(camera->Right.x, 0.0f, camera->Right.z));

  glm::vec3 moveDirection(0.0f);

  switch (direction)
  {
  case P_FORWARD:
    moveDirection += front;
    break;
  case P_BACKWARD:
    moveDirection -= front;
    break;
  case P_LEFT:
    moveDirection -= right;
    break;
  case P_RIGHT:
    moveDirection += right;
    break;
  case P_JUMP:
    HandleJump();
    return;
  }

  // Normalize diagonal movement
  if (glm::length(moveDirection) > 0.0f)
  {
    moveDirection = glm::normalize(moveDirection);
  }

  // Apply horizontal movement (only affect X and Z, reset each frame)
  glm::vec3 horizontalMovement = moveDirection * speed;
  velocity.x = horizontalMovement.x;
  velocity.z = horizontalMovement.z;
}

void Player::ProcessCombinedMovement(bool forward, bool backward, bool left, bool right, bool jump, float deltaTime, bool running)
{
  if (godMode)
  {
    // This shouldn't be called in god mode, but handle it just in case
    return;
  }

  // Player mode movement with combined inputs
  isRunning = running;
  float speed = isRunning ? runSpeed : walkSpeed;

  // Get movement direction based on camera's front and right vectors
  glm::vec3 front = glm::normalize(glm::vec3(camera->Front.x, 0.0f, camera->Front.z));
  glm::vec3 rightDir = glm::normalize(glm::vec3(camera->Right.x, 0.0f, camera->Right.z));

  glm::vec3 moveDirection(0.0f);

  // Accumulate all movement directions
  if (forward)
    moveDirection += front;
  if (backward)
    moveDirection -= front;
  if (left)
    moveDirection -= rightDir;
  if (right)
    moveDirection += rightDir;

  // Handle jump
  if (jump)
  {
    HandleJump();
  }

  // Normalize diagonal movement to prevent faster diagonal movement
  if (glm::length(moveDirection) > 0.0f)
  {
    moveDirection = glm::normalize(moveDirection);
  }

  // Apply horizontal movement (only affect X and Z)
  glm::vec3 horizontalMovement = moveDirection * speed;
  velocity.x = horizontalMovement.x;
  velocity.z = horizontalMovement.z;

  // If no movement keys are pressed, stop horizontal movement
  if (!forward && !backward && !left && !right)
  {
    velocity.x = 0.0f;
    velocity.z = 0.0f;
  }
}

void Player::ToggleGodMode()
{
  godMode = !godMode;

  if (godMode)
  {
    std::cout << "God Mode: ENABLED (Fly camera)" << std::endl;
    // When entering god mode, sync camera with current position
    camera->Position = GetCameraPosition();
  }
  else
  {
    std::cout << "Player Mode: ENABLED (Physics + Collision)" << std::endl;
    // When entering player mode, place player on ground
    position = camera->Position;
    position.y = FLOOR_HEIGHT;  // Start on floor
    velocity = glm::vec3(0.0f); // Reset velocity
  }
}

bool Player::CheckCollision(const glm::vec3 &newPos, const MazeGenerator &maze)
{
  return CheckWallCollision(newPos, maze);
}

glm::vec3 Player::GetCameraPosition() const
{
  return position + glm::vec3(0.0f, eyeHeight, 0.0f);
}

void Player::SetPosition(const glm::vec3 &newPos, const MazeGenerator &maze)
{
  if (!CheckCollision(newPos, maze))
  {
    position = newPos;
    if (camera)
    {
      camera->Position = GetCameraPosition();
    }
  }
}

bool Player::CheckWallCollision(const glm::vec3 &pos, const MazeGenerator &maze)
{
  // Check collision with walls using player's radius
  // Test multiple points around the player's circular hitbox

  const int numTests = 8; // Test 8 points around the circle
  for (int i = 0; i < numTests; i++)
  {
    float angle = (float)i / numTests * 2.0f * M_PI;
    float testX = pos.x + cos(angle) * radius;
    float testZ = pos.z + sin(angle) * radius;

    // Convert world coordinates to grid coordinates
    // World position (x * CELL_SIZE, 0, z * CELL_SIZE) maps to grid (x, z)
    // So world coordinate w maps to grid coordinate (int)(w / CELL_SIZE + 0.5) for proper rounding
    int gridX = (int)floor(testX / CELL_SIZE + 0.5f);
    int gridZ = (int)floor(testZ / CELL_SIZE + 0.5f);

    // Check if this position is a wall
    if (maze.isWall(gridX, gridZ))
    {
      return true; // Collision detected
    }
  }

  return false; // No collision
}

bool Player::CheckGroundCollision(const glm::vec3 &pos, const MazeGenerator &maze)
{
  // Player is on ground if they're at or below floor level
  // and not inside a wall
  return pos.y <= FLOOR_HEIGHT && !CheckWallCollision(pos, maze);
}

glm::vec3 Player::ResolveCollision(const glm::vec3 &oldPos, const glm::vec3 &newPos, const MazeGenerator &maze)
{
  // If no collision, accept the new position
  if (!CheckCollision(newPos, maze))
  {
    return newPos;
  }

  // Try moving only horizontally (X and Z separately)
  glm::vec3 testPos;

  // Try X movement only
  testPos = glm::vec3(newPos.x, oldPos.y, oldPos.z);
  if (!CheckCollision(testPos, maze))
  {
    // X movement is ok, now try adding Z
    testPos.z = newPos.z;
    if (!CheckCollision(testPos, maze))
    {
      return glm::vec3(newPos.x, newPos.y, newPos.z); // Both X and Z ok
    }
    else
    {
      return glm::vec3(newPos.x, newPos.y, oldPos.z); // Only X ok
    }
  }

  // Try Z movement only
  testPos = glm::vec3(oldPos.x, oldPos.y, newPos.z);
  if (!CheckCollision(testPos, maze))
  {
    return glm::vec3(oldPos.x, newPos.y, newPos.z); // Only Z ok
  }

  // No horizontal movement possible, but allow vertical
  return glm::vec3(oldPos.x, newPos.y, oldPos.z);
}

void Player::ApplyGravity(float deltaTime)
{
  if (!isGrounded)
  {
    velocity.y += gravity * deltaTime;
  }
}

void Player::HandleJump()
{
  if (isGrounded && !godMode)
  {
    velocity.y = jumpForce;
    isGrounded = false;
  }
}
