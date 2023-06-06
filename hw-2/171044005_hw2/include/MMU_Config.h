#ifndef HW_2_MMU_CONFIG_H
#define HW_2_MMU_CONFIG_H

#include <cstdint>
#include <string>


class MMU_Config {
    friend class MMU;


public:
    enum ReplacementAlgorithm {
        NONE = 0, SC, LRU, WSC
    };


private:
    MMU_Config();

    uint32_t frameSize{};
    uint32_t numPhysical{};
    uint32_t numVirtual{};
    ReplacementAlgorithm algo;
    std::string path;
    uint32_t printThreshold{};

public:

    ~MMU_Config();

    class Builder {
    private:
        uint32_t frameSize = 0;
        uint32_t numPhysical = 0;
        uint32_t numVirtual = 0;
        ReplacementAlgorithm algo = NONE;
        size_t printThreshold = 0;
        std::string path;

    public:
        Builder &setFrameSize(uint32_t frameSize) {
            this->frameSize = frameSize;
            return *this;
        }

        Builder &setNumPhysical(uint32_t numPhysical) {
            this->numPhysical = numPhysical;
            return *this;
        }

        Builder &setNumVirtual(uint32_t numVirtual) {
            this->numVirtual = numVirtual;
            return *this;
        }

        Builder &setAlgo(ReplacementAlgorithm algo) {
            this->algo = algo;
            return *this;
        }


        Builder &setPath(const std::string &path) {
            this->path = path;
            return *this;
        }

        MMU_Config build() {
            return MMU_Config(frameSize, numPhysical, numVirtual, algo, printThreshold, path);
        }

        Builder &setPrintThreshold(uint32_t printThreshold) {
            this->printThreshold = printThreshold;
            return *this;
        }
    };

    MMU_Config(uint32_t frameSize, uint32_t numPhysical, uint32_t numVirtual, ReplacementAlgorithm algo,
               uint32_t printThreshold,
               const std::string &path)
            : frameSize(frameSize), numPhysical(numPhysical), numVirtual(numVirtual), algo(algo), printThreshold(printThreshold),
              path(path) {}
};

#endif //HW_2_MMU_CONFIG_H
