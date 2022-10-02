#include "vkImpl.hpp"
#include "command.hpp"
#include "phosStartVk.hpp"
#include <iostream>

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow *win = glfwCreateWindow(800, 800, "Phosphene", NULL, NULL);

  PhosHelper::infoInstance();

  while(!glfwWindowShouldClose(win)) {
    glfwPollEvents();
  }

  glfwDestroyWindow(win);
  glfwTerminate();

  return (0);
}
