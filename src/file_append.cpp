#include "file_append.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

bool append_to_file(const std::string &data, const std::string &filename,
                    int line, int pos) {
  std::ifstream infile(filename);
  if (!infile.is_open()) {
    std::cerr << "Cannot open the file: " << filename << std::endl;
    return false;
  }

  std::vector<std::string> lines;
  std::string current_line;
  while (std::getline(infile, current_line)) {
    lines.push_back(current_line);
  }
  infile.close();

  if (line == -1) {
    line = static_cast<int>(lines.size()) + 1;
  } else if (line < 1 || line > static_cast<int>(lines.size()) + 1) {
    std::cerr << "Line number " << line << " is out of range." << std::endl;
    return false;
  }

  if (line > static_cast<int>(lines.size())) {
    lines.push_back("");
  }

  std::string &target_line = lines[line - 1];

  if (pos == -1) {
    pos = static_cast<int>(target_line.size());
  } else if (pos < 0 || pos > static_cast<int>(target_line.size())) {
    std::cerr << "Position " << pos << " is out of range in the line."
              << std::endl;
    return false;
  }

  target_line.insert(pos, data);

  std::ofstream outfile(filename);
  if (!outfile.is_open()) {
    std::cerr << "Cannot open the file: " << filename << std::endl;
    return false;
  }

  for (const auto &l : lines) {
    outfile << l << "\n";
  }

  outfile.close();
  return true;
}
