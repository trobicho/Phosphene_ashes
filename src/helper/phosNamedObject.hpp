#pragma once
#include "phosHelper.hpp"
  
struct  PhosNamedObject {
  std::string name;

  template <class T>
  static uint32_t getIdFromName(std::vector<T>& objects, const std::string name) {
    static_assert(std::is_base_of<PhosNamedObject, T>::value, "T must derive from PhosNamedObject");
    uint32_t  id;
    if (getIdFromName(objects, name, &id))
      return (id);
    throw PhosHelper::BasicError(std::string("Object{" + name + "} is not present"));
  }
  template <class T>
  static bool     getIdFromName(std::vector<T>& objects, const std::string name, uint32_t* id) {
    static_assert(std::is_base_of<PhosNamedObject, T>::value, "T must derive from PhosNamedObject");
    for (uint32_t idx = 0; idx < objects.size(); idx++) {
      if (objects[idx].name == name) {
        if (id != nullptr)
          *id = idx;
        return (true);
      }
    }
    return (false);
  }
  template <class T>
  static T*       getObjectFromName(std::vector<T>& objects, const std::string name, uint32_t* id = nullptr) {
    static_assert(std::is_base_of<PhosNamedObject, T>::value, "T must derive from PhosNamedObject");
    for (uint32_t idx = 0; idx < objects.size(); idx++) {
      if (objects[idx].name == name) {
        if (id != nullptr)
          *id = idx;
        return (&objects[idx]);
      }
    }
    return (nullptr);
  }
};
