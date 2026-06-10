#include "PlayerView.h"
#include "../core/Camera.h"
#include <filesystem>
#include <iostream>
#include <cmath>
#include <tuple>

PlayerView::PlayerView() = default;

// Loads the player skin texture and initializes atlas states.
void PlayerView::loadTextures(const std::string& assetsPath) {
    std::string skinPath = assetsPath + "/minecraft/textures/entity/steve.png";

    if (std::filesystem::exists(skinPath)) {
        m_skinLoaded = m_skinTex.loadFromFile(skinPath);
        if (m_skinLoaded) {
            std::cout << "PlayerView: Steve skin loaded from " << skinPath << "\n";
        }
    } else {
        std::cout << "PlayerView: Steve skin not found at " << skinPath << "\n";
    }

    m_blockAtlasLoaded = false;
    m_itemAtlasLoaded = false;
}

// Builds an axis-aligned box mesh with per-face UV coordinates.
void PlayerView::buildBox(MeshData& mesh,
                          const Vector3& min, const Vector3& max,
                          float fu0, float fv0, float fu1, float fv1,
                          float bu0, float bv0, float bu1, float bv1,
                          float tu0, float tv0, float tu1, float tv1,
                          float du0, float dv0, float du1, float dv1,
                          float ru0, float rv0, float ru1, float rv1,
                          float lu0, float lv0, float lu1, float lv1) {

    auto addFace = [&](const Vector3& a, const Vector3& b,
                       const Vector3& c, const Vector3& d,
                       float u0, float v0, float u1, float v1) {

        unsigned int base = (unsigned int)mesh.verts.size();

        mesh.verts.insert(mesh.verts.end(), {a, b, c, d});

        mesh.indices.insert(mesh.indices.end(), {
            base, base + 1, base + 2,
            base, base + 2, base + 3
        });

        mesh.uvs.insert(mesh.uvs.end(), {
            {u0, v1},
            {u1, v1},
            {u1, v0},
            {u0, v0}
        });

        mesh.tints.insert(mesh.tints.end(), 4, sf::Color::White);
    };

    addFace({min.x, min.y, max.z}, {max.x, min.y, max.z},
            {max.x, max.y, max.z}, {min.x, max.y, max.z},
            fu0, fv0, fu1, fv1);

    addFace({max.x, min.y, min.z}, {min.x, min.y, min.z},
            {min.x, max.y, min.z}, {max.x, max.y, min.z},
            bu0, bv0, bu1, bv1);

    addFace({min.x, max.y, max.z}, {max.x, max.y, max.z},
            {max.x, max.y, min.z}, {min.x, max.y, min.z},
            tu0, tv0, tu1, tv1);

    addFace({min.x, min.y, min.z}, {max.x, min.y, min.z},
            {max.x, min.y, max.z}, {min.x, min.y, max.z},
            du0, dv0, du1, dv1);

    addFace({max.x, min.y, max.z}, {max.x, min.y, min.z},
            {max.x, max.y, min.z}, {max.x, max.y, max.z},
            ru0, rv0, ru1, rv1);

    addFace({min.x, min.y, min.z}, {min.x, min.y, max.z},
            {min.x, max.y, max.z}, {min.x, max.y, min.z},
            lu0, lv0, lu1, lv1);
}

// Builds a box mesh using a single block texture atlas tile for all faces.
void PlayerView::buildBlockCube(MeshData& mesh,
                                const Vector3& min, const Vector3& max,
                                unsigned int tileIndex,
                                const TextureManager& texMgr) {

    auto uvs = texMgr.getUV(tileIndex);

    float u0 = uvs[0].x;
    float v0 = uvs[0].y;
    float u1 = uvs[2].x;
    float v1 = uvs[2].y;

    buildBox(mesh, min, max,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1);
}

// Builds a box mesh for a held item, using item atlas for armor or block atlas otherwise.
void PlayerView::buildHeldItemCube(MeshData& mesh,
                                   const Vector3& min, const Vector3& max,
                                   const ItemStack& stack,
                                   const Item& item,
                                   const TextureManager& texMgr) {

    bool isArmorItem = Inventory::isArmor(stack.type);

    unsigned int tileIndex;
    std::array<sf::Vector2f, 4> uvs;

    if (isArmorItem) {
        tileIndex = texMgr.getItemTileIndex(item.textureName);
        uvs = texMgr.getItemUV(tileIndex);
    } else {
        tileIndex = texMgr.getTileIndex(item.textureName);
        uvs = texMgr.getUV(tileIndex);
    }

    float u0 = uvs[0].x, v0 = uvs[0].y;
    float u1 = uvs[2].x, v1 = uvs[2].y;

    buildBox(mesh, min, max,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1,
             u0,v0,u1,v1);
}

// Renders the first-person hand and held item with swing and click animations.
void PlayerView::renderHand(Renderer& renderer,
                            const Vector3& cameraPos,
                            const Vector3& cameraForward,
                            const Inventory& inventory,
                            const TextureManager& texMgr,
                            const ItemRegistry& itemReg,
                            const Matrix4x4& view,
                            const Matrix4x4& proj,
                            float deltaTime,
                            bool isMoving) {

    if (!m_skinLoaded) return;

    // Lazy-load atlas textures from the texture manager
    if (!m_blockAtlasLoaded) {
        (void)m_blockAtlasTex.loadFromImage(texMgr.getImage());
        m_blockAtlasLoaded = true;
    }
    if (!m_itemAtlasLoaded) {
        (void)m_itemAtlasTex.loadFromImage(texMgr.getItemImage());
        m_itemAtlasLoaded = true;
    }

    // Smooth swing animation
    float targetSwingSpeed = isMoving ? 8.0f : 0.0f;
    m_swingSpeed += (targetSwingSpeed - m_swingSpeed) * deltaTime * 5.0f;
    m_swingTime += m_swingSpeed * deltaTime;

    float targetAmplitude = isMoving ? 1.0f : 0.0f;
    m_swingAmplitude += (targetAmplitude - m_swingAmplitude) * deltaTime * 4.0f;

    float swing = std::sin(m_swingTime) * m_swingAmplitude;
    float bobY = std::abs(swing) * 0.04f;

    // Click animation
    float clickAnim = 0.0f;
    if (m_leftClickTime > 0) {
        m_leftClickTime -= deltaTime;
        clickAnim = std::sin(m_leftClickTime * 20.0f) * 0.2f;
    }
    if (m_rightClickTime > 0) {
        m_rightClickTime -= deltaTime;
        clickAnim = std::sin(m_rightClickTime * 20.0f) * 0.15f;
    }

    // Anchor to body position
    Vector3 bodyPos = cameraPos - Vector3(0, PLAYER_HEIGHT, 0);
    float cameraYaw = std::atan2(cameraForward.x, cameraForward.z);

    // Left hand position
    float handX = -0.6f;
    float handY = 1.0f + bobY;
    float handZ = 0.0f + swing * 0.06f + clickAnim;

    // Hand model matrix with swing and click rotations
    Matrix4x4 handModel = Matrix4x4::translation(bodyPos.x, bodyPos.y, bodyPos.z)
                        * Matrix4x4::rotationY(cameraYaw)
                        * Matrix4x4::translation(handX, handY, handZ)
                        * Matrix4x4::rotationX(swing * 0.25f + clickAnim * 0.5f)
                        * Matrix4x4::rotationZ(std::abs(swing) * 0.08f);

    // Render the arm mesh
    MeshData armMesh;
    buildBox(armMesh,
        {-0.15f, -0.25f, -0.25f}, {0.15f, 0.25f, 0.15f},
        44,20,48,32, 52,20,56,32, 44,16,48,20, 48,16,52,20, 40,20,44,32, 48,20,52,32);

    sf::Image skinImg = m_skinTex.copyToImage();
    renderer.drawMesh(armMesh.verts, armMesh.indices, armMesh.uvs, armMesh.tints,
                      skinImg, handModel, view, proj, cameraPos, true);

    // Render held item/block
    const ItemStack& selected = inventory.getSelectedItem();
    if (!selected.isEmpty()) {
        const Item& item = itemReg.getItem(selected.type);

        // Item offset relative to the hand
        float itemOffsetX = 0.0f;
        float itemOffsetY = -0.5f;
        float itemOffsetZ = 0.18f;

        // Item animation parameters
        float itemRotZ = swing * 0.5f;
        float itemRotX = clickAnim * 0.8f;
        float itemSwing = swing * 0.8f;

        // Item model matrix attached to the hand
        Matrix4x4 itemModel = Matrix4x4::translation(bodyPos.x, bodyPos.y, bodyPos.z)
                            * Matrix4x4::rotationY(cameraYaw)
                            * Matrix4x4::translation(handX + itemOffsetX, handY + itemOffsetY, handZ + itemOffsetZ)
                            * Matrix4x4::rotationY(itemSwing)
                            * Matrix4x4::rotationZ(itemRotZ)
                            * Matrix4x4::rotationX(itemRotX);

        MeshData itemMesh;

        // Item size varies by type
        float itemSize = 0.12f;
        if (selected.type >= BlockType::STONE && selected.type <= BlockType::LEATHER_BOOTS) {
            itemSize = 0.11f;
        }

        buildHeldItemCube(itemMesh,
                         {-itemSize, -itemSize, -itemSize},
                         {itemSize, itemSize, itemSize},
                         selected, item, texMgr);

        sf::Image atlasImg = Inventory::isArmor(selected.type)
            ? m_itemAtlasTex.copyToImage()
            : m_blockAtlasTex.copyToImage();

        renderer.drawMesh(itemMesh.verts, itemMesh.indices, itemMesh.uvs, itemMesh.tints,
                         atlasImg, itemModel, view, proj, cameraPos, true);
    }
}

// Renders the full player model (body, head, arms, legs) based on camera view type.
void PlayerView::renderPlayer(Renderer& renderer,
                              const Vector3& position,
                              const Vector3& forward,
                              float yaw,
                              float pitch,
                              const Matrix4x4& view,
                              const Matrix4x4& proj,
                              const Vector3& cameraPos,
                              bool isMoving,
                              float deltaTime,
                              CameraViewType viewType) {

    if (!m_skinLoaded) return;

    sf::Image skinImg = m_skinTex.copyToImage();

    // Configure body and head rotations based on camera view
    float bodyYaw;
    float headPitch;

    switch (viewType) {
        case CameraViewType::ThirdPersonFront:
            bodyYaw = -yaw + 3.14159f / 2.0f;
            headPitch = std::clamp(-pitch, -(3.14159f / 2.0f), (3.14159f / 2.0f));
            break;

        case CameraViewType::ThirdPersonBack:
            bodyYaw = -(yaw - 3.14159f / 2.0f);
            headPitch = std::clamp(pitch, -(3.14159f / 2.0f), (3.14159f / 2.0f));
            break;

        case CameraViewType::Inventory:
            bodyYaw = 3.14159f;
            headPitch = 0.0f;
            break;

        default:
            bodyYaw = -yaw + 3.14159f / 2.0f;
            headPitch = std::clamp(-pitch, -(3.14159f / 2.0f), (3.14159f / 2.0f));
            break;
    }

    // Walk cycle animation
    if (isMoving) {
        m_walkCycle += deltaTime * 10.0f;
        m_swingTime = std::sin(m_walkCycle) * 0.6f;
    } else {
        m_walkCycle *= 0.95f;
        m_swingTime *= 0.95f;
    }
    float limbSwing = m_swingTime;

    // Body model matrix
    Matrix4x4 bodyModel = Matrix4x4::translation(position.x, position.y, position.z)
                        * Matrix4x4::rotationY(bodyYaw);

    // Scale down for inventory preview
    if (viewType == CameraViewType::Inventory) {
        Matrix4x4 scaleMat;
        scaleMat.m[0][0] = 0.6f;
        scaleMat.m[1][1] = 0.6f;
        scaleMat.m[2][2] = 0.6f;
        bodyModel = bodyModel * scaleMat;
    }

    // Torso
    MeshData bodyMesh;
    buildBox(bodyMesh,
        {-0.25f, 0.75f, -0.125f}, {0.25f, 1.5f, 0.125f},
        20,20,28,32, 32,20,40,32, 20,16,28,20, 28,16,36,20, 16,20,20,32, 28,20,32,32);
    renderer.drawMesh(bodyMesh.verts, bodyMesh.indices, bodyMesh.uvs, bodyMesh.tints,
                      skinImg, bodyModel, view, proj, cameraPos);

    // Head with pitch rotation around neck pivot
    MeshData headMesh;
    buildBox(headMesh,
        {-0.25f, 1.5f, -0.25f}, {0.25f, 2.0f, 0.25f},
        8,8,16,16, 24,8,32,16, 8,0,16,8, 16,0,24,8, 16,8,24,16, 0,8,8,16);

    Matrix4x4 headModel = bodyModel
                         * Matrix4x4::translation(0, 1.5f, 0)
                         * Matrix4x4::rotationX(headPitch)
                         * Matrix4x4::translation(0, -1.5f, 0);

    renderer.drawMesh(headMesh.verts, headMesh.indices, headMesh.uvs, headMesh.tints,
                      skinImg, headModel, view, proj, cameraPos);

    // Limb matrix helper: rotates around a pivot point on the Y axis
    auto limbMatrix = [&](float pivotY, float angleX) -> Matrix4x4 {
        return bodyModel
             * Matrix4x4::translation(0, pivotY, 0)
             * Matrix4x4::rotationX(angleX)
             * Matrix4x4::translation(0, -pivotY, 0);
    };

    // Right arm
    MeshData rightArm;
    buildBox(rightArm,
        {-0.5f, 0.75f, -0.125f}, {-0.25f, 1.5f, 0.125f},
        44,20,48,32, 52,20,56,32, 44,16,48,20, 48,16,52,20, 40,20,44,32, 48,20,52,32);
    renderer.drawMesh(rightArm.verts, rightArm.indices, rightArm.uvs, rightArm.tints,
                      skinImg, limbMatrix(1.5f, limbSwing), view, proj, cameraPos);

    // Left arm
    MeshData leftArm;
    buildBox(leftArm,
        {0.25f, 0.75f, -0.125f}, {0.5f, 1.5f, 0.125f},
        36,52,40,64, 44,52,48,64, 36,48,40,52, 40,48,44,52, 32,52,36,64, 40,52,44,64);
    renderer.drawMesh(leftArm.verts, leftArm.indices, leftArm.uvs, leftArm.tints,
                      skinImg, limbMatrix(1.5f, -limbSwing), view, proj, cameraPos);

    // Right leg
    MeshData rightLeg;
    buildBox(rightLeg,
        {-0.25f, 0.0f, -0.125f}, {0.0f, 0.75f, 0.125f},
        4,20,8,32, 12,20,16,32, 4,16,8,20, 8,16,12,20, 0,20,4,32, 8,20,12,32);
    renderer.drawMesh(rightLeg.verts, rightLeg.indices, rightLeg.uvs, rightLeg.tints,
                      skinImg, limbMatrix(0.75f, -limbSwing), view, proj, cameraPos);

    // Left leg
    MeshData leftLeg;
    buildBox(leftLeg,
        {0.0f, 0.0f, -0.125f}, {0.25f, 0.75f, 0.125f},
        20,52,24,64, 28,52,32,64, 20,48,24,52, 24,48,28,52, 16,52,20,64, 24,52,28,64);
    renderer.drawMesh(leftLeg.verts, leftLeg.indices, leftLeg.uvs, leftLeg.tints,
                      skinImg, limbMatrix(0.75f, limbSwing), view, proj, cameraPos);
}

// Builds a box mesh from 8 arbitrary corner vertices with per-face UVs.
void PlayerView::buildOrientedBox(
    MeshData& mesh,
    const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3,
    const Vector3& p4, const Vector3& p5, const Vector3& p6, const Vector3& p7,
    float fu0, float fv0, float fu1, float fv1,
    float bu0, float bv0, float bu1, float bv1,
    float tu0, float tv0, float tu1, float tv1,
    float du0, float dv0, float du1, float dv1,
    float ru0, float rv0, float ru1, float rv1,
    float lu0, float lv0, float lu1, float lv1)
{
    auto addSide = [&](const Vector3& a, const Vector3& b,
                       const Vector3& c, const Vector3& d,
                       float u0, float v0, float u1, float v1)
    {
        unsigned int base = (unsigned int)mesh.verts.size();

        mesh.verts.insert(mesh.verts.end(), {a,b,c,d});

        mesh.indices.insert(mesh.indices.end(), {
            base, base+1, base+2,
            base, base+2, base+3
        });

        mesh.uvs.insert(mesh.uvs.end(), {
            {u0, v1}, {u1, v1}, {u1, v0}, {u0, v0}
        });

        mesh.tints.insert(mesh.tints.end(), 4, sf::Color::White);
    };

    addSide(p0,p3,p2,p1, tu0,tv0,tu1,tv1);
    addSide(p4,p5,p6,p7, du0,dv0,du1,dv1);

    addSide(p0,p1,p5,p4, ru0,rv0,ru1,rv1);
    addSide(p1,p2,p6,p5, fu0,fv0,fu1,fv1);
    addSide(p2,p3,p7,p6, lu0,lv0,lu1,lv1);
    addSide(p3,p0,p4,p7, bu0,bv0,bu1,bv1);
}

// Builds a held-item box from 8 arbitrary corner vertices using item or block texture.
void PlayerView::buildHeldItemCubeOriented(
    MeshData& mesh,
    const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3,
    const Vector3& p4, const Vector3& p5, const Vector3& p6, const Vector3& p7,
    const ItemStack& stack,
    const Item& item,
    const TextureManager& texMgr)
{
    bool isArmorItem = Inventory::isArmor(stack.type);

    unsigned int tileIndex;
    std::array<sf::Vector2f,4> uvs;

    if (isArmorItem) {
        tileIndex = texMgr.getItemTileIndex(item.textureName);
        uvs = texMgr.getItemUV(tileIndex);
    } else {
        tileIndex = texMgr.getTileIndex(item.textureName);
        uvs = texMgr.getUV(tileIndex);
    }

    float u0 = uvs[0].x, v0 = uvs[0].y;
    float u1 = uvs[2].x, v1 = uvs[2].y;

    buildOrientedBox(mesh,
        p0,p1,p2,p3,p4,p5,p6,p7,
        u0,v0,u1,v1,
        u0,v0,u1,v1,
        u0,v0,u1,v1,
        u0,v0,u1,v1,
        u0,v0,u1,v1,
        u0,v0,u1,v1);
}
