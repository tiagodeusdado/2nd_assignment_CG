#ifndef SCENE_EXTRAS_H
#define SCENE_EXTRAS_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Shader.h"

class SceneExtras {
public:
    SceneExtras();
    ~SceneExtras();

    void init();
    void renderSkybox(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDir);
    void renderPointLightMarker(const Shader& emissiveShader, const glm::vec3& lightPos, const glm::vec3& lightColor);
    void renderRunwayLights(const Shader& emissiveShader);
    void renderAirportBuildings(const Shader& phongShader);
    void renderSurroundingTerrain(const Shader& phongShader);

private:
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubeVAO, cubeVBO;
    GLuint terrainVAO, terrainVBO;
    
    std::vector<glm::vec3> runwayLightPositions;
    std::vector<glm::vec3> runwayLightColors;

    void setupSkybox();
    void setupCube();
    void setupTerrain();
    void generateRunwayLights();
};

#endif
