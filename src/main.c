#include <stdio.h>
#include "vulkanBackend/vulkanBackend.h"




int main() {
    GLFWwindow** window; 
    initWindow(window);
    initVulkan(*window);

    while (!glfwWindowShouldClose(*window)) {
        glfwPollEvents();
        drawFrame();
    }

    waitIdle();
    cleanupWindow(window);
}