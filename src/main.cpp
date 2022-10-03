#include "phosphene.hpp"
#include <iostream>

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow *win = glfwCreateWindow(800, 800, "Phosphene", NULL, NULL);

  PhosHelper::infoInstance();
  Phosphene phosphene(win);

  while(!glfwWindowShouldClose(win)) {
    glfwPollEvents();
  }

  phosphene.destroy();
  glfwDestroyWindow(win);
  glfwTerminate();

  return (0);
}
