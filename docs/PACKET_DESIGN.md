# Packet design notes - future milestone

The starter applications currently exchange plain text. This file is a planning
record; it does not describe an implemented wire format.

Before implementing packet classes, decide and document:

- Header version, type, transfer ID, sequence number and data length.
- Exact field widths, field order and byte order.
- CRC/checksum coverage and how its own field is treated during calculation.
- ACK meaning: last contiguous packet received, or next expected packet.
- Maximum datagram size and exact length validation rules.
- META, DATA, ACK, FIN and error/request messages.
- Repeated metadata and FIN behaviour when replies are lost.
- File-size limits, safe receiver-controlled paths and incomplete-file handling.
- Sequence wraparound rules and transfer-ID separation.
- For encryption: algorithm, key provisioning, unique nonces, tag validation,
  authenticated header fields, replay handling and retry behaviour.

Encode and decode fields explicitly into byte buffers. Do not send a C++ struct
by copying its raw memory: padding and byte order can make the wire format wrong.

For Selective Repeat with wrapping sequence numbers, the sequence space must
be at least twice the window size. Keep receive and send windows bounded.
