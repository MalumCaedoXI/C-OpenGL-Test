#include "vertex.h"

VkVertexInputBindingDescription vertexGetBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

VkVertexInputAttributeDescription vertexGetPositionAttributeDescrition()
{
    VkVertexInputAttributeDescription attributeDescription = {};
    attributeDescription.binding = 0;
    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescription.offset = offsetof(Vertex, pos);

    return attributeDescription;
}

VkVertexInputAttributeDescription vertexGetColorAttributeDescrition()
{
    VkVertexInputAttributeDescription attributeDescription = {};
    attributeDescription.binding = 0;
    attributeDescription.location = 1;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = offsetof(Vertex, col);
    
    return attributeDescription;
}


void normalizeVector3(float vector[3])
{
    float normalizationFactor = sqrtf(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

    for (int i = 0; i < 3; i++)
    {
        vector[i] /= normalizationFactor;
    }
}

void rotateMatrix(float inMatrix[4][4], float angle, float vector[3])
{
    normalizeVector3(vector);
    float angleInRad = (angle * (3.14159265358979323846/180.0)) / 2.0;
    float sinAngle = sin(angleInRad);
    float quaternion[4] = {cos(angleInRad), sinAngle * vector[0], sinAngle * vector[1], sinAngle * vector[2]};
    float quaternionUnit[4];
    float normalizationFactor = sqrtf(quaternion[0] * quaternion[0] + quaternion[1] * quaternion[1] + quaternion[2] * quaternion[2] + quaternion[3] * quaternion[3]);
    for (int i = 0; i < 4; i++)
    {
        quaternionUnit[i] = quaternion[i]/normalizationFactor;
    }

    //Warning: Maths ahead!
    float rotationMatrix[4][4] = {{quaternionUnit[0] * quaternionUnit[0] + quaternionUnit[1] * quaternionUnit[1] - (quaternionUnit[2] * quaternionUnit[2]) - (quaternionUnit[3] * quaternionUnit[3]), (2.0f * quaternionUnit[1] * quaternionUnit[2]) - (2.0f * quaternionUnit[0] * quaternionUnit[3]), (2.0f * quaternionUnit[1] * quaternionUnit[3]) + (2.0f * quaternionUnit[0] * quaternionUnit[2]), 0 },
    {(2.0f * quaternionUnit[1] * quaternionUnit[2]) + (2.0f * quaternionUnit[0] * quaternionUnit[3]), quaternionUnit[0] * quaternionUnit[0] + quaternionUnit[2] * quaternionUnit[2] - (quaternionUnit[1] * quaternionUnit[1]) - (quaternionUnit[3] * quaternionUnit[3]), (2.0f * quaternionUnit[2] * quaternionUnit[3]) - (2.0f * quaternionUnit[0] * quaternionUnit[1]), 0},
    {(2.0f * quaternionUnit[1] * quaternionUnit[3]) - (2.0f * quaternionUnit[0] * quaternionUnit[2]), (2.0f * quaternionUnit[2] * quaternionUnit[3]) + (2.0f * quaternionUnit[0] * quaternionUnit[1]), quaternionUnit[0] * quaternionUnit[0] + quaternionUnit[3] * quaternionUnit[3] - (quaternionUnit[1] * quaternionUnit[1]) - (quaternionUnit[2] * quaternionUnit[2])},
    {0.0f, 0.0f, 0.0f, 1.0f}};


    float result[4][4] = {0};
    for(int i = 0; i < 4; i++) 
	{
        for(int j = 0; j < 4; j++) 
		{
            for(int k = 0; k < 4; k++) 
			{
                result[i][j] += inMatrix[i][k] * rotationMatrix[k][j];
            }
        }
    }

    for(int i = 0; i < 4; i++) 
	{
        for(int j = 0; j < 4; j++) 
		{
            inMatrix[i][j] = result[i][j];
        }
    }
}
