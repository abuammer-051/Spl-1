#include <cstdint>

using namespace std;

uint32_t updateChecksum(uint32_t sum, const char* data, int length, bool& odd) {
 for (int i = 0; i < length; i++) {
  unsigned char value = static_cast<unsigned char>(data[i]);
  if (odd) sum += value;
  else sum += static_cast<uint32_t>(value) << 8;
  sum = (sum & 0xffffu) + (sum >> 16);
  odd = !odd;
 }
 return sum;
}

uint16_t finishChecksum(uint32_t sum) {
 while (sum >> 16) sum = (sum & 0xffffu) + (sum >> 16);
 return static_cast<uint16_t>(~sum);
}

uint16_t calculateChecksum(const char* data, int length) {
 bool odd = false;
 return finishChecksum(updateChecksum(0, data, length, odd));
}
