#include "../shaders/hostDevice.h"

class Camera {
  public:
    Camera();

    glm::vec3 position;
    glm::mat4 projection;
    glm::mat4 transform;
};
