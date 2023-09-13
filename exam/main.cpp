#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <shader.h>
#include "camera.h"
#include "glmutils.h"
#include "primitives.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// structure to hold render info
// -----------------------------
struct SceneObject {
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
    }

    void drawTessSceneObject() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_PATCHES, 0, vertexCount);
    }
};

// function declarations
// ---------------------
unsigned int createArrayBuffer(const std::vector<float> &array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array);
unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<float> &normals, const std::vector<unsigned int> &indices);
void setup();
void drawObjects();
void draw(SceneObject object, glm::mat4 model);
void drawGui();
void updateActiveShader();

// glfw and input functions
// ------------------------
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// global variables used for rendering
// -----------------------------------
SceneObject triangle;
SceneObject triangleSpread;
std::vector<Shader> shaderPrograms;
Shader* activeShader;

// global variables used for control
// ---------------------------------
const float PI = 3.14159f;
const float linearSpeed = 0.15f; // Camera movement speed
const float rotationGain = 2.0f; // Camera rotation speed
bool isPaused = false;

float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
float currentTime;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f));

// Structure to hold all user defined configurations
struct Config {
    bool useTesselation   = false;
    bool usePolygonMode   = false;
    bool useLevelOfDetail = false;
    bool useEqualSpacing  = true;
    bool useOddSpacing    = false;
    bool useEvenSpacing   = false;

    bool useZNormals = true;
    bool useSpreadNormals = false;

    float tessInner  = 1.0f;
    float tessOuter1 = 1.0f;
    float tessOuter2 = 1.0f;
    float tessOuter3 = 1.0f;

    float uWeight = 1.0f;
    float vWeight = 1.0f;
    float wWeight = 1.0f;
    float normalDist = 0.0f;
} config;

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tesselation Exam Project by Oliver Astrup", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetKeyCallback(window, key_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setup mesh objects
    // ---------------------------------------
    setup();

    glDepthRange(-1, 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    while (!glfwWindowShouldClose(window)) {
        static float lastFrame = 0.0f;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        updateActiveShader();
        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (config.usePolygonMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        drawObjects();

        if (isPaused) {
            drawGui();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup of resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}

void drawObjects() {
    glm::mat4 scale = glm::scale(0.5f, 1.0f, 0.5f);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewProjection = projection * view;
    activeShader->setMat4("viewProjection", viewProjection);

    if (config.useTesselation)
    {
        activeShader->setBool("LOD", config.useLevelOfDetail);
        activeShader->setVec3("camPos", camera.Position);
        activeShader->setFloat("levelInner", config.tessInner);
        activeShader->setFloat("levelOuter0", config.tessOuter1);
        activeShader->setFloat("levelOuter1", config.tessOuter2);
        activeShader->setFloat("levelOuter2", config.tessOuter3);
        activeShader->setFloat("uWeight", config.uWeight);
        activeShader->setFloat("vWeight", config.vWeight);
        activeShader->setFloat("wWeight", config.wWeight);
        activeShader->setFloat("normalDist", config.normalDist);
    }

    glm::mat4 model = glm::translate(-1.0f, 1.0f, 0.0f) * glm::scale(2.5f, 2.0f, 2.5f);
    if (config.useZNormals)
        draw(triangle, model);
    else
        draw(triangleSpread, model);
}

void draw(SceneObject object, glm::mat4 model) {
    activeShader->setMat4("model", model);

    if (config.useTesselation)
        object.drawTessSceneObject();
    else
        object.drawSceneObject();
}

void setup() {
    shaderPrograms.emplace_back("shaders/shaderN.vert", "shaders/shader.frag");
    shaderPrograms.emplace_back("shaders/shader.vert", "shaders/shader.frag", "shaders/shader.tcs", "shaders/shader.tes");
    shaderPrograms.emplace_back("shaders/shader.vert", "shaders/shader.frag", "shaders/shader.tcs", "shaders/shaderOS.tes");
    shaderPrograms.emplace_back("shaders/shader.vert", "shaders/shader.frag", "shaders/shader.tcs", "shaders/shaderES.tes");
    activeShader = &shaderPrograms[0];
    activeShader->use();

    triangle.VAO = createVertexArray(triangleVertices, triangleColors, triangleNormals, triangleIndices);
    triangle.vertexCount = triangleIndices.size();

    triangleSpread.VAO = createVertexArray(triangleVertices, triangleColors, triangleNormals2, triangleIndices);
    triangleSpread.vertexCount = triangleIndices.size();
}

void updateActiveShader() {
    if (config.useTesselation)
    {
        if (config.useEqualSpacing && activeShader->ID != shaderPrograms[1].ID)
        {
            activeShader = &shaderPrograms[1];
            activeShader->use();
        }
        else if (config.useOddSpacing && activeShader->ID != shaderPrograms[2].ID)
        {
            activeShader = &shaderPrograms[2];
            activeShader->use();
        }
        else if (config.useEvenSpacing && activeShader->ID != shaderPrograms[3].ID)
        {
            activeShader = &shaderPrograms[3];
            activeShader->use();
        }
    }
    else
    {
        if (activeShader->ID != shaderPrograms[0].ID)
        {
            activeShader = &shaderPrograms[0];
            activeShader->use();
        }
    }
}

unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<float> &normals, const std::vector<unsigned int> &indices) {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // set vertex shader attribute "pos"
    createArrayBuffer(positions);
    int posAttributeLocation = glGetAttribLocation(shaderPrograms[1].ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "color"
    createArrayBuffer(colors);
    int colorAttributeLocation = glGetAttribLocation(shaderPrograms[1].ID, "color");
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "normal"
    createArrayBuffer(normals);
    int normalAttributeLocation = glGetAttribLocation(shaderPrograms[1].ID, "normal");
    glEnableVertexAttribArray(normalAttributeLocation);
    glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // creates and bind the EBO
    createElementArrayBuffer(indices);

    return VAO;
}

unsigned int createArrayBuffer(const std::vector<float> &array) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);

    return VBO;
}

unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array) {
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}

void drawGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Settings");
        if (ImGui::RadioButton("Enable Tesselation", config.useTesselation))
            { config.useTesselation = !config.useTesselation; }

        if (ImGui::RadioButton("Enable Polygon Mode", config.usePolygonMode))
            { config.usePolygonMode = !config.usePolygonMode; }

        if (ImGui::RadioButton("Enable Level of Detail", config.useLevelOfDetail))
            { config.useLevelOfDetail = !config.useLevelOfDetail; }

        ImGui::Separator();
        ImGui::Text("Tessellation spacing: ");
        {
            if (ImGui::RadioButton("Equal Spacing", config.useEqualSpacing)) {
                config.useEqualSpacing = true;
                config.useOddSpacing = false;
                config.useEvenSpacing = false;
            }

            if (ImGui::RadioButton("Odd Spacing", config.useOddSpacing)) {
                config.useEqualSpacing = false;
                config.useOddSpacing = true;
                config.useEvenSpacing = false;
            }

            if (ImGui::RadioButton("Even Spacing", config.useEvenSpacing)) {
                config.useEqualSpacing = false;
                config.useOddSpacing = false;
                config.useEvenSpacing = true;
            }
        }

        ImGui::Separator();
        ImGui::Text("Tessellation levels: ");
        ImGui::SliderFloat("Inner Level 0", &config.tessInner, 1.0f, 20.0f);
        ImGui::SliderFloat("Outer Level 0", &config.tessOuter1, 1.0f, 20.0f);
        ImGui::SliderFloat("Outer Level 1", &config.tessOuter2, 1.0f, 20.0f);
        ImGui::SliderFloat("Outer Level 2", &config.tessOuter3, 1.0f, 20.0f);

        ImGui::Separator();
        ImGui::Text("Vertex Weights: ");
        ImGui::SliderFloat("U", &config.uWeight, 0.0f, 20.0f);
        ImGui::SliderFloat("V", &config.vWeight, 0.0f, 20.0f);
        ImGui::SliderFloat("W", &config.wWeight, 0.0f, 20.0f);

        ImGui::Separator();
        ImGui::Text("Normal Vectors: ");
        {
            if (ImGui::RadioButton("Z-align", config.useZNormals)) {
                config.useZNormals = true;
                config.useSpreadNormals = false;
            }

            if (ImGui::RadioButton("Spreading", config.useSpreadNormals)) {
                config.useZNormals = false;
                config.useSpreadNormals = true;
            }
        };

        ImGui::SliderFloat("Multiplier", &config.normalDist, 0.0f, 5.0f);

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = posX;
        lastY = posY;
        firstMouse = false;
    }

    float xoffset = posX - lastX;
    float yoffset = lastY - posY; // reversed since y-coordinates go from bottom to top

    lastX = posX;
    lastY = posY;

    if (isPaused)
        return;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (isPaused)
        return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}