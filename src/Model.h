#ifndef MODEL_H
#define MODEL_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Model {
public:
    std::string name;
    GLuint VAO, VBO;
    size_t vertexCount;

    // Separate buffer for canopy/cockpit roof so it can be hidden in cockpit camera
    GLuint canopyVAO, canopyVBO;
    size_t canopyVertexCount;

    glm::vec3 minBounds;
    glm::vec3 maxBounds;
    glm::vec3 center;
    glm::vec3 size;

    glm::vec3 defaultColor;
    float shininess;

    Model();
    ~Model();

    bool loadOBJ(const std::string& filePath, const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f));
    void render(bool renderCanopy = true) const;

private:
    void setupBuffers(const std::vector<Vertex>& vertices, const std::vector<Vertex>& canopyVerts);
};

#endif
