#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "FlightSimulator.h"
#include "SceneExtras.h"

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera and Controls
Camera camera(glm::vec3(0.0f, 15.0f, -2640.0f));
FlightSimulator flightSim;
SceneExtras sceneExtras;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseLookActive = false; // Right mouse button for orbit in chase camera
bool cursorCaptured = false;  // CS2 style cursor lock for free camera

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Forward declaration
void setCameraMode(GLFWwindow* window, CameraMode newMode);

// Point Light in 3D scene (Assignment requirement)
glm::vec3 pointLightPos = glm::vec3(-35.0f, 18.0f, -2300.0f);
glm::vec3 pointLightColor = glm::vec3(1.0f, 0.95f, 0.85f);
float pointLightIntensity = 1.6f;

// Sun / Directional Light
glm::vec3 sunDir = glm::normalize(glm::vec3(0.5f, 0.8f, 0.35f));
glm::vec3 sunColor = glm::vec3(1.0f, 0.98f, 0.92f);

// GLFW callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // In Free Camera mode (CS2 style): rotates infinitely whenever cursor is captured
    if (camera.mode == FREE_CAMERA) {
        if (cursorCaptured) {
            camera.processMouseMovement(xoffset, yoffset);
        }
    } else if (camera.mode == CHASE_CAMERA) {
        // Orbit around aircraft when holding right mouse button
        if (mouseLookActive) {
            camera.processMouseMovement(xoffset, yoffset);
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            mouseLookActive = true;
        } else if (action == GLFW_RELEASE) {
            mouseLookActive = false;
        }
    }
    // Clicking left button in Free Camera re-locks the cursor (like CS2)
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (camera.mode == FREE_CAMERA && !cursorCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursorCaptured = true;
            firstMouse = true;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processMouseScroll(static_cast<float>(yoffset));
}

void setCameraMode(GLFWwindow* window, CameraMode newMode) {
    camera.mode = newMode;
    firstMouse = true;
    if (newMode == FREE_CAMERA) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        cursorCaptured = true;
        std::cout << "[Camera] Mode 2: FREE FLY (CS2 Style: infinite 360 mouse rotation). Press [TAB] to release cursor." << std::endl;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        cursorCaptured = false;
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Flight controls & Free Camera movement
    if (camera.mode == FREE_CAMERA) {
        bool sprint = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
                       glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processKeyboard(0, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processKeyboard(1, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processKeyboard(2, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processKeyboard(3, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.processKeyboard(4, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.processKeyboard(5, deltaTime, sprint);
    } else {
        // Aircraft manual pitch/yaw if in manual mode or override
        if (!flightSim.isAutoPilot) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                flightSim.adjustPitch(-18.0f * deltaTime); // Push stick forward (nose down)
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                flightSim.adjustPitch(18.0f * deltaTime);  // Pull stick back (nose UP / rotate)
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                flightSim.adjustYaw(-10.0f * deltaTime);   // Rudder left
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                flightSim.adjustYaw(10.0f * deltaTime);    // Rudder right
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                flightSim.adjustThrottle(0.3f * deltaTime);
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                flightSim.adjustThrottle(-0.3f * deltaTime);
        }
    }

    // Move Point Light in scene to demo Phong Shading reactivity
    float lightSpeed = 30.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) pointLightPos.z += lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) pointLightPos.z -= lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) pointLightPos.x -= lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) pointLightPos.x += lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) pointLightPos.y += lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) pointLightPos.y -= lightSpeed;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Camera selection
        if (key == GLFW_KEY_1) setCameraMode(window, CHASE_CAMERA);
        if (key == GLFW_KEY_2) setCameraMode(window, FREE_CAMERA);
        if (key == GLFW_KEY_3) setCameraMode(window, COCKPIT_CAMERA);
        if (key == GLFW_KEY_4) setCameraMode(window, TOWER_CAMERA);

        // Tab toggles cursor lock in Free Camera
        if (key == GLFW_KEY_TAB && camera.mode == FREE_CAMERA) {
            cursorCaptured = !cursorCaptured;
            if (cursorCaptured) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        if (key == GLFW_KEY_SPACE) {
            flightSim.startTakeoff();
        }
        if (key == GLFW_KEY_P) {
            flightSim.togglePause();
        }
        if (key == GLFW_KEY_R) {
            flightSim.reset();
            std::cout << "[Simulator] Flight reset to runway threshold." << std::endl;
        }
        if (key == GLFW_KEY_M) {
            flightSim.toggleControlMode();
            std::cout << "[Simulator] Mode switched to: " 
                      << (flightSim.isAutoPilot ? "AUTOPILOT (TOLD GUIDED)" : "MANUAL PILOT") << std::endl;
        }
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "  UBI - Computacao Grafica: Assignment 2\n";
    std::cout << "  Aircraft Takeoff Simulator in R3 (Modern OpenGL)\n";
    std::cout << "  Student #50123 - Tiago Deusdado (Takeoff Procedure)\n";
    std::cout << "=======================================================\n\n";

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "UBI CG - Aircraft Flight Simulator (Takeoff)", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glCullFace(GL_BACK);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n\n";

    // Compile Shaders
    std::cout << "[Shaders] Compiling Phong, Skybox, and Emissive shaders..." << std::endl;
    Shader phongShader("shaders/phong.vert", "shaders/phong.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");
    Shader emissiveShader("shaders/emissive.vert", "shaders/emissive.frag");

    // Load 3D Models
    std::cout << "\n[Models] Loading 3D .obj models required by assignment..." << std::endl;
    Model runwayModel;
    if (!runwayModel.loadOBJ("runway.obj", glm::vec3(0.35f, 0.35f, 0.37f))) {
        std::cerr << "Warning: Could not load runway.obj!" << std::endl;
    }

    Model aircraftModel;
    if (!aircraftModel.loadOBJ("f104starfighter.obj", glm::vec3(0.78f, 0.80f, 0.84f))) {
        std::cerr << "Warning: Could not load f104starfighter.obj!" << std::endl;
    }

    // Initialize Scene Extras (Skybox, Runway Lights, Airport Buildings, Terrain)
    std::cout << "\n[SceneExtras] Initializing Airport extras (Skybox, Runway Lights, Control Tower, Hangars)..." << std::endl;
    sceneExtras.init();

    std::cout << "\n-------------------------------------------------------\n";
    std::cout << "CONTROLS:\n";
    std::cout << "  [SPACE]     : Start Takeoff / Throttle up\n";
    std::cout << "  [P]         : Pause / Resume simulation\n";
    std::cout << "  [R]         : Reset aircraft to runway start\n";
    std::cout << "  [M]         : Toggle Autopilot (TOLD guided) vs Manual Pilot\n";
    std::cout << "  [1, 2, 3, 4]: Cameras: 1-Chase, 2-Free Fly (CS2 style), 3-Cockpit, 4-Tower\n";
    std::cout << "  [TAB]       : Release / Re-capture mouse cursor in Free Fly Camera\n";
    std::cout << "  [SHIFT]     : Sprint / Fast movement in Free Fly Camera\n";
    std::cout << "  Right Mouse : Hold to look around / orbit camera in Chase mode\n";
    std::cout << "  Scroll      : Zoom / Camera distance\n";
    std::cout << "  [W/S/A/D]   : Move in Free Cam (or pitch/yaw in Manual Flight)\n";
    std::cout << "  [I/K/J/L]   : Move Point Light in 3D space (Phong Shading demo)\n";
    std::cout << "  [ESC]       : Exit application\n";
    std::cout << "-------------------------------------------------------\n\n";

    float titleTimer = 0.0f;

    // Main Render Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process Input
        processInput(window);

        // Update Flight Simulation (Takeoff Dynamics & TOLD)
        flightSim.update(deltaTime);

        // Update Window Title with Live Avionics / Telemetry HUD
        titleTimer += deltaTime;
        if (titleTimer >= 0.15f) {
            titleTimer = 0.0f;
            std::stringstream ss;
            ss << std::fixed << std::setprecision(0);
            ss << "UBI CG Takeoff | IAS: " << flightSim.getAirspeedKnots() << " kts";
            ss << " | ALT: " << flightSim.getAltitudeFeet() << " ft";
            ss << " | VS: " << (flightSim.getVerticalSpeedFPM() >= 0 ? "+" : "") << flightSim.getVerticalSpeedFPM() << " fpm";
            ss << " | PITCH: " << std::setprecision(1) << flightSim.pitch << " deg";
            ss << " | [" << flightSim.getPhaseString() << "]";
            std::string camStr = (camera.mode == CHASE_CAMERA) ? "CHASE" :
                                 (camera.mode == FREE_CAMERA) ? "FREE" :
                                 (camera.mode == COCKPIT_CAMERA) ? "COCKPIT" : "TOWER";
            ss << " | CAM: " << camStr;
            glfwSetWindowTitle(window, ss.str().c_str());
        }

        // Clear buffers
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Projection and View matrices
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (height > 0) ? (float)width / (float)height : 1.777f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), aspect, 0.05f, 25000.0f);

        glm::mat4 view = camera.getViewMatrix(flightSim.position, 
                                              flightSim.getForwardVector(), 
                                              flightSim.getUpVector());
        glm::vec3 camPos = camera.getCameraPosition(flightSim.position, 
                                                    flightSim.getForwardVector(), 
                                                    flightSim.getUpVector());

        // 1. Render Skybox (Atmospheric Zenith & Sun)
        skyboxShader.use();
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setMat4("view", view);
        skyboxShader.setVec3("sunDir", sunDir);
        sceneExtras.renderSkybox(view, projection, sunDir);

        // 2. Setup Phong Shader uniforms
        phongShader.use();
        phongShader.setMat4("projection", projection);
        phongShader.setMat4("view", view);
        phongShader.setVec3("viewPos", camPos);

        // Directional Sun lighting
        phongShader.setVec3("dirLightDir", sunDir);
        phongShader.setVec3("dirLightColor", sunColor);
        phongShader.setFloat("ambientStrength", 0.32f);

        // Point Light uniforms (Assignment required)
        phongShader.setVec3("pointLightPos", pointLightPos);
        phongShader.setVec3("pointLightColor", pointLightColor);
        phongShader.setFloat("pointLightConstant", 1.0f);
        phongShader.setFloat("pointLightLinear", 0.007f);
        phongShader.setFloat("pointLightQuadratic", 0.0002f);
        phongShader.setFloat("pointLightIntensity", pointLightIntensity);

        // 3. Render Surrounding Terrain & Airport Buildings
        sceneExtras.renderSurroundingTerrain(phongShader);
        sceneExtras.renderAirportBuildings(phongShader);

        // 4. Render Runway Mesh (.obj)
        {
            phongShader.setVec3("objectColor", glm::vec3(0.42f, 0.43f, 0.45f)); // Asphalt grey
            phongShader.setVec3("specularColor", glm::vec3(0.15f, 0.15f, 0.15f));
            phongShader.setFloat("shininess", 16.0f);

            glm::mat4 model = glm::mat4(1.0f);
            phongShader.setMat4("model", model);
            glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(model)));
            phongShader.setMat3("normalMatrix", normMat);

            runwayModel.render();
        }

        // 5. Render Aircraft Mesh (.obj)
        {
            // Military aircraft polished aluminum / metal fuselage
            phongShader.setVec3("objectColor", glm::vec3(0.82f, 0.84f, 0.88f));
            phongShader.setVec3("specularColor", glm::vec3(0.75f, 0.75f, 0.8f));
            phongShader.setFloat("shininess", 64.0f); // High specular shine for metallic F-104

            glm::mat4 model = flightSim.getModelMatrix();
            phongShader.setMat4("model", model);
            glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(model)));
            phongShader.setMat3("normalMatrix", normMat);

            bool renderCanopy = (camera.mode != COCKPIT_CAMERA);
            aircraftModel.render(renderCanopy);
        }

        // 6. Render Emissive Scene Elements (Point Light Marker & Runway Lights)
        emissiveShader.use();
        emissiveShader.setMat4("projection", projection);
        emissiveShader.setMat4("view", view);

        // Point Light glowing marker in 3D scene
        sceneExtras.renderPointLightMarker(emissiveShader, pointLightPos, pointLightColor);

        // Runway lights (Green threshold, White edge, Red end)
        sceneExtras.renderRunwayLights(emissiveShader);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
