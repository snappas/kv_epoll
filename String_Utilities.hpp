#ifndef KV_MESSAGE_PROCESSOR_H
#define KV_MESSAGE_PROCESSOR_H

#include <vector>
#include <string>
#include <sstream>

class String_Utilities {
 public:
  static std::vector<std::string> tokenize_ss(std::string message) {
    std::vector<std::string> argv;
    std::string arg;
    std::stringstream ss(message);
    while (ss >> arg) {
      argv.push_back(arg);
    }
    return argv;
  }
};


#endif //KV_MESSAGE_PROCESSOR_H
