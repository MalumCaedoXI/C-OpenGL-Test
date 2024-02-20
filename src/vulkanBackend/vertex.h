#include <vulkan/vulkan.h>
#include <math.h>
#include <string.h>

typedef float mat4[4][4];

typedef struct UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct VertexPosition{
    float x;
    float y;
} VertexPosition;

typedef struct VertexColor{
    float r;
    float g;
    float b;
} VertexColor;

typedef struct Vertex{
    VertexPosition pos;
    VertexColor col;
} Vertex;


VkVertexInputBindingDescription vertexGetBindingDescription();
VkVertexInputAttributeDescription vertexGetPositionAttributeDescrition();
VkVertexInputAttributeDescription vertexGetColorAttributeDescrition();
void rotateMatrix(float inMatrix[4][4], float angle, float vector[3]);
void lookAt(float mat[4][4], float eye[3], float center[3], float up[3]);
void perspective(float matrix[4][4], float fovDegrees, float aspect, float near, float far);
void oldRotateMatrix(float mat[4][4], char axis, float angle);