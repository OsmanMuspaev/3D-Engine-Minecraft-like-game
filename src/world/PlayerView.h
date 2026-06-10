#pragma once
#include <SFML/Graphics.hpp>
#include "Inventory.h"
#include "TextureManager.h"
#include "Item.h"
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"
#include "../renderer/Renderer.h"

class PlayerView {
public:
    enum class CameraViewType {
        FirstPerson,
        ThirdPersonBack,
        ThirdPersonFront,
        Inventory
    };

    PlayerView();

    void loadTextures(const std::string& assetsPath);

    void renderHand(Renderer& renderer, const Vector3& cameraPos,
                    const Vector3& cameraForward, const Inventory& inventory,
                    const TextureManager& texMgr, const ItemRegistry& itemReg,
                    const Matrix4x4& view, const Matrix4x4& proj,
                    float deltaTime, bool isMoving);

    void renderPlayer(Renderer& renderer, const Vector3& position,
                      const Vector3& forward, float yaw, float pitch,
                      const Matrix4x4& view, const Matrix4x4& proj,
                      const Vector3& cameraPos,
                      bool isMoving, float deltaTime, CameraViewType viewType);

    void onLeftClick() { m_leftClickTime = 0.25f; }
    void onRightClick() { m_rightClickTime = 0.25f; }

private:
    struct MeshData {
        std::vector<Vector3> verts;
        std::vector<unsigned int> indices;
        std::vector<sf::Vector2f> uvs;
        std::vector<sf::Color> tints;
    };

    sf::Texture m_skinTex;
    sf::Texture m_blockAtlasTex;
    sf::Texture m_itemAtlasTex;
    bool m_skinLoaded = false;
    bool m_blockAtlasLoaded = false;
    bool m_itemAtlasLoaded = false;

    float m_swingTime = 0.0f;
    float m_walkCycle = 0.0f;

    float m_swingSpeed = 0.0f;
    float m_swingAmplitude = 0.0f;

    float m_leftClickTime = 0.0f;
    float m_rightClickTime = 0.0f;

    void buildBox(MeshData& mesh,
                  const Vector3& min, const Vector3& max,
                  float fu0, float fv0, float fu1, float fv1,
                  float bu0, float bv0, float bu1, float bv1,
                  float tu0, float tv0, float tu1, float tv1,
                  float du0, float dv0, float du1, float dv1,
                  float ru0, float rv0, float ru1, float rv1,
                  float lu0, float lv0, float lu1, float lv1);

    void buildBlockCube(MeshData& mesh, const Vector3& min, const Vector3& max,
                        unsigned int tileIndex, const TextureManager& texMgr);

    void buildHeldItemCube(MeshData& mesh, const Vector3& min, const Vector3& max,
                           const ItemStack& stack, const Item& item,
                           const TextureManager& texMgr);

    void buildOrientedBox(MeshData& mesh,
                          const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3,
                          const Vector3& p4, const Vector3& p5, const Vector3& p6, const Vector3& p7,
                          float fu0, float fv0, float fu1, float fv1,
                          float bu0, float bv0, float bu1, float bv1,
                          float tu0, float tv0, float tu1, float tv1,
                          float du0, float dv0, float du1, float dv1,
                          float ru0, float rv0, float ru1, float rv1,
                          float lu0, float lv0, float lu1, float lv1);

    void buildHeldItemCubeOriented(MeshData& mesh,
                                   const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3,
                                   const Vector3& p4, const Vector3& p5, const Vector3& p6, const Vector3& p7,
                                   const ItemStack& stack, const Item& item,
                                   const TextureManager& texMgr);
};
