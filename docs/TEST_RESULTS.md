# Starter verification - 2026-10-21

Environment: Windows, MSVC 14.51.36231, x64, C++17, Winsock 2.2.

| Check | Result |
| --- | --- |
| Build using scripts/build.bat from a normal shell | PASS; compiler automatically located. |
| /W4 compilation of both C++ programs | PASS; no warnings reported. |
| Hello/reply on 127.0.0.1:5000 | PASS, repeated three times. |
| Nine-byte payload and expected reply | PASS. |
| Second receiver binding to the same endpoint | PASS; bind error, nonzero exit. |
| Sender with receiver absent | PASS; error and bounded termination. |
| Incorrect reply content from the configured endpoint | PASS; rejected. |
| Correct reply text from an unexpected port | PASS; rejected. |
| A listening endpoint that does not reply | PASS; receive timeout 10060, nonzero exit. |

Only test processes created for these checks were stopped. No firewall settings
were changed. No LAN, file transfer, reliability, sliding-window or encryption
result is claimed for this milestone.

The automated verification helper used to generate this record lives outside
the student project. The project itself needs only the C++ toolchain and Windows
socket API to build and run the first two programs.
