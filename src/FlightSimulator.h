#ifndef FLIGHT_SIMULATOR_H
#define FLIGHT_SIMULATOR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

enum FlightPhase {
    PHASE_READY = 0,
    PHASE_GROUND_ROLL = 1,
    PHASE_ROTATION = 2,
    PHASE_LIFTOFF = 3,
    PHASE_CLIMB = 4
};

class FlightSimulator {
public:
    // Position and Orientation
    glm::vec3 position;
    float pitch;    // degrees (nose up/down)
    float yaw;      // degrees (heading)
    float roll;     // degrees (bank)

    // Aircraft dynamics
    float speed;            // m/s
    float acceleration;     // m/s^2
    float throttle;         // 0.0 to 1.0 (0% - 100%)
    float groundLevel;      // Y height of runway wheels contact

    // Takeoff Performance Data (TOLD) for F-104 Starfighter
    float v1_speed;         // Decision speed (~140 knots = 72 m/s)
    float vr_speed;         // Rotation speed (~165 knots = 85 m/s)
    float vlof_speed;       // Liftoff speed (~185 knots = 95 m/s)
    float v2_speed;         // Climb speed (~210 knots = 108 m/s)

    // Simulation state
    FlightPhase phase;
    bool isAutoPilot;
    bool isRunning;
    float simulationTime;

    // Runway start coordinates
    glm::vec3 runwayStartPos;

    FlightSimulator();

    void reset();
    void update(float deltaTime);

    // Controls
    void startTakeoff();
    void togglePause();
    void toggleControlMode();
    void adjustThrottle(float delta);
    void adjustPitch(float delta);
    void adjustYaw(float delta);
    void adjustRoll(float delta);

    // Avionics / Telemetry getters
    float getAirspeedKnots() const { return speed * 1.94384f; }
    float getAltitudeFeet() const { return (position.y - groundLevel) * 3.28084f; }
    float getVerticalSpeedFPM() const;
    std::string getPhaseString() const;
    glm::mat4 getModelMatrix() const;
    glm::vec3 getForwardVector() const;
    glm::vec3 getUpVector() const;

private:
    float lastAltitude;
    float verticalSpeedSmoothed;
};

#endif
