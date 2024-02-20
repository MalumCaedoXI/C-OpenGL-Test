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


void cross(const float a[3], const float b[3], float result[3])//Thanks buddy!
{
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

float dot(const float a[3], const float b[3])
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void lookAt(float mat[4][4], float eye[3], float center[3], float up[3])
{
    float f[3] = {center[0] - eye[0], center[1] - eye[1], center[2] - eye[2]};
    normalizeVector3(f);

    normalizeVector3(up);
    float s[3];
    cross(f, up, s);
    normalizeVector3(s);

    float u[3];
    cross(s, f, u);

    memset(mat, 0, sizeof(float) * 16);
    mat[0][0] = s[0];
    mat[1][0] = s[1];
    mat[2][0] = s[2];

    mat[0][1] = u[0];
    mat[1][1] = u[1];
    mat[2][1] = u[2];

    mat[0][2] = -f[0];
    mat[1][2] = -f[1];
    mat[2][2] = -f[2];

    mat[3][0] = -dot(s, eye);
    mat[3][1] = -dot(u, eye);
    mat[3][2] = dot(f, eye);
    mat[3][3] = 1.0f;
}

void translate(float mat[4][4], float x, float y, float z)
{
    // Initialize the matrix to identity matrix
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mat[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    // Set the translation components
    mat[3][0] = x;
    mat[3][1] = y;
    mat[3][2] = z;
}

void perspective(float matrix[4][4], float fovDegrees, float aspect, float near, float far)
{
    float tanHalfFov = (float)tan(fovDegrees / 2.0f * (3.14159265358979323846 / 180.0f));

    // Initialize matrix to identity
    for (int i = 0; i < 4; ++i)
	{
        for (int j = 0; j < 4; ++j)
		{
            matrix[i][j] = 0.0f;
        }
    }

    matrix[0][0] = 1.0f / (aspect * tanHalfFov);
    matrix[1][1] = 1.0f / tanHalfFov;
	matrix[2][2] = (far + near) / (near - far);

    matrix[2][3] = -1.0f;
	matrix[3][2] = (2 * far * near) / (near - far);
	matrix[3][3] = 1.0f;  // Set the homogenous coordinate to 1

}

void oldRotateMatrix(float mat[4][4], char axis, float angle) 
{
    float rotationMatrix[4][4] = 
	{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    
    float c = cosf(angle);
    float s = sinf(angle);

    switch(axis) 
	{
        case 'x':
        case 'X':
            rotationMatrix[1][1] = c;
            rotationMatrix[1][2] = -s;
            rotationMatrix[2][1] = s;
            rotationMatrix[2][2] = c;
            break;
        case 'y':
        case 'Y':
            rotationMatrix[0][0] = c;
            rotationMatrix[0][2] = s;
            rotationMatrix[2][0] = -s;
            rotationMatrix[2][2] = c;
            break;
        case 'z':
        case 'Z':
            rotationMatrix[0][0] = c;
            rotationMatrix[0][1] = -s;
            rotationMatrix[1][0] = s;
            rotationMatrix[1][1] = c;
            break;
        default:
            // Invalid axis
            return;
    }

    float result[4][4] = {0};
    for(int i = 0; i < 4; i++) 
	{
        for(int j = 0; j < 4; j++) 
		{
            for(int k = 0; k < 4; k++) 
			{
                result[i][j] += mat[i][k] * rotationMatrix[k][j];
            }
        }
    }

    // Copying the result back into the input matrix
    for(int i = 0; i < 4; i++) 
	{
        for(int j = 0; j < 4; j++) 
		{
            mat[i][j] = result[i][j];
        }
    }
}