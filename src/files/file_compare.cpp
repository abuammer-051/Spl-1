#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: file_compare <first file> <second file>\n";
        return 2;
    }

    ifstream firstFile(argv[1], ios::binary);
    ifstream secondFile(argv[2], ios::binary);
    if (!firstFile || !secondFile) {
        cerr << "Could not open both files\n";
        return 2;
    }

    char firstBuffer[1024];
    char secondBuffer[1024];
    long long offset = 0;

    while (true) {
        firstFile.read(firstBuffer, sizeof(firstBuffer));
        streamsize firstCount = firstFile.gcount();

        secondFile.read(secondBuffer, sizeof(secondBuffer));
        streamsize secondCount = secondFile.gcount();

        if (firstFile.bad() || secondFile.bad() ||
            (firstFile.fail() && !firstFile.eof()) ||
            (secondFile.fail() && !secondFile.eof())) {
            cerr << "Could not read both files\n";
            return 2;
        }

        if (firstCount != secondCount) {
            cout << "Files are different: unequal lengths\n";
            return 1;
        }

        for (streamsize i = 0; i < firstCount; i++) {
            if (firstBuffer[i] != secondBuffer[i]) {
                cout << "Files differ at byte " << offset + i << '\n';
                return 1;
            }
        }

        offset += firstCount;
        if (firstFile.eof() && secondFile.eof()) {
            break;
        }
    }

    cout << "Files are identical: " << offset << " bytes\n";
    return 0;
}
