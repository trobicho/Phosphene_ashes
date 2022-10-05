#include "../shaders/hostDevice.h"

class Camera {
  public:
    Camera();

    glm::vec3 m_position{0.f, 0.f, 0.f};
    glm::vec3 m_forward{0.f, 0.f, 1.f};
    glm::vec3 m_up{0.f, -1.f, 0.f};
    glm::vec3 m_right{1.f, 0.f, 0.f};
    glm::mat4 m_projection;
    glm::mat4 m_view;
};
