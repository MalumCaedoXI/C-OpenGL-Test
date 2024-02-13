#include <stdio.h>
#include "vulkanBackend/vulkanBackend.h"




int main() {
    GLFWwindow** window; 
    initWindow(window);
    initVulkan();

    while (!glfwWindowShouldClose(*window)) {
        glfwPollEvents();
    }

    
    cleanupWindow(window);
}