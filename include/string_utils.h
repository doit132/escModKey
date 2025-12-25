#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <algorithm>
#include <cctype>
#include <string>

namespace StringUtils {

// Convert string to lowercase
inline std::string toLower(const std::string &str) {
  std::string result = str;
  for (char &c : result) {
    c = std::tolower(static_cast<unsigned char>(c));
  }
  return result;
}

// Generate ID from name (lowercase, no spaces)
// Example: "Left Ctrl" -> "leftctrl", "CapsLock" -> "capslock"
inline std::string generateIdFromName(const std::string &name) {
  std::string id = toLower(name);
  id.erase(std::remove(id.begin(), id.end(), ' '), id.end());
  return id;
}

} // namespace StringUtils

#endif // STRING_UTILS_H
