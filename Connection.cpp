#include <queue>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>

#include "Connection.hpp"
#include "String_Utilities.hpp"

Connection::Connection(int fd_, std::shared_ptr<KV_Manager> kv_)
    : fd(fd_), kv(kv_) {
}
Connection::~Connection() {
  close(fd);
  std::cout << "closed peer_fd=" << fd << std::endl;
}

int Connection::getDescriptor() {
  return fd;
}

uint8_t Connection::read_from_socket() {
  ssize_t bytes_received;
  char buf[4096];

  /*
   * read everything from the socket and store it until it returns:
   * 0, peer disconnected
   * -1, there's nothing left to read (EAGAIN) or error
   */
  while ((bytes_received = read(fd, buf, sizeof buf)) > 0) {
    read_buffer.append(buf, bytes_received);
    std::memset(buf, 0, 4096); //reset the buffer for the next read
  }
  /*
   * inspect -1 for an error, otherwise the read finished ok
   */
  if (bytes_received == -1) {
    if (errno != EAGAIN) {
      /*
       * Terminate the connection because of error
       */
      return 3;
    }
  } else if (bytes_received == 0) {
    /*
     * Terminate the connection because of disconnect
     */
    return 3;
  }

  /*
   * Inspect the last character to see if it's a newline
   */
  if (read_buffer.back() == '\n') {
    if (process_message()) {
      /*
       * Ready to write
       */
      return 1;
    } else {
      /*
       * Terminate the connection because of quit message or error
       */
      return 3;
    }
  }
  /*
   * Keep reading, last character wasn't a newline
   */
  return 2; //didn't get the whole message, read was busy
}

bool Connection::process_message() {
  /*
   * read_from_socket() populates read_buffer
   */
  std::stringstream ss(read_buffer);
  std::queue<std::string> lines;
  /*
   * parse the read buffer for newlines and queue them to be processed
   */
  for (std::string line; std::getline(ss, line); lines.push(line));
  /*
   * populate the write_queue by processing the queue of operations
   */
  while (!lines.empty()) {
    std::vector<std::string> argv = String_Utilities::tokenize_ss(lines.front());
    lines.pop();
    switch (argv.size()) {
      case 3: //set
        if (argv.front().compare("set") == 0) {
          write_queue.push_back(kv->set_key(argv[1], argv[2]));
        }
        break;
      case 2: //get
        if (argv.front().compare("get") == 0) {
          write_queue.push_back(kv->get_value(argv[1]));
        }
        break;
      case 1: //quit
        if (argv.front().compare("quit") == 0) {
          return false;
        }
        break;
      default: //unexpected error
        return false;
    }
  }

  read_buffer.clear();
  return true;
}

uint8_t Connection::write_to_socket() {
  /*
   * process_message() populates write_queue
   */
  while (!write_queue.empty()) {
    std::string peer_message = write_queue.front();
    write_queue.pop_front();
    /*
     * Prepare the std::string message for the socket
     */
    const char *outgoing_message = peer_message.c_str();
    size_t length = peer_message.length();

    /*
     * Perform the send until the nonblocking return value is:
     * -1, send buffer is full or error
     * or length is 0, entire message was sent
     */
    ssize_t bytes_sent = 0;
    while (length != 0 && (bytes_sent = write(fd, outgoing_message, length)) != -1) {
      outgoing_message += bytes_sent;
      length -= bytes_sent;
    }

    /*
     * Inspect -1 return value:
     * if send buffer was full (EAGAIN), add the remainder of the message to the front of the queue
     * otherwise, handle the error by terminating the connection
     */
    if (bytes_sent == -1) {
      if (errno == EAGAIN) {
        std::string remainder(outgoing_message, length);
        write_queue.push_front(remainder);
        /*
         * Continue writing when send buffer is available
         */
        return 1;
      } else {
        /*
         * Terminate the connection
         */
        return 3;
      }
    }
  }
  /*
   * Write queue is empty, go back to reading
   */
  return 2;
}


