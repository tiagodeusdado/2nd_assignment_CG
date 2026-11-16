#ifndef CAMERA_H
#define CAMERA_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMode {
    CHASE_CAMERA = 1,
    FREE_CAMERA = 2,
    COCKPIT_CAMERA = 3,
    TOWER_CAMERA = 4
};

class Camera {
public:
    CameraMode mode;

    // Free camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler angles (Free Camera)
    float yaw;
    float pitch;

    // Camera settings
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // Chase camera settings
    float chaseDistance;
    float chaseHeight;
    float chaseOrbitYaw;    // Orbit angle around aircraft
    float chaseOrbitPitch;

    // Tower camera position
    glm::vec3 towerPosition;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 15.0f, -2650.0f))
        : mode(CHASE_CAMERA),
          position(startPos),
          front(glm::vec3(0.0f, 0.0f, 1.0f)),
          worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
          yaw(90.0f),
          pitch(0.0f),
          movementSpeed(75.0f),
          mouseSensitivity(0.12f),
          zoom(45.0f),
          chaseDistance(28.0f),
          chaseHeight(8.0f),
          chaseOrbitYaw(0.0f),
          chaseOrbitPitch(0.0f),
          towerPosition(glm::vec3(-82.0f, 48.0f, -1200.0f)) {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix(const glm::vec3& targetPos, const glm::vec3& targetFront, const glm::vec3& targetUp) {
        if (mode == CHASE_CAMERA) {
            // Compute chase position relative to aircraft orientation and optional orbit offset
            float combinedYaw = glm::radians(chaseOrbitYaw);
            glm::vec3 rotatedFront = glm::rotate(glm::mat4(1.0f), combinedYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(targetFront, 0.0f);
            
            glm::vec3 camPos = targetPos - rotatedFront * chaseDistance + glm::vec3(0.0f, chaseHeight + chaseOrbitPitch, 0.0f);
            glm::vec3 lookAtTarget = targetPos + targetFront * 8.0f;
            return glm::lookAt(camPos, lookAtTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (mode == COCKPIT_CAMERA) {
    // Posiciona a câmara à frente do nariz do avião, olhando para a pista
    glm::vec3 cockpitOffset = targetFront * 12.0f + targetUp * 1.5f; 
    glm::vec3 camPos = targetPos + cockpitOffset;
    glm::vec3 lookAtTarget = camPos + targetFront * 100.0f;
    return glm::lookAt(camPos, lookAtTarget, targetUp);
}

        else if (mode == TOWER_CAMERA) {
            // Tower observation balcony with crystal-clear view of the entire takeoff run
            return glm::lookAt(towerPosition, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            // Free fly camera (CS2 style)
            return glm::lookAt(position, position + front, up);
        }
    }

    glm::vec3 getCameraPosition(const glm::vec3& targetPos, const glm::vec3& targetFront, const glm::vec3& targetUp) const {
        if (mode == CHASE_CAMERA) {
            float combinedYaw = glm::radians(chaseOrbitYaw);
            glm::vec3 rotatedFront = glm::rotate(glm::mat4(1.0f), combinedYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(targetFront, 0.0f);
            return targetPos - rotatedFront * chaseDistance + glm::vec3(0.0f, chaseHeight + chaseOrbitPitch, 0.0f);
        } else if (mode == COCKPIT_CAMERA) {
            return targetPos + targetFront * 4.60f + targetUp * 1.68f;
        } else if (mode == TOWER_CAMERA) {
            return towerPosition;
        } else {
            return position;
        }
    }

    void processKeyboard(int direction, float deltaTime, bool sprint = false) {
        if (mode != FREE_CAMERA) return;

        float currentSpeed = sprint ? (movementSpeed * 3.5f) : movementSpeed;
        float velocity = currentSpeed * deltaTime;

        if (direction == 0) // FORWARD (W)
            position += front * velocity;
        if (direction == 1) // BACKWARD (S)
            position -= front * velocity;
        if (direction == 2) // LEFT (A)
            position -= right * velocity;
        if (direction == 3) // RIGHT (D)
            position += right * velocity;
        if (direction == 4) // UP (E / Space)
            position += worldUp * velocity;
        if (direction == 5) // DOWN (Q / Ctrl)
            position -= worldUp * velocity;
    }

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        if (mode == FREE_CAMERA) {
            yaw += xoffset;
            pitch += yoffset;

            if (constrainPitch) {
                if (pitch > 89.0f) pitch = 89.0f;
                if (pitch < -89.0f) pitch = -89.0f;
            }
            updateCameraVectors();
        } 
        else if (mode == CHASE_CAMERA) {
            chaseOrbitYaw += xoffset;
            chaseOrbitPitch = glm::clamp(chaseOrbitPitch + yoffset * 0.1f, -10.0f, 25.0f);
        }
    }

    void processMouseScroll(float yoffset) {
        if (mode == FREE_CAMERA) {
            zoom -= yoffset * 2.0f;
            if (zoom < 10.0f) zoom = 10.0f;
            if (zoom > 90.0f) zoom = 90.0f;
        } else {
            chaseDistance -= yoffset * 2.5f;
            if (chaseDistance < 6.0f) chaseDistance = 6.0f;
            if (chaseDistance > 120.0f) chaseDistance = 120.0f;
        }
    }

private:
    void updateCameraVectors() {
        glm::vec3 f;
        f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        f.y = sin(glm::radians(pitch));
        f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(f);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

#endif
