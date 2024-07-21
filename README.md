# OpenGL Graph and Text Rendering

This project demonstrates how to create a simple graph and render text using OpenGL and GLFW. It includes shaders for rendering both the graph and the text, and uses the `stb_truetype` library for text rendering.

## Features

- Render a 2D graph
- Render text on the screen
- Basic input handling to close the application

## Requirements

- OpenGL
- GLFW
- GLAD
- stb_truetype

## Getting Started

### Prerequisites

- Make sure you have the required libraries installed:
  - OpenGL
  - GLFW
  - GLAD
  - stb_truetype

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/yourproject.git
2. Navigate to the project directory:
  cd yourproject
3.Compile the project using your preferred C++ compiler. For example, using g++:
  g++ main.cpp -o graph_renderer -lglfw -ldl -lGL -lGLU
4.Running the Application
Run the compiled executable:
  ./graph_renderer
5 .Code Overview
Shaders
vertexShaderSource: Vertex shader for rendering the graph.
fragmentShaderSource: Fragment shader for rendering the graph.
textVertexShaderSource: Vertex shader for rendering text.
textFragmentShaderSource: Fragment shader for rendering text.
Functions
framebuffer_size_callback: Adjusts the viewport when the window size changes.
processInput: Handles user input.
compileShader: Compiles a shader from source code.
createShaderProgram: Links vertex and fragment shaders into a shader program.
renderGraph: Renders the graph using the provided data.
renderAxes: Renders the axes of the graph.
renderText: Renders text using the provided font and shaders.
Main Function
Initializes GLFW and creates a window.
Sets up OpenGL context and loads GLAD.
Compiles and links shaders.
Sets up vertex data and buffers.
Enters the render loop, which:
Processes input
Clears the screen
Renders the graph and text
Swaps buffers and polls for events
stb_truetype Integration
The stb_truetype.h library is used for font rendering.
Font data is baked into a texture for efficient rendering.
Contributing
Contributions are welcome! Please fork the repository and create a pull request with your changes.

License
This project is licensed under the MIT License. See the LICENSE file for details.

Acknowledgments
GLFW
GLAD
stb_truetype


You can adjust the installation and running instructions according to your specific environment and requirements. Let me know if you need any further modifications or additions! &#8203;:citation[oaicite:0]{index=0}&#8203;
