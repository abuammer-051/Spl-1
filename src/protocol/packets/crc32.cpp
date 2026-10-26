#include <cstdint>

using namespace std;

uint32_t updateCrc32(uint32_t crc, const char* data, int length) {
    for (int i = 0; i < length; i++) {
        crc ^= static_cast<unsigned char>(data[i]);

        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1u) {
                crc = (crc >> 1) ^ 0xedb88320u;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

uint32_t calculateCrc32(const char* data, int length) {
    return ~updateCrc32(0xffffffffu, data, length);
}
