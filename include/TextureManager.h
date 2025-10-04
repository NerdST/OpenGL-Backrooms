#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <glad/glad.h>
#include <string>
#include <unordered_map>

class TextureManager
{
public:
  static TextureManager &getInstance()
  {
    static TextureManager instance;
    return instance;
  }

  unsigned int loadTexture(const std::string &path);
  unsigned int getTexture(const std::string &path);
  void cleanup();

private:
  std::unordered_map<std::string, unsigned int> loadedTextures;

  TextureManager() = default;
  ~TextureManager() { cleanup(); }

  // Prevent copying
  TextureManager(const TextureManager &) = delete;
  TextureManager &operator=(const TextureManager &) = delete;
};

#endif