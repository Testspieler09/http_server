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
  std::string generate_response(const unsigned int &status,
                                const std::string &content = "",
                                const std::string &content_type = "text/html");
  std::string get_content_type(const std::string &path);
  std::string evaluate_request(const std::string &req);
  std::string get_request(const std::string &path);
  std::string post_request(const std::string &req, const std::string &path);
  std::string delete_request(const std::string &req, const std::string &path);
  std::string head_request(const std::string &path);
};

#endif
