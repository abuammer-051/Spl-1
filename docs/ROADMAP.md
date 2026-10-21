# Project milestones

| Order | Learn and implement | Completion check |
| --- | --- | --- |
| 1 | UDP hello/reply; current code | Both applications build and exchange text. |
| 2 | Binary file reading/writing | Local byte-for-byte copy of empty, small and binary files. |
| 3 | Packet encoding/decoding, types, sequence numbers, CRC | Round trips match; malformed lengths and corrupt packets are rejected. |
| 4 | Stop-and-Wait, metadata, ACK, timeout, retries, FIN | A file arrives intact; duplicate packets do not append duplicate bytes. |
| 5 | Controlled DATA/ACK loss, corruption, duplication, delay and reordering | Reproducible tests complete correctly or report bounded failure. |
| 6 | Go-Back-N | Cumulative ACKs and retransmission from the oldest outstanding packet work. |
| 7 | Selective Repeat | Receiver buffers out-of-order data; only missing packets are resent. |
| 8 | Supervisor-approved encryption and key/nonce handling | Published algorithm test vectors pass before integration. |
| 9 | Approved additional features | Each selected feature has separate acceptance tests. |
| 10 | Compare protocols, document and present | Verified file output, goodput, elapsed time and retransmissions recorded. |

Keep the user interface command-line based initially. Use a small fixed window
before making sizes configurable. Do not assume UDP is faster than TCP.

For encryption, prefer a reviewed authenticated-encryption design approved by
the supervisor. XOR is not secure encryption. CRC detects accidental corruption
and does not authenticate a peer. Never reuse a key/nonce pair for different
encrypted messages; retransmit the already encoded packet when retrying.

Additional features are subject to scope selection. Suggested early additions
are receiver acceptance, queue/priority and name-conflict handling. Resume,
shared-file browsing and other proposed features follow a working core.
