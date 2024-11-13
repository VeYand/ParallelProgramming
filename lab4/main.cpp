#include <format>
#include <iostream>
#include <string>
#include "BMPUtils.h"

struct Args {
    std::string inputFilePath;
    std::string outputFilePath;
    unsigned coreCount;
    std::vector<int> threads;
};

Args ParseArgs(const int argc, char **argv) {
    if (argc <= 4) {
        throw std::invalid_argument(
            std::format(
                "Usage: {} <input-file-path> <output-file-path> <core-count> <first-thread-priority> <second-thread-priority> ...",
                argv[0])
        );
    }

    std::vector<int> threads{};
    for (int i = 4; i < argc; ++i) {
        threads.push_back(std::stoi(argv[i]));
    }

    return {
        argv[1],
        argv[2],
        static_cast<unsigned>(std::stoi(argv[3])),
        threads
    };
}


int main(const int argc, char **argv) {
    try {
        const time_t startTime = std::clock();
        constexpr auto count = 1;
        const auto [inputFilePath, outputFilePath , coreCount, threads] = ParseArgs(argc, argv);
        auto originalBMP = ReadBMP(inputFilePath);
        if (originalBMP.infoHeader.bitCount != 16) {
            throw std::invalid_argument("Bit count must be equals to 16");
        }
        std::cout << "ok? ";
        std::string ok;
        std::cin >> ok;
        BMPImage blurredBMP;
        for (int i = 0; i < count; ++i) {
            blurredBMP = BlurBMP(originalBMP, threads);
        }
        WriteBMP(outputFilePath, blurredBMP);
        std::cout << coreCount << '\t' << threads.size() << '\t' << std::clock() - startTime << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
