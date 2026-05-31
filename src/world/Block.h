#pragma once

enum FaceSide {
    FACE_PX = 0, // +X (Право)
    FACE_NX = 1, // -X (Лево)
    FACE_PY = 2, // +Y (Верх)
    FACE_NY = 3, // -Y (Низ)
    FACE_PZ = 4, // +Z (Перед)
    FACE_NZ = 5  // -Z (Зад)
};

enum class BlockType : unsigned int {
    AIR = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
    WOOD = 4,
    LEAVES = 5
};

struct Block {
    BlockType type = BlockType::AIR;

    Block() : type(BlockType::AIR) {}
    Block(BlockType t) : type(t) {}

    bool isSolid() const {
        return type != BlockType::AIR;
    }

    // Централизованная логика текстур для каждого типа блока
    static std::array<unsigned int, 6> getTextures(BlockType type) {
        std::array<unsigned int, 6> texs;
        unsigned int id = static_cast<unsigned int>(type);
        
        // По умолчанию все грани одинаковые (id из enum)
        texs.fill(id);

        switch (type) {
            case BlockType::GRASS:
                texs[FACE_PY] = (13*64) + (28); // Верх: Трава (ID 1)
                texs[FACE_NY] = (8*64) + (23); // Низ: Земля (ID 2)
                // Бока: Трава с землей (допустим, ID 0 в атласе или 6)
                texs[FACE_PX] = texs[FACE_NX] = texs[FACE_PZ] = texs[FACE_NZ] = (11*64) + (27); 
                break;

            case BlockType::WOOD:
                texs[FACE_PY] = texs[FACE_NY] = (19*64) + (10); // Спилы (Кольца)
                texs[FACE_PX] = texs[FACE_NX] = texs[FACE_PZ] = texs[FACE_NZ] = (19*64) + (9); // Кора
                break;

            case BlockType::LEAVES:
                texs.fill((1*64) + (9));
                break;

            case BlockType::STONE:
                texs.fill((27*64) + (21));
                break;

            default:
                break;
        }
        return texs;
    }
};