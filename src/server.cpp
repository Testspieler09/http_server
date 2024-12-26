#include "auth.hpp"
#include "file_append.hpp"
#include "respone_header.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

// NOTE: Some helper functions

bool ends_with(const std::string &value, const std::string &suffix) {
  if (suffix.size() > value.size())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

std::string to_lower(const std::string &str) {
  std::string lower_str = str;
  std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return lower_str;
}

std::string trim(const std::string &str) {
  const char *whitespace = " \t\n\r\f\v";
  std::string trimmed = str;
  trimmed.erase(trimmed.find_last_not_of(whitespace) + 1);
  trimmed.erase(0, trimmed.find_first_not_of(whitespace));
  return trimmed;
}

// NOTE: Start of the server class

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
  std::unordered_map<std::string, std::string>
  parse_headers(const std::string &req);
  std::pair<int, int> get_append_position(const std::string &body);
  std::string evaluate_request(const std::string &req);
  std::string get_request(const std::string &path);
  std::string post_request(const std::string &req, const std::string &path);
  std::string put_request(const std::string &req, const std::string &path);
  std::string delete_request(const std::string &path);
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
  if (ends_with(path, ".txt"))
    return "text/plain";
  else if (ends_with(path, ".html") || ends_with(path, ".htm"))
    return "text/html";
  else if (ends_with(path, ".css"))
    return "text/css";
  else if (ends_with(path, ".js"))
    return "application/javascript";
  else if (ends_with(path, ".xml"))
    return "text/xml";
  else if (ends_with(path, ".png"))
    return "image/png";
  else if (ends_with(path, ".jpg") || ends_with(path, ".jpeg"))
    return "image/jpeg";
  else if (ends_with(path, ".gif"))
    return "image/gif";
  return "application/octet-stream";
}

std::unordered_map<std::string, std::string>
Server::parse_headers(const std::string &req) {
  std::unordered_map<std::string, std::string> headers;
  std::istringstream stream(req);
  std::string line;

  // Skip the request line
  std::getline(stream, line);

  // Parse headers
  while (std::getline(stream, line) && line != "\r") {
    std::string::size_type pos = line.find(": ");
    if (pos != std::string::npos) {
      std::string key = to_lower(line.substr(0, pos));
      std::string value = trim(line.substr(pos + 2));
      headers[key] = value;
    }
  }
  return headers;
}

std::pair<int, int>
Server::get_append_position(const std::string &header_values) {
  int line = -1; // Default value for EOF
  int pos = -1;  // Default value for EOL

  std::size_t idx_line = header_values.find("line=");
  std::size_t idx_pos = header_values.find("pos=");

  if (idx_line != std::string::npos) {
    std::size_t end_idx = header_values.find(' ', idx_line);
    if (end_idx == std::string::npos) {
      end_idx = header_values.length();
    }
    line = std::stoi(header_values.substr(idx_line + 5, end_idx));
  }
  if (idx_pos != std::string::npos) {
    pos = std::stoi(header_values.substr(idx_pos + 4));
  }

  std::cout << line << ", " << pos << "\n";
  return {line, pos};
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
  } else if (request_method == "PUT") {
    res = this->put_request(req, path);
  } else if (request_method == "DELETE") {
    res = this->delete_request(path);
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

  // NOTE: Simple auth with a server-side whitelist / no user profiles
  if (!access_allowed(path)) {
    return this->generate_response(
        403, "The file is not contained in the server's whitelist");
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  file.close();

  return this->generate_response(200, ss.str(), get_content_type(path));
}

std::string Server::post_request(const std::string &req,
                                 const std::string &path) {

  if (!allowed_to_post_put(path)) {
    return this->generate_response(
        403, "The file is not contained in the server's whitelist");
  }

  std::unordered_map<std::string, std::string> headers = parse_headers(req);
  if (headers.find("content-type") != headers.end()) {
    std::string content_type = headers["content-type"];
    if (content_type != this->get_content_type(path)) {
      return this->generate_response(
          415, "The Content-Type header and the filepath do not match");
    }
  } else {
    return this->generate_response(400, "Missing Content-Type header");
  }

  std::string body;

  std::string::size_type body_pos = req.find("\r\n\r\n");
  if (body_pos != std::string::npos) {
    body = req.substr(body_pos + 4);
  } else {
    return this->generate_response(400, "Missing Body");
  }

  int line = -1, pos = -1;
  std::string append_pos =
      headers["append-position"]; // Assuming 'Append-Position:
                                  // line=<line>,pos=<pos>' is sent in headers
  if (!append_pos.empty()) {
    std::pair<int, int> pos_info = get_append_position(append_pos);
    line = pos_info.first;
    pos = pos_info.second;
  }
  if (std::filesystem::exists(path)) {
    if (append_to_file(body, path, line, pos)) {
      return this->generate_response(201, "Successfully appended to file");
    } else {
      return this->generate_response(500, "Failed to append to file");
    }
  }

  std::ofstream file(path);
  if (!file.is_open()) {
    return this->generate_response(500, "Could not write to file");
  }
  file << body;
  file.close();

  return this->generate_response(201);
}

std::string Server::put_request(const std::string &req,
                                const std::string &path) {
  std::string body;

  std::string::size_type body_pos = req.find("\r\n\r\n");
  if (body_pos != std::string::npos) {
    body = req.substr(body_pos + 4);
  } else {
    return this->generate_response(400, "Missing Body");
  }

  std::ofstream file(path);
  if (!file.is_open()) {
    return this->generate_response(500, "Could not write to file");
  }
  file << body;
  file.close();

  return this->generate_response(201);
}

std::string Server::delete_request(const std::string &path) {

  if (!allowed_to_delete(path)) {
    return this->generate_response(403, "Not allowed to delete the file");
  }

  if (!std::filesystem::exists(path)) {
    return this->generate_response(404, "File does not exist.");
  }

  if (!std::filesystem::remove(path)) {
    return this->generate_response(500);
  }

  return this->generate_response(204);
}

std::string Server::head_request(const std::string &path) {

  // Check if the file exists
  if (!std::filesystem::exists(path)) {
    return this->generate_response(404, "File does not exist");
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

// NOTE: End of the server class
