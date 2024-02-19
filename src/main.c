
#include <stdio.h>
#include "vulkanBackend/vulkanBackend.h"
#include "vulkanBackend/timeWindows.h"





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

    
    GLFWwindow** window; 
    initWindow(window);
    if(initVulkan(*window))
    {

        while (!glfwWindowShouldClose(*window)) {
            glfwPollEvents();
            drawFrame();
        }
    }
    waitIdle();
    cleanupWindow(window);

}