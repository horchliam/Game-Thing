// external libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// my headers
#include "Window.hpp"
#include "WindowManager.hpp"

// stds
#include <iostream>
#include <string>
#include <vector>

void processInput(GLFWwindow *window);

// globals
GLFWwindow* window;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// shaders
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"out vec4 ourColor;\n"
"out vec2 texCoord;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4((aPos.x/400) - 1, 1 - (aPos.y/300), 0.0, 1.0);\n"
"   ourColor = vec4(aColor, 1.0);\n"
"   texCoord = vec2(aTexCoord.x, 1 - aTexCoord.y);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 ourColor;\n"
"in vec2 texCoord;\n"
"uniform sampler2D myTexture;"
//"uniform vec2 offset;"
"void main()\n"
"{\n"
"   FragColor = texture(myTexture, texCoord) * ourColor;\n"
"}\n\0";

const int TEX_SZ = 16.0F;

template <typename T>
void appendArray(std::vector<T> &vec, T arr[], int size) {
    for(int i = 0; i < size; i++) {
        vec.push_back(arr[i]);
    }
}

void string_to_vbo(std::string s, unsigned int &vbo, float x_at, float y_at, float width_at, float size) {
    std::vector<float> vertices;
    
    float y = y_at + size;
    float x = x_at;
    for(int i = 0; i < s.length(); i++) {
        int ascii_code = s[i];
        
        // work out row and column in atlas
        int col = ( ascii_code - ' ' ) % TEX_SZ;
        int row = ( ascii_code - ' ' ) / TEX_SZ;
        std::cout << "Col: " << col << " and row: " << row << std::endl;
        
        float vertices_tmp[] = {
            // positions          // colors           // texture coords
            x+size, y-size, 0.0f,   1.0f, 0.0f, 1.0f,   (1.0f + col)/TEX_SZ, 1 - (0.0f + row)/TEX_SZ,   // top right
            x, y-size, 0.0f,        1.0f, 0.0f, 1.0f,   (0.0f + col)/TEX_SZ, 1 - (0.0f + row)/TEX_SZ,   // top left
            x,  y, 0.0f,            1.0f, 0.0f, 1.0f,   (0.0f + col)/TEX_SZ, 1 - (1.0f + row)/TEX_SZ,    // bottom left
            x+size, y-size, 0.0f,   1.0f, 0.0f, 1.0f,   (1.0f + col)/TEX_SZ, 1 - (0.0f + row)/TEX_SZ,   // top right
            x+size,  y, 0.0f,       1.0f, 0.0f, 1.0f,   (1.0f + col)/TEX_SZ, 1 - (1.0f + row)/TEX_SZ,    // bottom right
            x,  y, 0.0f,            1.0f, 0.0f, 1.0f,   (0.0f + col)/TEX_SZ, 1 - (1.0f + row)/TEX_SZ    // bottom left
        };
        
        appendArray(vertices, vertices_tmp, sizeof(vertices_tmp) / sizeof(float));
        
        x += 2*size/3;
        if(x >= width_at + x_at) {
            y += size;
            x = x_at;
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float* v_convert = &vertices[0];
    glBufferData(GL_ARRAY_BUFFER, s.length() * 48 * sizeof(float), v_convert, GL_DYNAMIC_DRAW);
}

int main()
{
    // window
    WindowManager wm = WindowManager(SCR_WIDTH, SCR_HEIGHT);
    if(!wm.SetupWindow()) { return 0; }
    
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    //texture
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("assets/handmade2.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << width << " and " << height << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    data = stbi_load("assets/monkeyJuice.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << width << " and " << height << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    
    stbi_image_free(data);
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "myTexture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    
    unsigned int VBO, VBO2, VAO, VAO2;
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO2);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    
    std::string testString = "It was the best of times it was the blurst of times!? You stupid monkey!!!";
    string_to_vbo(testString, VBO, SCR_WIDTH/2, SCR_HEIGHT/2, SCR_WIDTH/2, 30.0f);
    string_to_vbo(testString, VBO2, 0, 0, SCR_WIDTH/2, 20.0f);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(VAO2);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    
    glBindVertexArray(VAO);
    glUseProgram(shaderProgram);
    
//    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);
        
        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
//        GLfloat offset[2] = {0.0f, 3.0f};
//        glUniform2f(glGetUniformLocation(shaderProgram, "offset"), offset[0], offset[1]);
//        glDrawElements(GL_TRIANGLES, 6 * testString.length(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6 * testString.length());
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 6 * testString.length());
        
//        GLfloat offset2[2] = {1.0f, 3.0f};
//        glUniform2f(glGetUniformLocation(shaderProgram, "offset"), offset2[0], offset2[1]);
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
