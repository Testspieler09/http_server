#include "auth.hpp"
#include <fstream>
#include <iostream>
#include <unordered_set>

const std::string LIST_FILE = "./server_lists.serverconf";

bool load_list(const std::string &list_file, const std::string &section,
               std::unordered_set<std::string> &list) {
  std::ifstream file(list_file);
  if (!file.is_open()) {
    std::cerr << "Failed to open list file: " << list_file << std::endl;
    return false;
  }

  std::string line;
  bool in_section = false;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }
    if (line[0] == '[' && line.back() == ']') {
      std::string current_section = line.substr(1, line.size() - 2);
      in_section = (current_section == section);
    } else if (in_section) {
      list.insert(line);
    }
  }

  file.close();
  return true;
}

bool access_allowed(const std::string &filename) {
  std::unordered_set<std::string> whitelist;
  if (!load_list(LIST_FILE, "whitelist", whitelist)) {
    return false;
  }

  return whitelist.find(filename) != whitelist.end();
}

bool allowed_to_delete(const std::string &filename) {
  std::unordered_set<std::string> deletelist;
  if (!load_list(LIST_FILE, "deletelist", deletelist)) {
    return false;
  }

  return deletelist.find(filename) != deletelist.end();
}

bool allowed_to_post_put(const std::string &filename) {
  std::unordered_set<std::string> deletelist;
  if (!load_list(LIST_FILE, "post_put_list", deletelist)) {
    return false;
  }

  return deletelist.find(filename) != deletelist.end();
}
