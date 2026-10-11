#include "text_parsers.hpp"

#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <utility>

namespace ntq {
namespace linux_system {

namespace text {

std::string trim(const std::string &value) {
  std::string::size_type first = 0;
  while (first < value.size() &&
         std::isspace(static_cast<unsigned char>(value[first]))) {
    ++first;
  }
  std::string::size_type last = value.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(value[last - 1]))) {
    --last;
  }
  return value.substr(first, last - first);
}

bool parse_unsigned(const std::string &text, std::uint64_t &value) {
  const std::string input = trim(text);
  if (input.empty() || input[0] == '-') {
    return false;
  }
  for (char character : input) {
    if (!std::isdigit(static_cast<unsigned char>(character))) {
      return false;
    }
  }
  errno = 0;
  char *end = nullptr;
  const unsigned long long parsed = std::strtoull(input.c_str(), &end, 10);
  if (errno == ERANGE || end == input.c_str() || *end != '\0') {
    return false;
  }
  value = static_cast<std::uint64_t>(parsed);
  return true;
}

std::vector<std::string> split(const std::string &value, char delimiter) {
  std::vector<std::string> fields;
  std::stringstream stream(value);
  std::string field;
  while (std::getline(stream, field, delimiter)) {
    fields.push_back(field);
  }
  if (!value.empty() && value[value.size() - 1] == delimiter) {
    fields.push_back(std::string());
  }
  return fields;
}

std::vector<std::string> whitespace_fields(const std::string &value) {
  std::istringstream stream(value);
  std::vector<std::string> fields;
  std::string field;
  while (stream >> field) {
    fields.push_back(field);
  }
  return fields;
}

std::string decode_mount_path(std::string path) {
  const std::pair<const char *, const char *> escapes[] = {
      {"\\040", " "}, {"\\011", "\t"}, {"\\012", "\n"},
      {"\\134", "\\"}};
  for (const auto &escape : escapes) {
    std::string::size_type position = 0;
    while ((position = path.find(escape.first, position)) !=
           std::string::npos) {
      path.replace(position, 4, escape.second);
      position += std::char_traits<char>::length(escape.second);
    }
  }
  return path;
}

} // namespace text
} // namespace linux_system
} // namespace ntq
