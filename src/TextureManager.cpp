#include "TextureManager.h"
#include <stb_image.h>
#include <iostream>

unsigned int TextureManager::loadTexture(const std::string &path)
{
  // Check if texture is already loaded
  auto it = loadedTextures.find(path);
  if (it != loadedTextures.end())
  {
    return it->second;
  }

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    // Cache the texture
    loadedTextures[path] = textureID;

    std::cout << "Loaded texture: " << path << " (ID: " << textureID << ")" << std::endl;
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

unsigned int TextureManager::getTexture(const std::string &path)
{
  return loadTexture(path); // Will return cached version if already loaded
}

void TextureManager::cleanup()
{
  for (auto &pair : loadedTextures)
  {
    glDeleteTextures(1, &pair.second);
  }
  loadedTextures.clear();
}