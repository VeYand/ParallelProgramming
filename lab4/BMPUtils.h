#pragma once
#include <chrono>
#include <cmath>
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <windows.h>

#pragma pack(push, 1) // Выравнивание по 1 байту

struct BMPFileHeader {
    uint16_t fileType; // Тип файла (должен быть 'BM')
    uint32_t fileSize; // Размер файла в байтах
    uint16_t reserved1; // Зарезервировано
    uint16_t reserved2; // Зарезервировано
    uint32_t offsetData; // Смещение до начала данных изображения
};

struct BMPInfoHeader {
    uint32_t size; // Размер этого заголовка
    int32_t width; // Ширина изображения
    int32_t height; // Высота изображения
    uint16_t planes; // Количество цветовых плоскостей (должно быть 1)
    uint16_t bitCount; // Количество бит на пиксель
    uint32_t compression; // Тип сжатия
    uint32_t sizeImage; // Размер данных изображения
    int32_t xPixelsPerMeter; // Горизонтальное разрешение
    int32_t yPixelsPerMeter; // Вертикальное разрешение
    uint32_t colorsUsed; // Количество используемых цветов
    uint32_t colorsImportant; // Количество важных цветов
};

#pragma pack(pop)

struct BMPImage {
    BMPFileHeader fileHeader{};
    BMPInfoHeader infoHeader{};
    std::vector<uint8_t> pixelData;
};

inline BMPImage ReadBMP(const std::string &filePath) {
    BMPImage bmpImage;

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    file.read(reinterpret_cast<char *>(&bmpImage.fileHeader), sizeof(bmpImage.fileHeader));

    if (bmpImage.fileHeader.fileType != 0x4D42) {
        throw std::runtime_error("Not a valid BMP file.");
    }

    file.read(reinterpret_cast<char *>(&bmpImage.infoHeader), sizeof(bmpImage.infoHeader));

    bmpImage.pixelData.resize(bmpImage.infoHeader.sizeImage);

    file.seekg(bmpImage.fileHeader.offsetData, std::ios::beg);
    file.read(reinterpret_cast<char *>(bmpImage.pixelData.data()), bmpImage.infoHeader.sizeImage);

    return bmpImage;
}

inline void WriteBMP(const std::string &filePath, const BMPImage &bmpImage) {
    std::ofstream file(filePath, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    file.write(reinterpret_cast<const char *>(&bmpImage.fileHeader), sizeof(bmpImage.fileHeader));
    file.write(reinterpret_cast<const char *>(&bmpImage.infoHeader), sizeof(bmpImage.infoHeader));
    file.write(reinterpret_cast<const char *>(bmpImage.pixelData.data()), bmpImage.pixelData.size());
}

struct ThreadData {
    int *index;
    BMPImage *bmpImage;
    BMPImage *blurredImage;
    int startRow;
    int endRow;
    std::ofstream *logFile;
    HANDLE *semaphore;
    std::chrono::time_point<std::chrono::system_clock> *startTime;
};

void SomeLongOperation() {
    for (int i = 0; i < 10; ++i) {
        const auto k = i * i;
        i += k;
    }
}

inline DWORD WINAPI BlurSection(const LPVOID param) {
    const ThreadData *data = static_cast<ThreadData *>(param);
    const auto index = data->index;
    const BMPImage *sourceImage = data->bmpImage;
    BMPImage *blurredImage = data->blurredImage;
    const int startRow = data->startRow;
    const int endRow = data->endRow;
    std::ofstream &logFile = *(data->logFile);
    const HANDLE *semaphore = data->semaphore;

    const int width = sourceImage->infoHeader.width;
    const int height = sourceImage->infoHeader.height;
    const int bytesPerPixel = sourceImage->infoHeader.bitCount / 8;

    for (int y = startRow; y < endRow; ++y) {
        for (int x = 0; x < width; ++x) {
            int redSum = 0, greenSum = 0, blueSum = 0, pixelCount = 0;
            for (int offsetY = -1; offsetY <= 1; ++offsetY) {
                for (int offsetX = -1; offsetX <= 1; ++offsetX) {
                    const int neighborY = y + offsetY;
                    const int neighborX = x + offsetX;

                    if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height) {
                        const int index = (neighborY * width + neighborX) * bytesPerPixel;
                        blueSum += sourceImage->pixelData[index];
                        greenSum += sourceImage->pixelData[index + 1];
                        redSum += sourceImage->pixelData[index + 2];
                        pixelCount++;
                    }
                }
            }

            int currentPixelIndex = (y * width + x) * bytesPerPixel;
            blurredImage->pixelData[currentPixelIndex] = blueSum / pixelCount;
            blurredImage->pixelData[currentPixelIndex + 1] = greenSum / pixelCount;
            blurredImage->pixelData[currentPixelIndex + 2] = redSum / pixelCount;

            if (x % 10 == 0) {
                // SomeLongOperation();
                auto currentTime = std::chrono::high_resolution_clock::now();
                const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - *data->startTime).count();
                // WaitForSingleObject(*semaphore, INFINITE);
                logFile << std::format("{}\t{}\n", *index, elapsedTime);
                // ReleaseSemaphore(*semaphore, 1, nullptr);
            }
        }
    }

    return 0;
}


inline BMPImage BlurBMP(BMPImage &bmpImage, const std::vector<int> &threads) {
    std::ofstream logFile("log.txt");
    std::ofstream log1File("log1.txt");
    std::ofstream log2File("log2.txt");
    std::ofstream log3File("log3.txt");

    HANDLE semaphore = CreateSemaphore(nullptr, 1, 1, nullptr);
    if (semaphore == nullptr) {
        std::cerr << "Error: Unable to create semaphore" << std::endl;
        return bmpImage;
    }


    BMPImage blurredImage = bmpImage;
    std::vector<HANDLE> threadsObj(threads.size());
    std::vector<ThreadData> threadData(threads.size());
    const int rowsPerThread = bmpImage.infoHeader.height / threads.size();

    auto startTime = std::chrono::high_resolution_clock::now();
    int threadIndexes[threads.size()];
    for (unsigned i = 0; i < threads.size(); ++i) {
        threadIndexes[i] = i + 1;
        const int startRow = i * rowsPerThread;
        const int endRow = (i == threads.size() - 1) ? bmpImage.infoHeader.height : startRow + rowsPerThread;

        std::ofstream& stream = (i == 1) ? log2File : (i == 2) ? log3File : log1File;

        threadData[i] = {
            &threadIndexes[i], &bmpImage, &blurredImage, startRow, endRow, &stream, &semaphore, &startTime
        };
        threadsObj[i] = CreateThread(nullptr, 0, BlurSection, &threadData[i], 0, nullptr);

        auto priority = THREAD_PRIORITY_NORMAL;
        if (threads[i] > 0) {
            priority = THREAD_PRIORITY_ABOVE_NORMAL;
        } else if (threads[i] < 0) {
            priority = THREAD_PRIORITY_BELOW_NORMAL;
        }
        SetThreadPriority(threadsObj[i], priority);
    }

    WaitForMultipleObjects(threads.size(), threadsObj.data(), TRUE, INFINITE);

    for (const HANDLE thread: threadsObj) {
        CloseHandle(thread);
    }

    return blurredImage;
}
