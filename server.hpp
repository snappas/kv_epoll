#ifndef EPOCHLABS_TEST_SERVER_HPP
#define EPOCHLABS_TEST_SERVER_HPP

#include <string>
#include <memory>
#include <map>
#include <unordered_map>

#include "KV_Manager.hpp"
#include "Epoll_Manager.hpp"


namespace EpochLabsTest {


class Server {
 public:
  Server(const std::string &listen_address, int listen_port);
  void run();
 private:
  int listen_fd;
  //add your members here
  std::unique_ptr<Epoll_Manager> event_handler;

  void throw_error(const char *msg_, int errno_);
  //add your methods here

  int bind_listen_socket(std::string listen_address, int listen_port);

};

}

#endif

