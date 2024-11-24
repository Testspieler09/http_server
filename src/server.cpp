#include "respone_header.hpp"
#include <arpa/inet.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

bool ends_with(const std::string &value, const std::string &suffix) {
  if (suffix.size() > value.size())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

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
    char buffer[2048];
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
    } else if (was_successful >= 2048) {
      std::cerr << "[ERROR] The client request is bigger than 2KB\n";
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

std::string Server::generate_response(const unsigned int &response_code,
                                      const std::string &content,
                                      const std::string &content_type) {

  std::ostringstream ss;
  ss << "HTTP/1.1 " << get_response(response_code) << "\r\n";
  if (content.length() != 0) {
    ss << "Content-Type: " << content_type << "\r\n";
    ss << "Content-Length: " << content.length() << "\r\n";
    ss << "\r\n";
    ss << content;
  }

  return ss.str();
}

std::string Server::get_content_type(const std::string &path) {
  if (ends_with(path, ".html"))
    return "text/html";
  else if (ends_with(path, ".css"))
    return "text/css";
  else if (ends_with(path, ".js"))
    return "application/javascript";
  else if (ends_with(path, ".png"))
    return "image/png";
  else if (ends_with(path, ".jpg") || ends_with(path, ".jpeg"))
    return "image/jpeg";
  else if (ends_with(path, ".gif"))
    return "image/gif";
  return "application/octet-stream";
}

std::string Server::evaluate_request(const std::string &req) {

  // NOTE: This is a debug print

  /*std::cout << "=============== Request ===============\n" << req << "\n";*/

  std::istringstream request_stream(req);
  std::string request_method, path;

  // Read the first line from the request stream
  if (!std::getline(request_stream, request_method, ' ') ||
      !std::getline(request_stream, path, ' '))
    return this->generate_response(400);

  if (path == "/")
    path = "/index.html";
  path = "." + path;

  std::string res = this->generate_response(501);

  if (request_method == "GET") {
    res = this->get_request(path);
  } else if (request_method == "POST") {
    res = this->post_request(req, path);
  } else if (request_method == "DELETE") {
    res = this->delete_request(req, path);
  } else if (request_method == "HEAD") {
    res = this->head_request(path);
  } else {
    std::cerr << "[ERROR] This HTTP server only supports GET, POST, DELETE "
                 "and HEAD "
                 "requests until now. The request's type was "
              << request_method << "\n";
  }

  std::cout << "[SENDING] " << res.substr(0, res.find("\n")) << "\n";

  return res;
}

std::string Server::get_request(const std::string &path) {

  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return this->generate_response(
        404, "<html><body><h1>404 Not Found</h1></body></html>");
  }

  // TODO: check if user is allowed to open the file.

  std::ostringstream ss;
  ss << file.rdbuf();

  file.close();

  return this->generate_response(200, ss.str(), get_content_type(path));
}

std::string Server::post_request(const std::string &req,
                                 const std::string &path) {
  return this->generate_response(200);
}

std::string Server::delete_request(const std::string &req,
                                   const std::string &path) {
  return this->generate_response(200);
}

std::string Server::head_request(const std::string &path) {

  // Check if the file exists
  if (!std::filesystem::exists(path)) {
    return this->generate_response(404);
  }

  // Get the last change date of the resource
  struct stat result;
  if (stat(path.c_str(), &result) != 0) {
    return this->generate_response(404);
  }

  std::time_t now = std::time(NULL);
  std::tm now_time;
  gmtime_r(&now, &now_time);

  std::time_t mod_time = result.st_mtime;
  std::tm gmt_time;
  gmtime_r(&mod_time, &gmt_time);

  std::string content_type = get_content_type(path);

  auto file_size = result.st_size;

  std::ostringstream ss;
  ss << "HTTP/1.1 200 OK\r\n";
  ss << "Content-Type: " << content_type << "\r\n";
  ss << "Content-Length: " << file_size << "\r\n";
  ss << "Date: " << std::put_time(&now_time, "%a, %d %b %Y %H:%M:%S GMT")
     << "\r\n";
  ss << "Last-Modified: "
     << std::put_time(&gmt_time, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
  ss << "\r\n";

  return ss.str();
}
