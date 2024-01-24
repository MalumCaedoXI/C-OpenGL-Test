#define GLFW_INCLUDE_NONE
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <Windows.h>


void error_callback(int error, const char* description)
{
    printf("GLFW Error: %s\n", description);
}

int main()
{
    if (!glfwInit())
    {
        printf("GLFW Init failed!\n");
    }
    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello Screen!", NULL, NULL);
    if (!window)
    {
        printf("Window Creation failed!\n");
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    

    while (!glfwWindowShouldClose(window))
    {
        printf("Hello there!\n");
        Sleep(100);
    }

    glfwTerminate();
}

