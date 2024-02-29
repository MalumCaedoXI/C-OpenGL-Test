
#include <stdio.h>
#include "vulkanBackend/vulkanBackend.h"
#include "vulkanBackend/timeWindows.h"

GLFWwindow* window;

int initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "Vulkan", NULL, NULL);
}

int main() {
    printf("Current Time is: %f\n", getTimeMSFloat());
    mat4 testMatrix = {{1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f}};
    float vec[3] = {0.0f, 1.0f, 1.0f};
    rotateMatrix(testMatrix, 90.0f, vec);

    for (int i = 0; i < 4; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            printf("X: %i, Y: %i. Value: %f\n",k, i, testMatrix[k][i]);
        }
    }

    
    
    initWindow();
    setupWindow(window);
    if(initVulkan(window))
    {

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
    }
    waitIdle();
    cleanupWindow(window);

}