#ifndef CLUSTERCLEANING_H
#define CLUSTERCLEANING_H

#include <stdint.h>
#include <vector>
#include "TROOT.h"

class ClusterCleaning{
public:
    explicit ClusterCleaning(std::vector<uint16_t> ampls);
    virtual ~ClusterCleaning();

    bool passClusterCleaning(int crosstalkInv = 0, uint16_t* exitCode = nullptr);

private:
    std::vector<uint16_t> amplitudes_;
    int crosstalkInv_;
    uint16_t* exitcode_ = nullptr;
    bool isClusterCleaning_;
};

#endif
