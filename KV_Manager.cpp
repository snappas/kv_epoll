#include <sstream>

#include "KV_Manager.hpp"

std::string KV_Manager::set_key(std::string key, std::string value) {
  std::ostringstream message;
  kv[key] = value;
  message << key << "=" << value << "\n";
  return message.str();
}

std::string KV_Manager::get_value(std::string key) {
  std::ostringstream message;
  std::string value;
  try {
    value = kv.at(key);
  } catch (std::out_of_range) {
    value = "null";
  }
  message << key << "=" << value << "\n";
  return message.str();
}
