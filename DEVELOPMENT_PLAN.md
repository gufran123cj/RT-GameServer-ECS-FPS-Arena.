# Topdown Multiplayer Game - ECS Dönüşüm Planı

## 🎯 Proje Hedefi

**Mevcut LDtk-SFML-Game projesini ECS mimarisi ile topdown multiplayer oyuna dönüştürmek**

### Mevcut Durum (LDtk-SFML-Game)
- ✅ LDtk map loading sistemi
- ✅ SFML rendering (TileMap, layers)
- ✅ Player movement (WASD + Ok tuşları)
- ✅ AABB collision detection
- ✅ Camera following
- ✅ Asset reload (F5)

### Hedef Durum
- 🎯 ECS mimarisi (Entity-Component-System)
- 🎯 Network katmanı (UDP, real-time)
- 🎯 Server-Client ayrımı
- 🎯 Multiplayer desteği (çoklu oyuncu)
- 🎯 LDtk map sistemi korunacak
- 🎯 SFML rendering korunacak

---

## 🏗️ Mimari Yaklaşım

### Genel Yapı
```
┌─────────────────────────────────────────────────────────┐
│                    Client (SFML)                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │              ECS World (Client)                   │  │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐       │  │
│  │  │  Input   │  │  Render  │  │ Network  │       │  │
│  │  │  System  │  │  System  │  │  System  │       │  │
│  │  └──────────┘  └──────────┘  └──────────┘       │  │
│  └──────────────────────────────────────────────────┘  │
│                    UDP Network                           │
└─────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────┐
│                    Server (Headless)                     │
│  ┌──────────────────────────────────────────────────┐  │
│  │              ECS World (Server)                   │  │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐       │  │
│  │  │Movement  │  │Collision │  │ Network  │       │  │
│  │  │  System  │  │  System  │  │  System  │       │  │
│  │  └──────────┘  └──────────┘  └──────────┘       │  │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐       │  │
│  │  │Snapshot  │  │Matchmaker│  │Anti-Cheat│       │  │
│  │  │  System  │  │          │  │          │       │  │
│  │  └──────────┘  └──────────┘  └──────────┘       │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### ECS Yapısı
- **Entity**: uint32_t ID
- **Component**: Veri yapıları (Position, Velocity, Sprite, vb.)
- **System**: Mantık (MovementSystem, CollisionSystem, RenderSystem)

---

## 📋 Geliştirme Fazları

### FAZE 1: ECS Core Framework (Hafta 1)

#### 1.1 ECS Temel Yapıları
- [ ] Entity ID sistemi
- [ ] Component storage (SparseSet veya Array-based)
- [ ] Component registry
- [ ] System manager
- [ ] World/Scene sınıfı

**Dosyalar:**
```
src/core/
  ├── Entity.hpp
  ├── Component.hpp
  ├── System.hpp
  ├── World.hpp/cpp
  └── ComponentRegistry.hpp/cpp
```

#### 1.2 Mevcut Kodu Analiz Et
- [ ] Game struct'ını component'lere ayır
- [ ] Player → Entity + Components
- [ ] Colliders → Entities + CollisionComponent
- [ ] TileMap → RenderSystem'de kullanılacak

**Dönüşüm Planı:**
- `Game::player` → Entity + `PositionComponent` + `SpriteComponent` + `PlayerComponent`
- `Game::colliders` → Entities + `CollisionComponent` + `StaticComponent`
- `Game::camera` → `CameraComponent` (player entity'ye bağlı)

---

### FAZE 2: Component'ler ve İlk Sistemler (Hafta 2)

#### 2.1 Temel Component'ler
- [ ] `PositionComponent` (sf::Vector2f)
- [ ] `VelocityComponent` (sf::Vector2f)
- [ ] `SpriteComponent` (sf::RectangleShape veya sf::Sprite)
- [ ] `CollisionComponent` (sf::FloatRect)
- [ ] `PlayerComponent` (player ID, color)
- [ ] `InputComponent` (keyboard state)
- [ ] `CameraComponent` (sf::View, bounds)

**Dosyalar:**
```
src/core/components/
  ├── PositionComponent.hpp
  ├── VelocityComponent.hpp
  ├── SpriteComponent.hpp
  ├── CollisionComponent.hpp
  ├── PlayerComponent.hpp
  ├── InputComponent.hpp
  └── CameraComponent.hpp
```

#### 2.2 İlk Sistemler
- [ ] `MovementSystem` (Velocity → Position)
- [ ] `InputSystem` (keyboard → InputComponent)
- [ ] `CollisionSystem` (AABB collision, mevcut mantığı kullan)
- [ ] `CameraSystem` (player'ı takip et)

**Dosyalar:**
```
src/core/systems/
  ├── MovementSystem.hpp/cpp
  ├── InputSystem.hpp/cpp
  ├── CollisionSystem.hpp/cpp
  └── CameraSystem.hpp/cpp
```

#### 2.3 Render System
- [ ] `RenderSystem` (TileMap + entities render)
- [ ] Mevcut TileMap sistemini koru
- [ ] Entity rendering (SpriteComponent)

**Dosyalar:**
```
src/core/systems/
  └── RenderSystem.hpp/cpp
```

---

### FAZE 3: Mevcut Kodu ECS'ye Dönüştür (Hafta 3)

#### 3.1 Game Struct'ını ECS World'e Dönüştür
- [ ] `Game` struct'ını `World` kullanacak şekilde refactor et
- [ ] Player entity oluştur
- [ ] Collider entities oluştur (LDtk'den)
- [ ] Camera entity oluştur

**Dönüşüm Örneği:**
```cpp
// ÖNCE (mevcut)
struct Game {
    sf::RectangleShape player;
    std::vector<sf::FloatRect> colliders;
    // ...
};

// SONRA (ECS)
class Game {
    World world;
    Entity playerEntity;
    std::vector<Entity> colliderEntities;
    // ...
};
```

#### 3.2 main.cpp'i Güncelle
- [ ] ECS World kullan
- [ ] Systems'leri register et
- [ ] Update loop'u systems ile çalıştır
- [ ] Render loop'u RenderSystem ile çalıştır

#### 3.3 LDtk Entegrasyonu
- [ ] LDtk entities → ECS entities
- [ ] LDtk colliders → ECS entities + CollisionComponent
- [ ] LDtk player → ECS player entity

**Dosyalar:**
```
src/core/
  └── LDtkLoader.hpp/cpp  // LDtk → ECS converter
```

---

### FAZE 4: Network Katmanı (Hafta 4-5)

#### 4.1 Socket Abstraction
- [ ] Platform-agnostic socket (Windows/Linux)
- [ ] UDP socket wrapper
- [ ] Address/Endpoint sınıfları

**Dosyalar:**
```
src/network/
  ├── Socket.hpp/cpp
  ├── Address.hpp/cpp
  └── PlatformSocket.hpp/cpp
```

#### 4.2 Packet System
- [ ] Packet header
- [ ] Packet types: CONNECT, INPUT, SNAPSHOT, DISCONNECT
- [ ] Serialization/Deserialization

**Dosyalar:**
```
src/network/
  ├── Packet.hpp/cpp
  ├── PacketTypes.hpp
  └── PacketSerializer.hpp/cpp
```

#### 4.3 Network Manager
- [ ] Connection management
- [ ] Packet sending/receiving
- [ ] Heartbeat

**Dosyalar:**
```
src/network/
  └── NetworkManager.hpp/cpp
```

---

### FAZE 5: Server Framework (Hafta 6-7)

#### 5.1 Server Main Loop
- [ ] 60 tick server loop
- [ ] Fixed timestep
- [ ] Server state management

**Dosyalar:**
```
src/server/
  ├── main.cpp
  ├── GameServer.hpp/cpp
  └── ServerConfig.hpp
```

#### 5.2 Server Systems
- [ ] `ServerMovementSystem` (authoritative)
- [ ] `ServerCollisionSystem` (authoritative)
- [ ] `ServerInputSystem` (client input'ları al)
- [ ] `SnapshotSystem` (world state → packet)

**Dosyalar:**
```
src/server/systems/
  ├── ServerMovementSystem.hpp/cpp
  ├── ServerCollisionSystem.hpp/cpp
  ├── ServerInputSystem.hpp/cpp
  └── SnapshotSystem.hpp/cpp
```

#### 5.3 Component Serialization
- [ ] Component → binary serialization
- [ ] Snapshot creation (tüm entities)
- [ ] Snapshot compression (basit)

**Dosyalar:**
```
src/core/
  └── ComponentSerializer.hpp/cpp
```

---

### FAZE 6: Client Network Entegrasyonu (Hafta 8)

#### 6.1 Client Network System
- [ ] Server'a bağlan
- [ ] Input gönder (InputComponent → Packet)
- [ ] Snapshot al (Packet → World state)
- [ ] Interpolation (basit)

**Dosyalar:**
```
src/client/systems/
  └── ClientNetworkSystem.hpp/cpp
```

#### 6.2 Client Main Loop Güncellemesi
- [ ] Network system'i ekle
- [ ] Input → Server gönder
- [ ] Server snapshot → World update
- [ ] Render (mevcut RenderSystem)

**Dosyalar:**
```
src/client/
  └── main.cpp  // mevcut main.cpp'i güncelle
```

---

### FAZE 7: Multiplayer Test ve İyileştirmeler (Hafta 9-10)

#### 7.1 Multiplayer Test
- [ ] İki client + bir server test
- [ ] Entity synchronization test
- [ ] Collision synchronization test
- [ ] Lag test

#### 7.2 İyileştirmeler
- [ ] Delta compression (snapshot diffing)
- [ ] Client-side prediction (basit)
- [ ] Lag compensation (basit)
- [ ] Anti-cheat (position validation)

**Dosyalar:**
```
src/server/
  ├── DeltaCompression.hpp/cpp
  └── AntiCheat.hpp/cpp
```

---

### FAZE 8: Matchmaking ve Room System (Hafta 11-12)

#### 8.1 Room System
- [ ] Room yönetimi
- [ ] Player assignment
- [ ] Room state

**Dosyalar:**
```
src/server/
  ├── Room.hpp/cpp
  └── RoomManager.hpp/cpp
```

#### 8.2 Matchmaking
- [ ] Player queue
- [ ] Match creation
- [ ] Room assignment

**Dosyalar:**
```
src/server/
  ├── Matchmaker.hpp/cpp
  └── PlayerQueue.hpp/cpp
```

---

## 📁 Proje Yapısı (Final)

```
LDtk-SFML-Game/
├── src/
│   ├── core/                    # ECS Framework
│   │   ├── Entity.hpp
│   │   ├── Component.hpp
│   │   ├── System.hpp
│   │   ├── World.hpp/cpp
│   │   ├── ComponentRegistry.hpp/cpp
│   │   ├── components/          # Component definitions
│   │   │   ├── PositionComponent.hpp
│   │   │   ├── VelocityComponent.hpp
│   │   │   ├── SpriteComponent.hpp
│   │   │   ├── CollisionComponent.hpp
│   │   │   ├── PlayerComponent.hpp
│   │   │   ├── InputComponent.hpp
│   │   │   └── CameraComponent.hpp
│   │   ├── systems/             # Systems
│   │   │   ├── MovementSystem.hpp/cpp
│   │   │   ├── InputSystem.hpp/cpp
│   │   │   ├── CollisionSystem.hpp/cpp
│   │   │   ├── CameraSystem.hpp/cpp
│   │   │   └── RenderSystem.hpp/cpp
│   │   └── LDtkLoader.hpp/cpp   # LDtk → ECS converter
│   │
│   ├── network/                 # Network Layer
│   │   ├── Socket.hpp/cpp
│   │   ├── Address.hpp/cpp
│   │   ├── Packet.hpp/cpp
│   │   ├── PacketTypes.hpp
│   │   ├── PacketSerializer.hpp/cpp
│   │   └── NetworkManager.hpp/cpp
│   │
│   ├── server/                  # Server
│   │   ├── main.cpp
│   │   ├── GameServer.hpp/cpp
│   │   ├── Room.hpp/cpp
│   │   ├── RoomManager.hpp/cpp
│   │   ├── Matchmaker.hpp/cpp
│   │   ├── systems/
│   │   │   ├── ServerMovementSystem.hpp/cpp
│   │   │   ├── ServerCollisionSystem.hpp/cpp
│   │   │   ├── ServerInputSystem.hpp/cpp
│   │   │   └── SnapshotSystem.hpp/cpp
│   │   └── AntiCheat.hpp/cpp
│   │
│   ├── client/                  # Client (mevcut kod)
│   │   ├── main.cpp            # Mevcut main.cpp (güncellenecek)
│   │   ├── systems/
│   │   │   └── ClientNetworkSystem.hpp/cpp
│   │   └── Game.hpp/cpp        # Mevcut Game struct (ECS'ye dönüştürülecek)
│   │
│   └── common/                  # Shared utilities
│       ├── Vec2.hpp/cpp
│       └── MathUtils.hpp/cpp
│
├── assets/                      # Mevcut (korunacak)
│   ├── maps/
│   └── tilesets/
│
├── CMakeLists.txt              # Güncellenecek (server + client)
├── README.md
└── DEVELOPMENT_PLAN.md         # Bu dosya
```

---

## 🔄 Dönüşüm Stratejisi

### Adım 1: ECS Core'u Kur (Faz 1-2)
- ECS framework'ü sıfırdan yaz (basit, minimal)
- Component ve System yapılarını oluştur
- Test et (basit entity + component + system)

### Adım 2: Mevcut Kodu Refactor Et (Faz 3)
- Game struct'ını parçala
- Player → Entity + Components
- Colliders → Entities
- Systems'leri yaz (mevcut mantığı kullan)

### Adım 3: Network Ekle (Faz 4-6)
- Network katmanını ekle
- Server'ı yaz
- Client'ı network'e bağla

### Adım 4: Multiplayer Test (Faz 7-8)
- Test et
- İyileştir
- Matchmaking ekle

---

## 🎯 Öncelikler

### MVP (Minimum Viable Product)
1. ✅ ECS Core Framework
2. ✅ Mevcut kodu ECS'ye dönüştür
3. ✅ Network katmanı
4. ✅ Basit server (tek oda)
5. ✅ Client-server bağlantısı
6. ✅ İki oyuncu multiplayer

### İyileştirmeler
1. Delta compression
2. Client-side prediction
3. Lag compensation
4. Matchmaking
5. Anti-cheat

---

## 💡 Teknik Notlar

### ECS Implementation Stratejisi
- **Basit başla**: SparseSet veya Array-based storage
- **Component pooling**: Memory efficiency
- **System queries**: Component combinations

### Network Stratejisi
- **UDP**: Real-time için ideal
- **Authoritative server**: Server tüm kararları verir
- **Snapshot frequency**: 20-30 Hz (client render 60 Hz)

### LDtk Entegrasyonu
- **Mevcut TileMap korunacak**: RenderSystem'de kullanılacak
- **LDtk entities → ECS entities**: Converter yazılacak
- **Map reload**: F5 tuşu çalışmaya devam edecek

---

## 📝 İlk Adımlar (Hemen Başlayabiliriz)

1. **ECS Core Framework'ü yaz**
   - Entity, Component, System base classes
   - World class
   - ComponentRegistry

2. **İlk Component'leri tanımla**
   - PositionComponent
   - SpriteComponent
   - PlayerComponent

3. **İlk System'i yaz**
   - MovementSystem (basit)

4. **Test et**
   - Tek entity, tek component, tek system

---

**Son Güncelleme:** 2024
**Plan Versiyonu:** 2.0 (ECS Dönüşüm Odaklı)
**Tahmini Süre:** 12 hafta (3 ay, part-time)
