#pragma once

#include <string>
#include <vector>

class BlockType
{
public:
    std::vector<unsigned int> textures;
    bool isSolid;
    std::string name;
    bool isTransparent;
    bool isAir;

    BlockType() : textures({}), isSolid(false), name("air"), isAir(true) {}

    BlockType(std::vector<unsigned int> textures, bool isSolid, std::string name, bool isTransparent = false, bool isAir = false)
    {
        this->textures = textures;
        this->isSolid = isSolid;
        this->name = name;
        this->isTransparent = isTransparent;
        this->isAir = isAir;
    }

    bool operator==(BlockType other)
    {
        return other.name == name;
    }

    bool operator!=(BlockType other)
    {
        return other.name != name;
    }

    void setTexture(unsigned int textureID, int index)
    {
        textures[index] = textureID;
    }

    void setTextures(unsigned int textureID)
    {
        for(int i = 0; i < 6; i++)
        {
            textures[i] = textureID;
        }
    }
};
