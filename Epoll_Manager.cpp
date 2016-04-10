#include <sys/epoll.h>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <memory>
#include <iostream>
#include <cstring>

#include "Epoll_Manager.hpp"


Epoll_Manager::Epoll_Manager(int listen_fd_) : listen_fd(listen_fd_) {
  /*
   * Create epoll instance
   */
  epollfd = epoll_create1(0);
  if (epollfd < 0) {
    perror("epoll_create");
  }
  /*
   * Add the listening descriptor to epoll's interest list
   */
  if (listen_fd_ != -1) {
    event.data.ptr = nullptr;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd_, &event) == -1) {
      perror("epoll_ctl");
    }
  }
  /*
   * Initialize the key-value manager shared pointer (shared with connections)
   */
  kv.reset(new KV_Manager);
}

int Epoll_Manager::accept_nonblocking() {
  sockaddr_in peer_addr;
  socklen_t peer_addr_size = sizeof(peer_addr);
  std::memset(&peer_addr, 0, peer_addr_size);
  /*
   * Accept new connections in non-blocking mode for use with edge-triggered epoll
   * it's ok if accept returns -1, we will use it to stop looking for new connections
   */
  int peer_fd = accept4(listen_fd, (struct sockaddr *) &peer_addr, &peer_addr_size, SOCK_NONBLOCK);

  return peer_fd;
}

/*
 * Epoll control methods (notify on writable, readable, monitor new descriptor)
 * EPOLLONESHOT removes the descriptor from the interest list after it's triggered
 * epoll_ctl_mod adds a ONESHOT descriptor back to the interest list
 */
void Epoll_Manager::epoll_set_fd_writable(Connection *connection) {
  event.data.ptr = connection;
  event.events = EPOLLET | EPOLLOUT | EPOLLONESHOT;
  if (epoll_ctl(epollfd, EPOLL_CTL_MOD, connection->getDescriptor(), &event) != 0) {
    perror("epoll_ctl epollout");
  }
}
void Epoll_Manager::epoll_set_fd_readable(Connection *connection) {
  event.data.ptr = connection;
  event.events = EPOLLET | EPOLLIN | EPOLLONESHOT;
  if (epoll_ctl(epollfd, EPOLL_CTL_MOD, connection->getDescriptor(), &event) != 0) {
    perror("epoll_ctl epollin");
  }
}
void Epoll_Manager::epoll_add_descriptor(int fd_) {
  if (fd_ != -1) {
    event.data.ptr = new Connection(fd_, kv);
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_, &event) == -1) {
      perror("epoll_ctl epolladd");
    }
  }
}


void Epoll_Manager::epoll_event_handler() {
  epoll_event events[64];
  /*
   * Timeout of -1 means epoll_wait will block until any monitored descriptors change
   */
  int nfds = epoll_wait(epollfd, events, 64, -1);
  for (int i = 0; i < nfds; ++i) {
    /*
     * The listening socket doesn't use a connection object
     * Accept connections until a -1 is found, since it is edge-triggered mode
     */
    if (events[i].data.ptr == nullptr) {
      while (1) {
        int peer_fd = accept_nonblocking();
        if (peer_fd == -1) {
          break;
        }
        std::cout << "accepted peer_fd=" << peer_fd << std::endl;
        epoll_add_descriptor(peer_fd);
      }
      /*
       * Use the event flags to perform read or write on the connection
       */
    } else {
      Connection *connection = reinterpret_cast<Connection *>(events[i].data.ptr);
      if (!(events[i].events & EPOLLOUT) && !(events[i].events & EPOLLIN)) {
        perror("epoll_wait");
        continue;
      }
      if (events[i].events & EPOLLOUT) {
        do_write(connection);
      } else if (events[i].events & EPOLLIN) {
        do_read(connection);
      }
    }
  }
}


void Epoll_Manager::do_write(Connection *connection) {
  switch (connection->write_to_socket()) {
    default:
      break;
    case 1: //write didn't finish
      epoll_set_fd_writable(connection);
      break;
    case 2: //write was successful, read next
      epoll_set_fd_readable(connection);
      break;
    case 3: //error
      delete connection;
      break;
  }
}

void Epoll_Manager::do_read(Connection *connection) {
  switch (connection->read_from_socket()) {
    default:
      break;
    case 1: //read was successful, write next
      epoll_set_fd_writable(connection);
      break;
    case 2: //read didn't finish
      epoll_set_fd_readable(connection);
      break;
    case 3: //read error or client disconnected
      delete connection;
      break;
  }
}

void Epoll_Manager::start() {
  /*
   * Event handler loop
   */
  while (1) {
    epoll_event_handler();
  }
}


