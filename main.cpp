#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "toojpeg.h"

using namespace std;

// returns the data of the block or nullptr if an error occurred
char* getCaffBlock(ifstream & file, char & id, unsigned long long & length) {
    // read block id and check if its valid
    file >> id;
    if (id != 1 and id != 2 and id != 3)
        return nullptr;

    // get block length field
    file.read(reinterpret_cast<char*>(&length), 8);

    // read data
    try {
        char* data = new char[length];
        file.read(data, length);
        return data;
    } catch (std::bad_alloc &e) {   // memory allocation failed
        return nullptr;
    }
}

std::ofstream outputFile;
// callback function for jpeg writer
void fileOutput (unsigned char byte) {
    outputFile << byte;
}

int parseCiff(char* data, unsigned long long maxLength) {
    // check
    if (maxLength < 37) {
        return -1;
    }
    if (strncmp(data, "CIFF", 4) != 0) {
        return -1;
    }

    // read numeric headers
    unsigned long long headerLength;
    memcpy(&headerLength, data + 4, sizeof(long long));
    unsigned long long contentSize;
    memcpy(&contentSize, data + 12, sizeof(long long));
    unsigned long long width;
    memcpy(&width, data + 20, sizeof(long long));
    //cout << width;
    unsigned long long height;
    memcpy(&height, data + 28, sizeof(long long));
    //cout << "*" << height << endl;

    if (contentSize != width * height * 3) {
        return -1;
    }

    // read caption
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
    //cout << caption << endl;

    // read tags portion
    int tagsPosition = 36 + i + 1;
    char* tagsData = data + tagsPosition;
    for (i = 0; i < headerLength - tagsPosition; i++) {
        currentChar = tagsData[i];
        /*if (currentChar != '\0')
            cout << currentChar;
        else
            cout << endl;*/
    }
    if (currentChar != '\0') {
        cout << "Tags field not properly terminated!";
        return -1;
    }

    // determine start position of pixel data in array
    int dataPosition = tagsPosition + i;

    try {
        // copy image data to separate array
        auto image = new unsigned char[contentSize];
        for (int j = 0; j < contentSize; j++) {
            image[j] = data[dataPosition + j];
        }

        // write out jpeg file
        bool success = TooJpeg::writeJpeg(fileOutput, image, width, height);

        delete[] image;

        outputFile.close();
        // determine return value, which should be 0 on success
        return success ? 0 : -1;
    }
    catch (std::bad_alloc &e) {     // memory allocation failed
        outputFile.close();
        return -1;
    }
}

int parseCaff(char* filePathArg) {
    ifstream file(filePathArg, ios::in | ios::binary);
    if (!file) {
        cout << "Cannot open file!" << endl;
        return -1;
    }

    char* data;
    char id;
    unsigned long long length;

    // read blocks one-by-one

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

    // get flag from arguments
    char* flag = new char[6];
    strncpy(flag, argv[1], 5);
    flag[5] = '\0';

    // get input filename (and path)
    string jpegFileName = "";
    char currentChar = *argv[2];
    for (int i = 1; currentChar != '\0'; i++) {
        jpegFileName += currentChar;
        currentChar = *(argv[2] + i);
    }
    // calculate output filename from input
    size_t dotPos = jpegFileName.find_last_of('.', jpegFileName.size() - 1);
    jpegFileName.resize(dotPos);
    if (jpegFileName.empty()) {
        delete[] flag;
        return -1;
    }
    jpegFileName += ".jpg";
    //cout << jpegFileName << endl;
    outputFile = std::ofstream (jpegFileName.c_str(), std::ios_base::out | std::ios_base::binary);

    if (strcmp(flag, "-caff") == 0) {
        delete[] flag;
        return parseCaff(argv[2]);
    }
    if (strcmp(flag, "-ciff") == 0) {
        delete[] flag;
        ifstream file(argv[2], ios::in | ios::binary);
        if (!file) {
            cout << "Cannot open file!" << endl;
            return -1;
        }

        // determine file size
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // read whole file
        char* fileData = new char[fileSize];
        file.read(fileData, fileSize);

        parseCiff(fileData, fileSize);

        file.close();
        if (!file.good()) {
            return -1;
        }
    }

    delete[] flag;

    return -1;
}
