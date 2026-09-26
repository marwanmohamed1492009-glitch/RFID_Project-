#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>

using namespace std;

vector<string> loadExistingCards(string fileName) {
    vector<string> cards;
    ifstream file(fileName);
    string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (!line.empty()) {
                cards.push_back(line);
            }
        }
        file.close();
    }
    return cards;
}

int main() {
    string fileName = "rfid_cards.txt";
    vector<string> savedCards = loadExistingCards(fileName);
    cout << "Loaded " << savedCards.size() << " existing cards from database." << endl;

    // !!! ركز هنا: غير COM3 لرقم البورت بتاع بوردتك الحقيقي !!!
    string portName = "\\\\.\\COM5";

    HANDLE hSerial = CreateFileA(portName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        cout << "Error: Cannot open COM Port! Make sure AVRDUDESS and PuTTY are closed." << endl;
        system("pause");
        return 1;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        cout << "Error: Cannot get port state!" << endl;
        CloseHandle(hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        cout << "Error: Cannot set port state!" << endl;
        CloseHandle(hSerial);
        return 1;
    }

    cout << "Listening for RFID cards on " << portName << "..." << endl;

    char readBuffer;
    DWORD bytesRead;
    string currentLine = "";

    while (true) {
        if (ReadFile(hSerial, &readBuffer, 1, &bytesRead, NULL) && bytesRead > 0) {
            char ch = readBuffer;
            if (ch != '\r' && ch != '\n') {
                currentLine += ch;
            }
            else if (ch == '\n' && !currentLine.empty()) {
                if (currentLine != "Hello") {
                    auto it = find(savedCards.begin(), savedCards.end(), currentLine);
                    if (it == savedCards.end()) {
                        cout << "New Card Detected & Saved: " << currentLine << endl;
                        ofstream file(fileName, ios::app);
                        if (file.is_open()) {
                            file << currentLine << endl;
                            file.close();
                        }
                        savedCards.push_back(currentLine);
                    }
                    else {
                        cout << "Card [ " << currentLine << " ] already exists. Ignored (Not saved)." << endl;
                    }
                }
                currentLine = "";
            }
        }
    }
    CloseHandle(hSerial);
    return 0;
}
