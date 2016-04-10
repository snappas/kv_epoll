#include "server.hpp"

#include <cstring>
#include <iostream>
#include <algorithm>
#include <arpa/inet.h>


namespace EpochLabsTest {

Server::Server(const std::string &listen_address, int listen_port)
    : listen_fd(-1) {
  std::cout << "creating server" << std::endl;

  listen_fd = bind_listen_socket(listen_address, listen_port);

  event_handler.reset(new Epoll_Manager(listen_fd));

}

int Server::bind_listen_socket(std::string listen_address, int listen_port) {
  int listen_fd;
  sockaddr_in listen_sockaddr_in;
  std::memset(&listen_sockaddr_in, 0, sizeof(listen_sockaddr_in));
  listen_sockaddr_in.sin_family = AF_INET;
  inet_aton(listen_address.c_str(), &listen_sockaddr_in.sin_addr);
  listen_sockaddr_in.sin_port = htons(listen_port);

  listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (listen_fd < 0) {
    throw_error("could not create socket", errno);
  }

  int t = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t))) {
    throw_error("could not set SO_REUSEADDR", errno);
  }

  if (bind(listen_fd, (struct sockaddr *) &listen_sockaddr_in, sizeof(listen_sockaddr_in))) {
    throw_error("could not bind listen socket", errno);
  }

  if (listen(listen_fd, 48)) {
    throw_error("could not listen on socket", errno);
  }
  //picked up by test_server.py to know server is ready
  //this line must be output after listen returns successfully
  std::cout << "listening on " << listen_address << ":" << listen_port << std::endl;
  return listen_fd;
}


void Server::throw_error(const char *msg_, int errno_) {
  std::string msg = msg_ + std::string(" errno=") + std::to_string(errno_);
  throw std::runtime_error(msg);
}

void Server::run() {
  std::cout << "running ..." << std::endl;
  event_handler->start();
}
}

