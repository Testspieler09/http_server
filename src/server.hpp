#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server {
public:
  Server(const std::string &ip, int port);
  Server(Server &&) = default;
  Server(const Server &) = default;
  Server &operator=(Server &&) = default;
  Server &operator=(const Server &) = default;
  ~Server();
  static void signal_handler(int signal);
  void run();

private:
  static int SERVER_SOCKET;
  void bind_server(const std::string &ip, int port);
  std::string evaluate_request(const std::string &req);
  std::string get_request(const std::string &req);
  std::string post_request(const std::string &req);
  std::string delete_request(const std::string &req);
  std::string head_request(const std::string &req);
};

#endif
