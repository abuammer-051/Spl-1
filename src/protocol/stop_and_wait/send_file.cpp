#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <fstream>
#include <iostream>

using namespace std;

int makePacket(char*, int, uint8_t, uint32_t, uint32_t, const char*, uint16_t);
bool readPacket(const char*, int, uint8_t&, uint32_t&, uint32_t&, char*, int, uint16_t&);
uint32_t updateCrc32(uint32_t, const char*, int);

static bool sendAndWait(SOCKET sock, const sockaddr_in& receiver, uint8_t type,
 uint32_t id, uint32_t sequence, const char* data, uint16_t length, int& retries) {
 char packet[1044];
 int size = makePacket(packet, sizeof(packet), type, id, sequence, data, length);
 if (size == 0) return false;

 for (int attempt = 0; attempt < 6; attempt++) {
  if (attempt > 0) {
   retries++;
   cout << "Retry packet " << sequence << '\n';
  }

  int sent = sendto(sock, packet, size, 0,
   reinterpret_cast<const sockaddr*>(&receiver), sizeof(receiver));
  if (sent != size) {
   cerr << "Could not send packet: " << WSAGetLastError() << '\n';
   return false;
  }

  ULONGLONG deadline = GetTickCount64() + 500;
  while (GetTickCount64() < deadline) {
   ULONGLONG now = GetTickCount64();
   if (now >= deadline) break;
   long left = static_cast<long>(deadline - now);
   timeval wait{left / 1000, (left % 1000) * 1000};
   fd_set ready;
   FD_ZERO(&ready);
   FD_SET(sock, &ready);
   int result = select(0, &ready, nullptr, nullptr, &wait);
   if (result == SOCKET_ERROR) return false;
   if (result == 0) break;

   char reply[1044], payload[1024];
   sockaddr_in peer{};
   int peerSize = sizeof(peer);
   int count = recvfrom(sock, reply, sizeof(reply), 0,
    reinterpret_cast<sockaddr*>(&peer), &peerSize);
   if (count == SOCKET_ERROR) {
    int error = WSAGetLastError();
    if (error == WSAEMSGSIZE || error == WSAECONNRESET || error == WSAETIMEDOUT) continue;
    return false;
   }
   if (peer.sin_addr.s_addr != receiver.sin_addr.s_addr || peer.sin_port != receiver.sin_port) continue;

   uint8_t replyType = 0;
   uint32_t replyId = 0, replySequence = 0;
   uint16_t replyLength = 0;
   if (!readPacket(reply, count, replyType, replyId, replySequence,
    payload, sizeof(payload), replyLength)) continue;

   if (replyType == 2 && replyId == id && replySequence == sequence && replyLength == 0) return true;
  }
 }

 cerr << "No ACK for packet " << sequence << '\n';
 return false;
}

bool sendFile(SOCKET sock, const sockaddr_in& receiver, const char* path) {
 ifstream file(path, ios::binary);
 if (!file) {
  cerr << "Could not open the input file\n";
  return false;
 }

 char data[1024];
 uint64_t fileSize = 0;
 uint32_t crc = 0xffffffffu;
 while (true) {
  file.read(data, sizeof(data));
  int count = static_cast<int>(file.gcount());
  if (file.bad() || (file.fail() && !file.eof())) {
   cerr << "Could not read the input file\n";
   return false;
  }
  fileSize += count;
  if (fileSize > 1073741824ull) {
   cerr << "Maximum file size is 1 GB\n";
   return false;
  }
  crc = updateCrc32(crc, data, count);
  if (file.eof()) break;
 }

 file.clear();
 file.seekg(0);
 if (!file) return false;

 char metadata[12];
 for (int i = 0; i < 8; i++) metadata[i] = static_cast<char>((fileSize >> (56 - 8 * i)) & 0xffu);
 crc = ~crc;
 for (int i = 0; i < 4; i++) metadata[8 + i] = static_cast<char>((crc >> (24 - 8 * i)) & 0xffu);

 uint32_t id = GetCurrentProcessId();
 uint32_t sequence = 0;
 int retries = 0;
 ULONGLONG start = GetTickCount64();
 if (!sendAndWait(sock, receiver, 3, id, sequence, metadata, sizeof(metadata), retries)) return false;

 uint64_t sentBytes = 0;
 sequence = 1;
 while (true) {
  file.read(data, sizeof(data));
  int count = static_cast<int>(file.gcount());
  if (file.bad() || (file.fail() && !file.eof())) {
   cerr << "Could not read the input file\n";
   return false;
  }

  if (count > 0) {
   if (sentBytes + count > fileSize) {
    cerr << "Input file changed during transfer\n";
    return false;
   }
   if (!sendAndWait(sock, receiver, 1, id, sequence, data, static_cast<uint16_t>(count), retries)) return false;
   sentBytes += count;
   sequence++;
  }
  if (file.eof()) break;
 }

 if (sentBytes != fileSize) {
  cerr << "Input file changed during transfer\n";
  return false;
 }
 if (!sendAndWait(sock, receiver, 4, id, sequence, nullptr, 0, retries)) return false;

 cout << "File sent: " << sentBytes << " bytes\n";
 cout << "Retransmissions: " << retries << '\n';
 cout << "Time: " << GetTickCount64() - start << " ms\n";
 return true;
}
