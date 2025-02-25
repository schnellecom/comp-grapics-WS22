/*===========================================================================*/
/*                                                                           *
 * Copyright (c) 2020, Computer Graphics Group RWTH Aachen University        *
 *                            All rights reserved                            *
 *                                                                           *
 * Basic Techniques in Computer Graphics Exercise                            *
 *                            DO NOT EDIT THIS FILE!                         *
 *                                                                           */
/*===========================================================================*/

#include "assignment.hh"

#include <fstream>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>

/// decide whether to build solution or task. only works if you have the solution file :^)
#if VIEW_SOLUTIONS
using namespace solution;
#else
using namespace task;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// some variables we'll need:
//
namespace
{
auto windowWidth = 1024;
auto windowHeight = 1024;
GLFWwindow* window;

// assigment specific includes:
#include "Tools/PNGReader.hh"

// assignment specific variables:
glm::mat4 projectionMatrix;
ShaderProgram* skyBoxShader; // don't edit this shader, it's just used for the skybox!
GLuint skyboxTextureArray;
GLuint skyboxTextureArrayDebug;

ArrayBuffer* abCube;
VertexArrayObject* vaoCube;
ArrayBuffer* abBunny;
ArrayBuffer* abEarth;

double runTime = 0;

bool objectRotates = false;
int meshNumber = 0;
bool cubeMapping = true;
bool debugTexture = false;
bool environmentOnly = true;

}

GenericCamera g_camera;
VertexArrayObject* g_vaoBunny;
VertexArrayObject* g_vaoEarth;
ShaderProgram* g_shader;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool prepareExercise()
{
    // here the shader gets created:
    initCustomResources();

    // don't edit this shader:
    skyBoxShader = new ShaderProgram(std::string(CURR_DIR) + "/assignment10/skybox.vsh", std::string(CURR_DIR) + "/assignment10/skybox.fsh");
    if (!skyBoxShader->link())
        exit(0);

    // Set uniforms that don't change:
    skyBoxShader->use();
    skyBoxShader->setUniform("uTexture", (int)0);

    ////////////////////////////////////////////////////////////////////////////
    // Define geometry:

    ABReader abreader;

    abBunny = abreader.readABFile(std::string(CURR_DIR) + "/assignment10/assets/bunny.ab");
    abEarth = abreader.readABFile(std::string(CURR_DIR) + "/assignment10/assets/sphere32.ab");
    abCube = abreader.readABFile(std::string(CURR_DIR) + "/assignment10/assets/cube.ab");

    // define VAOs:
    g_vaoBunny = new VertexArrayObject();
    g_vaoBunny->attachAllMatchingAttributes(abBunny, g_shader);

    g_vaoEarth = new VertexArrayObject();
    g_vaoEarth->attachAllMatchingAttributes(abEarth, g_shader);

    vaoCube = new VertexArrayObject();
    vaoCube->attachAllMatchingAttributes(abCube, skyBoxShader);

    glEnable(GL_DEPTH_TEST);


    /// skybox arrays:
    PNGReader pngreader;
    TextureData* texture;

    glGenTextures(1, &skyboxTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArray);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/negy.png");
    size_t layerSize = texture->getWidth() * texture->getHeight() * 4;
    unsigned char* buffer = new unsigned char[layerSize * 6]; // 6 RGBA textures
    memcpy(buffer + 0 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/posx.png");
    memcpy(buffer + 1 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/posy.png");
    memcpy(buffer + 2 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/negx.png");
    memcpy(buffer + 3 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/posz.png");
    memcpy(buffer + 4 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/negz.png");
    memcpy(buffer + 5 * layerSize, texture->getData(), layerSize);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB, texture->getWidth(), texture->getHeight(), 6, 0, texture->getFormat(), texture->getType(), buffer);
    delete texture;
    delete[] buffer;

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // debug skybox:

    glGenTextures(1, &skyboxTextureArrayDebug);
    glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArrayDebug);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/negyd.png");
    layerSize = texture->getWidth() * texture->getHeight() * 4;
    buffer = new unsigned char[layerSize * 6]; // 6 RGBA textures
    memcpy(buffer + 0 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/posxd.png");
    memcpy(buffer + 1 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/posyd.png");
    memcpy(buffer + 2 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/negxd.png");
    memcpy(buffer + 3 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/poszd.png");
    memcpy(buffer + 4 * layerSize, texture->getData(), layerSize);
    delete texture;

    texture = pngreader.readFile(std::string(CURR_DIR) + "/assignment10/assets/negzd.png");
    memcpy(buffer + 5 * layerSize, texture->getData(), layerSize);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB, texture->getWidth(), texture->getHeight(), 6, 0, texture->getFormat(), texture->getType(), buffer);
    delete texture;
    delete[] buffer;

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY); // will fail if not all 6 faces were given!

    const glm::vec3 vLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    const float fSpecularityExponent = 800.0f;

    g_shader->use();
    g_shader->setUniform("uLightColor", vLightColor);
    g_shader->setUniform("uSpecularityExponent", fSpecularityExponent);

    return true;
}

void cleanupExercise()
{
    delete skyBoxShader;
    glDeleteTextures(1, &skyboxTextureArrayDebug);
    glDeleteTextures(1, &skyboxTextureArray);

    delete vaoCube;
    delete abCube;
}


glm::mat4 buildFrustum(float phiInDegree, float aspectRatio, float near, float far)
{
    float phiHalfInRadians = 0.5f * phiInDegree * (M_PI / 180.0f);
    float t = near * tan(phiHalfInRadians);
    float b = -t;
    float left = b * aspectRatio;
    float right = t * aspectRatio;

    return glm::frustum(left, right, b, t, near, far);
}


void resizeCallback(GLFWwindow*, int newWidth, int newHeight)
{
    windowWidth = newWidth;
    windowHeight = newHeight;

    glViewport(0, 0, newWidth, newHeight);
    projectionMatrix = buildFrustum(50.0f, (newWidth / (float)newHeight), 0.5f, 1000.0f);
}

void renderSkybox(bool debugTexture, glm::mat4 viewMatrix)
{
    // skybox rendering:
    skyBoxShader->use();
    skyBoxShader->setUniform("uMVP", projectionMatrix * viewMatrix * glm::scale(glm::mat4(1.f), glm::vec3(500.0f)));
    glActiveTexture(GL_TEXTURE0);
    if (debugTexture)
        glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArrayDebug);
    else
        glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArray);
    vaoCube->bind();
    for (int i = 0; i < 6; i++)
    {
        skyBoxShader->setUniform("layer", i);
        glDrawArrays(GL_TRIANGLES, i * 2 * 3, 2 * 3);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

auto key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) -> void
{ /// for infrequently occuring events
    (void)mods;
    (void)scancode;
    if (action == GLFW_PRESS || action == GLFW_REPEAT || action == GLFW_KEY_DOWN)
    {
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        else if (key == GLFW_KEY_1)
            objectRotates = !objectRotates;
        else if (key == GLFW_KEY_2)
            meshNumber = (meshNumber + 1) % 2;
        else if (key == GLFW_KEY_3)
            cubeMapping = !cubeMapping;
        else if (key == GLFW_KEY_4)
            debugTexture = !debugTexture;
        else if (key == GLFW_KEY_5)
            environmentOnly = !environmentOnly;
    }
}

void handleInput(double time)
{ /// for frequently occuring events
    static double timeOfLastFrame = 0.0;
    if (timeOfLastFrame == 0.0)
    {
        timeOfLastFrame = time;
        // ignore the first frame, as the user has not done useful inputs till now anyway
        return;
    }

    // make camera movements based on the elapsed time and not based on frames rendered!
    double timeElapsed = time - timeOfLastFrame;

    double speed = 10.; // magic value to scale the camera speed

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
        speed *= 10.;

    // as long as the keys are hold down, these are triggered each frame:
    if (glfwGetKey(window, GLFW_KEY_W))
        g_camera.moveForward(timeElapsed * speed);
    if (glfwGetKey(window, GLFW_KEY_A))
        g_camera.moveLeft(timeElapsed * speed);
    if (glfwGetKey(window, GLFW_KEY_S))
        g_camera.moveBack(timeElapsed * speed);
    if (glfwGetKey(window, GLFW_KEY_D))
        g_camera.moveRight(timeElapsed * speed);
    if (glfwGetKey(window, GLFW_KEY_E))
        g_camera.moveUp(timeElapsed * speed);
    if (glfwGetKey(window, GLFW_KEY_Q))
        g_camera.moveDown(timeElapsed * speed);

    timeOfLastFrame = time;
}

void mouseMoveCallback(GLFWwindow*, double x, double y)
{
    static glm::vec2 initialPosition;
    static bool leftMouseButtonDown = false;

    if (!leftMouseButtonDown && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
    {
        leftMouseButtonDown = true;
        initialPosition = glm::vec2(x, y);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); /// disable cursor
        return;
    }
    else if (leftMouseButtonDown && !glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
    {
        leftMouseButtonDown = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); /// show cursor
        return;
    }

    glm::vec2 movement = glm::vec2(x, y) - initialPosition;

    if (leftMouseButtonDown)
    {
        glm::vec2 realtiveMovement = glm::vec2(movement) / glm::vec2(windowWidth, windowHeight);
        g_camera.FPSstyleLookAround(realtiveMovement.x, realtiveMovement.y);
        initialPosition = glm::vec2(x, y);
    }
}


int main(int /*argc*/, char* /*argv*/[])
{
    // Initialise GLFW
    if (!glfwInit())
    {
        std::cerr << "[Error] Init of GLFW failed. Terminating." << std::endl;
        glfwTerminate();
        return -1;
    }

    window = common::createWindow(windowWidth, windowHeight, "Basic Techniques in Computer Graphics - Assignment 8");

    if (!window)
        std::cerr << "[Error] Window could not be created!" << std::endl;

    if (!common::init(window))
    {
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
    glfwSetWindowSizeCallback(window, resizeCallback);

    // Enable vertical sync (on cards that support it)
    // vertical sync
    int vSync = 1;
    glfwSwapInterval(vSync);
    glGetError(); // clear errors


    initCustomResources();
    if (!prepareExercise())
        exit(-1);

    resizeCallback(NULL, windowWidth, windowHeight);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetKeyCallback(window, key_callback);


    g_camera.setPosition(glm::vec3(0.0, 2.5, 10.0));

    glEnable(GL_FRAMEBUFFER_SRGB);

    double startTimeInSeconds = glfwGetTime();
    static double rotTime = startTimeInSeconds;
    do
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (objectRotates)
            rotTime += ((glfwGetTime() - startTimeInSeconds) - runTime);
        runTime = glfwGetTime() - startTimeInSeconds;

        handleInput(runTime);

        // matrices:
        static glm::vec3 eyePos = glm::vec3(0.0f, 3.0f, 10.0f);
        static float objectRotation = 0.0f;

        const glm::vec3 vLightPosition = glm::vec3(-10.0f, 10.0f, 4.0f);

        float angle = -2.f * M_PI * rotTime / 10.0f;
        if (objectRotates)
            objectRotation = angle; // if the camera is fix, the object should rotate

        // glm::mat4 viewMatrix = glm::lookAt(eyePos, glm::vec3(0, 3.0, 0), glm::vec3(0, 1, 0));
        glm::mat4 viewMatrix = g_camera.getViewMatrix();
        eyePos = g_camera.getPosition();
        glm::mat4 modelMatrixEarth
            = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 3.0f, 0.0f)) * glm::rotate(glm::mat4(1.f), objectRotation, glm::vec3(0.0f, -1.0f, 0.0f))
              * glm::rotate(glm::mat4(1.f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.f), glm::vec3(3.0f, 3.0f, 3.0f));
        glm::mat4 modelMatrixBunny = glm::rotate(glm::mat4(1.f), objectRotation, glm::vec3(0.0f, -1.0f, 0.0f));


        g_shader->use();
        if (meshNumber == 0)
        {
            glm::mat4 invTranspModelView = glm::inverse(glm::transpose(viewMatrix * modelMatrixEarth));
            g_shader->setUniform("uModelMatrix", modelMatrixEarth);
            g_shader->setUniform("uInvTranspModelViewMatrix", invTranspModelView);

            glm::mat3 uInvTranspModelMatrix = glm::inverse(glm::transpose(glm::mat3(modelMatrixEarth)));
            g_shader->setUniform("uInvTranspModelMatrix", uInvTranspModelMatrix);
        }
        else
        {
            glm::mat4 invTranspModelView = glm::inverse(glm::transpose(viewMatrix * modelMatrixBunny));
            g_shader->setUniform("uModelMatrix", modelMatrixBunny);
            g_shader->setUniform("uInvTranspModelViewMatrix", invTranspModelView);

            glm::mat3 uInvTranspModelMatrix = glm::inverse(glm::transpose(glm::mat3(modelMatrixBunny)));
            g_shader->setUniform("uInvTranspModelMatrix", uInvTranspModelMatrix);
        }

        // per frame changing uniforms:
        g_shader->setUniform("uProjectionMatrix", projectionMatrix);
        g_shader->setUniform("uCameraPosition", eyePos);
        g_shader->setUniform("uViewMatrix", viewMatrix);
        glm::vec3 worldLightPos = glm::vec3(viewMatrix * glm::vec4(vLightPosition, 1.0));
        g_shader->setUniform("uLightPosition", worldLightPos);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderSkybox(debugTexture, viewMatrix);

        g_shader->use();
        drawScene(environmentOnly, meshNumber, cubeMapping, debugTexture);

        // Swap buffers
        glfwSwapBuffers(window);

    } // Check if the window was closed
    while (!glfwWindowShouldClose(window));

    // clean up:
    deleteCustomResources();
    cleanupExercise();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
