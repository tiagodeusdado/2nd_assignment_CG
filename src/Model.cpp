#include "Model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

Model::Model() 
    : VAO(0), VBO(0), vertexCount(0),
      canopyVAO(0), canopyVBO(0), canopyVertexCount(0),
      minBounds(glm::vec3(1e9f)), maxBounds(glm::vec3(-1e9f)),
      center(glm::vec3(0.0f)), size(glm::vec3(0.0f)),
      defaultColor(glm::vec3(0.7f)), shininess(32.0f) {}

Model::~Model() {
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (canopyVBO != 0) glDeleteBuffers(1, &canopyVBO);
    if (canopyVAO != 0) glDeleteVertexArrays(1, &canopyVAO);
}

struct OBJIndex {
    int v;
    int vt;
    int vn;
};

static inline OBJIndex parseOBJIndex(const std::string& token, int vCount, int vtCount, int vnCount) {
    OBJIndex idx = {0, 0, 0};
    size_t firstSlash = token.find('/');
    if (firstSlash == std::string::npos) {
        idx.v = std::stoi(token);
    } else {
        idx.v = std::stoi(token.substr(0, firstSlash));
        size_t secondSlash = token.find('/', firstSlash + 1);
        if (secondSlash == std::string::npos) {
            std::string vtStr = token.substr(firstSlash + 1);
            if (!vtStr.empty()) idx.vt = std::stoi(vtStr);
        } else {
            std::string vtStr = token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
            if (!vtStr.empty()) idx.vt = std::stoi(vtStr);
            std::string vnStr = token.substr(secondSlash + 1);
            if (!vnStr.empty()) idx.vn = std::stoi(vnStr);
        }
    }

    if (idx.v < 0) idx.v = vCount + idx.v + 1;
    if (idx.vt < 0) idx.vt = vtCount + idx.vt + 1;
    if (idx.vn < 0) idx.vn = vnCount + idx.vn + 1;

    return idx;
}

bool Model::loadOBJ(const std::string& filePath, const glm::vec3& color) {
    name = filePath;
    defaultColor = color;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Model ERROR] Could not open OBJ file: " << filePath << std::endl;
        return false;
    }

    std::cout << "[Model] Loading " << filePath << " ... " << std::flush;

    std::vector<glm::vec3> tempPositions;
    std::vector<glm::vec3> tempNormals;
    std::vector<glm::vec2> tempTexCoords;

    std::vector<Vertex> finalVertices;
    std::vector<Vertex> canopyVertices;

    tempPositions.reserve(120000);
    tempNormals.reserve(120000);
    finalVertices.reserve(320000);

    minBounds = glm::vec3(1e9f);
    maxBounds = glm::vec3(-1e9f);

    std::string currentObject = "default";
    bool isCanopyObject = false;

    std::string line;
    line.reserve(256);

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) continue;

        // Aceita tanto 'o' (object) como 'g' (group) do ficheiro .obj
        if ((line[start] == 'o' || line[start] == 'g') && line[start + 1] == ' ') {
            currentObject = line.substr(start + 2);
            
            // ---> ADICIONA ESTA LINHA DE DEBUG <---
            std::cout << "\n[OBJ Debug] Encontrado objeto/grupo: " << currentObject << std::flush;

            std::string lowerObj = currentObject;
            std::transform(lowerObj.begin(), lowerObj.end(), lowerObj.begin(), ::tolower);
            
            isCanopyObject = (lowerObj.find("canopy") != std::string::npos || 
                              lowerObj.find("windshield") != std::string::npos ||
                              lowerObj.find("glass") != std::string::npos ||
                              lowerObj.find("027") != std::string::npos ||
                              lowerObj.find("050") != std::string::npos ||
                              lowerObj.find("window") != std::string::npos);
        }
        else if (line[start] == 'v' && line[start + 1] == ' ') {
            float x, y, z;
            if (sscanf(line.c_str() + start + 2, "%f %f %f", &x, &y, &z) == 3) {
                tempPositions.emplace_back(x, y, z);
                minBounds = glm::min(minBounds, glm::vec3(x, y, z));
                maxBounds = glm::max(maxBounds, glm::vec3(x, y, z));
            }
        }
        else if (line[start] == 'v' && line[start + 1] == 'n') {
            float x, y, z;
            if (sscanf(line.c_str() + start + 3, "%f %f %f", &x, &y, &z) == 3) {
                tempNormals.emplace_back(x, y, z);
            }
        }
        else if (line[start] == 'v' && line[start + 1] == 't') {
            float u, v;
            if (sscanf(line.c_str() + start + 3, "%f %f", &u, &v) == 2) {
                tempTexCoords.emplace_back(u, v);
            }
        }
        else if (line[start] == 'f' && line[start + 1] == ' ') {
            std::stringstream ss(line.substr(start + 2));
            std::string token;
            std::vector<OBJIndex> faceIndices;
            while (ss >> token) {
                faceIndices.push_back(parseOBJIndex(token, 
                                                    (int)tempPositions.size(), 
                                                    (int)tempTexCoords.size(), 
                                                    (int)tempNormals.size()));
            }

            if (faceIndices.size() < 3) continue;

            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                OBJIndex idx[3] = { faceIndices[0], faceIndices[i], faceIndices[i + 1] };

                glm::vec3 faceNormal(0.0f, 1.0f, 0.0f);
                bool needAutoNormal = (idx[0].vn == 0 || idx[1].vn == 0 || idx[2].vn == 0);

                if (needAutoNormal) {
                    glm::vec3 p0 = tempPositions[idx[0].v - 1];
                    glm::vec3 p1 = tempPositions[idx[1].v - 1];
                    glm::vec3 p2 = tempPositions[idx[2].v - 1];
                    faceNormal = glm::cross(p1 - p0, p2 - p0);
                    float len = glm::length(faceNormal);
                    if (len > 1e-6f) faceNormal /= len;
                    else faceNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                for (int k = 0; k < 3; ++k) {
                    Vertex vert;
                    if (idx[k].v > 0 && idx[k].v <= (int)tempPositions.size()) {
                        vert.position = tempPositions[idx[k].v - 1];
                    } else {
                        vert.position = glm::vec3(0.0f);
                    }

                    if (idx[k].vn > 0 && idx[k].vn <= (int)tempNormals.size()) {
                        vert.normal = tempNormals[idx[k].vn - 1];
                    } else {
                        vert.normal = faceNormal;
                    }

                    if (idx[k].vt > 0 && idx[k].vt <= (int)tempTexCoords.size()) {
                        vert.texCoords = tempTexCoords[idx[k].vt - 1];
                    } else {
                        vert.texCoords = glm::vec2(0.0f);
                    }

                    if (isCanopyObject) {
                        canopyVertices.push_back(vert);
                    } else {
                        finalVertices.push_back(vert);
                    }
                }
            }
        }
    }

    file.close();

    center = (minBounds + maxBounds) * 0.5f;
    size = maxBounds - minBounds;

    setupBuffers(finalVertices, canopyVertices);

    std::cout << "Done! Loaded " << finalVertices.size() << " main vertices, " 
              << canopyVertices.size() << " canopy vertices." << std::endl;

    return true;
}

void Model::setupBuffers(const std::vector<Vertex>& vertices, const std::vector<Vertex>& canopyVerts) {
    vertexCount = vertices.size();
    if (vertexCount > 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);
    }

    canopyVertexCount = canopyVerts.size();
    if (canopyVertexCount > 0) {
        glGenVertexArrays(1, &canopyVAO);
        glGenBuffers(1, &canopyVBO);

        glBindVertexArray(canopyVAO);
        glBindBuffer(GL_ARRAY_BUFFER, canopyVBO);
        glBufferData(GL_ARRAY_BUFFER, canopyVerts.size() * sizeof(Vertex), canopyVerts.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);
    }
}

void Model::render(bool renderCanopy) const {
    if (VAO != 0 && vertexCount > 0) {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertexCount);
        glBindVertexArray(0);
    }
    if (renderCanopy && canopyVAO != 0 && canopyVertexCount > 0) {
        glBindVertexArray(canopyVAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)canopyVertexCount);
        glBindVertexArray(0);
    }
}  