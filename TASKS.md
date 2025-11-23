# Topdown Multiplayer Game - Task Listesi

## 🎯 Proje Hedefi
Mevcut LDtk-SFML-Game projesini ECS mimarisi ile topdown multiplayer oyuna dönüştürmek.

**Teknik Kararlar:**
- ✅ Sıfırdan ECS implementasyonu
- ✅ UDP network protokolü
- ✅ Authoritative server (server tüm kararları verir)
- ✅ Tek oda mimarisi

---

## 📋 TASK LİSTESİ

### FAZE 1: ECS Core Framework

#### Task 1.1: Entity ID Sistemi
- [ ] `src/core/Entity.hpp` dosyası oluştur
- [ ] Entity ID type tanımla (uint32_t)
- [ ] Entity ID generation sistemi
- [ ] Entity ID validation
- [ ] Test: Entity ID oluşturma ve doğrulama

**Dosya:** `src/core/Entity.hpp`

---

#### Task 1.2: Component Base Yapısı
- [ ] `src/core/Component.hpp` dosyası oluştur
- [ ] Component base class veya concept tanımla
- [ ] Component type ID sistemi (typeid veya custom)
- [ ] Component size ve alignment kontrolü
- [ ] Test: Component type ID'leri

**Dosya:** `src/core/Component.hpp`

---

#### Task 1.3: Component Storage Sistemi
- [ ] `src/core/ComponentStorage.hpp/cpp` dosyaları oluştur
- [ ] Component storage container (std::vector veya SparseSet)
- [ ] Component ekleme/kaldırma metodları
- [ ] Component erişim metodları (Entity ID ile)
- [ ] Memory management (pool allocation)
- [ ] Test: Component ekleme, erişim, kaldırma

**Dosyalar:** `src/core/ComponentStorage.hpp/cpp`

---

#### Task 1.4: Component Registry
- [ ] `src/core/ComponentRegistry.hpp/cpp` dosyaları oluştur
- [ ] Component type → Storage mapping
- [ ] Component registration sistemi
- [ ] Component storage erişimi
- [ ] Type-safe component erişimi
- [ ] Test: Component registration ve erişim

**Dosyalar:** `src/core/ComponentRegistry.hpp/cpp`

---

#### Task 1.5: System Base Yapısı
- [ ] `src/core/System.hpp` dosyası oluştur
- [ ] System base class
- [ ] System update metodları
- [ ] System dependency sistemi (optional)
- [ ] Test: Basit system oluşturma

**Dosya:** `src/core/System.hpp`

---

#### Task 1.6: System Manager
- [ ] `src/core/SystemManager.hpp/cpp` dosyaları oluştur
- [ ] System registration
- [ ] System update loop
- [ ] System sıralama (dependency-based)
- [ ] Test: Birden fazla system registration ve update

**Dosyalar:** `src/core/SystemManager.hpp/cpp`

---

#### Task 1.7: World/Scene Sınıfı
- [ ] `src/core/World.hpp/cpp` dosyaları oluştur
- [ ] Entity creation/destruction
- [ ] Component attachment/detachment
- [ ] System registration
- [ ] World update loop
- [ ] Entity query sistemi (basit)
- [ ] Test: Entity oluşturma, component ekleme, system çalıştırma

**Dosyalar:** `src/core/World.hpp/cpp`

---

#### Task 1.8: ECS Core Test
- [ ] `tests/test_ecs_core.cpp` dosyası oluştur
- [ ] Entity oluşturma testi
- [ ] Component ekleme/kaldırma testi
- [ ] System çalıştırma testi
- [ ] Basit integration testi
- [ ] Test sonuçlarını doğrula

**Dosya:** `tests/test_ecs_core.cpp`

---

### FAZE 2: Temel Component'ler

#### Task 2.1: PositionComponent
- [ ] `src/core/components/PositionComponent.hpp` dosyası oluştur
- [ ] Position verisi (sf::Vector2f veya Vec2)
- [ ] Position getter/setter
- [ ] Test: PositionComponent oluşturma ve değer atama

**Dosya:** `src/core/components/PositionComponent.hpp`

---

#### Task 2.2: VelocityComponent
- [ ] `src/core/components/VelocityComponent.hpp` dosyası oluştur
- [ ] Velocity verisi (sf::Vector2f veya Vec2)
- [ ] Velocity getter/setter
- [ ] Test: VelocityComponent oluşturma ve değer atama

**Dosya:** `src/core/components/VelocityComponent.hpp`

---

#### Task 2.3: SpriteComponent
- [ ] `src/core/components/SpriteComponent.hpp` dosyası oluştur
- [ ] Sprite verisi (sf::RectangleShape veya sf::Sprite)
- [ ] Color/Texture bilgisi
- [ ] Size bilgisi
- [ ] Test: SpriteComponent oluşturma ve render hazırlığı

**Dosya:** `src/core/components/SpriteComponent.hpp`

---

#### Task 2.4: CollisionComponent
- [ ] `src/core/components/CollisionComponent.hpp` dosyası oluştur
- [ ] AABB verisi (sf::FloatRect)
- [ ] Collision flags (static, dynamic)
- [ ] Test: CollisionComponent oluşturma ve AABB değerleri

**Dosya:** `src/core/components/CollisionComponent.hpp`

---

#### Task 2.5: PlayerComponent
- [ ] `src/core/components/PlayerComponent.hpp` dosyası oluştur
- [ ] Player ID
- [ ] Player color
- [ ] Connection ID (network için)
- [ ] Test: PlayerComponent oluşturma

**Dosya:** `src/core/components/PlayerComponent.hpp`

---

#### Task 2.6: InputComponent
- [ ] `src/core/components/InputComponent.hpp` dosyası oluştur
- [ ] Keyboard state (WASD + Ok tuşları)
- [ ] Input timestamp
- [ ] Test: InputComponent oluşturma ve input state

**Dosya:** `src/core/components/InputComponent.hpp`

---

#### Task 2.7: CameraComponent
- [ ] `src/core/components/CameraComponent.hpp` dosyası oluştur
- [ ] Camera view (sf::View)
- [ ] Camera bounds (sf::FloatRect)
- [ ] Target entity (player entity'yi takip etmek için)
- [ ] Test: CameraComponent oluşturma

**Dosya:** `src/core/components/CameraComponent.hpp`

---

### FAZE 3: Temel Sistemler

#### Task 3.1: MovementSystem
- [ ] `src/core/systems/MovementSystem.hpp/cpp` dosyaları oluştur
- [ ] VelocityComponent → PositionComponent integration
- [ ] Delta time kullanımı
- [ ] MovementSystem'i World'e register et
- [ ] Test: Entity hareket ettirme

**Dosyalar:** `src/core/systems/MovementSystem.hpp/cpp`

---

#### Task 3.2: InputSystem
- [ ] `src/core/systems/InputSystem.hpp/cpp` dosyaları oluştur
- [ ] Keyboard input okuma (SFML)
- [ ] InputComponent'e yazma
- [ ] WASD + Ok tuşları desteği
- [ ] InputSystem'i World'e register et
- [ ] Test: Input okuma ve component'e yazma

**Dosyalar:** `src/core/systems/InputSystem.hpp/cpp`

---

#### Task 3.3: CollisionSystem
- [ ] `src/core/systems/CollisionSystem.hpp/cpp` dosyaları oluştur
- [ ] AABB-AABB collision detection (mevcut mantığı kullan)
- [ ] Collision response (basit push-out)
- [ ] Static vs Dynamic collision
- [ ] CollisionSystem'i World'e register et
- [ ] Test: İki entity collision testi

**Dosyalar:** `src/core/systems/CollisionSystem.hpp/cpp`

---

#### Task 3.4: CameraSystem
- [ ] `src/core/systems/CameraSystem.hpp/cpp` dosyaları oluştur
- [ ] Player entity'yi takip et
- [ ] Camera bounds kontrolü
- [ ] Smooth camera movement (mevcut mantığı kullan)
- [ ] CameraSystem'i World'e register et
- [ ] Test: Camera player'ı takip ediyor mu?

**Dosyalar:** `src/core/systems/CameraSystem.hpp/cpp`

---

#### Task 3.5: RenderSystem
- [ ] `src/core/systems/RenderSystem.hpp/cpp` dosyaları oluştur
- [ ] TileMap rendering (mevcut TileMap kullan)
- [ ] Entity rendering (SpriteComponent)
- [ ] Layer sıralaması (background → entities → foreground)
- [ ] Camera view uygulama
- [ ] RenderSystem'i World'e register et
- [ ] Test: Entity ve map render ediliyor mu?

**Dosyalar:** `src/core/systems/RenderSystem.hpp/cpp`

---

### FAZE 4: Mevcut Kodu ECS'ye Dönüştür

#### Task 4.1: LDtk Entity Converter
- [ ] `src/core/LDtkConverter.hpp/cpp` dosyaları oluştur
- [ ] LDtk Project → ECS World converter
- [ ] LDtk entities → ECS entities
- [ ] LDtk colliders → ECS entities + CollisionComponent
- [ ] LDtk player → ECS player entity
- [ ] Test: LDtk map yükleme ve ECS'ye dönüştürme

**Dosyalar:** `src/core/LDtkConverter.hpp/cpp`

---

#### Task 4.2: Game Struct Refactoring
- [ ] `src/client/Game.hpp/cpp` dosyaları oluştur
- [ ] Mevcut `Game` struct'ını `Game` class'ına dönüştür
- [ ] World kullanımı
- [ ] Entity referansları (playerEntity, colliderEntities)
- [ ] Mevcut init() metodunu ECS kullanacak şekilde güncelle
- [ ] Test: Game initialization ECS ile çalışıyor mu?

**Dosyalar:** `src/client/Game.hpp/cpp`

---

#### Task 4.3: main.cpp Güncelleme
- [ ] `src/client/main.cpp` dosyasını güncelle
- [ ] ECS World kullanımı
- [ ] Systems'leri register et
- [ ] Update loop'u systems ile çalıştır
- [ ] Render loop'u RenderSystem ile çalıştır
- [ ] Mevcut F5 reload özelliğini koru
- [ ] Test: Oyun ECS ile çalışıyor mu? (tek oyuncu)

**Dosya:** `src/client/main.cpp`

---

#### Task 4.4: ECS Integration Test
- [ ] Mevcut oyunu ECS ile çalıştır
- [ ] Player movement testi
- [ ] Collision testi
- [ ] Camera following testi
- [ ] Render testi
- [ ] Tüm özellikler çalışıyor mu kontrol et

---

### FAZE 5: Network Katmanı - Socket

#### Task 5.1: Platform Socket Abstraction
- [ ] `src/network/PlatformSocket.hpp/cpp` dosyaları oluştur
- [ ] Windows socket wrapper (WinSock2)
- [ ] Linux socket wrapper (POSIX) - placeholder
- [ ] Platform detection
- [ ] Test: Socket oluşturma (Windows)

**Dosyalar:** `src/network/PlatformSocket.hpp/cpp`

---

#### Task 5.2: Address/Endpoint Sınıfı
- [ ] `src/network/Address.hpp/cpp` dosyaları oluştur
- [ ] IP address + Port
- [ ] String → Address conversion
- [ ] Address → String conversion
- [ ] Test: Address oluşturma ve conversion

**Dosyalar:** `src/network/Address.hpp/cpp`

---

#### Task 5.3: UDP Socket Sınıfı
- [ ] `src/network/Socket.hpp/cpp` dosyaları oluştur
- [ ] UDP socket creation
- [ ] Socket binding
- [ ] Socket send/receive
- [ ] Non-blocking mode
- [ ] Error handling
- [ ] Test: UDP socket send/receive testi

**Dosyalar:** `src/network/Socket.hpp/cpp`

---

### FAZE 6: Network Katmanı - Packet

#### Task 6.1: Packet Header
- [ ] `src/network/PacketTypes.hpp` dosyası oluştur
- [ ] Packet type enum (CONNECT, INPUT, SNAPSHOT, DISCONNECT, HEARTBEAT)
- [ ] Packet header structure
- [ ] Packet size limits
- [ ] Test: Packet type definitions

**Dosya:** `src/network/PacketTypes.hpp`

---

#### Task 6.2: Packet Sınıfı
- [ ] `src/network/Packet.hpp/cpp` dosyaları oluştur
- [ ] Packet buffer management
- [ ] Packet header read/write
- [ ] Packet data read/write
- [ ] Packet validation
- [ ] Test: Packet oluşturma, yazma, okuma

**Dosyalar:** `src/network/Packet.hpp/cpp`

---

#### Task 6.3: Component Serialization
- [ ] `src/core/ComponentSerializer.hpp/cpp` dosyaları oluştur
- [ ] PositionComponent serialization
- [ ] VelocityComponent serialization
- [ ] SpriteComponent serialization
- [ ] PlayerComponent serialization
- [ ] Serialization/Deserialization testleri

**Dosyalar:** `src/core/ComponentSerializer.hpp/cpp`

---

#### Task 6.4: Packet Serializer
- [ ] `src/network/PacketSerializer.hpp/cpp` dosyaları oluştur
- [ ] Component → Packet serialization
- [ ] Packet → Component deserialization
- [ ] Snapshot serialization (tüm entities)
- [ ] Input serialization
- [ ] Test: Component serialization/deserialization

**Dosyalar:** `src/network/PacketSerializer.hpp/cpp`

---

### FAZE 7: Network Katmanı - Manager

#### Task 7.1: Connection Sınıfı
- [ ] `src/network/Connection.hpp/cpp` dosyaları oluştur
- [ ] Connection state (disconnected, connecting, connected)
- [ ] Connection address
- [ ] Last heartbeat time
- [ ] Connection timeout
- [ ] Test: Connection state management

**Dosyalar:** `src/network/Connection.hpp/cpp`

---

#### Task 7.2: NetworkManager (Client)
- [ ] `src/client/ClientNetworkManager.hpp/cpp` dosyaları oluştur
- [ ] Server'a bağlanma
- [ ] Packet gönderme
- [ ] Packet alma
- [ ] Heartbeat gönderme
- [ ] Connection state yönetimi
- [ ] Test: Client server'a bağlanıyor mu?

**Dosyalar:** `src/client/ClientNetworkManager.hpp/cpp`

---

#### Task 7.3: NetworkManager (Server)
- [ ] `src/server/ServerNetworkManager.hpp/cpp` dosyaları oluştur
- [ ] Client bağlantılarını kabul etme
- [ ] Client bağlantılarını yönetme
- [ ] Packet gönderme (broadcast, specific client)
- [ ] Packet alma
- [ ] Heartbeat kontrolü
- [ ] Connection timeout handling
- [ ] Test: Server client'ları kabul ediyor mu?

**Dosyalar:** `src/server/ServerNetworkManager.hpp/cpp`

---

### FAZE 8: Server Framework

#### Task 8.1: Server Config
- [ ] `src/server/ServerConfig.hpp` dosyası oluştur
- [ ] Server port (7777)
- [ ] Tick rate (60)
- [ ] Max players
- [ ] Config dosyası okuma (optional)
- [ ] Test: Config değerleri

**Dosya:** `src/server/ServerConfig.hpp`

---

#### Task 8.2: GameServer Sınıfı
- [ ] `src/server/GameServer.hpp/cpp` dosyaları oluştur
- [ ] Server main loop (60 tick)
- [ ] Fixed timestep (delta time)
- [ ] World instance
- [ ] Network manager integration
- [ ] Server state management
- [ ] Test: Server loop çalışıyor mu?

**Dosyalar:** `src/server/GameServer.hpp/cpp`

---

#### Task 8.3: Server Systems - Input
- [ ] `src/server/systems/ServerInputSystem.hpp/cpp` dosyaları oluştur
- [ ] Client input packet'lerini al
- [ ] InputComponent'e yaz (player entity'ye)
- [ ] Input validation (basit)
- [ ] ServerInputSystem'i World'e register et
- [ ] Test: Client input server'da işleniyor mu?

**Dosyalar:** `src/server/systems/ServerInputSystem.hpp/cpp`

---

#### Task 8.4: Server Systems - Movement
- [ ] `src/server/systems/ServerMovementSystem.hpp/cpp` dosyaları oluştur
- [ ] Authoritative movement (server karar verir)
- [ ] InputComponent → VelocityComponent
- [ ] VelocityComponent → PositionComponent
- [ ] ServerMovementSystem'i World'e register et
- [ ] Test: Server'da entity hareket ediyor mu?

**Dosyalar:** `src/server/systems/ServerMovementSystem.hpp/cpp`

---

#### Task 8.5: Server Systems - Collision
- [ ] `src/server/systems/ServerCollisionSystem.hpp/cpp` dosyaları oluştur
- [ ] Authoritative collision (server karar verir)
- [ ] AABB collision detection
- [ ] Collision response
- [ ] ServerCollisionSystem'i World'e register et
- [ ] Test: Server'da collision çalışıyor mu?

**Dosyalar:** `src/server/systems/ServerCollisionSystem.hpp/cpp`

---

#### Task 8.6: Server Systems - Snapshot
- [ ] `src/server/systems/SnapshotSystem.hpp/cpp` dosyaları oluştur
- [ ] World state → Snapshot oluşturma
- [ ] Tüm entities serialize et
- [ ] Snapshot packet oluşturma
- [ ] Client'lara snapshot gönderme (broadcast)
- [ ] Snapshot frequency (20-30 Hz)
- [ ] SnapshotSystem'i World'e register et
- [ ] Test: Server snapshot gönderiyor mu?

**Dosyalar:** `src/server/systems/SnapshotSystem.hpp/cpp`

---

#### Task 8.7: Server Main
- [ ] `src/server/main.cpp` dosyası oluştur
- [ ] GameServer instance
- [ ] Server initialization
- [ ] Server loop başlatma
- [ ] Shutdown handling
- [ ] Test: Server başlatılıyor ve çalışıyor mu?

**Dosya:** `src/server/main.cpp`

---

### FAZE 9: Client Network Entegrasyonu

#### Task 9.1: Client Network System
- [ ] `src/client/systems/ClientNetworkSystem.hpp/cpp` dosyaları oluştur
- [ ] Server'a bağlanma
- [ ] Input gönderme (InputComponent → Packet)
- [ ] Snapshot alma (Packet → World state)
- [ ] World state update (snapshot interpolation - basit)
- [ ] ClientNetworkSystem'i World'e register et
- [ ] Test: Client server'a bağlanıyor ve snapshot alıyor mu?

**Dosyalar:** `src/client/systems/ClientNetworkSystem.hpp/cpp`

---

#### Task 9.2: Client Main Güncelleme
- [ ] `src/client/main.cpp` dosyasını güncelle
- [ ] ClientNetworkManager ekle
- [ ] ClientNetworkSystem ekle
- [ ] Network loop (input gönder, snapshot al)
- [ ] Render loop (mevcut RenderSystem)
- [ ] Test: Client multiplayer modda çalışıyor mu?

**Dosya:** `src/client/main.cpp`

---

#### Task 9.3: Input Sending
- [ ] InputSystem'i güncelle
- [ ] InputComponent → Packet conversion
- [ ] Server'a input gönderme
- [ ] Input timestamp ekleme
- [ ] Test: Client input server'a gidiyor mu?

---

#### Task 9.4: Snapshot Receiving
- [ ] Snapshot packet → World state conversion
- [ ] Entity synchronization
- [ ] Component update (server state → client state)
- [ ] Basit interpolation (optional)
- [ ] Test: Server snapshot client'ta görünüyor mu?

---

### FAZE 10: Multiplayer Test ve İyileştirmeler

#### Task 10.1: İki Client Test
- [ ] İki client instance başlat
- [ ] Her ikisi de server'a bağlan
- [ ] Her iki client'ta player görünüyor mu?
- [ ] Her iki client'ta hareket senkronize mi?
- [ ] Collision senkronize mi?
- [ ] Test sonuçlarını dokümante et

---

#### Task 10.2: Lag Test
- [ ] Network lag simülasyonu (optional)
- [ ] Snapshot gecikmesi testi
- [ ] Input gecikmesi testi
- [ ] Lag durumunda oyun çalışıyor mu?
- [ ] Test sonuçlarını dokümante et

---

#### Task 10.3: Basit Anti-Cheat
- [ ] `src/server/AntiCheat.hpp/cpp` dosyaları oluştur
- [ ] Position validation (sınırlar içinde mi?)
- [ ] Speed validation (maksimum hız kontrolü)
- [ ] Input validation (geçerli input mu?)
- [ ] AntiCheat'i server'a entegre et
- [ ] Test: Hileli input tespit ediliyor mu?

**Dosyalar:** `src/server/AntiCheat.hpp/cpp`

---

#### Task 10.4: Delta Compression (Optional)
- [ ] `src/server/DeltaCompression.hpp/cpp` dosyaları oluştur
- [ ] Snapshot diffing (önceki snapshot ile karşılaştır)
- [ ] Sadece değişen component'leri gönder
- [ ] Delta packet oluşturma
- [ ] Client'ta delta apply
- [ ] Test: Delta compression çalışıyor mu?

**Dosyalar:** `src/server/DeltaCompression.hpp/cpp`

---

### FAZE 11: CMake ve Build Sistemi

#### Task 11.1: CMakeLists.txt Güncelleme
- [ ] Server executable ekle
- [ ] Client executable ekle
- [ ] Shared source files (core, network)
- [ ] Platform-specific settings (Windows/Linux)
- [ ] Dependency management (SFML, LDtkLoader)
- [ ] Test: Server ve client build ediliyor mu?

**Dosya:** `CMakeLists.txt`

---

#### Task 11.2: Build Scripts
- [ ] Build script oluştur (PowerShell - Windows)
- [ ] Clean script
- [ ] Test script
- [ ] Test: Scripts çalışıyor mu?

---

### FAZE 12: Dokümantasyon ve Finalizasyon

#### Task 12.1: README Güncelleme
- [ ] Proje açıklaması
- [ ] Build talimatları
- [ ] Çalıştırma talimatları
- [ ] Network setup (port, firewall)
- [ ] Test: README doğru mu?

**Dosya:** `README.md`

---

#### Task 12.2: Kod Dokümantasyonu
- [ ] Kod içi yorumlar (Doxygen format)
- [ ] API dokümantasyonu (core, network, server)
- [ ] Architecture dokümantasyonu
- [ ] Test: Dokümantasyon tamam mı?

---

#### Task 12.3: Final Test
- [ ] Tüm özellikler test et
- [ ] Multiplayer test (2+ client)
- [ ] Performance test
- [ ] Memory leak kontrolü
- [ ] Final rapor oluştur

---

## 📊 İlerleme Takibi

### Tamamlanan Task'lar
- [ ] Task 1.1: Entity ID Sistemi
- [ ] Task 1.2: Component Base Yapısı
- [ ] ... (devam edecek)

### Şu Anki Task
**Task 1.1: Entity ID Sistemi**

### Sonraki Task
**Task 1.2: Component Base Yapısı**

---

## 🎯 Notlar

- Her task tamamlandığında checkbox'ı işaretle
- Her task'tan sonra test yap
- Sorun varsa task'a geri dön ve düzelt
- Her faz tamamlandığında integration test yap

---

**Son Güncelleme:** 2024
**Toplam Task Sayısı:** ~60 task
**Tahmini Süre:** 12 hafta (3 ay, part-time)

