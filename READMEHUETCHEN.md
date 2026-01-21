# Hütchenspiel - Code-Dokumentation

## Übersicht

Dieses Projekt implementiert ein "Hütchenspiel" für NAO-Roboter im RoboCup SPL-Kontext. Der Roboter beobachtet, hinter welchem von drei verschiedenfarbigen Robotern ein Ball versteckt wird, verfolgt die Position während einer Mischphase und identifiziert anschließend den korrekten Roboter anhand seiner Trikotfarbe. Und findet so den Ball wieder.

### Projektziel

- **Primäres Ziel**: Roboter soll beobachten, welcher von 3 farbigen "Bechern" (Robotern) einen Ball versteckt
- **Tracking-Mechanismus**: Verwendung von Trikot-Farberkennung 
- **Robustheit**: System muss mit Wahrnehmungsfehlern umgehen können (Roboter schaut weg, Hindernisse nicht sichtbar)
- **Integration**: Nahtlose Einbindung in das B-Human Framework

### Aktueller Stand

**Was funktioniert:**
- Zustandsautomat mit 4 Zuständen (WAITING_FOR_SETUP, OBSERVING_HIDE, TRACKING_BALL, REVEALING_POSITION)
- Trikotfarberkennung für 7 Farben (ROT, BLAU, GELB, GRÜN, ORANGE, LILA, BRAUN)
- Robuste Wartemechanismen (keine vorzeitigen Zustandswechsel)
- Timeout-Mechanismen (10s für Mischphase, 5s für Aufdeckphase)
- Debug-Ausgaben mit Farbnamen und Positionsinformationen

**Was noch hinzugefügt werden kann:**
- Roboter durch echte Hütchen ersetzen (Neuronales Netz)
- Tracking überarbeiten, sodass alle Hütchen die gleiche Farben haben können
-

## Ausführung

### 0.Download
-Codebase installieren: https://github.com/AK-Smart-Machines-HS-KL/R2K-SPL/wiki/Die-Codebase-installieren

### 1. Kompilierung
- In Visual Studio 2022 (Bhuman.sln offen!)
- Mit Develop starten lassen (Grüner Pfeil)

### 2. SimRobot starten
- Nach erfolgreichem Build öffnet sich SimRobot
- Im SimRobot Fenster oben links auf File open, und dann 1vs3.ros2 öffnen
- 1vs3.ros2 ist eine eigene Szene für das Hütchenspiel mit einem Spieler und drei Gegnern in verschiedenfarbigen Jerseys

### 3. Hütchen spielen
- Jetzt den Ball einem der Gegner-Roboter vor die Füße legen
- In die Console eingeben: gc playing
- Dann sieht man, für welchen Roboter die HuetchenSpielerCard aktiv ist: Roboter 4
- Wir befinden uns im ersten Zustand: WAITING_FOR_SETUP
- Sobald der Roboter den Ball sieht, gehen wir in Zustand: OBSERVING_HIDE über
- Den Ball kann man jetzt den drei Robotern abwechselnd vor die Füße legen, und bekommt eine Nachricht in der Console, welcher dre drei den Ball aktuell hat. (In Form von: LINKS,MITTE,RECHTS)
- **Fehleranfällig** wenn ein Ball zu schnell von links nach rechts oder von rechts nach links gelegt wird(Mitte wird übersprungen) -> Dann denkt der Roboter, der Ball wurde versteckt und springt in Zustand TRACKING_BALL
- Sobald man sich für einen Roboter entschieden hat, der den Ball verstecken soll, legt man den Ball HINTER diesen Roboter. 
- Wenn der Ball verschwindet, erscheint: DER BALL WURDE VERSTECKT und wo und bei wem der Ball zuletzt war (bspw. PLAYER BLAU MIT BALL IST LINKS)
- Es gab einen Zustandswechsel in: TRACKING_BALL. 
- Nun wird gemischt. Unser Spieler-Roboter wartet jetzt 10 Sekunden, und solange können wir die drei Roboter miteinander tauschen.
- WICHTIG: Den Ball wieder verstecken!
- Nach den 10 Sekunden sucht unser Spieler-Roboter nach der gemerkten Jersey-Farbe, in Zustand: REVEALING_POSITION.
- Und es erscheint auf der Console: bspw SPIELER MIT FARBE BLAU GEFUNDEN. SPIELER MIT BALL IST RECHTS.
- Danach sind wir wieder im ersten Zustand: WAITING_FOR_SETUP. Zum Weiterspielen :)






### Datenfluss

#### Globale Pipeline (Perception → Modeling → Behavior)
1. **Kamerabild (ECImage)** → JerseyClassifierProvider
2. **detectJerseyColor()** analysiert Pixel → ermittelt TEAM_*-Farbe
3. **ObstaclesFieldPercept** erhält jerseyColor-Feld
4. **ObstacleModelProvider** fusioniert Daten → Obstacle.jerseyColor
5. **HuetchenSpielerCard** liest ObstaclesFieldPercept und trifft Entscheidungen

#### HuetchenSpielerCard - Verwendete Datenstrukturen

**INPUT (REQUIRES)**:
- **FieldBall**: 
  - `ballWasSeen()`: Ballsichtbarkeit wird geprüft
  - `positionRelative.x/y()` : Ballposition relativ zum Roboter (für Links/Mitte/Rechts-Klassifikation)

- **ObstaclesFieldPercept**: 
  - `obstacles[]` : Liste aller erkannten Hindernisse im aktuellen Frame
  - `obstacle.center` :  zentrale Position des Hindernisses(für Abstandsberechnung zum Ball)
  - `obstacle.jerseyColor` :  TEAM_*-Konstante für Farbidentifikation
  - `obstacle.type` : Unterscheidung zwischen ownPlayer/opponentPlayer


**OUTPUT (CALLS)**:
- **Stand**: Roboter steht still während der Beobachtung
- **LookActive**: Aktive Kopfbewegung zum Scannen der Szene
- **Activity**: Setzt BehaviorStatus für GUI/Logging

#### State-spezifischer Datenfluss

**WAITING_FOR_SETUP:**
```
FieldBall.ballWasSeen() == true?
  ├─ JA  → Zustandswechsel zu OBSERVING_HIDE
  └─ NEIN → Warte bis der Ball gesehen wurde
```

**OBSERVING_HIDE:**
```
FieldBall.ballWasSeen() == true?
  ├─ JA  : Analysiere FieldBall.positionRelative.y()
  │         ├─ y < -200mm : BALL IST RECHTS
  │         ├─ y > +200mm : BALL IST LINKS
  │         └─ sonst      : BALL IST IN DER MITTE
  └─ NEIN (Ball versteckt): Finde das nächste Obstacle zu lastBallPosition
              ├─ ObstaclesFieldPercept.obstacles[] durchsuchen
              ├─ Abstand zu savedOpponentPosition (y Koordinate) berechnen
              ├─ Speichere trackedJerseyColor vom dem Obstacle, der am nächsten zum Ball ist
              └─ Zustandswechsel zu TRACKING_BALL
```

**TRACKING_BALL:**
```
waitStartTime + 10000ms vergangen?
  ├─ JA   : Zustandswechsel zu REVEALING_POSITION
  │        (Mischphase beendet, jetzt wird gesucht)
  └─ NEIN : Warte weiter (Roboter werden noch gemischt)
```

**REVEALING_POSITION:**
```
ObstaclesFieldPercept.obstacles[] durchsuchen:
  ├─ obstacle.jerseyColor == trackedJerseyColor?
  │   └─ JA : OUTPUT Position(LINKS/MITTIG/RECHTS), Reset zu WAITING_FOR_SETUP
  └─ revealingStartTime + 5000ms vergangen?
      └─ JA (Timeout) → Reset zu WAITING_FOR_SETUP 
```


## Modifizierte Dateien

### 1. HuetchenSpielerCard.cpp (NEU ERSTELLT)
**Pfad**: `\Src\Modules\BehaviorControl\BehaviorControl\Cards\R2K\HuetchenSpielerCard.cpp`

**Zweck**: Hauptverhalten für das Hütchenspiel mit 4-Zustands-Automat

**Zustandsautomat**:
```cpp
enum State {
  WAITING_FOR_SETUP,    // Wartet, bis der Ball gesehen wurde
  OBSERVING_HIDE,       // Beobachtet, welcher Roboter Ball versteckt und Merkt sich jerseyColor dieses Roboters
  TRACKING_BALL,        // Wartet 10 Sekunden
  REVEALING_POSITION    // Findet Roboter mit gemerkter jerseyColor wieder
};

State state = WAITING_FOR_SETUP;
int targetJerseyColor = -1;
unsigned waitStartTime = 0;
unsigned revealingStartTime = 0;
```


### 2. JerseyClassifierProvider.cpp
**Pfad**: `\Src\Modules\Perception\PlayersPerceptors\JerseyClassifierProvider.cpp`

**Zweck**: Erkennt Trikotfarben in Kamerabildern durch Hue-Analyse

**Neue Methode**: `detectJerseyColor()` (Zeilen 321-405)

**Algorithmus**:
1. **Scan-Bereich**: Obere 60% des erkannten Hindernisses (Trikot-Region)
2. **Sampling**: 5-Pixel-Schritte für Performance (y+=5, x+=5)
3. **Sättigungs-Filter**: Nur Pixel mit sat > 40 (filtert Grau/Weiß/Schwarz)
4. **Hue-Matching**: Zirkuläre Distanz zu jerseyHues[] mit Wraparound
5. **Toleranz**: ±30 Hue-Einheiten pro Farbe
6. **Dominanz-Schwelle**: Farbe muss >25% aller gültigen Samples ausmachen
7. **Rückgabewert**: `TEAM_*`-Konstante (z.B `TEAM_RED`,`TEAM_BLUE`,...)

**Integration in Perception-Pipeline**: Zeilen 36-42, 193, 201-203





### 3. ObstaclesFieldPercept.h
**Pfad**: `\Src\Representations\Perception\ObstaclesPercepts\ObstaclesFieldPercept.h`

**Hinzugefügt (Zeile 40)**:
```cpp
(int)(-1) jerseyColor, /**< TEAM_* color constant or -1 if unknown */
```
**Zweck**: Erweitert Hindernis-Perzept um Trikotfarbe 



### 4. JerseyClassifier.h
**Pfad**: `\Src\Representations\Perception\ObstaclesPercepts\JerseyClassifier.h`

**Änderung (Zeilen 29)**:
```cpp
FUNCTION(int(const ObstaclesImagePercept::Obstacle&)) detectJerseyColor;
/**< Detects jersey color. Returns TEAM_* constant or -1 */
```
**Zweck**: Deklariert detectJerseyColor als aufrufbare Funktion 



### 5. Obstacle.h
**Pfad**: `\Src\Tools\Modeling\Obstacle.h`

**Änderung (Zeile 41)**:
```cpp
(int)(-1) jerseyColor, /**< TEAM_* color constant or -1 if unknown */
```
**Zweck**: Erweitert die Obstacle-Datenstruktur um Trikotfarbe



### 6. ObstacleModelProvider.cpp
**Pfad**: `\Src\Modules\Modeling\ObstacleModelProvider\ObstacleModelProvider.cpp`

**Änderungen**:
- **Zeile 206** (addPlayerPercepts): `obstacle.jerseyColor = percept.jerseyColor;`
- **Zeile 255,256** (tryToMerge): 
  ```cpp
  if(measurement.jerseyColor != -1) {
    obstacleHypotheses[atMerge].jerseyColor = measurement.jerseyColor;
  }
  ```
- **Zeile 371-373** (mergeOverlapping): 
  ```cpp
   if(actual.jerseyColor == -1 && other.jerseyColor != -1)
          actual.jerseyColor = other.jerseyColor;
  ```

**Zweck**: Stellt sicher, dass jerseyColor korrekt beibehalten wird





### Farbkonstanten (TEAM_*)
```cpp
TEAM_BLUE    = 0  // Blau (Hue ~210-240)
TEAM_RED     = 1  // Rot (Hue ~0-20 oder ~340-360)
TEAM_YELLOW  = 2  // Gelb (Hue ~40-60)
TEAM_GREEN   = 5  // Grün (Hue ~100-140)
TEAM_ORANGE  = 6  // Orange (Hue ~20-40)
TEAM_PURPLE  = 7  // Lila (Hue ~270-300)
TEAM_BROWN   = 8  // Braun (Hue ~20-30)
```
**Hinweis**: SCHWARZ, WEISS, GRAU werden nicht unterstützt 

### Timeouts & Robustheit
- **Mischphase**: 10 Sekunden (ermöglicht manuelles Mischen)
- **Aufdeckphase**: 5 Sekunden (verhindert Endlosschleifen)
- **Warteschleifen**: System bleibt in Zustand wenn Hindernisse nicht sichtbar 




### Console-Ausgaben interpretieren

**Wichtige Meldungen**:
- `"X Hindernisse gefunden"` → Prüfe ob 3 Roboter im Sichtfeld
- `"Ball ist weg"` → Übergang zu OBSERVING_HIDE funktioniert
- `"Ball bei [FARBE] gefunden!"` → Tracking erfolgreich gestartet
- `"PLAYER [FARBE] MIT BALL IST [POSITION]"` → Ball versteckt, Position gemerkt
- `"SPIELER MIT FARBE [FARBE] GEFUNDEN"` → Finaler Roboter korrekt identifiziert
- `"Timeout erreicht!"` → Roboter nicht im Sichtfeld, Zustand zurückgesetzt



**Projekt-Status**: ✅ Funktional mit 80-90% Erfolgsrate bei optimalen Bedingungen

**Framework**: B-Human 2025/2026 | **Build**: Visual Studio 2022 | **Entwicklungszeit**: Januar 2026
