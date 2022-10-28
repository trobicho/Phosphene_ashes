#pragma once

#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <glm/glm.hpp>
  
namespace PhosHelper {
  void  infoInstance();
  void  infoRaytracingProperties(VkPhysicalDevice physicalDevice);
  void  printMatrix(glm::mat4 matrix);
  VkTransformMatrixKHR  matrixToVkTransformMatrix(glm::mat4 matrix);
}

namespace PhosHelper {
  std::vector<char>     readBinaryFile(const std::string &filename);
  VkShaderModule	createShaderModuleFromCode(VkDevice device, std::vector<char> code);
  inline VkShaderModule createShaderModuleFromFile(VkDevice device, const std::string &filename) {
    return (createShaderModuleFromCode(device, readBinaryFile(filename)));
  }
}

namespace PhosHelper {
  class	FatalError: public std::exception
  {
    public:
      //FatalError(const string& msg);
      FatalError(std::string msg): message(msg) {};

      const char * what () const throw ()
      {
        return message.c_str();
      }
    private:
      std::string message;
  };

  class	FatalVulkanInitError: public std::exception
  {
    public:
      FatalVulkanInitError(std::string msg): message(msg) {};

      const char * what () const throw ()
      {
        return message.c_str();
      }
    private:
      std::string message;
  };

  class	BasicError: public std::exception
  {
    public:
      BasicError(std::string msg): message(msg) {};

      const char * what () const throw ()
      {
        return message.c_str();
      }
    private:
      std::string message;
  };

  class	FileError: public std::exception
  {
    public:
      FileError(std::string msg): message(msg) {};

      const char * what () const throw ()
      {
        return message.c_str();
      }
    private:
      std::string message;
  };
}
