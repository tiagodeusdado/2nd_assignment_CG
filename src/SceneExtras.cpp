#include "SceneExtras.h"
#include <iostream>

SceneExtras::SceneExtras() 
    : skyboxVAO(0), skyboxVBO(0), cubeVAO(0), cubeVBO(0), terrainVAO(0), terrainVBO(0) {}

SceneExtras::~SceneExtras() {
    if (skyboxVAO) glDeleteVertexArrays(1, &skyboxVAO);
    if (skyboxVBO) glDeleteBuffers(1, &skyboxVBO);
    if (cubeVAO) glDeleteVertexArrays(1, &cubeVAO);
    if (cubeVBO) glDeleteBuffers(1, &cubeVBO);
    if (terrainVAO) glDeleteVertexArrays(1, &terrainVAO);
    if (terrainVBO) glDeleteBuffers(1, &terrainVBO);
}

void SceneExtras::init() {
    setupSkybox();
    setupCube();
    setupTerrain();
    generateRunwayLights();
}

void SceneExtras::setupSkybox() {
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void SceneExtras::setupCube() {
    // 36 vertices with normals for rendering buildings and light markers
    float cubeVertices[] = {
        // positions          // normals
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Position (attr 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    // Normal (attr 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void SceneExtras::setupTerrain() {
    // Large ground plane spanning 6km x 10km around airport
    float halfW = 4000.0f;
    float halfL = 6000.0f;
    float y = -0.3f; // just below runway surface

    float terrainVertices[] = {
        // pos                      // normal
        -halfW, y, -halfL,          0.0f, 1.0f, 0.0f,
         halfW, y, -halfL,          0.0f, 1.0f, 0.0f,
         halfW, y,  halfL,          0.0f, 1.0f, 0.0f,
         halfW, y,  halfL,          0.0f, 1.0f, 0.0f,
        -halfW, y,  halfL,          0.0f, 1.0f, 0.0f,
        -halfW, y, -halfL,          0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void SceneExtras::generateRunwayLights() {
    runwayLightPositions.clear();
    runwayLightColors.clear();

    float edgeX = 31.0f; // Runway width is ~60m, lights at +/-31m

    // Runway Edge Lights: white, spaced every 50 meters
    for (float z = -3600.0f; z <= 3600.0f; z += 50.0f) {
        // Left side
        runwayLightPositions.emplace_back(-edgeX, 0.3f, z);
        runwayLightColors.emplace_back(1.0f, 0.98f, 0.85f); // bright warm white

        // Right side
        runwayLightPositions.emplace_back(edgeX, 0.3f, z);
        runwayLightColors.emplace_back(1.0f, 0.98f, 0.85f);
    }

    // Threshold lights at takeoff start (Z = -2800m): Green
    for (float x = -28.0f; x <= 28.0f; x += 3.5f) {
        runwayLightPositions.emplace_back(x, 0.3f, -2800.0f);
        runwayLightColors.emplace_back(0.1f, 1.0f, 0.3f); // vivid green
    }

    // Runway End lights at far end (Z = 3500m): Red
    for (float x = -28.0f; x <= 28.0f; x += 3.5f) {
        runwayLightPositions.emplace_back(x, 0.3f, 3500.0f);
        runwayLightColors.emplace_back(1.0f, 0.1f, 0.1f); // red
    }
}

void SceneExtras::renderSkybox(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDir) {
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void SceneExtras::renderPointLightMarker(const Shader& emissiveShader, const glm::vec3& lightPos, const glm::vec3& lightColor) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(4.0f)); // visible bright bulb
    emissiveShader.setMat4("model", model);
    emissiveShader.setVec3("lightColor", lightColor);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void SceneExtras::renderRunwayLights(const Shader& emissiveShader) {
    glBindVertexArray(cubeVAO);
    for (size_t i = 0; i < runwayLightPositions.size(); ++i) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, runwayLightPositions[i]);
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        emissiveShader.setMat4("model", model);
        emissiveShader.setVec3("lightColor", runwayLightColors[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

void SceneExtras::renderSurroundingTerrain(const Shader& phongShader) {
    // Rich countryside airfield grass color
    phongShader.setVec3("objectColor", glm::vec3(0.18f, 0.28f, 0.15f));
    phongShader.setVec3("specularColor", glm::vec3(0.02f));
    phongShader.setFloat("shininess", 4.0f);

    glm::mat4 model = glm::mat4(1.0f);
    phongShader.setMat4("model", model);
    glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(model)));
    phongShader.setMat3("normalMatrix", normMat);

    glBindVertexArray(terrainVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SceneExtras::renderAirportBuildings(const Shader& phongShader) {
    glBindVertexArray(cubeVAO);

    // 1. Control Tower Base / Shaft (Concrete grey) - positioned beside mid-runway
    {
        phongShader.setVec3("objectColor", glm::vec3(0.72f, 0.72f, 0.75f));
        phongShader.setVec3("specularColor", glm::vec3(0.2f));
        phongShader.setFloat("shininess", 16.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-100.0f, 20.0f, -1200.0f));
        model = glm::scale(model, glm::vec3(16.0f, 40.0f, 16.0f));
        phongShader.setMat4("model", model);
        phongShader.setMat3("normalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 2. Control Tower Observation Cabin (Glass cyan tint)
    {
        phongShader.setVec3("objectColor", glm::vec3(0.35f, 0.55f, 0.65f));
        phongShader.setVec3("specularColor", glm::vec3(0.8f));
        phongShader.setFloat("shininess", 64.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-100.0f, 44.0f, -1200.0f));
        model = glm::scale(model, glm::vec3(24.0f, 8.0f, 24.0f));
        phongShader.setMat4("model", model);
        phongShader.setMat3("normalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 3. Control Tower Roof & Radar Mount
    {
        phongShader.setVec3("objectColor", glm::vec3(0.2f, 0.2f, 0.22f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-100.0f, 49.0f, -1200.0f));
        model = glm::scale(model, glm::vec3(12.0f, 2.0f, 12.0f));
        phongShader.setMat4("model", model);
        phongShader.setMat3("normalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 4. Aircraft Hangars (Corrugated metal arches) - well set back behind the tower
    for (int h = 0; h < 3; ++h) {
        float hZ = -900.0f + h * 120.0f;
        phongShader.setVec3("objectColor", glm::vec3(0.55f, 0.58f, 0.62f));
        phongShader.setVec3("specularColor", glm::vec3(0.3f));
        phongShader.setFloat("shininess", 24.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-180.0f, 12.0f, hZ));
        model = glm::scale(model, glm::vec3(60.0f, 24.0f, 80.0f));
        phongShader.setMat4("model", model);
        phongShader.setMat3("normalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 5. Airport Passenger Terminal Building - set back away from runway
    {
        phongShader.setVec3("objectColor", glm::vec3(0.85f, 0.85f, 0.88f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-250.0f, 9.0f, -300.0f));
        model = glm::scale(model, glm::vec3(80.0f, 18.0f, 220.0f));
        phongShader.setMat4("model", model);
        phongShader.setMat3("normalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);
}
