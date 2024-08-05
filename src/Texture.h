#ifndef TEXTURE_H
#define TEXTURE_H

#include <tiny_gltf.h>
#include <GL/glew.h>
#include <string>
#include <vector>

GLuint createTexture(const tinygltf::Image& image);
GLuint loadCubeMap(const std::vector<std::string>& faces);

#endif
