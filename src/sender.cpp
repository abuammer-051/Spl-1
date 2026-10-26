#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <iostream>

using namespace std;

bool sendFile(SOCKET sock, const sockaddr_in& receiver, const char* path);

int makePacket(char* buffer, int capacity, uint8_t type, uint32_t transferId,
               uint32_t sequence, const char* data, uint16_t dataLength);
bool readPacket(const char* buffer, int packetLength, uint8_t& type,
                uint32_t& transferId, uint32_t& sequence, char* data,
                int capacity, uint16_t& dataLength);

int main(int argc, char* argv[]) {
    if (argc != 1 && argc != 2) {
        cerr << "Usage: sender [input file]\n";
        return 1;
    }
    constexpr unsigned short PORT = 5000;
    constexpr int BUFFER_SIZE = 1044;

    WSADATA winsockData{};
    int startupResult = WSAStartup(MAKEWORD(2, 2), &winsockData);
    if (startupResult != 0) {
        cerr << "WSAStartup failed: " << startupResult << '\n';
        return 1;
    }

    SOCKET senderSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (senderSocket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    DWORD timeoutMs = 3000;
    if (setsockopt(senderSocket, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeoutMs),
                   sizeof(timeoutMs)) == SOCKET_ERROR) {
        cerr << "timeout setup failed: " << WSAGetLastError() << '\n';
        closesocket(senderSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(PORT);
    receiverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (argc == 2) {
        bool sent = sendFile(senderSocket, receiverAddress, argv[1]);
        closesocket(senderSocket);
        WSACleanup();
        return sent ? 0 : 1;
    }

    const char message[] = "Hello UDP";
    const uint16_t messageLength = static_cast<uint16_t>(sizeof(message) - 1);
    const uint32_t transferId = GetCurrentProcessId();
    const uint32_t sequence = 0;
    char packet[BUFFER_SIZE];
    int packetLength = makePacket(packet, BUFFER_SIZE, 1, transferId, sequence,
                                  message, messageLength);
    if (packetLength == 0) {
        cerr << "Could not create the data packet\n";
        closesocket(senderSocket);
        WSACleanup();
        return 1;
    }

    int sentBytes = sendto(
        senderSocket, packet, packetLength, 0,
        reinterpret_cast<sockaddr*>(&receiverAddress), sizeof(receiverAddress));

    if (sentBytes == SOCKET_ERROR || sentBytes != packetLength) {
        if (sentBytes == SOCKET_ERROR) {
            cerr << "sendto failed: " << WSAGetLastError() << '\n';
        } else {
            cerr << "Unexpected message byte count\n";
        }
        closesocket(senderSocket);
        WSACleanup();
        return 1;
    }
    cout << "Sent packet " << sequence << " (" << messageLength
         << " data bytes): " << message << '\n';

    char buffer[BUFFER_SIZE];
    sockaddr_in replyAddress{};
    int replyAddressLength = sizeof(replyAddress);
    int receivedBytes = recvfrom(
        senderSocket, buffer, BUFFER_SIZE, 0,
        reinterpret_cast<sockaddr*>(&replyAddress), &replyAddressLength);

    bool success = false;
    if (receivedBytes == SOCKET_ERROR) {
        cerr << "recvfrom failed: " << WSAGetLastError()
             << ". Start the receiver first.\n";
    } else if (replyAddress.sin_addr.s_addr != receiverAddress.sin_addr.s_addr ||
               replyAddress.sin_port != receiverAddress.sin_port) {
        cerr << "Reply came from an unexpected endpoint\n";
    } else {
        uint8_t replyType = 0;
        uint32_t replyTransferId = 0;
        uint32_t replySequence = 0;
        uint16_t replyLength = 0;
        char replyData[1024];
        bool validPacket = readPacket(buffer, receivedBytes, replyType,
            replyTransferId, replySequence, replyData, sizeof(replyData), replyLength);

        if (!validPacket) {
            cerr << "Invalid or corrupted reply packet\n";
        } else if (replyType != 2 || replyTransferId != transferId ||
                   replySequence != sequence || replyLength != 0) {
            cerr << "Unexpected acknowledgement\n";
        } else {
            success = true;
            cout << "ACK received for packet " << sequence << '\n';
            cout << "UDP packet exchange completed.\n";
        }
    }

    closesocket(senderSocket);
    WSACleanup();
    return success ? 0 : 1;
}
