#pragma once
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

inline void SetCoreCount(unsigned count) {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    const auto maxCoreCount = info.dwNumberOfProcessors;

    if (count > maxCoreCount) {
        count = maxCoreCount;
    }

    const auto hProcess = GetCurrentProcess();
    const auto mask = static_cast<DWORD_PTR>(std::pow(2, count) - 1);

    SetThreadAffinityMask(hProcess, mask);
}

struct ThreadData {
    BMPImage *bmpImage;
    BMPImage *blurredImage;
    int startRow;
    int endRow;
};

inline DWORD WINAPI BlurSection(const LPVOID param) {
    const ThreadData *data = static_cast<ThreadData *>(param);
    const BMPImage *bmpImage = data->bmpImage;
    BMPImage *blurredImage = data->blurredImage;
    const int startRow = data->startRow;
    const int endRow = data->endRow;

    const int width = bmpImage->infoHeader.width;
    const int height = bmpImage->infoHeader.height;
    const int bytesPerPixel = bmpImage->infoHeader.bitCount / 8;

    for (int y = startRow; y < endRow; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    const int ny = y + ky;
                    if (const int nx = x + kx; nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        const int index = (ny * width + nx) * bytesPerPixel;
                        b += bmpImage->pixelData[index];
                        g += bmpImage->pixelData[index + 1];
                        r += bmpImage->pixelData[index + 2];
                        count++;
                    }
                }
            }
            int index = (y * width + x) * bytesPerPixel;
            blurredImage->pixelData[index] = b / count;
            blurredImage->pixelData[index + 1] = g / count;
            blurredImage->pixelData[index + 2] = r / count;
        }
    }

    return 0;
}

inline BMPImage BlurBMP(BMPImage &bmpImage, const unsigned threadCount) {
    BMPImage blurredImage = bmpImage;
    std::vector<HANDLE> threads(threadCount);
    std::vector<ThreadData> threadData(threadCount);
    const int rowsPerThread = bmpImage.infoHeader.height / threadCount;

    for (unsigned i = 0; i < threadCount; ++i) {
        const int startRow = i * rowsPerThread;
        const int endRow = (i == threadCount - 1) ? bmpImage.infoHeader.height : startRow + rowsPerThread;
        threadData[i] = {&bmpImage, &blurredImage, startRow, endRow};
        threads[i] = CreateThread(nullptr, 0, BlurSection, &threadData[i], 0, nullptr);
    }

    WaitForMultipleObjects(threadCount, threads.data(), TRUE, INFINITE);

    for (const HANDLE thread: threads) {
        CloseHandle(thread);
    }

    return blurredImage;
}
