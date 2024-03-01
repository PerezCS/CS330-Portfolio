#include <iostream>         // cout, cerr
#include <vector>
#include <cmath>
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include "camera.h" // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "7-1 Project Tyten Perez"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint ebo;
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // mesh data
    GLMesh gMeshCube; // basic cube
    GLMesh gMeshCubeChargerBody;
    GLMesh gMeshCubeChargerProng1;
    GLMesh gMeshCubeChargerProng2;
    GLMesh gMeshPlane; // basic plane
    GLMesh gMeshPyramid; // basic pyramid
    GLMesh gMeshCylinder; // Battery body
    GLMesh gMeshCylinderTop; // battery top face (for different texture)
    GLMesh gMeshCylinderCathode; // battery top cylinder terminal
    GLMesh gMeshSphere; // basic cube

    // Texture
    GLuint gTexture1;
    GLuint gTexture2;
    GLuint gTexture3;
    GLuint gTexture4;
    GLuint gTexture5;
    GLuint gTexture6;
    GLuint gTexture7;

    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader program
    GLuint gCubeProgramId;
    GLuint gLampProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 5.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;
    bool isPerspective = true; // True for perspective projection, false for orthographic

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gCubeScale(3.0f);

    // Cube and light color
    glm::vec3 gObjectColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightColor(0.5f, 0.5f, 0.5f);
    glm::vec3 gLightColor2(0.95f, 0.94f, 0.93f);
    glm::vec3 gLightColor3(0.6f, 0.6f, 0.6f); 

    // Light position and scale
    glm::vec3 gLightPosition(-10.0f, 4.0f, 0.0f); 
    glm::vec3 gLightPosition2(0.0f, 3.0f, 7.0f); 
    glm::vec3 gLightPosition3(5.0f, 7.0f, -12.0f);
    glm::vec3 gLightScale(0.01f);

    // Lamp animation
    bool gIsLampOrbiting = false;
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
void UCreateMeshCube(GLMesh& mesh);
void UCreateMeshPlane(GLMesh& mesh);
void UCreateMeshPyramid(GLMesh& mesh);
void UCreateMeshCylinder(GLMesh& mesh);
void UCreateMeshSphere(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
void renderObject(const GLMesh& mesh, const glm::mat4& model, GLuint textureID, GLint modelLoc);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light colors, light positions, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor[3]; // Array to hold multiple light colors
uniform vec3 lightPos[3]; // Array to hold multiple light positions
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /* Phong lighting model calculations to generate ambient, diffuse, and specular components */
    float ambientStrength = 0.01f; // Set ambient or global lighting strength
    vec3 ambient = vec3(0.0f); // Initialize ambient component
    vec3 diffuse = vec3(0.0f); // Initialize diffuse component
    vec3 specular = vec3(0.0f); // Initialize specular component

    for (int i = 0; i < 3; ++i) { // Loop through each light source
        // Calculate Ambient lighting
        ambient += ambientStrength * lightColor[i]; // Generate ambient light color for each light

        // Calculate Diffuse lighting
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(lightPos[i] - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        diffuse += impact * lightColor[i]; // Generate diffuse light color for each light

        // Calculate Specular lighting
        float specularIntensity = 0.6f; // Set specular light strength
        float highlightSize = 16.0f; // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
        // Calculate specular component for each light
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        specular += specularIntensity * specularComponent * lightColor[i];
    }

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU

}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);

/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
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

    // Toy puzzle cube
    UCreateMeshCube(gMeshCube);
    
    // Phone charger adapter
    UCreateMeshCube(gMeshCubeChargerBody);
    UCreateMeshCube(gMeshCubeChargerProng1);
    UCreateMeshCube(gMeshCubeChargerProng2);
    
    // Wood surface
    UCreateMeshPlane(gMeshPlane);
    
    // testing
    UCreateMeshPyramid(gMeshPyramid);
    
    // Battery
    UCreateMeshCylinder(gMeshCylinder);
    UCreateMeshCylinder(gMeshCylinderTop);
    UCreateMeshCylinder(gMeshCylinderCathode);

    // Toy ball
    UCreateMeshSphere(gMeshSphere);

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // texture images/sources
    const char* textureToy = "toypuzzle.png";                    // Image credit: me
    const char* textureWood = "wood.png";                        // Image credit: polyhaven.com, Creative Commons - CC0 1.0 Universal
    const char* textureBattery = "batterybody.png";              // Image credit: me
    const char* textureBatteryTop = "batterytop.png";            // Image credit: me
    const char* chargerAdapterBody = "chargeradapterbody.png";   // Image credit: me
    const char* chargerAdapterProng = "chargeradapterprong.png"; // Image credit: me
    const char* textureBall = "plasticball.png";                 // Image credit: texturecan.com, Creative Commons - CC0 1.0 Universal

    // Load textures
    if (!UCreateTexture(textureToy, gTexture1))
    {
        cout << "Failed to load texture " << textureToy << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(textureWood, gTexture2))
    {
        cout << "Failed to load texture " << textureWood << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(textureBattery, gTexture3))
    {
        cout << "Failed to load texture " << textureBattery << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(textureBatteryTop, gTexture4))
    {
        cout << "Failed to load texture " << textureBatteryTop << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(chargerAdapterBody, gTexture5))
    {
        cout << "Failed to load texture " << chargerAdapterBody << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(chargerAdapterProng, gTexture6))
    {
        cout << "Failed to load texture " << chargerAdapterProng << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(textureBall, gTexture7))
    {
        cout << "Failed to load texture " << textureBall << endl;
        return EXIT_FAILURE;
    }



    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gCubeProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
    UDestroyMesh(gMeshCube);
    UDestroyMesh(gMeshCubeChargerBody);
    UDestroyMesh(gMeshCubeChargerProng1);
    UDestroyMesh(gMeshCubeChargerProng2);
    UDestroyMesh(gMeshPlane);
    UDestroyMesh(gMeshPyramid);
    UDestroyMesh(gMeshCylinder);
    UDestroyMesh(gMeshCylinderTop);
    UDestroyMesh(gMeshCylinderCathode);
    UDestroyMesh(gMeshSphere);

    // Release texture
    UDestroyTexture(gTexture1);
    UDestroyTexture(gTexture2);
    UDestroyTexture(gTexture3);
    UDestroyTexture(gTexture4);
    UDestroyTexture(gTexture5);
    UDestroyTexture(gTexture6);
    UDestroyTexture(gTexture7);

    // Release shader program
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);


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
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;
    static bool keyPressed = false;
    // End program
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // WASD Movement inputs
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    // Camera perspective toggle
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!keyPressed) // Prevents toggling too fast
        {
            isPerspective = !isPerspective;
            keyPressed = true;
        }
    }
    else
    {
        keyPressed = false;
    }
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

// Rendering function for each frame
void URender()
{

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gCubeProgramId);

    glm::mat4 view = gCamera.GetViewMatrix();
    glm::mat4 projection;
    if (isPerspective) {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else {
        float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
        projection = glm::ortho(-aspectRatio * 2.0f, aspectRatio * 2.0f, -2.0f, 2.0f, 0.1f, 100.0f);
    }

    // Set view and projection matrices only once for all objects
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set light and camera data to the shader
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

    glm::vec3 lightPositions[] = { gLightPosition, gLightPosition2, gLightPosition3 };
    glm::vec3 lightColors[] = { gLightColor, gLightColor2, gLightColor3 };

    glUniform3fv(lightPositionLoc, 3, glm::value_ptr(lightPositions[0]));
    glUniform3fv(lightColorLoc, 3, glm::value_ptr(lightColors[0]));
    glUniform3f(viewPositionLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);

    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


    // Plane transformations (desk surface)
    glm::mat4 planeModel =
        glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) *
        glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f)) *
        glm::scale(glm::vec3(56.0f, 1.0f, 24.0f));
    renderObject(gMeshPlane, planeModel, gTexture2, modelLoc);

    // Pyramid transformations (testing)
    glm::mat4 pyramidModel =
        glm::translate(glm::vec3(20.0f, -0.96f, 0.0f)) *
        glm::rotate(-0.5f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
    renderObject(gMeshPyramid, pyramidModel, gTexture1, modelLoc);

    // Cylinder (battery)
    // -----------------------
    // Cylinder (battery body)
    glm::mat4 cylinderModel =
        glm::translate(glm::vec3(-1.2f, -0.47f, 2.0f)) *
        glm::rotate(3.14f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
    renderObject(gMeshCylinder, cylinderModel, gTexture3, modelLoc);
    // Cylinder (battery top face)
    glm::mat4 cylinderModelTop =
        glm::translate(glm::vec3(-1.2f, 0.032f, 2.0f)) *
        glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f)) *
        glm::scale(glm::vec3(0.98f, 0.01f, 0.98f));
    renderObject(gMeshCylinderTop, cylinderModelTop, gTexture4, modelLoc);
    // Cylinder (battery top positive terminal)
    glm::mat4 cylinderModelCathode =
        glm::translate(glm::vec3(-1.2f, 0.032f, 2.0f)) *
        glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f)) *
        glm::scale(glm::vec3(0.5f, 0.05f, 0.5f));
    renderObject(gMeshCylinderCathode, cylinderModelCathode, gTexture4, modelLoc);

    // cube transformations (Charging Adapter)
    // ----------------------------------------------
    // cube transformations (Charging Adapter's body)
    glm::mat4 cubeModelChargerBody =
        glm::translate(glm::vec3(2.0f, -0.5f, -0.1f)) *
        glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f)) *
        glm::scale(glm::vec3(0.55f, 1.0f, 0.7f));
    renderObject(gMeshCubeChargerBody, cubeModelChargerBody, gTexture5, modelLoc);
    // cube transformations (Charging Adapter's prong 1)
    glm::mat4 cubeModelChargerProng1 =
        glm::translate(cubeModelChargerBody, glm::vec3(0.0f, 0.65f, 0.22f)) *
        glm::rotate(1.571f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::vec3(0.01f, 0.30f, 0.2f));
    renderObject(gMeshCubeChargerProng1, cubeModelChargerProng1, gTexture6, modelLoc);
    // cube transformations (Charging Adapter's prong 2)
    glm::mat4 cubeModelChargerProng2 =
        glm::translate(cubeModelChargerBody, glm::vec3(0.0f, 0.65f, -0.22f)) *
        glm::rotate(1.571f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::vec3(0.01f, 0.30f, 0.2f));
    renderObject(gMeshCubeChargerProng2, cubeModelChargerProng2, gTexture6, modelLoc);

    // sphere transformations (Toy ball)
    glm::mat4 sphereModel =
        glm::translate(glm::vec3(-2.0f, -0.4f, 1.0f)) *
        glm::rotate(1.5708f, glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::scale(glm::vec3(0.6f, 0.6f, 0.6f));
    renderObject(gMeshSphere, sphereModel, gTexture7, modelLoc);

    // cube transformations (Toy puzzle)
    glm::mat4 cubeModel =
        glm::translate(glm::vec3(0.0f, -0.25f, 0.0f)) *
        glm::rotate(0.769f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
    renderObject(gMeshCube, cubeModel, gTexture1, modelLoc);

    // Render each light
    glUseProgram(gLampProgramId);
    for (int i = 0; i < 3; ++i) {
        glm::mat4 lampModel = glm::translate(lightPositions[i]) * glm::scale(gLightScale);
        glUniformMatrix4fv(glGetUniformLocation(gLampProgramId, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));
        glUniformMatrix4fv(glGetUniformLocation(gLampProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(gLampProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glDrawElements(GL_TRIANGLES, gMeshCube.nVertices, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glfwSwapBuffers(gWindow);
}

// render given object with model matrix and texture
void renderObject(const GLMesh& mesh, const glm::mat4& model, GLuint textureID, GLint modelLoc)
{
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(mesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glDrawElements(GL_TRIANGLES, mesh.nVertices, GL_UNSIGNED_INT, 0);
    //glBindVertexArray(0);
}

// Implements the UCreateMesh function
void UCreateMeshCube(GLMesh& mesh)
{
    // Vertex data for cube with texture coordinates
    GLfloat verts[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,    0.0f, 1.0f,// 0
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // 1
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // 2
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // 3

        // Right face
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // 4
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // 5
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // 6
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // 7

        // Back face
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,   0.0f, 1.0f, // 8
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,   1.0f, 1.0f, // 9
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,   1.0f, 0.0f, // 10
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  -1.0f,   0.0f, 0.0f, // 11

        // Left face
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // 12
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // 13
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // 14
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // 15

        // Top face
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, // 16
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f, // 17
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // 18
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f, // 19

        // Bottom face
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f, // 20
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // 21
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // 22
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // 23
    };

    // Indices for the cube
    GLuint indices[] = {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Right face
        4, 5, 6, 4, 6, 7,
        // Back face
        8, 9, 10, 8, 10, 11,
        // Left face
        12, 13, 14, 12, 14, 15,
        // Top face
        16, 17, 18, 16, 18, 19,
        // Bottom face
        20, 21, 22, 20, 22, 23
    };


    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

// plane shape (table surface)
void UCreateMeshPlane(GLMesh& mesh)
{
    // Vertex data for plane
    GLfloat verts[] = {
        // Positions          // normals     // texture coordinates
        -0.5f, 0.0f, -0.5f,   0.0f,  1.0f,   0.0f, 0.0f, 2.0f, // 0
         0.5f, 0.0f, -0.5f,   0.0f,  1.0f,   0.0f, 2.0f, 2.0f, // 1
         0.5f, 0.0f,  0.5f,   0.0f,  1.0f,   0.0f, 2.0f, 0.0f, // 2
        -0.5f, 0.0f,  0.5f,   0.0f,  1.0f,   0.0f, 0.0f, 0.0f, // 3
    };

    GLuint indices[] = {
        0, 1, 2, 0, 2, 3 // Base
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateMeshPyramid(GLMesh& mesh)
{
    // Vertex data for pyramid with texture coordinates and normals
    GLfloat verts[] = {
        // Base
        -0.5f, 0.0f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // 0
         0.5f, 0.0f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // 1
         0.5f, 0.0f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // 2
        -0.5f, 0.0f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // 3

        // Front face
        -0.5f, 0.0f, -0.5f,  0.0f,  0.5f,  0.5f,  0.0f, 0.0f, // 4
         0.5f, 0.0f, -0.5f,  0.0f,  0.5f,  0.5f,  1.0f, 0.0f, // 5
         0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f, 1.0f, // 6 top

        // Right face
         0.5f, 0.0f, -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 0.0f, // 7
         0.5f, 0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f, 0.0f, // 8
         0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.0f,  0.5f, 1.0f, // 9 top

        // Back face
         0.5f, 0.0f,  0.5f,  0.0f,  0.5f, -0.5f,  0.0f, 0.0f, // 10
        -0.5f, 0.0f,  0.5f,  0.0f,  0.5f, -0.5f,  1.0f, 0.0f, // 11
         0.0f,  1.0f,  0.0f,  0.0f,  0.5f, -0.5f,  0.5f, 1.0f, // 12 top

        // Left face
        -0.5f, 0.0f,  0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 0.0f, // 13
        -0.5f, 0.0f, -0.5f, -0.5f,  0.5f,  0.0f,  1.0f, 0.0f, // 14
         0.0f,  1.0f,  0.0f, -0.5f,  0.5f,  0.0f,  0.5f, 1.0f  // 15 rop
    };

    // Indices for the pyramid
    GLuint indices[] = {
        // Base
        0, 1, 2, 0, 2, 3,
        // Sides
        4, 5, 6, // Front
        7, 8, 9, // Right
        10, 11, 12, // Back
        13, 14, 15  // Left
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateMeshCylinder(GLMesh& mesh) {
    int segments = 12; // cylinder sides
    float height = 1.0f;
    float radius = 0.15f;

    std::vector<float> vertices;
    std::vector<int> indices;
    float segmentAngle = 2 * 3.14159 / segments;

    // Generate vertices for the cylinder sides
    for (int i = 0; i <= segments; ++i) {
        float angle = i * segmentAngle;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Top vertex
        vertices.insert(vertices.end(), { x, height / 2, z, x, 0, z, (float)i / segments, 1.0f });
        // Bottom vertex
        vertices.insert(vertices.end(), { x, -height / 2, z, x, 0, z, (float)i / segments, 0.0f });
    }

    // Indices for the sides
    for (int i = 0; i < segments; ++i) {
        int base = i * 2;
        indices.insert(indices.end(), { base, base + 1, base + 3, base, base + 3, base + 2 });
    }

    // Function to add cap vertices and indices
    auto addCap = [&](float y, bool top) {
        // Center vertex for the cap
        int centerIndex = vertices.size() / 8;
        vertices.insert(vertices.end(), { 0, y, 0, 0, top ? 1.0f : -1.0f, 0, 0.5f, 0.5f });

        // Edge vertices for the cap
        for (int i = 0; i <= segments; ++i) {
            float angle = i * segmentAngle;
            float x = cos(angle) * radius;
            float z = sin(angle) * radius;
            vertices.insert(vertices.end(), { x, y, z, 0, top ? 1.0f : -1.0f, 0, x * 0.5f + 0.5f, z * 0.5f + 0.5f });
        }

        // Indices for the cap
        for (int i = 0; i < segments; ++i) {
            indices.push_back(centerIndex);
            indices.push_back(centerIndex + i + 1);
            indices.push_back(centerIndex + i + 2);
        }
    };

    // Add top and bottom caps
    addCap(height / 2, true);  // Top cap
    addCap(-height / 2, false); // Bottom cap

    // Setup mesh (VBO, VAO, EBO)
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coords attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    mesh.nVertices = indices.size();
}

void UCreateMeshSphere(GLMesh& mesh) {
        int sectorCount = 24;
        int stackCount = 12;
        float radius = 1.0f;
        float M_PI = 3.14159;

        std::vector<float> vertices;
        std::vector<int> indices;

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = M_PI / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;

                vertices.insert(vertices.end(), { x, y, z, nx, ny, nz, s, t });
            }
        }

        int k1, k2;
        for (int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                // 2 triangles per sector excluding 1st and last stacks
                // k1 => k2 => k1+1
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        // Setup mesh (VBO, VAO, EBO)
        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &mesh.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Texture coords attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        mesh.nVertices = indices.size();
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
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
    glGenTextures(1, &textureId);
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
