#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

int makePacket(char*, int, uint8_t, uint32_t, uint32_t, const char*, uint16_t);
bool readPacket(const char*, int, uint8_t&, uint32_t&, uint32_t&, char*, int, uint16_t&);
uint32_t updateCrc32(uint32_t, const char*, int);

static bool sendAck(SOCKET sock, const sockaddr_in& peer, uint32_t id, uint32_t sequence) {
 char ack[20];
 int length = makePacket(ack, sizeof(ack), 2, id, sequence, nullptr, 0);
 if (length == 0) return false;
 return sendto(sock, ack, length, 0,
  reinterpret_cast<const sockaddr*>(&peer), sizeof(peer)) == length;
}

bool receiveFile(SOCKET sock, const char* path) {
 if (path == nullptr || path[0] == '\0') return false;
 if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
  cerr << "The output path already exists\n";
  return false;
 }
 DWORD error = GetLastError();
 if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND) {
  cerr << "Could not check the output path\n";
  return false;
 }

 string partial = string(path) + ".part";
 HANDLE file = CreateFileA(partial.c_str(), GENERIC_WRITE, 0, nullptr,
  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
 if (file == INVALID_HANDLE_VALUE) {
  cerr << "Could not create a new partial file\n";
  return false;
 }

 bool started = false, finished = false;
 sockaddr_in sender{};
 uint32_t transferId = 0, expected = 1, expectedCrc = 0;
 uint32_t crc = 0xffffffffu;
 uint64_t fileSize = 0, written = 0;
 char metadata[12];
 ULONGLONG deadline = GetTickCount64() + 30000;

 while (true) {
  ULONGLONG now = GetTickCount64();
  if (now >= deadline) {
   if (!finished) cerr << "File receive timed out\n";
   break;
  }
  long left = static_cast<long>(deadline - now);
  timeval wait{left / 1000, (left % 1000) * 1000};
  fd_set ready;
  FD_ZERO(&ready);
  FD_SET(sock, &ready);
  int result = select(0, &ready, nullptr, nullptr, &wait);
  if (result == SOCKET_ERROR) {
   cerr << "Socket wait failed\n";
   break;
  }
  if (result == 0) continue;

  char packet[1044], data[1024];
  sockaddr_in peer{};
  int peerSize = sizeof(peer);
  int count = recvfrom(sock, packet, sizeof(packet), 0,
   reinterpret_cast<sockaddr*>(&peer), &peerSize);
  if (count == SOCKET_ERROR) {
   int socketError = WSAGetLastError();
   if (socketError == WSAEMSGSIZE || socketError == WSAECONNRESET || socketError == WSAETIMEDOUT) continue;
   cerr << "Could not receive a packet\n";
   break;
  }

  uint8_t type = 0;
  uint32_t id = 0, sequence = 0;
  uint16_t length = 0;
  if (!readPacket(packet, count, type, id, sequence, data, sizeof(data), length)) continue;

  if (!started) {
   if (type != 3 || sequence != 0 || length != 12) continue;
   uint64_t size = 0;
   uint32_t checksum = 0;
   for (int i = 0; i < 8; i++) size = (size << 8) | static_cast<unsigned char>(data[i]);
   for (int i = 8; i < 12; i++) checksum = (checksum << 8) | static_cast<unsigned char>(data[i]);
   if (size > 1073741824ull) continue;

   fileSize = size;
   expectedCrc = checksum;
   transferId = id;
   sender = peer;
   for (int i = 0; i < 12; i++) metadata[i] = data[i];
   started = true;
   deadline = GetTickCount64() + 6000;
   cout << "Receiving " << fileSize << " bytes\n";
   sendAck(sock, sender, transferId, 0);
   continue;
  }

  if (peer.sin_addr.s_addr != sender.sin_addr.s_addr || peer.sin_port != sender.sin_port || id != transferId) continue;

  if (finished) {
   if (type == 4 && sequence == expected && length == 0) sendAck(sock, sender, transferId, sequence);
   continue;
  }

  if (type == 3 && sequence == 0 && length == 12) {
   bool same = true;
   for (int i = 0; i < 12; i++) {
    if (metadata[i] != data[i]) same = false;
   }
   if (same) sendAck(sock, sender, transferId, 0);
   continue;
  }

  if (type == 1) {
   if (sequence > 0 && sequence < expected) {
    sendAck(sock, sender, transferId, sequence);
    continue;
   }
   if (sequence != expected || written >= fileSize) continue;
   uint64_t remaining = fileSize - written;
   uint16_t wanted = static_cast<uint16_t>(remaining > 1024 ? 1024 : remaining);
   if (length != wanted) continue;

   DWORD stored = 0;
   if (!WriteFile(file, data, length, &stored, nullptr) || stored != length) {
    cerr << "Could not write the file\n";
    break;
   }
   crc = updateCrc32(crc, data, length);
   written += length;
   expected++;
   deadline = GetTickCount64() + 6000;
   sendAck(sock, sender, transferId, sequence);
   continue;
  }

  if (type == 4 && sequence == expected && length == 0) {
   if (written != fileSize || ~crc != expectedCrc) {
    cerr << "File size or checksum does not match\n";
    break;
   }
   bool flushed = FlushFileBuffers(file) != 0;
   bool closed = CloseHandle(file) != 0;
   file = INVALID_HANDLE_VALUE;
   if (!flushed || !closed || !MoveFileA(partial.c_str(), path)) {
    cerr << "Could not save the completed file\n";
    break;
   }

   finished = true;
   deadline = GetTickCount64() + 3500;
   cout << "File received: " << written << " bytes\n";
   sendAck(sock, sender, transferId, sequence);
  }
 }

 if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
 if (!finished) DeleteFileA(partial.c_str());
 return finished;
}
