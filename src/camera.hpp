#include "../shaders/hostDevice.h"

#define CAMERA_EVENT_ASPECT_CHANGE                0b000001

#define CAMERA_KEY_STATE_RIGHT                    0b000001
#define CAMERA_KEY_STATE_LEFT                     0b000010
#define CAMERA_KEY_STATE_FORWARD                  0b000100
#define CAMERA_KEY_STATE_BACKWARD                 0b001000
#define CAMERA_KEY_STATE_ROLL_CLOCKWISE           0b010000 
#define CAMERA_KEY_STATE_ROLL_COUNTER_CLOCKWISE   0b100000


class Camera {
  public:
    Camera();

    void  step();
    void  rotate(double x, double y);
    bool  buildGlobalUniform(GlobalUniforms &uniform);
    void  forceBuildGlobalUniform(GlobalUniforms &uniform);
    bool  asUpdate() {return (m_update);}

    void  setPosition(glm::vec3 position) {
      m_position = position;
      m_update = true;
    }
    void  resetKeyState() {m_keyState = 0;}
    void  setKeyState(uint32_t state) {m_keyState |= state;}
    void  unsetKeyState(uint32_t state) {m_keyState &= ((~state) & 0b111111);}

    void  setAllowMoving(bool allow) {m_allowMoving = allow;}
    void  setAllowRotation(bool allow) {
      m_allowRotation = allow;
      m_lastX = 0.5;
      m_lastY = 0.5;
    }
    void  setAllowRotation(bool allow, double lastX, double lastY) {
      m_allowRotation = allow;
      m_lastX = lastX;
      m_lastY = lastY;
    }

    void  setFov(float degree) {
      m_fovY = glm::radians(degree);
      m_update = true;
    }
    void  eventChangeAspectRatio(uint32_t width, uint32_t height) {
      m_newAspectRatio = (float)width / (float)height;
      m_event |= CAMERA_EVENT_ASPECT_CHANGE;
    }
    void  setAspectRatio(uint32_t width, uint32_t height) {
      m_aspectRatio = (float)width / (float)height;
      m_update = true;
    }
    void  setFarNearPlane(float zNear, float zFar) {
      m_zNear = zNear;
      m_zFar = zFar;
      m_update = true;
    }

    bool  m_allowRotation = false;
    bool  m_allowMoving = false;

  private:
    glm::vec3 m_position{0.f, 0.f, 0.f};
    glm::vec3 m_forward{0.f, 0.f, 1.f};
    glm::vec3 m_up{0.f, 1.f, 0.f};
    glm::vec3 m_right{-1.f, 0.f, 0.f};
    glm::mat4 m_projection;
    glm::mat4 m_view;

    uint32_t  m_event = 0;
    uint32_t  m_keyState = 0;
    float     m_speed = 0.05;

    double    m_lastX = 0.0;
    double    m_lastY = 0.0;

    float     m_fovY = glm::radians(45.f);
    float     m_aspectRatio = 1.f;
    float     m_newAspectRatio = 1.f;
    float     m_zNear = 0.1f;
    float     m_zFar = 100.f;
    bool      m_update = true;
};
