# Kameraeinstellung vor einem Spiel - Überexposure-Vermeidung

**Ziel:** Optimale Kameraeinstellungen vor einem Spiel, um Überexposure zu vermeiden und so Linien und Bälle besser zu erkennen.

---

## Inhaltsverzeichnis
1. [Schnellstart](#schnellstart)
2. [Grundverständnis der Kameraeinstellungen](#grundverständnis-der-kameraeinstellungen)
3. [Überexposure erkennen und vermeiden](#überexposure-erkennen-und-vermeiden)
4. [Einstellung der Kameras vor dem Spiel](#einstellung-der-kameras-vor-dem-spiel)
5. [Konfigurationsdatei verwenden/bearbeiten](#konfigurationsdatei-verwendendbearbeiten)
6. [Tipps und Best Practices](#tipps-und-best-practices)
7. [Häufig auftretende Probleme](#häufig-auftretende-probleme)

---

## Schnellstart

Vor einem Spiel:
1. **Simulator:** Im SimRobot Interface -> `Debugging -> Representations -> CameraSettings` anpassen
2. **Echte NAO:** Configuration-Datei `Config/Locations/Default/cameraSettings.cfg` bearbeiten und deployen
3. **Überexposure prüfen:** Beide Kamerabilder (obere/untere Kamera) kontrollieren
4. **Exposure reduzieren:** Falls zu hell, `autoExposureBrightness` senken oder `exposure`/`gain` manuell anpassen

---

## Grundverständnis der Kameraeinstellungen

Der NAO V6 hat **zwei Kameras**: eine obere (oben am Kopf) und eine untere (oben an der Brust).

### Wichtigste Kameraparameter:

| Parameter | Bereich | Einheit | Auswirkung auf Bild |
|-----------|---------|--------|---------------------|
| **autoExposure** | true/false | - | Auto-Belichtung an/aus |
| **autoExposureBrightness** | -255 bis 255 | - | Zielhelligkeitswert bei Auto-Belichtung |
| **exposure** | 0 bis 1.048.575 | Mikrosekunden | Belichtungszeit (nur wenn autoExposure=false) |
| **gain** | 0 bis 1023 | - | Verstärkung (nur wenn autoExposure=false) |
| **saturation** | 0 bis 255 | - | Farbsättigung (Farbintensität) |
| **contrast** | 0 bis 255 | - | Kontrast zwischen hell/dunkel |
| **sharpness** | 0 bis 9 | - | Bildschärfe |
| **autoWhiteBalance** | true/false | - | Auto-Weißabgleich an/aus |
| **redGain** | 0 bis 4095 | - | Rot-Farbkanal (wenn autoWB=false) |
| **greenGain** | 0 bis 4095 | - | Grün-Farbkanal (wenn autoWB=false) |
| **blueGain** | 0 bis 4095 | - | Blau-Farbkanal (wenn autoWB=false) |
| **autoFocus** | true/false | - | Autofokus an/aus |
| **focus** | 0 bis 250 | 25er-Schritte | Fokus-Entfernung (wenn autoFocus=false) |

---

## Überexposure erkennen und vermeiden

### Was ist Überexposure?
- **Überexposure** = Bild ist überbelichtet, zu Hell, weiße Flecken/Bereiche
- Folge: Linien und Bälle verschmelzen mit dem hellen Hintergrund
- **Unterexposure** = Bild ist unterbelichtet, zu Dunkel, kann auch problematisch sein

### Überexposure erkennen:
1. **Im SimRobot-Simulator:** In `Debugging -> Representations -> CameraImage` oder `CameraImageUpper` / `CameraImageLower` anschauen
2. **Auf echtem NAO:** SSH Zugang -> `/dev/video-top` (obere Kamera) oder `/dev/video-bottom` (untere Kamera) mit Tool wie `guvcview` anschauen
3. **Merkmale von Überexposure:**
   - Große helle/weiße Bereiche im Bild
   - Feldlinien sind kaum zu sehen
   - Ball und Feldmarkierungen sind verschwommen

### Hauptursachen:
- **Zu hohe autoExposureBrightness:** Auto-Belichtung versucht Bild zu hell zu machen
- **Zu hoher gain / exposure:** Belichtungszeit oder Verstärkung zu hoch
- **Schlechte Lichtverhältnisse:** Sehr grelles/direktes Licht

---

## Einstellung der Kameras vor dem Spiel

### Szenario 1: Auto-Belichtung verwenden (empfohlen für Anfänger)

**Standardeinstellung im Projekt:**
```
autoExposure = true;
autoExposureBrightness = 0;
```

**Falls Überexposure auftritt:**
1. `autoExposureBrightness` um **-20 bis -50** reduzieren
   - Dunklere Zielhelligkeitswert → weniger überbelichtet
   - Beispiel: Von 0 → -30

2. `saturation` und `contrast` anpassen:
   - `saturation` erhöhen (z.B. 80 → 100), um Farblineien besser zu sehen
   - `contrast` erhöhen (z.B. 65 → 85), um Kontrast zwischen Linie und Feld zu verstärken

**Beispielkonfiguration (für sehr grelles Licht):**
```
autoExposure = true;
autoExposureBrightness = -40;
saturation = 100;
contrast = 80;
```

---

### Szenario 2: Manuelle Belichtung (für sehr präzise Kontrolle)

Wenn Auto-Belichtung nicht stabil genug ist:

1. Wechseln Sie zu `autoExposure = false`
2. Passen Sie `exposure` und `gain` manuell an:
   - **Exposure starten:** 2000 bis 3000 Mikrosekunden
   - **Gain starten:** 160 bis 200
   - Experimentieren Sie: Reduzieren Sie beide, bis das Bild optimal ist
   - Nicht zu dunkel machen! Ball muss erkennbar sein.

**Beispiel für grelles Licht:**
```
autoExposure = false;
exposure = 1500;      # Weniger Belichtungszeit
gain = 120;           # Weniger Verstärkung
saturation = 90;
contrast = 75;
```

---

### Szenario 3: Unterschiedliche Lichtverhältnisse

#### Hallenlicht (Standard RoboCup)
```
autoExposure = true;
autoExposureBrightness = -20;
saturation = 85;
contrast = 70;
sharpness = 4;
```

#### Sehr grelles/direktes Licht (Sonnenlicht, Spotlights)
```
autoExposure = true;
autoExposureBrightness = -50;
saturation = 100;
contrast = 85;
sharpness = 5;
```

#### Gedimmtes Licht (Schule, schlechte Halle)
```
autoExposure = true;
autoExposureBrightness = 10;
saturation = 80;
contrast = 65;
sharpness = 4;
```

---

## Konfigurationsdatei verwenden/bearbeiten

### Dateiort:
```
Config/Locations/Default/cameraSettings.cfg
```

### Struktur der Datei:
```properties
cameras = {
  upper = {
    # Einstellungen für obere Kamera (oben am Kopf)
    autoExposure = true;
    ...
  };
  lower = {
    # Einstellungen für untere Kamera (Brustkamera)
    autoExposure = true;
    ...
  };
};
```

### Bearbeitung vor dem Spiel:
1. Datei öffnen: `Config/Locations/Default/cameraSettings.cfg`
2. `upper` Sektion anpassen:
   - `autoExposureBrightness` ggf. reduzieren (z.B. -30 statt 0)
   - `saturation` erhöhen (z.B. 85-90)
   - `contrast` erhöhen (z.B. 70-75)
3. `lower` Sektion auf gleiche Weise anpassen
4. Datei speichern
5. **NAO neu starten / Simulator neu laden**

---

### Vollständiges Beispiel (optimiert für Überexposure-Vermeidung):

```properties
cameras = {
  upper = {
    autoExposure = true;
    autoExposureBrightness = -30;      # Dunklere Zielhelligkeitswert
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 90;                   # Mehr Farbsättigung
    contrast = 75;                     # Besserer Kontrast
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
  lower = {
    autoExposure = true;
    autoExposureBrightness = -30;      # Dunklere Zielhelligkeitswert
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 90;                   # Mehr Farbsättigung
    contrast = 80;                     # Besserer Kontrast (untere Kamera oft mehr Probleme)
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
};
```

---

## Tipps und Best Practices

### ✅ Allgemeine Tipps:

1. **Tests vor Ort:** Machen Sie Mini-Tests auf dem echten Spielfeld, bevor das Spiel beginnt
2. **Beide Kameras prüfen:** Die obere und untere Kamera haben oft unterschiedliche Lichtverhältnisse. Die obere schaut nach oben (Ball), die untere nach unten (Feldlinien)
3. **Konservativ sein:** Lieber ein bisschen zu dunkel als überbelichtet. Zu dunkel lässt sich später noch justieren
4. **Saturation & Contrast vor Spiel erhöhen:** Dies hilft bei erkennbaren Farben und Kontrasten
5. **Sharpness auf 4-5 belassen:** Nicht zu hoch, sonst wird das Bild künstlich wirken

### 🔧 Einstellungs-Workflow vor Spiel:

1. **Ort erkunden:** 5-10 Minuten vor dem Spiel auf den Platz gehen
2. **Lichtverhältnisse beurteilen:**
   - Direkte Sonne? → Sehr grelles Licht
   - Hallenlicht? → Standardlicht
   - Gemischt? → Basieren Sie auf Majorität ("wo wird der Robot am meisten sein?")
3. **Config anpassen:** entsprechend wählen (siehe Szenario 3)
4. **Testen:** Robot Kamerabild anschauen (Simulator oder SSH)
5. **Nachfeinjustierung:** Klein-Anpassungen (autoExposureBrightness ±10, saturation ±5)

### 📱 Im Simulator testen:

Im VS Code / SimRobot:
1. Öffnen Sie `SimRobot`
2. Gehen Sie zu `Debugging -> Representations -> CameraSettings`
3. Passen Sie die Werte **live** an (ohne Neustart)
4. Schauen Sie sich `CameraImage` und `CameraImageUpper` / `CameraImageLower` an
5. Wenn zufrieden → Werte in `cameraSettings.cfg` übertragen

---

## Häufig auftretende Probleme

### 🔴 Problem 1: Bild ist überbelichtet (zu hell), Linien sind nicht zu sehen

**Symptome:**
- Großes weißes "Flatt-Bild"
- Feldlinien verschwunden
- Ball kaum zu sehen

**Lösung:**
1. Reduzieren Sie `autoExposureBrightness` um 20-50 Punkte
   - Versuchen Sie: -20, dann -40, dann -60
2. Erhöhen Sie `saturation` und `contrast` um 10-20 Punkte
3. Falls immer noch überbelichtet: Wechseln Sie zu `autoExposure = false` und reduzieren Sie `exposure` um 500 Punkte

**Schnell-Fix (3 Zeilen):**
```properties
autoExposureBrightness = -40;  # Statt 0
saturation = 95;               # Statt 80
contrast = 80;                 # Statt 65-70
```

---

### 🔴 Problem 2: Bild ist unterbelichtet (zu dunkel)

**Symptome:**
- Dunkles Bild, man sieht fast nichts
- Ball und Feld sind nicht erkennbar

**Lösung:**
1. Erhöhen Sie `autoExposureBrightness` um 10-30 Punkte
2. Oder: Erhöhen Sie `exposure` und `gain` (wenn manuell)
3. Prüfen Sie, ob Hallenlicht aus ist oder Kamera-LED funktioniert

**Schnell-Fix:**
```properties
autoExposureBrightness = 20;   # Statt 0 oder -X
saturation = 85;
contrast = 70;
```

---

### 🔴 Problem 3: Obere Kamera OK, untere Kamera überbelichtet (oder umgekehrt)

**Symptome:**
- Eine Kamera sieht gut aus, die andere ist zu hell/dunkel

**Lösung:**
- Passen Sie **nur** die problematische Kamera an (upper oder lower Sektion)
- Es ist normal, dass die beiden Kameras unterschiedliche Einstellungen brauchen!

**Beispiel:**
```properties
cameras = {
  upper = {
    autoExposureBrightness = -20;    # OK
    saturation = 90;
    contrast = 75;
  };
  lower = {
    autoExposureBrightness = -40;    # Sehr grelles Licht auf untere Kamera
    saturation = 100;
    contrast = 85;
  };
};
```

---

### 🔴 Problem 4: Auto-Belichtung ist zu instabil, Bild flackert

**Symptome:**
- Bildhelligkeitswert ändert sich ständig
- Flackern im Livestream

**Lösung:**
1. **Nicht zu aggressive autoExposureBrightness:** -40 bis 0, nicht extremer
2. Oder: Wechseln Sie zu `autoExposure = false` und nutzen Sie manuelle Werte
3. Stellen Sie `exposure` und `gain` auf feste Werte

**Beispiel (stabile Manuelle):**
```properties
autoExposure = false;
exposure = 2500;
gain = 150;
saturation = 90;
contrast = 75;
sharpness = 4;
```

---

### 🔴 Problem 5: Farben wirken falsch (zu rot/grün/blau)

**Symptom:**
- Feldlinien haben falsche Farbe
- White Balance zu aggressiv

**Lösung:**
1. Prüfen Sie `autoWhiteBalance`:
   - Wenn true: In Ordnung, lässt Auto die Farben kalibrieren
   - Wenn false: Passen Sie manuell `redGain`, `greenGain`, `blueGain` an (alle auf ~2000 setzen)
2. Setzen Sie `autoHue = true` (ermöglicht Farb-Autokalibrierung)

**Schnell-Fix:**
```properties
autoWhiteBalance = true;  # Auto-Farben-Kalibrierung
autoHue = true;           # Auto-Farbton
redGain = 2048;
greenGain = 2048;
blueGain = 2048;
```

---

## Zusammenfassung: Checkliste vor dem Spiel

- [ ] **Lichtverhältnisse erkunden** → Hallenlicht / Grelles Licht / Gedimmt?
- [ ] **Config auswählen** →  Entsprechendes Szenario aus dieser Anleitung
- [ ] **Datei bearbeiten** → `Config/Locations/Default/cameraSettings.cfg`
- [ ] **Beide Kameras anpassen** → `upper` und `lower` Sektionen
- [ ] **Auf Überexposure testen** → Simulator oder SSH Kamerabild anschauen
- [ ] **autoExposureBrightness sensibel anpassen** → Nicht zu aggressiv
- [ ] **Saturation & Contrast erhöhen** → Bessere Farbund Kontrast-Erkennung
- [ ] **Finale Tests** → Im Simulator oder auf echtem Feld vor Spielstart
- [ ] **Nach Spiel dokumentieren** → Welche Einstellungen haben gut funktioniert?

---

## Weitere Ressourcen

- **Code:** [NaoCamera.cpp](../Src/Platform/Nao/NaoCamera.cpp) - Kamera-Implementierung
- **Settings struct:** [CameraSettings.h](../Src/Representations/Configuration/CameraSettings.h) - Alle Einstellungstypen
- **Auto-Exposure:** [AutoExposureWeightTable.cpp](../Src/Representations/Configuration/AutoExposureWeightTable.cpp) - Gewichtungs-Tabelle für Auto-Belichtung

---

**Letzte Überarbeitung:** Februar 2026  
**Erstellt für:** R-ZWEI KICKERS RoboCup SPL Team
