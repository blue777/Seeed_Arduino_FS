/*
    Connect the SD card to the following pins:

    SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
*/

#ifdef KENDRYTE_K210
    #include <SPIClass.h>
#else
    #include <SPI.h>
#endif
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#define SERIAL Serial

#define csPin 4
#ifdef ARDUINO_ARCH_SAMD
    #undef SERIAL Serial
    #define SERIAL SerialUSB
#endif


void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
    SERIAL.print("Listing directory: ");
    SERIAL.println(dirname);

    File root = fs.open(dirname);
    if (!root) {
        SERIAL.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        SERIAL.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            SERIAL.print("  DIR : ");
            SERIAL.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            SERIAL.print("  FILE: ");
            SERIAL.print(file.name());
            SERIAL.print("  SIZE: ");
            SERIAL.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS& fs, const char* path) {
    SERIAL.print("Creating Dir: ");
    SERIAL.println(path);
    if (fs.mkdir(path)) {
        SERIAL.println("Dir created");
    } else {
        SERIAL.println("mkdir failed");
    }
}

void removeDir(fs::FS& fs, const char* path) {
    SERIAL.print("Removing Dir: ");
    SERIAL.println(path);
    if (fs.rmdir(path)) {
        SERIAL.println("Dir removed");
    } else {
        SERIAL.println("rmdir failed");
    }
}

void readFile(fs::FS& fs, const char* path) {
    SERIAL.print("Reading Dir: ");
    SERIAL.println(path);
    File file = fs.open(path);
    if (!file) {
        SERIAL.println("Failed to open file for reading");
        return;
    }

    SERIAL.print("Read from file: ");
    while (file.available()) {
        SERIAL.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
    SERIAL.print("Writing file: ");
    SERIAL.println(path);
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        SERIAL.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        SERIAL.println("File written");
    } else {
        SERIAL.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS& fs, const char* path, const char* message) {
    SERIAL.print("Appending to file: ");
    SERIAL.println(path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        SERIAL.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        SERIAL.println("Message appended");
    } else {
        SERIAL.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS& fs, const char* path1, const char* path2) {
    SERIAL.print("Renaming file ");
    SERIAL.print(path1);
    SERIAL.print(" to ");
    SERIAL.println(path2);
    if (fs.rename(path1, path2)) {
        SERIAL.println("File renamed");
    } else {
        SERIAL.println("Rename failed");
    }
}

void deleteFile(fs::FS& fs, const char* path) {
    SERIAL.print("Deleting file: ");
    SERIAL.println(path);
    if (fs.remove(path)) {
        SERIAL.println("File deleted");
    } else {
        SERIAL.println("Delete failed");
    }
}

void testFileIO(fs::FS& fs, const char* path) {
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file) {
        len = file.size();
        size_t flen = len;
        start = millis();
        while (len) {
            size_t toRead = len;
            if (toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        SERIAL.print(flen);
        SERIAL.print(" bytes read for ");
        SERIAL.print(end);
        SERIAL.println(" ms");
        file.close();
    } else {
        SERIAL.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if (!file) {
        SERIAL.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++) {
        file.write(buf, 512);
    }
    end = millis() - start;
    SERIAL.print(2048 * 512);
    SERIAL.print(" bytes read for ");
    SERIAL.print(end);
    SERIAL.println(" ms");
    file.close();
}

void setup() {
    SERIAL.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (!SERIAL) {};
    while (!SD.begin(csPin, SPI, 12500000)) {
        SERIAL.println("Card Mount Failed");
        return;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        SERIAL.println("No SD card attached");
        return;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);

    SERIAL.print("SD Card Size: ");
    SERIAL.print((uint32_t)cardSize);
    SERIAL.println("MB");

    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    SERIAL.print("Total space: ");
    SERIAL.print((uint32_t)SD.totalBytes() / (1024 * 1024));
    SERIAL.println("MB");
    SERIAL.print("Used space: ");
    SERIAL.print((uint32_t)SD.usedBytes() / (1024 * 1024));
    SERIAL.println("MB");
}

void loop() {

}