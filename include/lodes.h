#pragma once

#include <string>

struct Lode
{
    std::string name;
    uint8_t blockID;

    // Noise Settings
    float scale;
    float threshold;
    float offset;

    // Gen Settings
    uint8_t minHeight;
    uint8_t maxHeight;

    Lode(std::string name, uint8_t blockID, float scale, float threshold, float offset, uint8_t minHeight, uint8_t maxHeight)
    {
        this->name = name;
        this->blockID = blockID;
        this->scale = scale;
        this->threshold = threshold;
        this->offset = offset;
        this->minHeight = minHeight;
        this->maxHeight = maxHeight;
    }

    bool operator!=(Lode other)
    {
        return 
        (
            other.name != name || 
            other.blockID != blockID || 
            other.scale != scale || 
            other.threshold != threshold || 
            other.offset != offset || 
            other.minHeight != minHeight || 
            other.maxHeight != maxHeight
        );
    }
};
