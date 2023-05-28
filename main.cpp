#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "toojpeg.h"

using namespace std;

// returns the data of the block or nullptr if an error occurred
char* getCaffBlock(ifstream & file, char & id, long long & length) {
    file >> id;
    if (id != 1 and id != 2 and id != 3)
        return nullptr;

    file.read(reinterpret_cast<char*>(&length), 8);

    char* data = new char[length];
    file.read(data, length);

    return data;
}

std::ofstream outputFile;
void fileOutput (unsigned char byte){
    outputFile << byte;
};

int parseCiff(char* data, long long maxLength) {
    if (maxLength < 37) {
        return -1;
    }
    if (strncmp(data, "CIFF", 4) != 0) {
        return -1;
    }

    long long headerLength;
    memcpy(&headerLength, data + 4, sizeof(long long));
    long long contentSize;
    memcpy(&contentSize, data + 12, sizeof(long long));
    //cout << contentSize << endl;
    long long width;
    memcpy(&width, data + 20, sizeof(long long));
    cout << width;
    long long height;
    memcpy(&height, data + 28, sizeof(long long));
    cout << "*" << height << endl;

    if (contentSize != width * height * 3) {
        return -1;
    }

    string caption = "";
    char* captionData = data + 36;
    int i = 0;
    char currentChar = captionData[i];
    while (currentChar != '\n') {
        caption += currentChar;
        if (i >= headerLength - 36)
            return -1;
        i++;
        currentChar = captionData[i];
    }
    cout << caption << endl;

    int tagsPosition = 36 + i + 1;
    char* tagsData = data + tagsPosition;
    for (i = 0; i < headerLength - tagsPosition; i++) {
        currentChar = tagsData[i];
        if (currentChar != '\0')
            cout << currentChar;
        else
            cout << endl;
    }
    if (currentChar != '\0') {
        cout << "Tags field not properly terminated!";
        return -1;
    }

    int dataPosition = tagsPosition + i + 1;

    auto image = new unsigned char[contentSize];
    for (int j = 0; j < contentSize; j++) {
        image[j] = data[dataPosition + j];
    }

    bool success = TooJpeg::writeJpeg(fileOutput, image, width, height);

    outputFile.close();
    return success ? 0 : -1;
}

int parseCaff(char* filePathArg) {
    ifstream file(filePathArg, ios::in | ios::binary);
    if (!file) {
        cout << "Cannot open file!" << endl;
        return -1;
    }

    char* data;
    char id;
    long long length;

    // header
    data = getCaffBlock(file, id, length);
    if (data == nullptr)
        return -1;
    if (strncmp(data, "CAFF", 4) != 0) {
        delete[] data;
        file.close();
        return -1;
    }
    /*long long num_anim;
    memcpy(&num_anim, data + 12, sizeof(long long));
    cout << num_anim << endl;*/
    delete[] data;

    // creator block
    data = getCaffBlock(file, id, length);
    if (data == nullptr)
        return -1;
    delete[] data;

    // animation block
    data = getCaffBlock(file, id, length);
    if (data == nullptr)
        return -1;
    char* ciffData = data + 8;
    int returnValue = parseCiff(ciffData, length - 8);
    delete[] data;

    file.close();
    if (!file.good()) {
        return -1;
    }

    return returnValue;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        return -1;
    }

    // heart :)
    //cout << (char)0x03 << endl;

    char* flag = new char[6];
    strncpy(flag, argv[1], 5);
    flag[5] = '\0';

    string jpegFileName = "";
    char currentChar = *argv[2];
    for (int i = 1; currentChar != '\0'; i++) {
        jpegFileName += currentChar;
        currentChar = *(argv[2] + i);
    }
    size_t dotPos = jpegFileName.find_last_of('.', jpegFileName.size() - 1);
    jpegFileName.resize(dotPos);
    if (jpegFileName.empty())
        return -1;
    jpegFileName += ".jpg";
    cout << jpegFileName << endl;
    outputFile = std::ofstream (jpegFileName.c_str(), std::ios_base::out | std::ios_base::binary);

    if (strcmp(flag, "-caff") == 0) {
        return parseCaff(argv[2]);
    }
    if (strcmp(flag, "-ciff") == 0) {
        ifstream file(argv[2], ios::in | ios::binary);
        if (!file) {
            cout << "Cannot open file!" << endl;
            return -1;
        }

        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        char* fileData = new char[fileSize];
        file.read(fileData, fileSize);

        parseCiff(fileData, fileSize);

        file.close();
        if (!file.good()) {
            return -1;
        }
    }

    return -1;
}
