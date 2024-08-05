#include <stb_image.h>
#include "Texture.h"
#include <iostream>

GLuint createTexture(const tinygltf::Image& image)
{
    // Check if the image data is valid
    if (image.width <= 0 || image.height <= 0 || image.image.size() == 0)
    {
        std::cerr << "Error: Image data is invalid (width: " << image.width << ", height: " << image.height <<
            ", size: " << image.image.size() << ")" << std::endl;
        return 0;
    }

    // Print image details
    std::cout << "Creating texture from image: width = " << image.width << ", height = " << image.height << ", size = "
        << image.image.size() << std::endl;

    // Generate and bind the texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Determine the format
    GLenum format = GL_RGBA;
    if (image.component == 3)
    {
        format = GL_RGB;
    }

    // Create the texture
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, &image.image[0]);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after glTexImage2D: " << error << std::endl;
        return 0;
    }

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Check for OpenGL errors after setting parameters
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after setting texture parameters: " << error << std::endl;
        return 0;
    }

    std::cout << "Texture created successfully with ID: " << textureID << std::endl;
    return textureID;
}

GLuint loadCubeMap(const std::vector<std::string>& faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        std::cout << "Loading cubemap texture at path: " << faces[i] << std::endl;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            std::cerr << "stbi_load error: " << stbi_failure_reason() << std::endl; // Print the reason for failure
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
