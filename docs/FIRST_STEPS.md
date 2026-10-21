# Understand and run the first two programs

## 1. Begin with receiver.cpp

Read these operations in order:

1. `WSAStartup`: initialize Windows socket services. Its return value is the
   error code when startup fails.
2. `socket`: request an IPv4 UDP endpoint from Windows.
3. `sockaddr_in`: describe the address and port. `htons` and `htonl` convert
   the port and address to network byte order.
4. `bind`: listen on this computer's 127.0.0.1, port 5000.
5. `recvfrom`: wait for one datagram and obtain its sender address.
6. `cout.write`: display exactly the received byte count. No null terminator
   is assumed in the incoming bytes.
7. `sendto`: return `Received` to the sender address obtained in step 5.
8. `closesocket` and `WSACleanup`: release resources.

`reinterpret_cast` adapts the address pointer for the API parameter. It does
not serialize a packet or encrypt data.

## 2. Then read sender.cpp

It initializes Winsock, creates a UDP socket and configures a 3000 ms receive
timeout. It sends nine bytes (`Hello UDP`, excluding the string's final null
character) to 127.0.0.1:5000 and waits for a reply.

Windows chooses the sender's local port automatically. The sender checks the
reply address, port and expected text with a manual loop. These checks are not
cryptographic authentication. There is no retry or file transfer yet.

## 3. Build and run

Follow the commands in the root README. The build script links `Ws2_32.lib`:
headers provide declarations; the import library resolves the Winsock calls
at link time. Both programs need that linker input.

## 4. Make one small change

Change `Hello UDP` to another short English message, rebuild, and observe the
new received byte count. The `sizeof(message) - 1` expression excludes the
string terminator and adapts to the new literal length.

Do not modify multiple networking settings together. Confirm localhost works
before attempting a different computer.

## Common errors

| Error | First check |
| --- | --- |
| cl not found | Use the build script or an x64 Native Tools Command Prompt. |
| winsock2.h missing | Check Windows SDK installation and developer environment. |
| unresolved WSAStartup/sendto | Check the Ws2_32.lib linker input. |
| bind 10048 | Another process may already use port 5000. Stop your earlier receiver. |
| recvfrom 10060 | Reply timed out. Start the receiver first. |
| recvfrom 10054 | UDP may report ICMP port-unreachable after a send to a closed port. |
| executable cannot be overwritten | Stop your running application, then rebuild. |

`sendto` succeeding does not guarantee delivery. A receiver can wait indefinitely
in this exercise; Ctrl+C stops it. The sender exits with an error when a reply
is unavailable or unexpected.
