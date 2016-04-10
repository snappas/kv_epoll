//
// Created by ryan on 2/24/16.
//

#ifndef KV_CONNECTION_H
#define KV_CONNECTION_H

#include <deque>
#include <string>
#include <memory>
#include <unistd.h>

#include "KV_Manager.hpp"

class Connection {
 public:
  Connection() { };
  Connection(int fd_, std::shared_ptr<KV_Manager> kv_);
  ~Connection();
  int getDescriptor();
  uint8_t write_to_socket();
  uint8_t read_from_socket();
 private:
  int fd;
  std::shared_ptr<KV_Manager> kv;
  std::deque<std::string> write_queue;
  std::string read_buffer;
  bool process_message();

};


#endif //KV_CONNECTION_H
