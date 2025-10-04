#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h>
#include <shader.h>
#include "Mesh.h"
#include "TextureManager.h"
#include "Primitives.h"
#include "MazeGenerator.h"

// ImGui includes
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <iostream>
#include <vector>
#include <memory>
#include <ctime>

// Function declarations
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void setupLighting(Shader &shader, const Camera &camera);
void renderMaze(const MazeGenerator &maze, Shader &shader, Shader &lightShader, Mesh &wallMesh, Mesh &floorMesh, Mesh &ceilingMesh,
                unsigned int wallTex, unsigned int floorTex, unsigned int ceilingTex);
void renderUI(const Camera &camera, MazeGenerator &maze);

// Settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// Camera - Start at center of maze
Camera camera(glm::vec3(50.0f, 1.0f, 50.0f)); // Center of a 50x50 maze
bool firstMouse = true;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool mouseDisabled = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Backrooms settings
bool enableFlashlight = true;
float ambientStrength = 0.05f;
float flashlightIntensity = 1.0f; // Flashlight brightness control
float flashlightAngle = 12.5f;    // Flashlight cone angle in degrees
float lightTileIntensity = 0.8f;  // Visual brightness of glowing tiles
glm::vec3 wallColor = glm::vec3(0.9f, 0.9f, 0.7f);
glm::vec3 floorColor = glm::vec3(0.4f, 0.6f, 0.3f);
glm::vec3 lightTileColor(1.0f, 1.0f, 0.9f);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Backrooms - Infinite Maze", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSwapInterval(0);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);

    Shader backroomsShader("data/shaders/backrooms.vs", "data/shaders/backrooms.fs");
    Shader lightTileShader("data/shaders/lightTile.vs", "data/shaders/lightTile.fs");

    auto wallMesh = Primitives::createWall(2.0f, 3.0f);
    auto floorMesh = Primitives::createFloor(2.0f, 2.0f);
    auto ceilingMesh = Primitives::createCeiling(2.0f, 2.0f);

    auto &texManager = TextureManager::getInstance();
    unsigned int wallTexture = texManager.loadTexture("data/textures/backrooms_wall.png");
    unsigned int floorTexture = texManager.loadTexture("data/textures/backrooms_floor.png");
    unsigned int ceilingTexture = texManager.loadTexture("data/textures/backrooms_ceiling.png");

    MazeGenerator maze(75, 75, 12345);
    maze.generateMaze();

    std::cout << "Generated maze with " << maze.getWidth() << "x" << maze.getHeight() << " cells" << std::endl;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        renderUI(camera, maze);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        backroomsShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        backroomsShader.setMat4("projection", projection);
        backroomsShader.setMat4("view", view);

        setupLighting(backroomsShader, camera);
        renderMaze(maze, backroomsShader, lightTileShader, wallMesh, floorMesh, ceilingMesh,
                   wallTexture, floorTexture, ceilingTexture);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void renderMaze(const MazeGenerator &maze, Shader &shader, Shader &lightShader, Mesh &wallMesh, Mesh &floorMesh, Mesh &ceilingMesh,
                unsigned int wallTex, unsigned int floorTex, unsigned int ceilingTex)
{
    const float CELL_SIZE = 2.0f;
    const float WALL_HEIGHT = 3.5f;
    const float CEILING_TILE_SIZE = CELL_SIZE / 2.0f; // 4 ceiling tiles per cell (2x2)
    const float WALL_OFFSET = CELL_SIZE * 0.5f;
    const float HALF_WALL_HEIGHT = WALL_HEIGHT / 2.0f;
    const float WALL_SCALE_Y = WALL_HEIGHT / 3.0f;
    const glm::vec3 DARKER_WALL_COLOR = wallColor * 0.85f;

    glm::vec3 camPos = camera.Position;
    int centerX = static_cast<int>(camPos.x / CELL_SIZE);
    int centerZ = static_cast<int>(camPos.z / CELL_SIZE);
    int renderDistance = 18; // Slightly reduced for better performance

    // Cache matrices for light shader to avoid recalculating
    glm::mat4 lightProjection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 lightView = camera.GetViewMatrix();

    for (int z = centerZ - renderDistance; z <= centerZ + renderDistance; ++z)
    {
        for (int x = centerX - renderDistance; x <= centerX + renderDistance; ++x)
        {
            if (!maze.isValidCell(x, z))
                continue;

            glm::vec3 position(x * CELL_SIZE, 0.0f, z * CELL_SIZE);
            glm::mat4 model;

            if (!maze.isWall(x, z))
            {
                // Render floor (texture binding moved outside loop when possible)
                shader.use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, floorTex);
                shader.setInt("texture1", 0);

                model = glm::mat4(1.0f);
                model = glm::translate(model, position);
                shader.setMat4("model", model);
                shader.setVec3("objectColor", floorColor);
                floorMesh.Draw(shader);

                // Render 4 ceiling tiles (2x2 grid) - bind ceiling texture once
                glBindTexture(GL_TEXTURE_2D, ceilingTex);

                const float ceilingScale = CEILING_TILE_SIZE / CELL_SIZE;
                for (int cz = 0; cz < 2; ++cz)
                {
                    for (int cx = 0; cx < 2; ++cx)
                    {
                        // Pre-calculate positions
                        glm::vec3 ceilingTilePos = position + glm::vec3(
                                                                  (cx - 0.5f) * CEILING_TILE_SIZE,
                                                                  WALL_HEIGHT,
                                                                  (cz - 0.5f) * CEILING_TILE_SIZE);

                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ceilingTilePos);
                        model = glm::scale(model, glm::vec3(ceilingScale));

                        // Check if this should be a light tile
                        int globalCeilingX = (x * 2) + cx;
                        int globalCeilingZ = (z * 2) + cz;

                        if (globalCeilingX % 4 == 0 && globalCeilingZ % 3 == 0)
                        {
                            // Use light tile shader for visual effect only (use cached matrices)
                            lightShader.use();
                            lightShader.setMat4("projection", lightProjection);
                            lightShader.setMat4("view", lightView);
                            lightShader.setMat4("model", model);
                            lightShader.setVec3("lightColor", lightTileColor);
                            lightShader.setFloat("intensity", ambientStrength * 15.0f + lightTileIntensity); // Linked to ambient
                            ceilingMesh.Draw(lightShader);
                            shader.use();
                        }
                        else
                        {
                            shader.setMat4("model", model);
                            shader.setVec3("objectColor", DARKER_WALL_COLOR);
                            ceilingMesh.Draw(shader);
                        }
                    }
                }

                // Render walls (bind wall texture once)
                glBindTexture(GL_TEXTURE_2D, wallTex);
                shader.setVec3("objectColor", wallColor);

                // North wall (+Z)
                if (maze.isWall(x, z + 1))
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, position + glm::vec3(0.0f, HALF_WALL_HEIGHT, WALL_OFFSET));
                    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3(1.0f, WALL_SCALE_Y, 1.0f));
                    shader.setMat4("model", model);
                    wallMesh.Draw(shader);
                }

                // South wall (-Z)
                if (maze.isWall(x, z - 1))
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, position + glm::vec3(0.0f, HALF_WALL_HEIGHT, -WALL_OFFSET));
                    model = glm::scale(model, glm::vec3(1.0f, WALL_SCALE_Y, 1.0f));
                    shader.setMat4("model", model);
                    wallMesh.Draw(shader);
                }

                // East wall (+X)
                if (maze.isWall(x + 1, z))
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, position + glm::vec3(WALL_OFFSET, HALF_WALL_HEIGHT, 0.0f));
                    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3(1.0f, WALL_SCALE_Y, 1.0f));
                    shader.setMat4("model", model);
                    wallMesh.Draw(shader);
                }

                // West wall (-X)
                if (maze.isWall(x - 1, z))
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, position + glm::vec3(-WALL_OFFSET, HALF_WALL_HEIGHT, 0.0f));
                    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3(1.0f, WALL_SCALE_Y, 1.0f));
                    shader.setMat4("model", model);
                    wallMesh.Draw(shader);
                }
            }
        }
    }
}

void processInput(GLFWwindow *window)
{
    float speed = 5.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime * speed);
}

void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *, double xpos, double ypos)
{
    if (!mouseDisabled)
        return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *, double, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void key_callback(GLFWwindow *window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
    {
        mouseDisabled = !mouseDisabled;
        if (mouseDisabled)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        enableFlashlight = !enableFlashlight;
    }
}

void setupLighting(Shader &shader, const Camera &camera)
{
    shader.setVec3("viewPos", camera.Position);
    shader.setFloat("ambientStrength", ambientStrength);

    if (enableFlashlight)
    {
        shader.setVec3("spotlight.position", camera.Position);
        shader.setVec3("spotlight.direction", camera.Front);
        shader.setFloat("spotlight.cutOff", glm::cos(glm::radians(flashlightAngle)));
        shader.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(flashlightAngle + 5.0f))); // 5 degree falloff
        shader.setVec3("spotlight.color", 1.0f, 0.9f, 0.8f);
        shader.setFloat("spotlight.intensity", flashlightIntensity);
    }
    else
    {
        shader.setFloat("spotlight.intensity", 0.0f);
    }
}

void renderUI(const Camera &camera, MazeGenerator &maze)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Backrooms Control Panel");

    ImGui::Text("FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Separator();

    ImGui::Text("Camera Position: %.1f, %.1f, %.1f", camera.Position.x, camera.Position.y, camera.Position.z);
    ImGui::Text("Camera Direction: %.2f, %.2f, %.2f", camera.Front.x, camera.Front.y, camera.Front.z);
    ImGui::Text("Looking: %s",
                (abs(camera.Front.z) > abs(camera.Front.x)) ? (camera.Front.z > 0 ? "North (+Z)" : "South (-Z)") : (camera.Front.x > 0 ? "East (+X)" : "West (-X)"));
    ImGui::Text("Press ALT to toggle mouse");
    ImGui::Text("Use WASD to move, Space/Shift for up/down");
    ImGui::Separator();

    ImGui::Checkbox("Enable Flashlight", &enableFlashlight);
    ImGui::SliderFloat("Ambient Light", &ambientStrength, 0.0f, 0.3f);
    ImGui::SliderFloat("Flashlight Intensity", &flashlightIntensity, 0.0f, 3.0f);
    ImGui::SliderFloat("Flashlight Angle", &flashlightAngle, 5.0f, 45.0f);
    ImGui::SliderFloat("Light Tile Glow", &lightTileIntensity, 0.0f, 2.0f);
    ImGui::ColorEdit3("Wall Color", &wallColor.x);
    ImGui::ColorEdit3("Floor Color", &floorColor.x);
    ImGui::ColorEdit3("Light Color", &lightTileColor.x);

    if (ImGui::Button("Generate New Maze"))
    {
        maze = MazeGenerator(75, 75, std::time(nullptr));
        maze.generateMaze();
    }

    if (ImGui::Button("Generate Backrooms Maze"))
    {
        maze = MazeGenerator(75, 75, std::time(nullptr));
        maze.generateBackroomsMaze();
    }

    ImGui::End();
}
