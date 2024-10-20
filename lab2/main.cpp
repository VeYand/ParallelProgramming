#include <format>
#include <iostream>
#include <string>
#include "BMPUtils.h"

struct Args {
    std::string inputFilePath;
    std::string outputFilePath;
    unsigned threadCount;
    unsigned coreCount;
};

Args ParseArgs(const int argc, char **argv) {
    if (argc != 5) {
        throw std::invalid_argument(
            std::format("Usage: {} <input-file-path> <output-file-path> <thread-count> <core-count>", argv[0])
        );
    }

    return {
        argv[1],
        argv[2],
        static_cast<unsigned>(std::stoi(argv[3])),
        static_cast<unsigned>(std::stoi(argv[4]))
    };
}


int main(const int argc, char **argv) {
    const auto [inputFilePath, outputFilePath, threadCount, coreCount] = ParseArgs(argc, argv);

    try {
        auto originalBMP = ReadBMP(inputFilePath);
        SetCoreCount(coreCount);
        const auto blurredBMP = BlurBMP(originalBMP, threadCount);
        WriteBMP(outputFilePath, blurredBMP);
        std::cout << "BMP image blurred successfully!" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
