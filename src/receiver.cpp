#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

using namespace std;

int main() {
    constexpr unsigned short PORT = 5000;
    constexpr int BUFFER_SIZE = 1024;

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

    cout << "Received " << receivedBytes << " bytes: ";
    cout.write(buffer, receivedBytes);
    cout << '\n';

    const char reply[] = "Received";
    const int replyLength = static_cast<int>(sizeof(reply) - 1);
    int sentBytes = sendto(
        receiverSocket, reply, replyLength, 0,
        reinterpret_cast<sockaddr*>(&senderAddress), senderAddressLength);

    if (sentBytes == SOCKET_ERROR) {
        cerr << "sendto failed: " << WSAGetLastError() << '\n';
    } else if (sentBytes != replyLength) {
        cerr << "Unexpected reply byte count\n";
    } else {
        cout << "Reply sent. Receiver finished.\n";
    }

    closesocket(receiverSocket);
    WSACleanup();
    return sentBytes == replyLength ? 0 : 1;
}
