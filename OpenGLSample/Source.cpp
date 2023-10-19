#include <iostream>             // cout, cerr
#include <cstdlib>              // EXIT_FAILURE

#include <GL/glew.h>            // GLEW library
#include <GLFW/glfw3.h>         // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h> // Camera class
//#include "shader.h"

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Paul Velazquez"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    GLuint floorVAO = 0;
    GLuint floorVBO = 0;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Cube mesh data
    GLMesh gCube1Mesh;
    GLMesh gCube2Mesh;
    GLMesh gCube3Mesh;
    // Cylinder Mesh
    GLMesh gCylinderMesh;
    GLMesh gCylinder2Mesh;
    // Sphere Mesh
    GLMesh gSphereMesh;
    // Texture IDs
    GLuint gCubeTextureId; // Texture ID for the cubes
    GLuint gFloorTextureId; // Texture ID for the floor
    GLuint gSphereTextureId; // Texture ID for the spheres
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_CLAMP_TO_EDGE;

    // Shader program
    GLuint gProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
// Function prototype for the key callback function
void UKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
// Sphere Mesh
void UCreateSphere(GLMesh& mesh);
// Cylinder Mesh
void UCreateCylinder(GLMesh& mesh);

bool isOrtho = false;

// build and compile our shader zprogram
//Shader lightingShader("1.colors.vs", "1.colors.fs");
//Shader lightCubeShader("1.light_cube.vs", "1.light_cube.fs");


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoordinate;
out vec2 vertexTextureCoordinate;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
out vec3 FragPos;
out vec3 Normal;

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexTextureCoordinate = textureCoordinate;
    FragPos = vec3(model * vec4(inPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * inNormal;
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
    // Use clamp to edge for texture wrapping
    fragmentColor = texture(uTexture, clamp(vertexTextureCoordinate * uvScale, 0.0, 1.0));
}
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the Cube 1 mesh
    UCreateMesh(gCube1Mesh); // Calls the function to create the Vertex Buffer Object

     // Create the Cube 2 mesh
    UCreateMesh(gCube2Mesh);

    // Create the Cube 3 mesh
    UCreateMesh(gCube3Mesh);

    // Create the Cylinder 1 mesh
    UCreateCylinder(gCylinderMesh);

    // Create the Cylinder 2 mesh
    UCreateCylinder(gCylinder2Mesh);

    // Create the sphere mesh
    UCreateSphere(gSphereMesh);

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* cubeTextureFilename = "dark.jpg";
    if (!UCreateTexture(cubeTextureFilename, gCubeTextureId))
    {
        cout << "Failed to load texture " << cubeTextureFilename << endl;
        return EXIT_FAILURE;
    }

    // Load floor texture
    const char* floorTextureFilename = "black.jpg";
    if (!UCreateTexture(floorTextureFilename, gFloorTextureId))
    {
        cout << "Failed to load texture " << floorTextureFilename << endl;
        return EXIT_FAILURE;
    }

    // Load sphere texture
    const char* sphereTextureFilename = "white.jpg";
    if (!UCreateTexture(sphereTextureFilename, gSphereTextureId))
    {
        cout << "Failed to load texture " << sphereTextureFilename << endl;
        return EXIT_FAILURE;
    }


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);

    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Set the key callback function
    glfwSetKeyCallback(gWindow, UKeyCallback);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gCube1Mesh);
    UDestroyMesh(gCube2Mesh);
    UDestroyMesh(gCube3Mesh);
    UDestroyMesh(gCylinderMesh);
    UDestroyMesh(gCylinder2Mesh);
    UDestroyMesh(gSphereMesh);

    // Release textures
    UDestroyTexture(gCubeTextureId);
    UDestroyTexture(gFloorTextureId);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

void UKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        isOrtho = !isOrtho; // Toggle the view mode
    }
}


// Function called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Modify the projection matrix based on the view mode
    glm::mat4 projection;
    if (isOrtho) {
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f); // Orthographic projection
    }
    else {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f); // Perspective projection
    }

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Set the texture unit for the fragment shader (uTexture)
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Set the uniform variable for uvScale
    glUniform2fv(glGetUniformLocation(gProgramId, "uvScale"), 1, glm::value_ptr(gUVScale));

    glUniform1i(glGetUniformLocation(gProgramId, "gTexWrapMode"), gTexWrapMode);


    // Render the cube
    // Bind the cube's VAO
    glBindVertexArray(gCube1Mesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCubeTextureId);

    // Set the texture unit for the fragment shader
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Apply cube transformations
    glm::mat4 cubeModel = glm::mat4(1.0f); // Identity matrix for cube
    cubeModel = glm::translate(cubeModel, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate the cube
    cubeModel = glm::scale(cubeModel, glm::vec3(0.6f, 0.6f, 0.6f)); // Scale the cube
    cubeModel = glm::rotate(cubeModel, glm::radians(40.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
    cubeModel = glm::rotate(cubeModel, glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
    cubeModel = glm::rotate(cubeModel, glm::radians(-5.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

    // Update view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(cubeModel));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw cube 1
    glDrawArrays(GL_TRIANGLES, 0, gCube1Mesh.nVertices);

    // Unbind the cube's VAO
    glBindVertexArray(0);


    // Render cube 2
    // Bind the cube's VAO
    glBindVertexArray(gCube2Mesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCubeTextureId);

    // Set the texture unit for the fragment shader
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Apply cube transformations
    glm::mat4 cube2Model = glm::mat4(1.0f); // Identity matrix for cube
    cube2Model = glm::translate(cube2Model, glm::vec3(-0.55f, 0.45f, -0.6f)); // Translate the cube
    cube2Model = glm::scale(cube2Model, glm::vec3(0.4f, 0.4f, 0.4f)); // Scale the cube
    cube2Model = glm::rotate(cube2Model, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
    cube2Model = glm::rotate(cube2Model, glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
    cube2Model = glm::rotate(cube2Model, glm::radians(-5.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

    // Update view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(cube2Model));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw cube 2
    glDrawArrays(GL_TRIANGLES, 0, gCube2Mesh.nVertices);

    // Unbind the cube's VAO
    glBindVertexArray(0);


    // Render cube 3
    // Bind the cube's VAO
    glBindVertexArray(gCube3Mesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCubeTextureId);

    // Set the texture unit for the fragment shader
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Apply cube transformations
    glm::mat4 cube3Model = glm::mat4(1.0f); // Identity matrix for cube
    cube3Model = glm::translate(cube3Model, glm::vec3(0.2f, -0.8f, 0.0f)); // Translate the cube
    cube3Model = glm::scale(cube3Model, glm::vec3(0.4f, 0.4f, 0.4f)); // Scale the cube
    cube3Model = glm::rotate(cube3Model, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
    cube3Model = glm::rotate(cube3Model, glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
    cube3Model = glm::rotate(cube3Model, glm::radians(-15.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

    // Update view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(cube3Model));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw cube 3
    glDrawArrays(GL_TRIANGLES, 0, gCube3Mesh.nVertices);

    // Unbind the cube's VAO
    glBindVertexArray(0);


    // Render the floor
    // Bind the floor's VAO
    glBindVertexArray(floorVAO);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFloorTextureId);

    // Set the texture unit for the fragment shader
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Apply floor transformations
    glm::mat4 floorModel = glm::mat4(1.0f); // Identity matrix for floor
    floorModel = glm::translate(floorModel, glm::vec3(0.0f, -1.0f, 0.0f)); // Translate the floor
    floorModel = glm::scale(floorModel, glm::vec3(5.0, 1.0, 5.0)); // Scale the floor
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(floorModel));

    // Update view and projection matrices (no need to update them again if they haven't changed)

    // Draw the floor
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Unbind the floor's VAO
    glBindVertexArray(0);

    // be sure to activate shader when setting uniforms/drawing objects
    //lightingShader.use();
    //lightingShader.setVec3("objectColor", 1.0f, 1.0f, 0.8f);
    //lightingShader.setVec3("lightColor", 1.0f, 1.0f, 0.6f);

    // Render the sphere
    glBindVertexArray(gSphereMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gSphereTextureId);

    // Set the texture unit for the fragment shader
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Apply sphere transformations
    glm::mat4 sphereModel = glm::mat4(1.0f);
    glm::vec3 translationVector(0.4f, 0.4f, 0.3f);
    sphereModel = glm::translate(sphereModel, translationVector);

    // Update view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));

    // Draw the sphere
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gSphereMesh.nVertices);

    // Unbind the sphere's VAO
    glBindVertexArray(0);

    // Apply transformations and render the second sphere
    glBindVertexArray(gSphereMesh.vao);

    glm::mat4 sphere2Model = glm::mat4(1.0f);
    glm::vec3 translationVector2(-0.6f, -0.3f, 0.3f); // Adjust the values as needed
    sphere2Model = glm::translate(sphere2Model, translationVector2);

    // Update view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(sphere2Model));

    // Draw the second sphere
    glDrawArrays(GL_TRIANGLES, 0, gSphereMesh.nVertices);

    // Unbind the second sphere's VAO
    glBindVertexArray(0);

    // Render the cylinder
    glBindVertexArray(gCylinderMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCubeTextureId);

    // Set the texture unit for the fragment shader
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Apply cylinder transformations
    glm::mat4 cylinderModel = glm::mat4(1.0f); // Identity matrix for cylinder
    cylinderModel = glm::translate(cylinderModel, glm::vec3(0.6f, 0.1f, -1.2f)); // Translate the cylinder
    cylinderModel = glm::scale(cylinderModel, glm::vec3(0.2f, 0.5f, 0.2f)); // Scale the cylinder
    cylinderModel = glm::rotate(cylinderModel, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
    cylinderModel = glm::rotate(cylinderModel, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
    cylinderModel = glm::rotate(cylinderModel, glm::radians(-25.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

    // Update view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(cylinderModel));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw cylinder
    glDrawArrays(GL_TRIANGLES, 0, gCylinderMesh.nVertices);

    // Unbind the cylinder VAO
    glBindVertexArray(0);

    // Render cylinder 2
    glBindVertexArray(gCylinderMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCubeTextureId);

    // Set the texture unit for the fragment shader
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Apply cylinder 2 transformations
    glm::mat4 cylinder2Model = glm::mat4(1.0f); // Identity matrix for cylinder
    cylinder2Model = glm::translate(cylinder2Model, glm::vec3(-0.1f, -0.9f, -0.7f)); // Translate the cylinder
    cylinder2Model = glm::scale(cylinder2Model, glm::vec3(0.2f, 0.5f, 0.2f)); // Scale the cylinder
    cylinder2Model = glm::rotate(cylinder2Model, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
    cylinder2Model = glm::rotate(cylinder2Model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
    cylinder2Model = glm::rotate(cylinder2Model, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

    // Update view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "model"), 1, GL_FALSE, glm::value_ptr(cylinder2Model));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw cylinder 2
    glDrawArrays(GL_TRIANGLES, 0, gCylinderMesh.nVertices);

    // Unbind the cylinder VAO
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat cubeVerts[] = {
        // Vertex Positions               // Texture Coordinates
       -0.5f, -0.5f, -0.5f,               0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,               1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,               1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,               1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,               0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,               0.0f, 0.0f,

       -0.5f, -0.5f,  0.5f,               0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,               1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,               1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,               1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f,               0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,               0.0f, 0.0f,

       -0.5f,  0.5f,  0.5f,               0.0f, 0.0f,
       -0.5f,  0.5f, -0.5f,               1.0f, 0.0f,
       -0.5f, -0.5f, -0.5f,               1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,               1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,               0.0f, 1.0f,
       -0.5f,  0.5f,  0.5f,               0.0f, 0.0f,

        0.5f,  0.5f,  0.5f,               1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,               0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,               0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,               0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,               1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,               1.0f, 0.0f,

       -0.5f, -0.5f, -0.5f,               0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,               1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,               1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,               0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,               1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,               0.0f, 1.0f,

       -0.5f,  0.5f, -0.5f,               0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,               1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,               1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,               1.0f, 0.0f,
       -0.5f,  0.5f,  0.5f,               0.0f, 0.0f,
       -0.5f,  0.5f, -0.5f,               0.0f, 1.0f
    };


    // Floor vertices
    GLfloat floorVerts[] = {
        // Vertex Positions               // Texture Coordinates
       -1.0f, -0.5f, -1.0f,               0.0f, 0.0f,
        1.0f, -0.5f, -1.0f,               1.0f, 0.0f,
        1.0f, -0.5f,  1.0f,               1.0f, 1.0f,

       -1.0f, -0.5f, -1.0f,               0.0f, 0.0f,
        1.0f, -0.5f,  1.0f,               1.0f, 1.0f,
       -1.0f, -0.5f,  1.0f,               0.0f, 1.0f
    };



    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(cubeVerts) / (sizeof(cubeVerts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    // Create VAO and VBO for the floor
    glGenVertexArrays(1, &floorVAO);
    glBindVertexArray(floorVAO);

    glGenBuffers(1, &floorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVerts), floorVerts, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers for the floor
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    // Unbind the VAO
    glBindVertexArray(0);
}

void UCreateSphere(GLMesh& mesh) {
    std::vector<GLfloat> sphereVertices;
    const int segments = 20;
    const int rings = 20;
    const float radius = 0.15f;

    for (int i = 0; i <= rings; i++) {
        float phi = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(rings);
        for (int j = 0; j <= segments; j++) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(segments);
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            // Add the vertex positions to the sphereVertices vector
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);

            // Add texture coordinates (you can set these as you like)
            sphereVertices.push_back(static_cast<float>(j) / static_cast<float>(segments));
            sphereVertices.push_back(static_cast<float>(i) / static_cast<float>(rings));
        }
    }

    mesh.nVertices = sphereVertices.size() / 5; // 5 components per vertex (x, y, z, s, t)

    glGenVertexArrays(1, &mesh.vao); // Generate VAO
    glBindVertexArray(mesh.vao); // Bind VAO

    glGenBuffers(1, &mesh.vbo); // Generate VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Bind VBO

    // Copy vertex data to VBO
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(GLfloat), &sphereVertices[0], GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Unbind VAO and VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UCreateCylinder(GLMesh& mesh)
{
    const int numSegments = 30; // Number of segments for the cylinder
    const float radius = 0.5f;
    const float height = 1.0f;

    const float PI = glm::pi<float>();
    const float step = 2.0f * PI / numSegments;

    std::vector<float> cylinderVertices;

    // Create vertices for the side of the cylinder
    for (int i = 0; i < numSegments; ++i)
    {
        float angle = i * step;
        float nextAngle = (i + 1) * step;

        // Vertices for two triangles forming a side face
        glm::vec3 v1(radius * std::cos(angle), -height / 2.0f, radius * std::sin(angle));
        glm::vec3 v2(radius * std::cos(nextAngle), -height / 2.0f, radius * std::sin(nextAngle));
        glm::vec3 v3(radius * std::cos(nextAngle), height / 2.0f, radius * std::sin(nextAngle));
        glm::vec3 v4(radius * std::cos(angle), height / 2.0f, radius * std::sin(angle));

        // Add vertices to the vector
        cylinderVertices.insert(cylinderVertices.end(), { v1.x, v1.y, v1.z });
        cylinderVertices.insert(cylinderVertices.end(), { v2.x, v2.y, v2.z });
        cylinderVertices.insert(cylinderVertices.end(), { v3.x, v3.y, v3.z });
        cylinderVertices.insert(cylinderVertices.end(), { v1.x, v1.y, v1.z });
        cylinderVertices.insert(cylinderVertices.end(), { v3.x, v3.y, v3.z });
        cylinderVertices.insert(cylinderVertices.end(), { v4.x, v4.y, v4.z });
    }

    // Create vertices for the top and bottom faces of the cylinder
    for (int i = 0; i < numSegments; ++i)
    {
        float angle = i * step;
        float nextAngle = (i + 1) * step;

        // Top face vertices
        glm::vec3 topCenter(0.0f, height / 2.0f, 0.0f);
        glm::vec3 topVertex1(radius * std::cos(angle), height / 2.0f, radius * std::sin(angle));
        glm::vec3 topVertex2(radius * std::cos(nextAngle), height / 2.0f, radius * std::sin(nextAngle));

        // Bottom face vertices
        glm::vec3 bottomCenter(0.0f, -height / 2.0f, 0.0f);
        glm::vec3 bottomVertex1(radius * std::cos(angle), -height / 2.0f, radius * std::sin(angle));
        glm::vec3 bottomVertex2(radius * std::cos(nextAngle), -height / 2.0f, radius * std::sin(nextAngle));

        // Add vertices to the vector
        cylinderVertices.insert(cylinderVertices.end(), { topCenter.x, topCenter.y, topCenter.z });
        cylinderVertices.insert(cylinderVertices.end(), { topVertex1.x, topVertex1.y, topVertex1.z });
        cylinderVertices.insert(cylinderVertices.end(), { topVertex2.x, topVertex2.y, topVertex2.z });

        cylinderVertices.insert(cylinderVertices.end(), { bottomCenter.x, bottomCenter.y, bottomCenter.z });
        cylinderVertices.insert(cylinderVertices.end(), { bottomVertex1.x, bottomVertex1.y, bottomVertex1.z });
        cylinderVertices.insert(cylinderVertices.end(), { bottomVertex2.x, bottomVertex2.y, bottomVertex2.z });
    }

    mesh.nVertices = cylinderVertices.size() / 3; // 3 floats per vertex

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, cylinderVertices.size() * sizeof(float), cylinderVertices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // Unbind VAO
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);

    // Delete floor VAO and VBO
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}

void UDestroyTexture(GLuint textureId)
{
    glDeleteTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader

    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}