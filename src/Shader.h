﻿#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h>
#include <glm/fwd.hpp>

#include "Light.h"

class Shader
{
public:
    GLuint ID;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setLight(const std::string& name, const Light& light) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    std::string readFile(const std::string& filePath);
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif
