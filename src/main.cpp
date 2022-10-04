#include "phosphene.hpp"
#include <iostream>

static void callbackWindowSize(GLFWwindow* window, int width, int height) {
  Phosphene *phosphenePtr = static_cast<Phosphene*>(glfwGetWindowUserPointer(window));
  phosphenePtr->callbackWindowResize(width, height);
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow *window = glfwCreateWindow(800, 800, "Phosphene", NULL, NULL);

  PhosHelper::infoInstance();
  Phosphene phosphene(window);

  { // GLFW Callback
    glfwSetWindowUserPointer(window, &phosphene);
    glfwSetWindowSizeCallback(window, callbackWindowSize);
  }

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  phosphene.destroy();
  glfwDestroyWindow(window);
  glfwTerminate();

  return (0);
}
