#include "camera.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/projection.hpp>
#include <iostream>

Camera::Camera() {
}

void  Camera::step(float deltaTime) {
  if (m_allowMoving && (m_keyState & 0b111111) != 0) {
    glm::vec3 translate{0.f};
    float     rotateRad = 0.f;

    if (m_keyState & CAMERA_KEY_STATE_RIGHT)
      translate += m_right * 1.0f;
    if (m_keyState & CAMERA_KEY_STATE_LEFT)
      translate -= m_right * 1.0f;
    if (m_keyState & CAMERA_KEY_STATE_FORWARD)
      translate += m_forward * 1.0f;
    if (m_keyState & CAMERA_KEY_STATE_BACKWARD)
      translate -= m_forward * 1.0f;

    float norme = sqrt(translate.x * translate.x + translate.y * translate.y + translate.z * translate.z);
    if (norme >= 0.5f) {
      translate = glm::normalize(translate);
      m_position += translate * m_speed * (float)(deltaTime / BASE_DELTA_TIME);
    }

    if (m_keyState & CAMERA_KEY_STATE_ROLL_CLOCKWISE)
      rotateRad += 0.002f;
    if (m_keyState & CAMERA_KEY_STATE_ROLL_COUNTER_CLOCKWISE)
      rotateRad -= 0.002f;
    if (m_keyState & CAMERA_KEY_STATE_ROLL_CLOCKWISE || m_keyState & CAMERA_KEY_STATE_ROLL_COUNTER_CLOCKWISE) {
      m_up = glm::rotate(m_up, rotateRad, m_forward);
      m_right = -glm::normalize(glm::cross(m_up, m_forward));
    }

    m_update = true;
  }
  if (m_event & CAMERA_EVENT_ASPECT_CHANGE) {
    m_aspectRatio = m_newAspectRatio;
    m_event &= ~(CAMERA_EVENT_ASPECT_CHANGE);
    m_update = true;
  }
}

void  Camera::rotate(double x, double y) {
  if (m_allowRotation) {
    float norme = sqrt((x - m_lastX) * (x - m_lastX) + (y - m_lastY) * (y - m_lastY));

    if (norme <= 0.01)
      return ;
    glm::vec3 axis = m_right * (float)(y - m_lastY) + -m_up * (float)(x - m_lastX);
    axis = glm::normalize(axis);
    m_forward = glm::rotate(m_forward, norme * glm::radians(0.1f), axis);
    //m_right = glm::rotate(m_right, norme * glm::radians(0.1f), axis);
    //m_right = glm::normalize(m_right);
    m_right = -glm::normalize(glm::cross(m_up, m_forward));
    m_update = true;
  }
  m_lastX = x;
  m_lastY = y;
}

bool  Camera::buildGlobalUniform(GlobalUniforms &uniform) {
  if (m_update) {
    forceBuildGlobalUniform(uniform);
    return (true);
  }
  return (false);
}

void  Camera::forceBuildGlobalUniform(GlobalUniforms &uniform) {
  m_view = glm::lookAt(m_position, m_position + m_forward, m_up);
  m_projection = glm::perspective(m_fovY, m_aspectRatio, m_zNear, m_zFar);
  uniform = {
    .viewProj = m_view ,
    .viewInverse = glm::inverse(m_view),
    .projInverse = glm::inverse(m_projection),
  };
  m_update = false;
}
