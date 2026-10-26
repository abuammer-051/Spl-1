#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <iostream>

using namespace std;

bool receiveFile(SOCKET sock, const char* path);

int makePacket(char* buffer, int capacity, uint8_t type, uint32_t transferId,
               uint32_t sequence, const char* data, uint16_t dataLength);
bool readPacket(const char* buffer, int packetLength, uint8_t& type,
                uint32_t& transferId, uint32_t& sequence, char* data,
                int capacity, uint16_t& dataLength);

int main(int argc, char* argv[]) {
    if (argc != 1 && argc != 2) {
        cerr << "Usage: receiver [output file]\n";
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

    SOCKET receiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiverSocket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(PORT);
    receiverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(receiverSocket,
             reinterpret_cast<sockaddr*>(&receiverAddress),
             sizeof(receiverAddress)) == SOCKET_ERROR) {
        cerr << "bind failed: " << WSAGetLastError() << '\n';
        closesocket(receiverSocket);
        WSACleanup();
        return 1;
    }

    if (argc == 2) {
        cout << "Receiver waiting for a file on 127.0.0.1:" << PORT << "..." << endl;
        bool received = receiveFile(receiverSocket, argv[1]);
        closesocket(receiverSocket);
        WSACleanup();
        return received ? 0 : 1;
    }

    cout << "Receiver waiting on 127.0.0.1:" << PORT
         << "..." << endl;

    char buffer[BUFFER_SIZE];
    sockaddr_in senderAddress{};
    int senderAddressLength = sizeof(senderAddress);
    int receivedBytes = recvfrom(
        receiverSocket, buffer, BUFFER_SIZE, 0,
        reinterpret_cast<sockaddr*>(&senderAddress), &senderAddressLength);

    if (receivedBytes == SOCKET_ERROR) {
        cerr << "recvfrom failed: " << WSAGetLastError() << '\n';
        closesocket(receiverSocket);
        WSACleanup();
        return 1;
    }

    uint8_t type = 0;
    uint32_t transferId = 0;
    uint32_t sequence = 0;
    uint16_t dataLength = 0;
    char data[1024];

    if (!readPacket(buffer, receivedBytes, type, transferId, sequence,
                    data, sizeof(data), dataLength)) {
        cerr << "Invalid or corrupted data packet\n";
        closesocket(receiverSocket);
        WSACleanup();
        return 1;
    }

    if (type != 1) {
        cerr << "Expected a data packet\n";
        closesocket(receiverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Received packet " << sequence << " (" << dataLength << " data bytes): ";
    cout.write(data, dataLength);
    cout << '\n';

    char reply[BUFFER_SIZE];
    int replyLength = makePacket(reply, BUFFER_SIZE, 2, transferId, sequence,
                                 nullptr, 0);
    if (replyLength == 0) {
        cerr << "Could not create the acknowledgement\n";
        closesocket(receiverSocket);
        WSACleanup();
        return 1;
    }

    int sentBytes = sendto(
        receiverSocket, reply, replyLength, 0,
        reinterpret_cast<sockaddr*>(&senderAddress), senderAddressLength);

    if (sentBytes == SOCKET_ERROR) {
        cerr << "sendto failed: " << WSAGetLastError() << '\n';
    } else if (sentBytes != replyLength) {
        cerr << "Unexpected reply byte count\n";
    } else {
        cout << "ACK sent. Receiver finished.\n";
    }

    closesocket(receiverSocket);
    WSACleanup();
    return sentBytes == replyLength ? 0 : 1;
}
