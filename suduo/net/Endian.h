#ifndef ENDIAN_H
#define ENDIAN_H
#include <endian.h>

#include <cstdint>
namespace suduo {
namespace net {
namespace socket {
inline uint64_t host_to_network_64(uint64_t host) { return htobe64(host); }
inline uint32_t host_to_network_32(uint32_t host) { return htobe32(host); }
inline uint16_t host_to_network_16(uint16_t host) { return htobe16(host); }

inline uint64_t network_to_host_64(uint64_t network) {
  return be64toh(network);
}
inline uint32_t network_to_host_32(uint32_t network) {
  return be32toh(network);
}
inline uint16_t network_to_host_16(uint16_t network) {
  return be16toh(network);
}

}  // namespace socket
}  // namespace net
}  // namespace suduo
#endif