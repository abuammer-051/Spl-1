# Reliable UDP File Transfer

Software Project Lab I

- Student: Abu Ammer
- Roll: bsse1724
- Supervisor: Dr. Md. Shariful Islam
- Platform: Windows, C++17, Winsock 2.2

## Current milestone: UDP hello exchange

The first two C++ programs send `Hello UDP` to localhost and return `Received`.
Each program completes one exchange and exits. This verifies the compiler,
linker, socket API, and two-terminal workflow.

File transfer, custom packets, reliability, CRC, sliding windows and encryption
are future milestones. The folders reserved for them do not contain working
implementations yet. The reply in this exercise is a plain message, not a
sequence-aware acknowledgement for file data.

## Build

Open PowerShell and run:

```powershell
Set-Location 'E:\SPL-1'
.\scripts\build.bat
```

The build script finds an installed Visual Studio C++ toolchain if `cl` is not
already available. It requires the Desktop development with C++ workload and
a Windows SDK. It does not install anything or permanently modify PATH.

Alternatively, open an x64 Native Tools Command Prompt, change to the project
folder, and run `scripts\build.bat`.

## Run in two terminals

Terminal 1, start the receiver first:

```powershell
Set-Location 'E:\SPL-1'
.\scripts\run_receiver.bat
```

Terminal 2:

```powershell
Set-Location 'E:\SPL-1'
.\scripts\run_sender.bat
```

Expected receiver output:

```text
Receiver waiting on 127.0.0.1:5000...
Received 9 bytes: Hello UDP
Reply sent. Receiver finished.
```

Expected sender output:

```text
Sent 9 bytes: Hello UDP
Reply: Received
UDP hello exchange completed.
```

Restart the receiver before each new test. Press Ctrl+C to stop a waiting
receiver. Both programs use IPv4 localhost and UDP port 5000; no router or
firewall change is needed for this initial exercise.

## Project structure

```text
SPL-1/
  src/
    receiver.cpp                 First receiving program
    sender.cpp                   First sending program
    common/                      Shared constants/types when needed
    transport/                   Future reusable UDP socket handling
    protocol/
      packets/                   Future encoding/decoding and CRC
      stop_and_wait/             First reliable transfer mode
      go_back_n/                 First sliding-window mode
      selective_repeat/          Second sliding-window mode
    files/                       Binary I/O and file verification
    crypto/                      Approved encryption implementation
    features/                    Supervisor-approved extra features
  scripts/
    build.bat                    Build both applications
    run_receiver.bat             Run the receiver
    run_sender.bat               Run the sender
  docs/
    FIRST_STEPS.md               What to understand in the starter code
    ROADMAP.md                   Ordered learning/implementation milestones
    PACKET_DESIGN.md             Future packet design notes
    TEST_RESULTS.md              Recorded starter verification
  tests/
    MANUAL_TESTS.md               Checks for this milestone
    fixtures/                    Small, non-private test files later
  data/
    incoming/                    Completed received files later
    outgoing/                    Files selected for sending later
    partial/                     Incomplete transfers later
  logs/                          Runtime logs later
  experiments/
    results/                     GBN/SR comparison results later
  build/                         Generated compiler/debug files
  bin/                           Generated receiver.exe and sender.exe
  .gitignore
  README.md
```

Only `sender.cpp` and `receiver.cpp` currently contain C++ code. Reserved source
folders contain `.gitkeep` placeholders. Extract shared modules as you learn
and implement them; do not start by writing every future module at once.

## Constraints and scope

Use instructor-approved standard facilities and OS APIs. Implement protocol,
checksums and required data-structure algorithms yourself. Choose an encryption
algorithm and implementation policy with the supervisor before implementing it.
The existing additional-feature list is a menu for scope selection, not a
commitment to implement every listed feature.

## Git workflow

The project uses the `main` branch. The GitHub repository is
https://github.com/abuammer-051/Spl-1, configured locally as `origin`.

Inspect changes and commit a tested milestone:

```text
git status
git diff
git add src scripts docs tests .gitignore README.md
git diff --staged
git commit -m "Add UDP hello exchange"
git push origin main
```

Set your own Git identity if Git requests it. Build products, incoming/outgoing
data and key files are ignored. Do not commit secrets or personal transfer data.

## Next action

Read `docs/FIRST_STEPS.md`, run the two applications yourself, then implement a
binary file-copy exercise before adding file transfer over UDP.
