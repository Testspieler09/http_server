#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

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
  std::string get_request(const std::string &req, const std::string &dir,
                          const std::string &version);
  std::string post_request(const std::string &req, const std::string &dir,
                           const std::string &version);
  std::string delete_request(const std::string &req, const std::string &dir,
                             const std::string &version);
  std::string head_request(const std::string &req, const std::string &dir,
                           const std::string &version);
};

int Server::SERVER_SOCKET = -1;

void Server::signal_handler(int signal) {
  if (signal == SIGINT) {
    std::cout << "[INFO] Shutting down server...\n";
    if (SERVER_SOCKET > 0) {
      close(SERVER_SOCKET);
    }
    exit(0);
  }
}

Server::Server(const std::string &ip, int port) {
  this->SERVER_SOCKET = socket(AF_INET, SOCK_STREAM, 0);
  if (this->SERVER_SOCKET < 0) {
    std::cerr << "[ERROR] Creation of server socket failed. errno: " << errno
              << " (" << strerror(errno) << ")\n";
    throw "[SERVER] Failed to created server socket\n";
  }

  try {
    this->bind_server(ip, port);
  } catch (const char *e) {
    throw e;
  }
}

Server::~Server() {
  if (this->SERVER_SOCKET > 0) {
    close(this->SERVER_SOCKET);
  }
}

void Server::bind_server(const std::string &ip, int port) {
  sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &server_address.sin_addr) <= 0) {
    throw std::runtime_error("Invalid IP address");
  }

  int was_successful =
      bind(this->SERVER_SOCKET, (struct sockaddr *)&server_address,
           sizeof(server_address));
  if (was_successful != 0) {
    std::cerr << "[ERROR] Binding the server failed. errno: " << errno << " ("
              << strerror(errno) << ")\n";
    throw "[SERVER] Binding the server failed\n";
  }

  std::cout << "[SERVER] Server bound\n";

  // Start listening on the socket
  if (listen(this->SERVER_SOCKET, 10) < 0) {
    throw "[ERROR] Failed to listen on server socket";
  }

  std::cout << "[SERVER] Server started and listening on http://" << ip << ":"
            << port << "\n";
}

void Server::run() {
  signal(SIGINT, Server::signal_handler);
  while (true) {
    sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int client_socket = accept(this->SERVER_SOCKET,
                               (struct sockaddr *)&clientAddress, &clientLen);

    if (client_socket < 0) {
      std::cerr << "[ERROR] Failed to accept client connection. errno: "
                << errno << " (" << strerror(errno) << ")\n";
      continue;
    }

    // Recieve request here
    char buffer[1024];
    int was_successful = recv(client_socket, &buffer, sizeof(buffer), 0);
    if (was_successful == -1) {
      std::cerr << "[ERROR] Failed to receive client request. errno: " << errno
                << " (" << strerror(errno) << ")\n";
      if (shutdown(client_socket, SHUT_RDWR) == -1) {
        std::cerr << "[Error] Failed to shutdown client socket. errno: "
                  << errno << " (" << strerror(errno) << ")\n";
      };
      close(client_socket);
      continue;
    } else if (was_successful >= 1024) {
      std::cerr << "[ERROR] The client request is bigger than 1KB\n";
      continue;
    }

    // Convert request to std::string
    buffer[was_successful] = '\0';
    std::string client_request = buffer;

    // Create response
    std::string res = this->evaluate_request(client_request);

    send(client_socket, res.c_str(), res.size(), 0);

    if (shutdown(client_socket, SHUT_RDWR) == -1) {
      std::cerr << "[Error] Failed to shutdown client socket. errno: " << errno
                << " (" << strerror(errno) << ")\n";
    }
    close(client_socket);
  }
  return;
}

std::string Server::evaluate_request(const std::string &req) {
  std::cout << "=============== Request ===============\n" << req << "\n";

  std::string res = "HTTP/1.1 501 Not Implemented\r\n";

  std::istringstream request_stream(req);
  std::string request_method, directory, http_version;

  // Read the first line from the request stream
  std::string bad_req = "HTTP/1.1 400 Bad Request\r\n";
  if (!(std::getline(request_stream, request_method, ' ')) ||
      !(std::getline(request_stream, directory, ' ')) ||
      !(std::getline(request_stream, http_version)))
    return bad_req;

  if (request_method == "GET") {
    res = this->get_request(req, directory, http_version);
  } else if (request_method == "POST") {
    res = this->post_request(req, directory, http_version);
  } else if (request_method == "DELETE") {
    res = this->delete_request(req, directory, http_version);
  } else if (request_method == "HEAD") {
    res = this->head_request(req, directory, http_version);
  } else {
    std::cerr
        << "[ERROR] This HTTP server only supports GET, POST, DELETE and HEAD "
           "requests until now. The request's type was "
        << request_method << "\n";
  }

  return res;
}

std::string Server::get_request(const std::string &req, const std::string &dir,
                                const std::string &version) {
  // Handle GET request
  /*std::filesystem::is_directory(req);*/
  std::string httpResponse = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: 13\r\n"
                             "\r\n"
                             "Hello, World!";

  return httpResponse;
}

std::string Server::post_request(const std::string &req, const std::string &dir,
                                 const std::string &version) {

  std::string httpResponse = "HTTP/1.1 200 OK\r\n";

  return httpResponse;
}

std::string Server::delete_request(const std::string &req,
                                   const std::string &dir,
                                   const std::string &version) {
  std::string httpResponse = "HTTP/1.1 200 OK\r\n";

  return httpResponse;
}

std::string Server::head_request(const std::string &req, const std::string &dir,
                                 const std::string &version) {

  std::string httpResponse = "HTTP/1.1 200 OK\r\n";

  return httpResponse;
}
