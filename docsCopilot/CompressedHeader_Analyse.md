# Compressed Header / Team Communication - Analyse & Erweiterungsmöglichkeiten

**Autor:** GitHub Copilot  
**Datum:** 23.02.2026  
**Kontext:** Aufgabe: "compressed header: copilot analyse geht da noch mehr info rein"

---

## 📊 1. WAS IST DER "COMPRESSED HEADER"?

Der **"Compressed Header"** bezieht sich auf das **Compressed Team Communication System** in R2K/B-Human.

### Technische Details:
- **Implementierung:** `Src/Tools/Communication/CompressedTeamCommunicationStreams.{h,cpp}`
- **Container:** `BHumanStandardMessage.compressedContainer` (Vector<uint8_t>)
- **Zweck:** Adaptive Kompression von Team-Daten zur Bandbreitenoptimierung

---

## 📏 2. SPL NACHRICHTEN-LIMITS

### SPL Standard Message (offiziell):
```cpp
#define SPL_STANDARD_MESSAGE_DATA_SIZE 474  // Bytes
```

**Struktur der SPL-Nachricht:**
```
┌─────────────────────────────────────┐
│ Header (4 bytes): "SPL "            │
│ Version (1 byte): 7                 │
│ PlayerNum (1 byte)                  │
│ TeamNum (1 byte)                    │
│ Fallen (1 byte)                     │
│ Pose[3] (12 bytes): x, y, theta     │  ~34 Bytes
│ BallAge (4 bytes)                   │  Pflichtfelder
│ Ball[2] (8 bytes): x, y             │
│ NumOfDataBytes (2 bytes)            │
├─────────────────────────────────────┤
│ Data[474] (474 bytes)               │  Verfügbar für
│   → BHumanStandardMessage           │  Team-Daten
│   → BHumanArbitraryMessage          │
└─────────────────────────────────────┘
```

### B-Human Standard Message (in data[474]):
```cpp
#define BHUMAN_STANDARD_MESSAGE_STRUCT_VERSION 13

Größenberechnung:
- Header (4 bytes): "BHUM"
- Version (1 byte): 13
- MagicNumber (1 byte)
- Timestamp (4 bytes)
- Flags + CompressedSize (2 bytes)
- NTP Messages (5 bytes × Anzahl)
- CompressedContainer (variabel)
────────────────────────────────────
Gesamt: ~12 Bytes + NTP + Compressed
```

**Verfügbarer Platz für komprimierte Daten:** ~450 Bytes (bei wenigen NTP-Nachrichten)

---

## 🔍 3. AKTUELL ÜBERTRAGENE DATEN

### 3.1 Komprimierte Daten (`BHumanCompressedMessageParticle`)

| Representation          | Typ        | Inhalt                                      | Datei                              |
|------------------------|------------|---------------------------------------------|------------------------------------|
| **TeamBehaviorStatus** | Komprimiert | teamActivity, timeToReachBall, teammateRoles, role | `Representations/BehaviorControl/TeamBehaviorStatus.h` |
| **Whistle**            | Komprimiert | confidenceOfLastWhistleDetection, channelsUsed, lastTimeWhistleDetected | `Representations/Modeling/Whistle.h` |
| **FrameInfo**          | Komprimiert | time (Timestamp)                            | `Representations/Infrastructure/FrameInfo.h` |

### 3.2 Standard übertragene Daten (`BHumanMessageParticle`)

| Representation            | Übertragungsart | Größe (geschätzt) | Datei                              |
|--------------------------|-----------------|-------------------|------------------------------------|
| **RobotPose**            | Standard        | ~30 Bytes         | `Representations/Modeling/RobotPose.h` |
| **BallModel**            | Standard        | ~25 Bytes         | `Representations/Modeling/BallModel.h` |
| **ObstacleModel**        | Standard        | variabel          | `Representations/Modeling/ObstacleModel.h` |
| **RobotHealth**          | Arbitrary       | variabel          | `Representations/Infrastructure/RobotHealth.h` |
| **FieldCoverage**        | Arbitrary       | variabel          | `Representations/Modeling/FieldCoverage.h` |
| **FieldFeatureOverview** | Standard        | variabel          | `Representations/Perception/FieldFeatures/FieldFeatureOverview.h` |

---

## 💡 4. MÖGLICHE ERWEITERUNGEN

### 4.1 Kandidaten für zusätzliche komprimierte Übertragung

#### **A) BehaviorStatus - Aktuelle Aktivität**
**Datei:** `Representations/BehaviorControl/BehaviorStatus.h`

**Potenzielle Daten:**
- `activity` (aktuelles Behavior) → **1 Byte** (Enum)
- `passTarget` (Passziel) → **1 Byte** (Player-ID)

**Nutzen:**
- Bessere Koordination bei Standardsituationen
- Vermeidung von Konflikten (z.B. zwei Roboter greifen Ball an)

**Implementierung:**
```cpp
STREAMABLE(BehaviorStatus, COMMA public BHumanCompressedMessageParticle<BehaviorStatus>
{
  // Existing code...
  (Activity) activity,
  (int) passTarget,
});
```

---

#### **B) WalkingTo - Zielposition**
**Motivation:** Mitspieler wissen, wo andere hinlaufen

**Potenzielle Daten:**
- `target` Position (x, y) → **ca. 4-6 Bytes** (komprimiert)
- `targetValid` → **1 Bit**

**Nutzen:**
- Bessere Pfadplanung (Kollisionsvermeidung)
- Verbesserte Positionierung im R2K-Konzept

**Implementierung:**
```cpp
STREAMABLE(WalkingToTarget, COMMA public BHumanCompressedMessageParticle<WalkingToTarget>
{,
  (bool)(false) hasTarget,
  (Vector2f)(Vector2f::Zero()) targetPosition,
});
```

---

#### **C) PassingInfo - Passinformationen**
**Motivation:** Koordination bei Pässen

**Potenzielle Daten:**
- `wantsToPass` → **1 Bit**
- `passReceiver` → **3 Bits** (Player 1-6)
- `estimatedPassTime` → **8 Bits** (0-255 Frames)

**Nutzen:**
- Empfänger kann sich vorbereiten
- Andere Roboter halten Abstand

**Implementierung:**
```cpp
STREAMABLE(PassingInfo, COMMA public BHumanCompressedMessageParticle<PassingInfo>
{,
  (bool)(false) wantsToPass,
  (int)(-1) passReceiver,
  (unsigned)(0) estimatedPassTime,
});
```

---

#### **D) DefensivePosition - Verteidigungsformation**
**Motivation:** Für R2K_DEFENSIVE_GAME kritisch

**Potenzielle Daten:**
- `defendingZone` (Enum: LEFT, CENTER, RIGHT) → **2 Bits**
- `priorityTarget` (Gegner-ID) → **3 Bits**

**Nutzen:**
- Bessere Zonenverteidigung
- Keine Überlappung der Defensivpositionen

---

#### **E) ObstaclePresence - Vereinfachte Hindernisse**
**Motivation:** Alternative zu vollem ObstacleModel

**Potenzielle Daten:**
- `opponentInOwnHalf` → **1 Bit**
- `closestOpponentDistance` → **8 Bits** (0-5m, komprimiert)
- `closestOpponentAngle` → **4 Bits** (grobere Richtung)

**Nutzen:**
- Schnelle Bedrohungsbewertung
- Geringerer Bandbreitenbedarf als volles ObstacleModel

---

### 4.2 Geschätzte Größen

| Zusätzliche Daten       | Größe (komprimiert) | Priorität |
|------------------------|---------------------|-----------|
| BehaviorStatus.activity| ~1 Byte             | ⭐⭐⭐       |
| WalkingToTarget        | ~5 Bytes            | ⭐⭐        |
| PassingInfo            | ~2 Bytes            | ⭐⭐⭐       |
| DefensivePosition      | ~1 Byte             | ⭐⭐        |
| ObstaclePresence       | ~2 Bytes            | ⭐         |
| **SUMME**              | **~11 Bytes**       |           |

**Verfügbar aktuell:** ~440 Bytes  
**Nach Erweiterung:** ~429 Bytes (immer noch ausreichend Platz!)

---

## ⚠️ 5. WICHTIGE ÜBERLEGUNGEN

### 5.1 Bandbreitenmanagement
- **Budget:** 1200 Nachrichten pro Spiel (10 Minuten)
- **Rate:** ~2 Nachrichten/Sekunde/Roboter
- Aktuell wird Event-Based Communication (EBC) verwendet → Optimierung bereits aktiv

### 5.2 Latenz vs. Bandbreite
- **Mehr Daten** = Größere Nachrichten = Längere Übertragungszeit
- **Empfehlung:** Nur kritische Daten komprimiert übertragen
- Nicht-kritische Daten → `BHumanArbitraryMessage` (bei Bedarf)

### 5.3 Kompressionseffizienz
Die aktuelle Implementierung nutzt:
- **Bit-genaue Kodierung** (IntegerType mit `bits` Parameter)
- **Relative Timestamps** (TimestampType mit `relativePast`/`relativeFuture`)
- **Bereichsbeschränkung** (min/max für Float/Integer)

**Beispiel aus `TeamBehaviorStatus`:**
```cpp
(TimeToReachBall) timeToReachBall  // Nutzt komprimierte Zeitdarstellung
```

---

## 🔧 6. IMPLEMENTIERUNGSVORSCHLAG

### Schritt 1: Neue Representation erstellen

**Datei:** `Src/Representations/BehaviorControl/SharedBehaviorData.h`

```cpp
/**
 * @file SharedBehaviorData.h
 * Additional behavior information shared via compressed team communication
 * @author Your Name
 */

#pragma once

#include "Tools/Communication/BHumanTeamMessageParts/BHumanMessageParticle.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Streams/Enum.h"

STREAMABLE(SharedBehaviorData, COMMA public BHumanCompressedMessageParticle<SharedBehaviorData>
{
  ENUM(DefendingZone,
  {,
    left,
    center,
    right,
    none,
  }),

  (bool)(false) wantsToPass,
  (int)(-1) passReceiver,
  (DefendingZone)(none) defendingZone,
});
```

### Schritt 2: Compressed Communication Schema definieren

**Datei:** Erweitern Sie die entsprechende `.def` Datei für CompressedTeamCommunication

```
theSharedBehaviorData wantsToPass: bool;
theSharedBehaviorData passReceiver: int min(-1) max(6) bits(3);
theSharedBehaviorData defendingZone: int min(0) max(3) bits(2);
```

### Schritt 3: Integration in TeamMessageHandler

**Datei:** `Src/Modules/Communication/TeamMessageHandler/TeamMessageHandler.cpp`

```cpp
// In update(BHumanMessageOutputGenerator&):
theSharedBehaviorData >> outputGenerator;
```

### Schritt 4: Testen
1. Nachrichtengröße überprüfen (soll < 474 Bytes bleiben)
2. Latenz messen
3. Team-Verhalten testen (besonders bei Pässen und Defensive)

---

## 📈 7. MESSBARE VERBESSERUNGEN

Nach Implementierung der Erweiterungen sollten folgende Metriken beobachtet werden:

| Metrik                          | Vorher | Nachher (Ziel) |
|--------------------------------|--------|----------------|
| Durchschnittliche Nachrichtengröße | ~XXX Bytes | < 450 Bytes |
| Pass-Erfolgsquote             | Baseline | +10-15% |
| Defensive Zonenabdeckung      | Baseline | +20% |
| Kollisionen zwischen Teammates | Baseline | -30% |

---

## ✅ 8. ZUSAMMENFASSUNG & EMPFEHLUNG

### **Antwort auf die Frage: "Geht da noch mehr Info rein?"**

**JA! Es ist möglich, zusätzliche Informationen in den Compressed Header aufzunehmen.**

**Empfohlene nächste Schritte:**

1. ⭐ **Priorität HOCH:** `PassingInfo` hinzufügen  
   → Direkte Verbesserung der Team-Koordination

2. ⭐ **Priorität HOCH:** `BehaviorStatus.activity` übertragen  
   → Konfliktvermeidung bei Ball-Annäherung

3. ⭐ **Priorität MITTEL:** `DefensivePosition` für R2K_DEFENSIVE_GAME  
   → Bessere Verteidigungsformation

4. ⭐ **Priorität NIEDRIG:** `WalkingToTarget`  
   → Feintuning der Positionierung

**Bandbreite vorhanden:** ~440 Bytes verfügbar, nur ~11 Bytes zusätzlich benötigt

**Risiko:** NIEDRIG (ausreichend Puffer vorhanden)

---

## 📚 9. REFERENZEN

- **SPL Rules:** [https://spl.robocup.org/downloads/](https://spl.robocup.org/downloads/)
- **Compressed Communication Streams:** `Src/Tools/Communication/CompressedTeamCommunicationStreams.{h,cpp}`
- **B-Human Message Parts:** `Src/Tools/Communication/BHumanTeamMessageParts/`
- **Team Message Handler:** `Src/Modules/Communication/TeamMessageHandler/`

---

## 🔗 10. VERWANDTE DATEIEN

### Hauptdateien für Änderungen:
- [CompressedTeamCommunicationStreams.h](../Src/Tools/Communication/CompressedTeamCommunicationStreams.h)
- [BHumanStandardMessage.h](../Src/Tools/Communication/BHumanTeamMessageParts/BHumanStandardMessage.h)
- [TeamBehaviorStatus.h](../Src/Representations/BehaviorControl/TeamBehaviorStatus.h)
- [TeamMessageHandler.cpp](../Src/Modules/Communication/TeamMessageHandler/TeamMessageHandler.cpp)

### Konfiguration:
- Event-Based Communication: `Src/Modules/Communication/EventBasedCommunicationHandler/`

---

**Letzte Aktualisierung:** 23.02.2026  
**Status:** ✅ Analyse abgeschlossen, Implementierung empfohlen
