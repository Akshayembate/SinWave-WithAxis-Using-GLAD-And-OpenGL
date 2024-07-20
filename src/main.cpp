#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

// stb_truetype.h needs to be included after defining STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Vertex shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 ourColor;
void main()
{
    FragColor = ourColor;
}
)";

// Vertex shader source code for text
const char* textVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
out vec2 TexCoords;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoords = aTex;
}
)";

// Fragment shader source code for text
const char* textFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform sampler2D text;
uniform vec3 textColor;
in vec2 TexCoords;
void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    FragColor = vec4(textColor, 1.0) * sampled;
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int compileShader(unsigned int type, const char* source);
unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
void renderGraph(const std::vector<float>& values1, const std::vector<float>& values2, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram);
void renderAxes(unsigned int VAO, unsigned int VBO, unsigned int shaderProgram);
void renderText(const std::string& text, float x, float y, float scale, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, GLuint texture, stbtt_bakedchar* cdata);

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Display Values", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Compile and create the shader programs
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    unsigned int textShaderProgram = createShaderProgram(textVertexShaderSource, textFragmentShaderSource);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    std::vector<float> values1;
    std::vector<float> values2;
    float time = 0.0f;
    const float deltaTime = 0.1f;  // Adjust the delta time as needed

    // Load font using stb_truetype
    stbtt_bakedchar cdata[96];
    unsigned char ttf_buffer[1 << 20];
    unsigned char temp_bitmap[512 * 512];

    std::ifstream fontFile("arial.ttf", std::ios::binary);
    fontFile.read(reinterpret_cast<char*>(ttf_buffer), 1 << 20);
    fontFile.close();

    stbtt_BakeFontBitmap(ttf_buffer, 0, 32.0, temp_bitmap, 512, 512, 32, 96, cdata);

    GLuint ftex;
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Projection matrix for graph rendering
    float left = -10.0f, right = 10.0f, bottom = -10.0f, top = 10.0f;
    float projection[16] = {
        2.0f / (right - left), 0, 0, 0,
        0, 2.0f / (top - bottom), 0, 0,
        0, 0, -1.0f, 0,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0, 1
    };

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Update values
        time += deltaTime;
        values1.push_back(sin(time));
        values2.push_back(cos(time));

        if (values1.size() > 100) {  // Keep the size of the vectors manageable
            values1.erase(values1.begin());
            values2.erase(values2.begin());
        }

        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the projection matrix
        glUseProgram(shaderProgram);
        int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection);

        // Render the axes
        renderAxes(VAO, VBO, shaderProgram);

        // Render the graph
        renderGraph(values1, values2, VAO, VBO, shaderProgram);

        // Render the text labels
        renderText("Time (s)", -0.95f, -0.95f, 0.005f, VAO, VBO, textShaderProgram, ftex, cdata);
        renderText("Magnitude", -0.95f, 0.95f, 0.005f, VAO, VBO, textShaderProgram, ftex, cdata);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void renderAxes(unsigned int VAO, unsigned int VBO, unsigned int shaderProgram) {
    std::vector<float> vertices = {
        // X-axis
        -10.0f, 0.0f,
         10.0f, 0.0f,
        // Y-axis
         0.0f, -10.0f,
         0.0f,  10.0f
    };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glUseProgram(shaderProgram);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set color to white for axes
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    if (vertexColorLocation == -1) {
        std::cerr << "Could not find uniform location for 'ourColor'" << std::endl;
    } else {
        glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    glDrawArrays(GL_LINES, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderGraph(const std::vector<float>& values1, const std::vector<float>& values2, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram) {
    std::vector<float> vertices;

    for (size_t i = 0; i < values1.size(); ++i) {
        vertices.push_back(static_cast<float>(i) - 50.0f); // Adjust the x coordinate for better visibility
        vertices.push_back(values1[i]);
    }

    for (size_t i = 0; i < values2.size(); ++i) {
        vertices.push_back(static_cast<float>(i) - 50.0f); // Adjust the x coordinate for better visibility
        vertices.push_back(values2[i]);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glUseProgram(shaderProgram);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set color for the first value (values1)
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    if (vertexColorLocation == -1) {
        std::cerr << "Could not find uniform location for 'ourColor'" << std::endl;
    } else {
        glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINE_STRIP, 0, values1.size());

        // Set color for the second value (values2)
        glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINE_STRIP, values1.size(), values2.size());
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderText(const std::string& text, float x, float y, float scale, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, GLuint texture, stbtt_bakedchar* cdata) {
    glUseProgram(shaderProgram);

    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shaderProgram, "text"), 0);

    glBindVertexArray(VAO);

    float xpos = x;
    float ypos = y;

    std::vector<float> vertices;

    for (char c : text) {
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, c - 32, &xpos, &ypos, &q, 1);

            float x0 = q.x0 * scale;
            float y0 = q.y0 * scale;
            float x1 = q.x1 * scale;
            float y1 = q.y1 * scale;

            vertices.insert(vertices.end(), {
                x0, y0, q.s0, q.t0,
                x1, y0, q.s1, q.t0,
                x1, y1, q.s1, q.t1,
                x0, y1, q.s0, q.t1
            });
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_QUADS, 0, vertices.size() / 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cerr << message << std::endl;
        delete[] message;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
