#include "server.hpp"
#include <argparse/argparse.hpp>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {

  argparse::ArgumentParser program("HTTP-Server", "HTTPServer v1.0.0");

  program.add_argument("-i", "--ipaddress")
      .help("The IP-Address of the HTTP-Server")
      .nargs(1)
      .default_value(std::string("127.0.0.1"))
      .action([](const std::string &value) { return value; });
  program.add_argument("-p", "--port")
      .help("The port you want the HTTP server to be open at.")
      .nargs(1)
      .default_value(8080)
      .scan<'i', int>();

  // Check if arguments where passed correctly
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  std::string ip_address = program.get<std::string>("ipaddress");
  int port = program.get<int>("port");

  try {
    Server server(ip_address, port);
    server.run();
  } catch (const char *e) {
    std::cerr << "Server error: " << e << "\n";
    return 1;
  }

  return 0;
}
