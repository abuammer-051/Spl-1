#include <cstdint>

using namespace std;

uint32_t calculateCrc32(const char* data, int length);

static void write16(char* buffer, uint16_t value) {
    buffer[0] = static_cast<char>((value >> 8) & 0xffu);
    buffer[1] = static_cast<char>(value & 0xffu);
}

static void write32(char* buffer, uint32_t value) {
    buffer[0] = static_cast<char>((value >> 24) & 0xffu);
    buffer[1] = static_cast<char>((value >> 16) & 0xffu);
    buffer[2] = static_cast<char>((value >> 8) & 0xffu);
    buffer[3] = static_cast<char>(value & 0xffu);
}

static uint16_t read16(const char* buffer) {
    uint16_t value = static_cast<unsigned char>(buffer[0]);
    value = static_cast<uint16_t>(value << 8);
    return static_cast<uint16_t>(value | static_cast<unsigned char>(buffer[1]));
}

static uint32_t read32(const char* buffer) {
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[i]);
    }
    return value;
}

int makePacket(char* buffer, int capacity, uint8_t type, uint32_t transferId,
               uint32_t sequence, const char* data, uint16_t dataLength) {
    const int headerSize = 16;
    const int checksumSize = 4;
    int packetLength = headerSize + dataLength + checksumSize;

    if (buffer == nullptr || capacity < packetLength || dataLength > 1024 ||
        (dataLength > 0 && data == nullptr) || type < 1 || type > 4 ||
        (type == 2 && dataLength != 0)) {
        return 0;
    }

    buffer[0] = 'U';
    buffer[1] = 'F';
    buffer[2] = 1;
    buffer[3] = static_cast<char>(type);
    write32(buffer + 4, transferId);
    write32(buffer + 8, sequence);
    write16(buffer + 12, dataLength);
    write16(buffer + 14, 0);

    for (int i = 0; i < dataLength; i++) {
        buffer[headerSize + i] = data[i];
    }

    uint32_t checksum = calculateCrc32(buffer, headerSize + dataLength);
    write32(buffer + headerSize + dataLength, checksum);
    return packetLength;
}

bool readPacket(const char* buffer, int packetLength, uint8_t& type,
                uint32_t& transferId, uint32_t& sequence, char* data,
                int capacity, uint16_t& dataLength) {
    const int headerSize = 16;
    const int checksumSize = 4;

    if (buffer == nullptr || packetLength < headerSize + checksumSize ||
        packetLength > headerSize + 1024 + checksumSize || capacity < 0) {
        return false;
    }

    if (buffer[0] != 'U' || buffer[1] != 'F' || buffer[2] != 1 ||
        read16(buffer + 14) != 0) {
        return false;
    }

    uint8_t packetType = static_cast<uint8_t>(buffer[3]);
    uint16_t length = read16(buffer + 12);
    if (packetType < 1 || packetType > 4 || length > 1024 ||
        packetLength != headerSize + length + checksumSize ||
        capacity < length || (length > 0 && data == nullptr) ||
        (packetType == 2 && length != 0)) {
        return false;
    }

    uint32_t checksum = read32(buffer + headerSize + length);
    if (checksum != calculateCrc32(buffer, headerSize + length)) {
        return false;
    }

    for (int i = 0; i < length; i++) {
        data[i] = buffer[headerSize + i];
    }

    type = packetType;
    transferId = read32(buffer + 4);
    sequence = read32(buffer + 8);
    dataLength = length;
    return true;
}
