#include "FlightSimulator.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FlightSimulator::FlightSimulator() {
    // Wheels touch runway at Y=0 when aircraft model origin is at Y=0.65m
    groundLevel = 0.65f; 
    
    // Start at runway threshold (Z = -2600m), facing forward (+Z direction)
    runwayStartPos = glm::vec3(0.0f, groundLevel, -2600.0f);
    
    // TOLD speeds for F-104 (converted to m/s: 1 knot = 0.514444 m/s)
    v1_speed = 72.0f;   // 140 knots
    vr_speed = 85.0f;   // 165 knots
    vlof_speed = 95.0f; // 185 knots
    v2_speed = 108.0f;  // 210 knots

    reset();
}

void FlightSimulator::reset() {
    position = runwayStartPos;
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;

    speed = 0.0f;
    acceleration = 0.0f;
    throttle = 0.0f;

    phase = PHASE_READY;
    isAutoPilot = true;
    isRunning = false;
    simulationTime = 0.0f;
    lastAltitude = 0.0f;
    verticalSpeedSmoothed = 0.0f;
}

void FlightSimulator::startTakeoff() {
    isRunning = true;
    if (phase == PHASE_READY) {
        phase = PHASE_GROUND_ROLL;
        throttle = 1.0f; // Full afterburner for takeoff
    }
}

void FlightSimulator::togglePause() {
    isRunning = !isRunning;
}

void FlightSimulator::toggleControlMode() {
    isAutoPilot = !isAutoPilot;
}

void FlightSimulator::adjustThrottle(float delta) {
    throttle = std::max(0.0f, std::min(1.0f, throttle + delta));
}

void FlightSimulator::adjustPitch(float delta) {
    pitch = std::max(-10.0f, std::min(30.0f, pitch + delta));
}

void FlightSimulator::adjustYaw(float delta) {
    yaw += delta;
}

void FlightSimulator::adjustRoll(float delta) {
    roll = std::max(-45.0f, std::min(45.0f, roll + delta));
}

void FlightSimulator::update(float deltaTime) {
    if (!isRunning) return;

    simulationTime += deltaTime;

    // Aircraft engine thrust & drag physics
    float maxThrustAcc = 7.5f; // m/s^2 (F-104 J79 with afterburner produces immense thrust-to-weight)
    float rollingFriction = 0.02f * 9.81f; // Wheel friction on runway
    float aeroDragCoeff = 0.00035f;

    if (isAutoPilot) {
        // Guided Takeoff Procedure (TOLD standards)
        if (phase == PHASE_READY) {
            throttle = 1.0f;
            phase = PHASE_GROUND_ROLL;
        }

        if (phase == PHASE_GROUND_ROLL) {
            throttle = 1.0f;
            pitch = 0.0f;
            roll = 0.0f;
            yaw = 0.0f;

            if (speed >= vr_speed) {
                phase = PHASE_ROTATION;
            }
        }
        else if (phase == PHASE_ROTATION) {
            // Smoothly rotate nose up to 12 degrees
            pitch = std::min(12.5f, pitch + 5.5f * deltaTime);

            if (speed >= vlof_speed) {
                phase = PHASE_LIFTOFF;
            }
        }
        else if (phase == PHASE_LIFTOFF) {
            // Wheels leave the ground, pitch increases to 14 degrees
            pitch = std::min(14.0f, pitch + 3.0f * deltaTime);

            if (getAltitudeFeet() > 35.0f) {
                phase = PHASE_CLIMB;
            }
        }
        else if (phase == PHASE_CLIMB) {
            // Steady climb out, maintain 15 degrees pitch
            pitch = 14.5f;
            // Slight bank / stabilization
            roll = std::sin(simulationTime * 0.5f) * 1.5f;
        }
    }

    // Engine acceleration
    float thrustAcc = throttle * maxThrustAcc;
    float dragAcc = aeroDragCoeff * (speed * speed);
    float frictionAcc = (position.y <= groundLevel + 0.05f) ? rollingFriction : 0.0f;

    acceleration = thrustAcc - dragAcc - frictionAcc;
    speed = std::max(0.0f, speed + acceleration * deltaTime);

    // Compute velocity vector based on current pitch and yaw
    float pitchRad = glm::radians(pitch);
    float yawRad = glm::radians(yaw);

    glm::vec3 forwardDir;
    forwardDir.x = std::sin(yawRad) * std::cos(pitchRad);
    forwardDir.y = std::sin(pitchRad);
    forwardDir.z = std::cos(yawRad) * std::cos(pitchRad);
    forwardDir = glm::normalize(forwardDir);

    // Move forward
    position += forwardDir * speed * deltaTime;

    // Ground constraint
    if (position.y < groundLevel) {
        position.y = groundLevel;
    }

    // Measure vertical speed
    float currentAlt = getAltitudeFeet();
    float instantVS = (currentAlt - lastAltitude) / (deltaTime > 0.0001f ? deltaTime : 0.016f) * 60.0f; // ft/min
    verticalSpeedSmoothed = glm::mix(verticalSpeedSmoothed, instantVS, 0.1f);
    lastAltitude = currentAlt;
}

float FlightSimulator::getVerticalSpeedFPM() const {
    return verticalSpeedSmoothed;
}

std::string FlightSimulator::getPhaseString() const {
    switch (phase) {
        case PHASE_READY: return "READY (TAXIWAY / HOLD)";
        case PHASE_GROUND_ROLL: return "TAKEOFF ROLL (SPOOL UP)";
        case PHASE_ROTATION: return "ROTATION (Vr REACHED)";
        case PHASE_LIFTOFF: return "LIFTOFF (AIRBORNE)";
        case PHASE_CLIMB: return "INITIAL CLIMB (POSITIVE RATE)";
        default: return "FLYING";
    }
}

glm::mat4 FlightSimulator::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    // Rotate yaw (around Y)
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    // Rotate pitch (around X)
    // Note: F-104 aircraft nose points towards +Z.
    // By the right-hand rule, rotation around +X moves +Z towards -Y (nose down).
    // Therefore, to pitch nose UP during takeoff/climb, we rotate by -pitch around +X!
    model = glm::rotate(model, glm::radians(-pitch), glm::vec3(1.0f, 0.0f, 0.0f));

    // Rotate roll (around Z)
    model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));

    return model;
}

glm::vec3 FlightSimulator::getForwardVector() const {
    float pitchRad = glm::radians(pitch);
    float yawRad = glm::radians(yaw);
    glm::vec3 f;
    f.x = std::sin(yawRad) * std::cos(pitchRad);
    f.y = std::sin(pitchRad);
    f.z = std::cos(yawRad) * std::cos(pitchRad);
    return glm::normalize(f);
}

glm::vec3 FlightSimulator::getUpVector() const {
    glm::mat4 rot = glm::mat4(1.0f);
    rot = glm::rotate(rot, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    rot = glm::rotate(rot, glm::radians(-pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    rot = glm::rotate(rot, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::normalize(glm::vec3(rot * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
}
