#include "time_conversion.hpp"

#include <stdexcept>

namespace ntq {
namespace metrics_detail {

// Convierte el delta timeval de getrusage a la representación interna común.
long double cpu_time_delta_nanoseconds(long seconds, long microseconds) {
  return (static_cast<long double>(seconds) * 1'000'000.0L +
          static_cast<long double>(microseconds)) *
         1'000.0L;
}

// Devuelve el factor por unidad; el estado guardado permanece en nanosegundos.
double nanoseconds_per_unit(TimeUnit unit) {
  switch (unit) {
  case TimeUnit::nanoseconds:
    return 1.0;
  case TimeUnit::microseconds:
    return 1'000.0;
  case TimeUnit::milliseconds:
    return 1'000'000.0;
  case TimeUnit::seconds:
    return 1'000'000'000.0;
  case TimeUnit::minutes:
    return 60'000'000'000.0;
  case TimeUnit::hours:
    return 3'600'000'000'000.0;
  }
  throw std::invalid_argument("Unidad de tiempo no válida");
}

} // namespace metrics_detail
} // namespace ntq
