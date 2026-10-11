#include "compiler_info.hpp"

#include <string>

namespace ntq {
namespace linux_system {
namespace {

CollectedValue<std::string> available(std::string value) {
  return CollectedValue<std::string>(value, CollectionStatus::available,
                                     "compiler predefined macros");
}

std::string language_standard_name() {
#if __cplusplus >= 202302L
  const char *standard = "++23";
#elif __cplusplus >= 202002L
  const char *standard = "++20";
#elif __cplusplus >= 201703L
  const char *standard = "++17";
#elif __cplusplus >= 201402L
  const char *standard = "++14";
#elif __cplusplus >= 201103L
  const char *standard = "++11";
#else
  const char *standard = "++03";
#endif

#ifdef __STRICT_ANSI__
  return std::string("c") + standard;
#else
  return std::string("gnu") + standard;
#endif
}

std::string target_architecture_name() {
#if defined(__x86_64__) || defined(_M_X64)
  return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
  return "x86";
#elif defined(__aarch64__)
  return "aarch64";
#elif defined(__arm__)
  return "arm";
#elif defined(__riscv)
  return "riscv";
#elif defined(__powerpc64__)
  return "powerpc64";
#else
  return std::string();
#endif
}

std::string optimization_profile() {
#if defined(__OPTIMIZE_SIZE__)
#if defined(__clang__)
  return "size optimization enabled; exact option is not inferable";
#else
  return "size optimization enabled; exact option is not inferable";
#endif
#elif defined(__OPTIMIZE__)
  return "optimization enabled; exact level is not inferable";
#else
  return "optimization disabled (effective default equivalent to -O0)";
#endif
}

} // namespace

CompilerInfo collect_compiler_info() {
  CompilerInfo info;

#if defined(__clang__)
  info.family = available("Clang");
#elif defined(__GNUC__)
  info.family = available("GCC");
#elif defined(__INTEL_LLVM_COMPILER)
  info.family = available("Intel oneAPI DPC++/C++");
#else
  info.family = CollectedValue<std::string>(
      std::string(), CollectionStatus::unavailable,
      "unsupported compiler predefined macros");
#endif

#if defined(__VERSION__)
  info.version = available(__VERSION__);
#elif defined(__clang_version__)
  info.version = available(__clang_version__);
#else
  info.version = CollectedValue<std::string>(
      std::string(), CollectionStatus::unavailable,
      "compiler version macro unavailable");
#endif

  info.language_standard = available(language_standard_name());
  info.optimization_profile = available(optimization_profile());

  const std::string target = target_architecture_name();
  info.target_architecture = CollectedValue<std::string>(
      target, target.empty() ? CollectionStatus::unavailable
                             : CollectionStatus::available,
      "compiler target architecture macros");
  return info;
}

} // namespace linux_system
} // namespace ntq
