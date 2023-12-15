#pragma once
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

GLFWwindow* CreateGLFWWindow(int w, int h, const char* title)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_SAMPLES, 4);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	return window;
}
