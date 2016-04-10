#ifndef KV_EPOLL_MANAGER_H
#define KV_EPOLL_MANAGER_H
#include <sys/epoll.h>
#include <memory>

#include "KV_Manager.hpp"
#include "Connection.hpp"

class Epoll_Manager {
 public:
  Epoll_Manager(int listen_fd);
  void start();

 private:
  int epollfd;
  int listen_fd;
  epoll_event event;
  std::shared_ptr<KV_Manager> kv;

  int accept_nonblocking();
  void epoll_add_descriptor(int fd_);
  void epoll_set_fd_readable(Connection *connection);
  void epoll_set_fd_writable(Connection *connection);
  void epoll_event_handler();
  void do_write(Connection *connection);
  void do_read(Connection *connection);

};


#endif //KV_EPOLL_MANAGER_H
