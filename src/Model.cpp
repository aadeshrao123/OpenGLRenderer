#include "Model.h"
#include "Texture.h"
#include <iostream>

Model::Model(const std::string& path)
{
    loadModel(path);
    createVAOs();
}

void Model::draw(GLuint shaderProgram)
{
    for (const auto& mesh : model.meshes)
    {
        int meshIdx = static_cast<int>(&mesh - &model.meshes[0]);
        if (primitiveMap.find(meshIdx) == primitiveMap.end())
        {
            std::cerr << "Error: Mesh index " << meshIdx << " not found in primitiveMap" << std::endl;
            continue;
        }
        for (const auto& glPrimitive : primitiveMap[meshIdx])
        {
            glBindVertexArray(glPrimitive.vao);

            if (glPrimitive.indexCount > 0)
            {
                glDrawElements(GL_TRIANGLES, glPrimitive.indexCount, GL_UNSIGNED_SHORT, 0);
            }
            else
            {
                glDrawArrays(GL_TRIANGLES, 0, glPrimitive.indexCount);
            }

            glBindVertexArray(0);
        }
    }
}

void Model::loadModel(const std::string& path)
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = false;
    if (path.substr(path.find_last_of(".") + 1) == "glb")
    {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path); // for binary glTF (.glb)
    }
    else
    {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, path); // for ASCII glTF (.gltf)
    }

    if (!warn.empty())
    {
        std::cout << "Warning: " << warn << std::endl;
    }
    if (!err.empty())
    {
        std::cerr << "Error: " << err << std::endl;
    }
    if (!ret)
    {
        std::cerr << "Failed to load glTF: " << path << std::endl;
    }

    // Debug: Check image data
    for (const auto& image : model.images)
    {
        std::cout << "Image name: " << image.name << ", width: " << image.width << ", height: " << image.height <<
            ", size: " << image.image.size() << std::endl;
    }
}

GLuint Model::createBuffer(const std::vector<unsigned char>& data, GLenum target)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, static_cast<GLsizei>(data.size()), data.data(), GL_STATIC_DRAW);
    return buffer;
}

void Model::createVAOs()
{
    for (size_t i = 0; i < model.meshes.size(); ++i)
    {
        const tinygltf::Mesh& mesh = model.meshes[i];
        for (const auto& primitive : mesh.primitives)
        {
            GLPrimitive glPrimitive;
            glGenVertexArrays(1, &glPrimitive.vao);
            glBindVertexArray(glPrimitive.vao);

            for (const auto& attrib : primitive.attributes)
            {
                if (model.accessors.size() <= attrib.second)
                {
                    std::cerr << "Error: Attribute index out of range: " << attrib.second << std::endl;
                    continue;
                }

                const tinygltf::Accessor& accessor = model.accessors[attrib.second];
                if (model.bufferViews.size() <= accessor.bufferView)
                {
                    std::cerr << "Error: Buffer view index out of range: " << accessor.bufferView << std::endl;
                    continue;
                }

                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                if (model.buffers.size() <= bufferView.buffer)
                {
                    std::cerr << "Error: Buffer index out of range: " << bufferView.buffer << std::endl;
                    continue;
                }

                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                GLuint vbo = createBuffer(buffer.data, GL_ARRAY_BUFFER);
                glPrimitive.vbo = vbo;

                GLuint attribIndex = 0; // Default to 0, change based on attribute name
                if (attrib.first == "POSITION")
                {
                    attribIndex = 0;
                }
                else if (attrib.first == "NORMAL")
                {
                    attribIndex = 1;
                }
                else if (attrib.first == "TEXCOORD_0")
                {
                    attribIndex = 2;
                }

                glEnableVertexAttribArray(attribIndex);
                glVertexAttribPointer(
                    attribIndex,
                    accessor.type == TINYGLTF_TYPE_SCALAR ? 1 : accessor.type,
                    accessor.componentType,
                    accessor.normalized ? GL_TRUE : GL_FALSE,
                    bufferView.byteStride,
                    reinterpret_cast<const void*>(accessor.byteOffset + bufferView.byteOffset)
                );
            }

            if (primitive.indices >= 0)
            {
                if (model.accessors.size() <= primitive.indices)
                {
                    std::cerr << "Error: Primitive index out of range: " << primitive.indices << std::endl;
                    continue;
                }

                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                if (model.bufferViews.size() <= accessor.bufferView)
                {
                    std::cerr << "Error: Buffer view index out of range: " << accessor.bufferView << std::endl;
                    continue;
                }

                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                if (model.buffers.size() <= bufferView.buffer)
                {
                    std::cerr << "Error: Buffer index out of range: " << bufferView.buffer << std::endl;
                    continue;
                }

                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                GLuint ebo = createBuffer(buffer.data, GL_ELEMENT_ARRAY_BUFFER);
                glPrimitive.ebo = ebo;
                glPrimitive.indexCount = static_cast<GLsizei>(accessor.count);
            }
            else
            {
                glPrimitive.indexCount = 0;
            }

            glBindVertexArray(0);
            primitiveMap[i].push_back(glPrimitive);
        }
    }
}

GLuint Model::loadTextureFromModel(int textureIndex)
{
    if (textureIndex < 0 || textureIndex >= model.textures.size())
    {
        std::cerr << "Error: Texture index out of range: " << textureIndex << std::endl;
        return 0;
    }

    const tinygltf::Texture& texture = model.textures[textureIndex];
    if (texture.source < 0 || texture.source >= model.images.size())
    {
        std::cerr << "Error: Image index out of range: " << texture.source << std::endl;
        return 0;
    }

    const tinygltf::Image& image = model.images[texture.source];
    return createTexture(image);
}
