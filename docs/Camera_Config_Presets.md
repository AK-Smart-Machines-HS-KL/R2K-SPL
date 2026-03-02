# Kamera-Konfigurationen: Vordefinierte Szenarien

Diese Datei enthält optimierte Konfigurationsblöcke für verschiedene Lichtsituationen.  
**Verwendung:** Copy & Paste den passenden Block in `Config/Locations/Default/cameraSettings.cfg`

---

## 1️⃣ GRELLES/DIREKTES LICHT (Sonnenlicht, starke Spotlights)

Überexposure-Gefahr ist HOCH. Dunklerer Zielwert erforderlich.

```properties
cameras = {
  upper = {
    autoExposure = true;
    autoExposureBrightness = -50;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 100;
    contrast = 85;
    sharpness = 5;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
  lower = {
    autoExposure = true;
    autoExposureBrightness = -50;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 100;
    contrast = 85;
    sharpness = 5;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
};
```

**Merkmale:**
- autoExposureBrightness = -50 (deutlich dunkler)
- saturation = 100 (höhere Farbsättigung zum Kompensieren)
- contrast = 85 (starker Kontrast)
- sharpness = 5 (extra scharf)

---

## 2️⃣ NORMALES HALLENLICHT (Standard RoboCup, ausgeglichenes Licht)

Basis-Konfiguration für typische Umgebungsbedingungen.

```properties
cameras = {
  upper = {
    autoExposure = true;
    autoExposureBrightness = -20;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 90;
    contrast = 75;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
  lower = {
    autoExposure = true;
    autoExposureBrightness = -25;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 90;
    contrast = 80;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
};
```

**Merkmale:**
- autoExposureBrightness = -20 (leicht dunkler)
- saturation = 90 (ausgeglichen)
- contrast = 75/80 (guter Kontrast, untere Kamera etwas höher)
- sharpness = 4 (Standard)

**Hinweis:** untere Kamera hat -25 statt -20, da sie oft mehr Überexposure anfällig ist.

---

## 3️⃣ SCHWACHES/GEDIMMTES LICHT (dunkle Halle, Schule, schlechte Ausleuchtung)

Hellerer Zielwert erforderlich, um ausreichend Bildhelligkeit zu haben.

```properties
cameras = {
  upper = {
    autoExposure = true;
    autoExposureBrightness = 10;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 85;
    contrast = 65;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
  lower = {
    autoExposure = true;
    autoExposureBrightness = 15;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 85;
    contrast = 65;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
};
```

**Merkmale:**
- autoExposureBrightness = 10-15 (heller statt dunkler!)
- saturation = 85 (leicht erhöht)
- contrast = 65 (nicht zu aggressiv)
- sharpness = 4 (Standard)

---

## 4️⃣ SEHR GRELLES LICHT - EXTREM (sehr aggressives Sonnenlicht)

Für extreme Situationen mit extrem hellem Licht (z.B. Freifeld-Turnier).

```properties
cameras = {
  upper = {
    autoExposure = true;
    autoExposureBrightness = -70;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 120;
    contrast = 95;
    sharpness = 6;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
  lower = {
    autoExposure = true;
    autoExposureBrightness = -70;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 120;
    contrast = 95;
    sharpness = 6;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
};
```

**Merkmale:**
- autoExposureBrightness = -70 (SEHR dunkel)
- saturation = 120 (maximal erhöht)
- contrast = 95 (maximal aggressiv)
- sharpness = 6 (extra scharf)

**⚠️ WARNUNG:** Nur verwenden, wenn Standardconfig nicht ausreicht!

---

## 5️⃣ MANUELLE BELICHTUNG - STABIL (wenn Auto-Belichtung flackert)

Vollständige manuelle Kontrolle ohne Auto-Belichtung.

```properties
cameras = {
  upper = {
    autoExposure = false;
    autoExposureBrightness = 0;
    exposure = 2500;
    gain = 140;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 90;
    contrast = 75;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
  lower = {
    autoExposure = false;
    autoExposureBrightness = 0;
    exposure = 2500;
    gain = 140;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 90;
    contrast = 80;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
};
```

**Merkmale:**
- autoExposure = **false** (manuell!)
- exposure = 2500 (Belichtungszeit in Mikrosekunden)
- gain = 140 (Verstärkung, reduziert vs. Standard 160)
- Stabilere Bildqualität, keine Flackern

**Verwendungsfall:** Wenn Auto-Belichtung zu viel "flackert" oder oszilliert.

---

## 6️⃣ KONSERVATIV (für Anfänger, sicher gegen Überexposure)

"Lieber zu dunkel als überbelichtet" - Philosophie.

```properties
cameras = {
  upper = {
    autoExposure = true;
    autoExposureBrightness = -35;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 95;
    contrast = 80;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
  lower = {
    autoExposure = true;
    autoExposureBrightness = -40;
    exposure = 2000;
    gain = 160;
    autoWhiteBalance = true;
    autoFocus = false;
    focus = 0;
    autoHue = false;
    hue = 0;
    saturation = 95;
    contrast = 85;
    sharpness = 4;
    redGain = 2048;
    greenGain = 2048;
    blueGain = 2048;
  };
};
```

**Merkmale:**
- autoExposureBrightness = -35 bis -40 (sicher gegen Überexposure)
- saturation/contrast = erhöht (kompensiert dunkleres Bild)
- Untere Kamera nochmal dunkler (-40 vs -35)

---

## 🔧 ANLEITUNG ZUM VERWENDEN:

1. **Lichtzustand erkunden** → Welche Szene passt am besten?
2. **Passende Config oben auswählen** → Copy den kompletten Block
3. **Datei öffnen:** `Config/Locations/Default/cameraSettings.cfg`
4. **Alten Content ersetzen** → Paste die neue Config
5. **Speichern**
6. **Robot/Simulator neu laden** → Config wird geladen
7. **Testen** → CameraImage anschauen
8. **Fine-tuning:** autoExposureBrightness ±10, saturation ±5

---

## 📊 QUICK-VERGLEICH DER CONFIGS:

| Szenario | autoExposureBrightness | saturation | contrast | Überexposure-Sicher |
|----------|------------------------|------------|----------|---------------------|
| Normales Licht | -20 | 90 | 75 | ★★★ |
| Grelles Licht | -50 | 100 | 85 | ★★★★★ |
| Sehr grelles Licht | -70 | 120 | 95 | ★★★★★ |
| Schwaches Licht | 10 | 85 | 65 | ★ |
| Manuell | 0 | 90 | 75 | ★★★★ |
| Konservativ | -35 bis -40 | 95 | 80-85 | ★★★★★ |

---

## 💡 TIPPS ZUM FEINEN ABSTIMMEN:

**autoExposureBrightness anpassen:**
- -3 bis -5 Punkte: Sehr feine Justiering
- -10 Punkte: Kleine Anpassung
- -20 Punkte: Deutliche Änderung
- Maximalwerte: -70 (sehr dunkel) bis +50 (sehr hell)

**saturation anpassen:**
- Zu niedrig (< 70): Farben wirken grau
- Optimal (80-95): Farblinien klar erkennbar
- Zu hoch (> 120): Übergesättigt, unrealistisch

**contrast anpassen:**
- Zu niedrig (< 50): Linien und Hintergrund verschmelzen
- Optimal (70-85): Guter Unterschied Linie-Feld
- Zu hoch (> 95): Zu aggressive Kantenbetonung

---

**Letzte Aktualisierung:** Februar 2026

