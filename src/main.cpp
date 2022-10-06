#include "phosphene.hpp"
#include <iostream>

static void callbackWindowSize(GLFWwindow* window, int width, int height) {
  Phosphene *phosphenePtr = static_cast<Phosphene*>(glfwGetWindowUserPointer(window));
  phosphenePtr->callbackWindowResize(width, height);
}

static void callbackKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
  Phosphene *phosphenePtr = static_cast<Phosphene*>(glfwGetWindowUserPointer(window));
  phosphenePtr->callbackKey(key, scancode, action, mods);
}

static void callbackCursor(GLFWwindow* window, double x_pos, double y_pos) {
  Phosphene *phosphenePtr = static_cast<Phosphene*>(glfwGetWindowUserPointer(window));
  phosphenePtr->callbackCursor(x_pos, y_pos);
}

static void callbackMouseButton(GLFWwindow* window, int button, int action, int mod) {
  Phosphene *phosphenePtr = static_cast<Phosphene*>(glfwGetWindowUserPointer(window));
  phosphenePtr->callbackMouseButton(button, action, mod);
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow *window = glfwCreateWindow(800, 800, "Phosphene", NULL, NULL);

  PhosHelper::infoInstance();
  Phosphene phosphene(window);

  { // GLFW Callback
    if (glfwRawMouseMotionSupported())
      glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetWindowUserPointer(window, &phosphene);
    glfwSetWindowSizeCallback(window, callbackWindowSize);
    glfwSetKeyCallback(window, callbackKey);
    glfwSetCursorPosCallback(window, callbackCursor);
    glfwSetMouseButtonCallback(window, callbackMouseButton);
  }

  phosphene.renderLoop();

  phosphene.destroy();
  glfwDestroyWindow(window);
  glfwTerminate();

  return (0);
}
