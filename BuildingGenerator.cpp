#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <ctime>
#include <vector>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;
using namespace std;

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

//size of the city
int CITY_SIZE = 100;

//window resolution
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//Frame rate of display
const float FPS = 60;

//Rotation speed
const float ROTATE_SPEED = 0.005f;

//Seed for city generation
int CITY_SEED = 1;

//DImensions for the city
float CITY_DIMENSION = 200.5f;

struct Building {
    float width, height, depth;
    float x, z;
    GLuint textureID;
};

vector<GLuint> textures;
vector<Building> buildings;
GLuint ground_texture;

GLuint program;
GLuint vbo, vao, ebo;
GLuint groundVBO, groundVAO;
float angleY = 0.0f;


void checkCompileErrors(GLuint shader, string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
        }
    }
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = nrChannels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        cerr << "Failed to load texture: " << path << endl;
    }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

vector<GLuint> loadTextures(const vector<const char*>& texturePaths) {
    vector<GLuint> textureIDs;
    for (const auto& path : texturePaths) {
        textureIDs.push_back(loadTexture(path));
    }
    return textureIDs;
}

GLuint initShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
    // Read vertex shader source code
    ifstream vertexShaderFile(vertexShaderPath);
    stringstream vertexShaderStream;
    vertexShaderStream << vertexShaderFile.rdbuf();
    string vertexShaderSource = vertexShaderStream.str();
    const char* vertexShaderCode = vertexShaderSource.c_str();

    // Read fragment shader source code
    ifstream fragmentShaderFile(fragmentShaderPath);
    stringstream fragmentShaderStream;
    fragmentShaderStream << fragmentShaderFile.rdbuf();
    string fragmentShaderSource = fragmentShaderStream.str();
    const char* fragmentShaderCode = fragmentShaderSource.c_str();

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    // Delete shaders (they're already linked into the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void generateCityScene(int numBuildings) {
    buildings.clear();
    srand(CITY_SEED);

    for (int i = 0; i < numBuildings; ++i) {
        Building building;
        building.width = (rand() % 10) + 5;
        building.height = (rand() % 80) + 20;
        building.depth = (rand() % 10) + 5;
        building.x = (float(rand()) / float((RAND_MAX)) * CITY_DIMENSION) - (CITY_DIMENSION / 2); //Random float
        building.z = (float(rand()) / float((RAND_MAX)) * CITY_DIMENSION) - (CITY_DIMENSION / 2); //Random float
        building.textureID = textures[rand() % textures.size()]; // Assign random texture
        buildings.push_back(building);
    }
}

void setupBuffers() {
    // Building vertices with positions, texture coordinates, normals
    GLfloat vertices[] = {
        // Positions          
        // Front face        // Texture coords       // Normals
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,          0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,          0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f,          0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,          0.0f,  0.0f,  1.0f,

        // Back face
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f,          0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,          0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,          0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f, 1.0f,          0.0f,  0.0f, -1.0f,

         // Left face
         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,         -1.0f,  0.0f,  0.0f,
         -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,         -1.0f,  0.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,         -1.0f,  0.0f,  0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,         -1.0f,  0.0f,  0.0f,

         // Right face
          0.5f, -0.5f,  0.5f,  0.0f, 0.0f,          1.0f,  0.0f,  0.0f,
          0.5f, -0.5f, -0.5f,  1.0f, 0.0f,          1.0f,  0.0f,  0.0f,
          0.5f,  0.5f, -0.5f,  1.0f, 1.0f,          1.0f,  0.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f, 1.0f,          1.0f,  0.0f,  0.0f,

          // Top face
          -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,          0.0f,  1.0f,  0.0f,
           0.5f,  0.5f,  0.5f,  0.0f, 0.0f,          0.0f,  1.0f,  0.0f,
           0.5f,  0.5f, -0.5f,  0.0f, 0.0f,          0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,          0.0f,  1.0f,  0.0f,
    };

    GLuint indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawBuilding(const Building& building) {
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, building.textureID); // Use building's texture

    GLint modelLoc = glGetUniformLocation(program, "model");

    float cubeHeight = 2.0f; // Height of each cube in the stack
    int numCubes = static_cast<int>(building.height / cubeHeight);

    for (int i = 0; i < numCubes; ++i) {
        mat4 model = translate(mat4(1.0f), vec3(building.x, i * cubeHeight + cubeHeight / 2, building.z)) *
                     scale(mat4(1.0f), vec3(building.width, cubeHeight, building.depth));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void drawGroundPlane() {
    // Ground plane vertices and indices
    GLfloat groundVertices[] = {
        // Positions                // Texture Coords   // Normals
        -400.0f, 0.0f, -400.0f,     0.0f, 0.0f,         0.0f,  1.0f,  0.0f,
         400.0f, 0.0f, -400.0f,     40.0f, 0.0f,        0.0f,  1.0f,  0.0f,
         400.0f, 0.0f,  400.0f,     40.0f, 40.0f,       0.0f,  1.0f,  0.0f,
        -400.0f, 0.0f,  400.0f,     0.0f, 40.0f,         0.0f,  1.0f,  0.0f,
    };

    GLuint groundIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);

    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Use the ground texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ground_texture);

    // Set model matrix (identity for ground plane)
    mat4 model = mat4(1.0f); // Identity matrix
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, value_ptr(model));

    // Draw ground plane
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);
    glDeleteBuffers(1, &groundEBO);
}

void drawCityScene() {
    for (const auto& building : buildings) {
        drawBuilding(building);
    }
}

void setLighting(GLuint program) {
    glUseProgram(program);

    GLint lightPosLoc = glGetUniformLocation(program, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(program, "viewPos");
    GLint lightColorLoc = glGetUniformLocation(program, "lightColor");
    GLint objectColorLoc = glGetUniformLocation(program, "objectColor");

    glUniform3f(lightPosLoc, 100.0f, 200.0f, 200.0f);  // position
    glUniform3f(viewPosLoc, 0.0f, 0.0f, 300.0f);       // view position
    glUniform3f(lightColorLoc, 1.0f, 0.99f, 0.98f);    // White light
    glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);     // object color
}

const float FPS_MS = 1000 / FPS;

void display() {
    // Clear screen and enable necessary features
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_MULTISAMPLE);
    glSampleCoverage(1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(program);

    // Set the lighting uniforms
    setLighting(program);

    mat4 view = lookAt(vec3(150 * sin(angleY), 100, 250 * cos(angleY)), vec3(0,20, 0), vec3(0, 100, 0));
    mat4 projection = perspective(radians(60.0f), 800.0f / 800.0f, 1.0f, 1000.0f);

    GLint viewLoc = glGetUniformLocation(program, "view");
    GLint projLoc = glGetUniformLocation(program, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

    //Draw ground
    drawGroundPlane();

    //Draw Buildings
    drawCityScene();

    glutSwapBuffers();

    Sleep(FPS_MS);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void OnIdle() {

    angleY += ROTATE_SPEED;
    if (angleY > 360)
        angleY -= 360;

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 32) { // Space key ASCII value
        CITY_SEED += 1;
        generateCityScene(CITY_SIZE);
    }
    if (key == 48) { // 0 key ASCII value
        CITY_SIZE = 100;
        CITY_DIMENSION = 200;
        generateCityScene(CITY_SIZE);
    }
    if (key == 43) { // + key ASCII value
        CITY_SIZE += 10;
        generateCityScene(CITY_SIZE);
    }
    if (key == 45) { // - key ASCII value
        CITY_SIZE -= 10;
        generateCityScene(CITY_SIZE);
    }
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        CITY_SIZE += 10;
        generateCityScene(CITY_SIZE);
        break;
    case GLUT_KEY_DOWN:
        CITY_SIZE -= 10;
        generateCityScene(CITY_SIZE);
        break;
    case GLUT_KEY_LEFT:
        CITY_DIMENSION -= 50;
        generateCityScene(CITY_SIZE);
        break;
    case GLUT_KEY_RIGHT:
        CITY_DIMENSION += 50;
        generateCityScene(CITY_SIZE);
        break;
    }
    if (CITY_DIMENSION < 0)
        CITY_DIMENSION = 0;
    if (CITY_SIZE < 0)
        CITY_SIZE = 0;
}

void init() {
    glClearColor(0.2f, 0.5f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "Error: " << glewGetErrorString(err) << endl;
        exit(-1);
    }

    program = initShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    vector<const char*> texturePaths = {
        "texture/texture1.jpg",
        "texture/texture2.jpg",
        "texture/texture3.jpg",
        "texture/texture4.jpg",
        "texture/texture5.jpg",
        "texture/texture6.jpg",
        "texture/texture7.jpg"
    };
    textures = loadTextures(texturePaths); // Load multiple textures

    ground_texture = loadTexture("texture/ground.jpg");  // load ground texture  

    setupBuffers();
    generateCityScene(CITY_SIZE);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("3D City Scene");
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(OnIdle);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys); 

    glutMainLoop();

    return 0;
}
