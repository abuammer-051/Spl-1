#include <iostream>
#include <fstream>
#include <windows.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: binary_copy <source> <destination>\n";
        return 2;
    }

    ifstream source(argv[1], ios::binary);
    if (!source) {
        cerr << "Could not open the source file\n";
        return 2;
    }

    DWORD attributes = GetFileAttributesA(argv[2]);
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        cerr << "The destination already exists\n";
        return 2;
    }

    DWORD error = GetLastError();
    if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND) {
        cerr << "Could not check the destination: " << error << '\n';
        return 2;
    }

    ofstream destination(argv[2], ios::binary);
    if (!destination) {
        cerr << "Could not create the destination file\n";
        return 2;
    }

    char buffer[1024];
    long long totalBytes = 0;

    while (true) {
        source.read(buffer, sizeof(buffer));
        streamsize bytesRead = source.gcount();

        if (source.bad() || (source.fail() && !source.eof())) {
            cerr << "Could not read the source file\n";
            return 2;
        }

        if (bytesRead > 0) {
            destination.write(buffer, bytesRead);
            if (!destination) {
                cerr << "Could not write the destination file\n";
                return 2;
            }
            totalBytes += bytesRead;
        }

        if (source.eof()) {
            break;
        }
    }

    destination.close();
    if (!destination) {
        cerr << "Could not finish writing the destination file\n";
        return 2;
    }

    cout << "Copied " << totalBytes << " bytes\n";
    return 0;
}
