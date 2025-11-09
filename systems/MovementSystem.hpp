#pragma once

#include "../ecs/System.hpp"
#include "../components/InputComponent.hpp"
#include "../components/Velocity.hpp"
#include "../components/Position.hpp"
#include "../components/Transform.hpp"
#include "../physics/Physics.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>

namespace game::systems {

// Movement System - Converts input to velocity and updates position
class MovementSystem
  : public ecs::SystemBase<
        components::InputComponent,
        components::Velocity,
        components::Position,
        components::Transform> {
private:
    const float MOVE_SPEED = 5.0f;        // Units per second
    const float SPRINT_MULTIPLIER = 1.5f;  // Sprint speed multiplier
    const float MAX_SPEED = 10.0f;         // Maximum movement speed

    static constexpr float DEG2RAD = 3.1415926535f / 180.0f;

public:
    MovementSystem() = default;

    int getPriority() const override {
        // Movement önce çalışsın
        return 10;
    }

    void process(ecs::World& /*world*/, float deltaTime, ecs::Entity& /*entity*/,
                 components::InputComponent& input,
                 components::Velocity& velocity,
                 components::Position& position,
                 components::Transform& transform) override
    {
        // Top-down 2D: X (sağ-sol) ve Y (ileri-geri) kullanıyoruz
        // X: sağ = pozitif, sol = negatif
        // Y: ileri (yukarı) = pozitif, geri (aşağı) = negatif
        
        // 1) Yerel hareket yönü (player'ın baktığı yöne göre)
        float moveForward = 0.0f;  // İleri-geri (W/S)
        float moveRight = 0.0f;    // Sağ-sol (A/D)
        
        if (input.isPressed(components::INPUT_FORWARD))  moveForward += 1.0f;  // W = ileri
        if (input.isPressed(components::INPUT_BACKWARD)) moveForward -= 1.0f;  // S = geri
        if (input.isPressed(components::INPUT_RIGHT))    moveRight += 1.0f;    // D = sağ
        if (input.isPressed(components::INPUT_LEFT))     moveRight -= 1.0f;    // A = sol

        // DEBUG: Input durumu
        static int debugCounter = 0;
        if (moveForward != 0.0f || moveRight != 0.0f || (debugCounter++ % 60 == 0)) {
            std::cout << "\n[MOVEMENT DEBUG] " << std::fixed << std::setprecision(2)
                      << "Yaw: " << input.mouseYaw << "° | "
                      << "Forward: " << moveForward << " | "
                      << "Right: " << moveRight << " | "
                      << "Pos: (" << position.value.x << ", " << position.value.y << ", " << position.value.z << ") | "
                      << "Vel: (" << velocity.value.x << ", " << velocity.value.y << ", " << velocity.value.z << ")"
                      << std::endl;
        }

        if (moveForward != 0.0f || moveRight != 0.0f) {
            // Normalize (çapraz hareket için)
            float len = std::sqrt(moveForward * moveForward + moveRight * moveRight);
            moveForward /= len;
            moveRight /= len;

            // 2) Yaw açısına göre döndür (basit 2D rotation)
            // Client'ta yaw 90° başlıyor (yukarı bakıyor)
            // Yaw 90° = forward (0, 1) olmalı
            // Yaw 0° = sağa (1, 0) olmalı
            // Yaw 180° = aşağı (0, -1) olmalı
            // Yaw 270° = sola (-1, 0) olmalı
            float adjustedYaw = input.mouseYaw - 90.0f; // 90° offset (yaw 90° = 0° rotation)
            float yawRad = adjustedYaw * DEG2RAD;
            float c = std::cos(yawRad);
            float s = std::sin(yawRad);

            // Basit 2D rotation: (forward, right) -> (worldX, worldY)
            // forward = (0, 1) yerel, right = (1, 0) yerel
            // Mouse yaw CW artıyorsa, rotation ters olmalı (CW rotation):
            // [cos  sin] [right ]   [worldX]
            // [-sin cos] [forward] = [worldY]
            float worldX = moveRight * c + moveForward * s;  // Sağ-sol
            float worldY = -moveRight * s + moveForward * c;  // İleri-geri

            // DEBUG: Rotation sonrası
            std::cout << "[ROTATION] yaw: " << input.mouseYaw << "° | adjustedYaw: " << adjustedYaw << "° | "
                      << "cos: " << c << " | sin: " << s << " | "
                      << "worldX: " << worldX << " | worldY: " << worldY << std::endl;

            // 3) Hız uygula (sprint dahil)
            float speed = MOVE_SPEED * (input.isPressed(components::INPUT_SPRINT) ? SPRINT_MULTIPLIER : 1.0f);
            velocity.value.x = worldX * speed;
            velocity.value.y = worldY * speed;  // Y eksenini kullan
            velocity.value.z = 0.0f;  // Z eksenini kullanma
        } else {
            // 4) Giriş yoksa sürtünme ile yavaşlat
            velocity.value = velocity.value * 0.8f;
            if (velocity.value.lengthSq() < 0.01f) {
                velocity.value = physics::Vec3(0.0f, 0.0f, 0.0f);
            }
        }

        // 5) Maksimum hız limiti
        float vlen = velocity.value.length();
        if (vlen > MAX_SPEED) {
            velocity.value = velocity.value.normalized() * MAX_SPEED;
        }

        // 6) Konumu güncelle
        position.value = position.value + (velocity.value * deltaTime);

        // 7) Transform senkronu
        transform.position = position.value;
        transform.rotation.y = input.mouseYaw;
    }
};

} // namespace game::systems

