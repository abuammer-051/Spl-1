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

    const char message[] = "Hello UDP";
    const int messageLength = static_cast<int>(sizeof(message) - 1);
    int sentBytes = sendto(
        senderSocket, message, messageLength, 0,
        reinterpret_cast<sockaddr*>(&receiverAddress), sizeof(receiverAddress));

    if (sentBytes == SOCKET_ERROR || sentBytes != messageLength) {
        if (sentBytes == SOCKET_ERROR) {
            cerr << "sendto failed: " << WSAGetLastError() << '\n';
        } else {
            cerr << "Unexpected message byte count\n";
        }
        closesocket(senderSocket);
        WSACleanup();
        return 1;
    }
    cout << "Sent " << sentBytes << " bytes: " << message << '\n';

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
        const char expectedReply[] = "Received";
        success = receivedBytes == static_cast<int>(sizeof(expectedReply) - 1);
        for (int i = 0; success && i < receivedBytes; ++i) {
            if (buffer[i] != expectedReply[i]) {
                success = false;
            }
        }

        if (success) {
            cout << "Reply: ";
            cout.write(buffer, receivedBytes);
            cout << "\nUDP hello exchange completed.\n";
        } else {
            cerr << "Unexpected reply content\n";
        }
    }

    closesocket(senderSocket);
    WSACleanup();
    return success ? 0 : 1;
}
