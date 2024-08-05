# 🚀 OpenGLRenderer

OpenGLRenderer is a testing and learning project developed in my free time to understand and experiment with real-time 3D rendering techniques. The project demonstrates the basics of setting up an OpenGL rendering pipeline, handling input, and loading 3D models using the glTF format.

## ✨ Features

- **OpenGL Integration**: Utilizes modern OpenGL for rendering.
- **GLFW**: Manages window creation, input handling, and context management.
- **GLEW**: Handles OpenGL extension loading.
- **tinygltf**: Supports loading 3D models in glTF format.
- **GLM**: Provides mathematical operations for graphics programming.
- **Camera Control**: Allows moving around the 3D scene using mouse and keyboard.
- **Basic Lighting**: Implements basic lighting to enhance the visual representation of 3D models.
- **Texture Handling**: Supports loading and displaying textures from glTF models.

## 📦 Setup Instructions

### Prerequisites

- CMake 3.10 or higher
- A C++17 compatible compiler
- OpenGL drivers

### Building the Project

1. **Clone the Repository**:
    ```sh
    git clone https://github.com/yourusername/OpenGLRenderer.git
    cd OpenGLRenderer
    ```

2. **Initialize and Update Submodules**:
    ```sh
    git submodule update --init --recursive
    ```

3. **Create a Build Directory**:
    ```sh
    mkdir build
    cd build
    ```

4. **Run CMake to Configure the Project**:
    ```sh
    cmake ..
    ```

5. **Build the Project**:
    ```sh
    cmake --build .
    ```

6. **Run the Executable**:
    ```sh
    ./OpenGLRenderer
    ```

## 🎮 Usage

### Camera Controls
- `W`: Move forward
- `S`: Move backward
- `A`: Move left
- `D`: Move right
- Mouse: Look around

### Lighting and Textures
- The project demonstrates basic lighting techniques.
- Textures from glTF models are loaded and displayed.

## 🛠️ Known Issues

- **Lighting and Texture Issues**: There are still unresolved issues related to lighting and texture rendering that need to be addressed.

## 🤝 Contributing

Contributions are welcome! Feel free to fork the repository and submit a pull request for any bug fixes, improvements, or new features.

## 📜 License

This project is licensed under the MIT License.

## 🙏 Acknowledgements

- [GLFW](https://github.com/glfw/glfw) for window and input management.
- [GLEW](https://github.com/nigels-com/glew) for OpenGL extension handling.
- [tinygltf](https://github.com/syoyo/tinygltf) for loading glTF models.
- [GLM](https://github.com/g-truc/glm) for mathematics operations.

---

Happy coding! 🎉
