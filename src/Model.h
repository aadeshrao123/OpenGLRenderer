#ifndef MODEL_H
#define MODEL_H

#include <tiny_gltf.h>
#include <GL/glew.h>
#include <unordered_map>
#include <vector>
#include <string>

struct GLPrimitive
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLsizei indexCount;
};

class Model
{
public:
    Model(const std::string& path);
    void draw(GLuint shaderProgram);

    tinygltf::Model model; // Make model public for easier access

private:
    std::unordered_map<int, std::vector<GLPrimitive>> primitiveMap;

    void loadModel(const std::string& path);
    GLuint createBuffer(const std::vector<unsigned char>& data, GLenum target);
    void createVAOs();
    GLuint loadTextureFromModel(int textureIndex);
};

#endif
