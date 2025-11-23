# Real-Time Game Server ECS (FPS-lite Arena) - Proje Dokümantasyonu

## 📋 İçindekiler

1. [Proje Genel Bakış](#proje-genel-bakış)
2. [Mimari Tasarım](#mimari-tasarım)
3. [Teknoloji Stack](#teknoloji-stack)
4. [Proje Yapısı](#proje-yapısı)
5. [Sıfırdan Kurulum Rehberi](#sıfırdan-kurulum-rehberi)
6. [Cursor IDE Kurulumu](#cursor-ide-kurulumu)
7. [Build Sistemi](#build-sistemi)
8. [Bağımlılıklar](#bağımlılıklar)
9. [Çalıştırma ve Test](#çalıştırma-ve-test)
10. [Geliştirme Notları](#geliştirme-notları)
11. [Sorun Giderme](#sorun-giderme)

---

## 🎯 Proje Genel Bakış

### Proje Tanımı

**Real-Time Game Server ECS (FPS-lite Arena)**, C++20 ile geliştirilmiş, ECS (Entity Component System) mimarisi kullanan, otoriter (authoritative) bir gerçek zamanlı oyun sunucusudur. FPS tarzı veya arena tipi oyunlar için temel altyapı sağlar.

### Temel Özellikler

- ✅ **ECS Mimarisi**: Esnek ve ölçeklenebilir oyun mantığı
- ✅ **Network Katmanı**: UDP socket soyutlaması (Windows/Linux uyumlu)
- ✅ **Room-Based Server**: Çoklu oyun odası desteği
- ✅ **60/120 Tick Server Loop**: Yüksek performanslı gerçek zamanlı simülasyon
- ✅ **Physics Layer**: BVH (Bounding Volume Hierarchy) ile çarpışma tespiti
- ✅ **Matchmaking**: Basit kuyruk tabanlı oyuncu eşleştirme sistemi
- ✅ **Anti-Cheat-Lite**: Temel hile önleme kontrolleri
- ✅ **Snapshot Serialization**: Component-based snapshot sistemi
- ✅ **150x150 Map**: Büyük oyun dünyası, statik engeller ve duvarlar
- ✅ **Multiplayer Support**: Eşzamanlı çoklu oyuncu desteği
- ✅ **Random Spawn System**: Çarpışma önleme ile rastgele spawn sistemi

### Proje Durumu
**📋 Planlanan:**
- Temel mimari
- ECS framework
- Network katmanı
- Server framework
- Physics sistemi (BVH, collision detection)
- Component serialization
- Snapshot sistemi
- Matchmaking sistemi
- Random spawn sistemi
- Map objeleri (duvarlar/engeller)
- 150x150 oyun dünyası
- Multiplayer desteği

- Delta compression implementasyonu
- Lag compensation
- Rollback/rewind

- Deterministic simulation
- Rating-based matchmaking
- Lua/AngelScript scripting
- Glicko-2 rating sistemi
- Profiling araçları
- Shooting mekanikleri
- Health sistemi iyileştirmeleri

---

## 🏗️ Mimari Tasarım

### Genel Mimari

```
┌─────────────────────────────────────────────────────────┐
│                    GameServer (main.cpp)                 │
│  ┌──────────────────────────────────────────────────┐  │
│  │              GameServer Class                     │  │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐       │  │
│  │  │ Network  │  │   ECS    │  │ Physics  │       │  │
│  │  │  Layer   │  │  World   │  │  System  │       │  │
│  │  └──────────┘  └──────────┘  └──────────┘       │  │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐       │  │
│  │  │Matchmaker│  │Anti-Cheat│  │Snapshot  │       │  │
│  │  └──────────┘  └──────────┘  └──────────┘       │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### ECS Mimarisi

**Entity-Component-System (ECS)** pattern kullanılmaktadır:

- **Entity**: Sadece bir ID (uint32_t)
- **Component**: Veri saklayan yapılar (Position, Velocity, Health, vb.)
- **System**: Component'ler üzerinde işlem yapan mantık (MovementSystem, PhysicsSystem)

**Component Tipleri:**
- `Position`: 3D pozisyon (Vec3)
- `Velocity`: 3D hız (Vec3)
- `Health`: Can puanı ve durumu
- `PlayerComponent`: Oyuncu kimliği
- `InputComponent`: Oyuncu input'ları
- `Transform`: Transform bilgileri
- `CollisionComponent`: Çarpışma sınırları (AABB)

### Network Mimarisi

- **Protocol**: UDP (connectionless)
- **Packet Types**: CONNECT, HEARTBEAT, INPUT, SNAPSHOT, FIND_MATCH, MATCH_FOUND, CANCEL_MATCH
- **Snapshot System**: Component-based serialization
- **Tick Rate**: 60 veya 120 tick/saniye (ayarlanabilir)

### Room Sistemi

- Her oda bağımsız bir ECS World içerir
- Oyuncular odalara atanır
- Matchmaking sistemi oyuncuları eşleştirip yeni odalar oluşturur
- Default room (ID: 0) her zaman mevcuttur

---

## 💻 Teknoloji Stack

### Programlama Dili
- **C++20** (CMakeLists.txt'de tanımlı)
- **C++17** uyumluluğu (eski derleyiciler için)

### Build Sistemi
- **CMake 3.20+** (minimum gereksinim)
- **Visual Studio 2022** (Windows için önerilen)
- **MSBuild** (derleme için)

### Kütüphaneler
- **Windows Socket API (ws2_32.lib)**: Network işlemleri
- **Standard C++ Library**: STL kullanımı
- **Chrono**: Zaman yönetimi

### Platform
- **Windows 10/11** (birincil platform)
- **Linux** desteği planlanmış (kod hazır)

### IDE ve Araçlar
- **Cursor IDE** (önerilen)
- **Visual Studio 2022** (derleme için)
- **CMake** (build sistemi)
- **PowerShell** (build script'leri)

---



### Dış Kaynaklar
- [CMake Documentation](https://cmake.org/documentation/)
- [Visual Studio Documentation](https://docs.microsoft.com/visualstudio/)
- [C++ Reference](https://en.cppreference.com/)
- [ECS Pattern](https://en.wikipedia.org/wiki/Entity_component_system)

---

## 📝 Notlar

### Önemli Hatırlatmalar

1. **C++ Standard**: Proje C++20 kullanıyor, ancak C++17 uyumlu olmalı
2. **Platform**: Şu an Windows odaklı, Linux desteği planlanmış
3. **Network**: UDP kullanılıyor, connectionless
4. **Tick Rate**: Varsayılan 60, 120'ye çıkarılabilir
5. **Port**: Varsayılan 7777, değiştirilebilir
6. **Firewall**: UDP port 7777 açık olmalı (multiplayer için)

### Geliştirme Önerileri

1. **Version Control**: Git kullanın, `.gitignore` dosyası ekleyin
2. **Testing**: Unit test framework'ü ekleyin
3. **Documentation**: Kod içi dokümantasyon ekleyin (Doxygen)
4. **Profiling**: Performans analizi için profiling araçları kullanın
5. **CI/CD**: GitHub Actions veya benzeri CI/CD pipeline ekleyin

---

## 🤝 Destek

Sorunlar için:
1. Bu dokümantasyonu kontrol edin
3. Issue açın (eğer repository'de ise)
4. Log dosyalarını kontrol edin

---

**Son Güncelleme:** 2024
**Dokümantasyon Versiyonu:** 1.0
**Proje Versiyonu:** 1.0.0

