#ifndef KV_KV_MANAGER_H
#define KV_KV_MANAGER_H

#include <unordered_map>
#include <mutex>
#include <map>


class KV_Manager {
 public:
  KV_Manager() { };
  ~KV_Manager() { };
  std::string set_key(std::string key, std::string value);
  std::string get_value(std::string key);
 private:
  std::unordered_map<std::string, std::string> kv;
};


#endif //KV_KV_MANAGER_H
