# Starter milestone manual tests

Run these only against your own starter applications on localhost.

| Test | Action | Expected result |
| --- | --- | --- |
| Build | Run scripts/build.bat | Both applications build without warnings. |
| Hello/reply | Receiver first, then sender in a second terminal | Nine bytes arrive and sender prints Reply: Received. |
| Repeat | Restart receiver and then sender | The exchange succeeds again. |
| Receiver absent | Run sender while no receiver is listening | Nonzero exit and an error; no indefinite wait. |
| Port in use | Start one receiver, then another | The second receiver exits with a bind error. |
| Message length | Change the sender's short message literal and rebuild | The receiver prints the exact new byte count. |

The receiver processes one datagram and exits after sending its reply. The
sender performs one send/receive attempt, not reliable delivery or retries.

During later milestones add empty and binary files, loss/corruption, duplicates,
reordered ACKs, retry exhaustion, interrupted completion, bad encryption tags,
and GBN/SR comparison tests. Record results under docs or experiments/results.
