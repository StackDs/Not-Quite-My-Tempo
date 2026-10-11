#include "file_reader.hpp"

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>

namespace ntq {
namespace linux_system {

CollectionStatus status_from_errno(int error) {
  if (error == EACCES || error == EPERM) {
    return CollectionStatus::permission_denied;
  }
  if (error == ENOENT || error == ENOTDIR || error == ENOTSUP ||
      error == EOPNOTSUPP) {
    return CollectionStatus::unavailable;
  }
  return CollectionStatus::read_error;
}

CollectedValue<std::string> read_text_file(const std::string &path) {
  const int descriptor = open(path.c_str(), O_RDONLY | O_CLOEXEC);
  if (descriptor == -1) {
    return CollectedValue<std::string>(std::string(), status_from_errno(errno),
                                       path);
  }

  std::string contents;
  char buffer[4096];
  for (;;) {
    const ssize_t bytes = read(descriptor, buffer, sizeof(buffer));
    if (bytes == 0) {
      break;
    }
    if (bytes == -1) {
      if (errno == EINTR) {
        continue;
      }
      const int error = errno;
      close(descriptor);
      return CollectedValue<std::string>(std::string(),
                                         status_from_errno(error), path);
    }
    contents.append(buffer, static_cast<std::size_t>(bytes));
  }
  close(descriptor);
  return CollectedValue<std::string>(contents, CollectionStatus::available,
                                     path);
}

} // namespace linux_system
} // namespace ntq
