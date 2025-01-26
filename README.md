# Whistle Identification Optimization

## Beschreibung
Bei Turnieren im Rahmen der vergangenen RoboCup-Wettbewerbe ist aufgefallen, dass unsere Roboter zwar recht gut darin sind, die verwendeten Whistles zu erkennen (wenige bis gar keine False Negatives, Erkennung von 60-70% aller Whistles im Spiel), aber einige Geräusche zu viel als solche interpretieren (einiges an False Positives).

Um False Positives zu vermeiden, soll in diesem Projekt die Signatur verwendeter Whistles (mit FFTs) untersucht werden, um festzustellen, ob und wenn ja, welche weiteren Optimierungen nötig/möglich sind, damit Störgeräusche (z.B. Kinderjauchzen) gefiltert werden können.

## Projektziel
Das Hauptziel dieses Projekts ist die Optimierung der Pfeiffenerkennung. Spezifische Herausforderungen, die gelöst werden sollen, sind zu viele falsch-positive Erkennungen, z.B. Pfeifen von Nachbarfeldern. Als erster Schritt wird das Abfangen von falsch-positiven Erkennungen durch die Dynamik des Gamecontrollers umgesetzt.

## Ansatz
Der bestehende Whistle-Detection-Code (zu finden in den Dateien `WhistleRecognition.cpp` und `.h`) wird verwendet, um versehentliche Pfeifenaufnahmen von benachbarten Feldern von "unserer" Pfeife zu unterscheiden. Dies soll zunächst über die Dynamik des Spielablaufes geschehen.

Wie erkennen wir "unsere" Pfeife:
1. Es werden alle Pfeifen detektiert und deren Zeitstempel und Namen der erkannten Pfeifen (z.B. "Fox40") in einem Vector gespeichert und "unsere" in einem String (siehe Diagramm unten).
2. Durch die Spieldynamik gegeben ist, dass der GameController 15 Sekunden nachdem "unsere" Pfeife gepfiffen wurde eine Bestätigung rausschickt, damit jeder Roboter – auch die, die diese überhört oder nicht erkannt haben – mit dem Spiel starten können.
3. Ab dem Zeitpunkt, an dem der GameController offiziell den Spielstatus PLAYING ausruft, wird im Code zurückgerechnet, welche der detektierten Pfeifen die beste absolute Zeitdifferenz aufweist. Die Pfeife mit der geringsten Zeitdifferenz wird als String in `closestWhistle` gespeichert (siehe Diagramm unten).

## Diagramm zum Ansatz
Das Diagramm beschreibt zum größten teil den derzeitigen Programfluss der Whistle Detection. Die zu implementierenden Teile sind sind bei "Find best Signature" und die Abzweigung "Game Start" zu sehen.

![Draw_io_Whistle_Recognizer_Process png](https://github.com/user-attachments/assets/33bd1934-cc9c-4eb0-b250-a3694255cb9e)

## Funktionalitäten
Die Hauptfunktionen des Projekts umfassen die Funktionen der FFTW3 Library sowie die bereits vorhandene Update- und Correlate-Funktion aus `WhistleRecognizer.cpp` und `.h`. Spezielle Algorithmen, die verwendet werden, sind die Fast Fourier Transformation (FFT) und die Diskrete Fourier Transformation (DFT).

## Technologien und Tools
Die verwendete Programmiersprache ist C++. Das Framework Qt wird ebenfalls genutzt. Als Codebasis dient das R2 Kickers GitHub Repository. Der grundlegende Code für den Roboter ist für eine Ubuntu Linux Plattform. Der Simulator ist auf mehreren Plattformen nutzbar, z.B. Windows und Linux.

## Spezielle Voraussetzungen
Wie bei jeder akademischen Prüfung werden keine Plagiate akzeptiert. Die Bearbeitung des Projekts erfolgt ausschließlich durch den Studenten.

## Lizenz
Dieses Projekt wird unter der GNU-Lizenz veröffentlicht.

## Kontaktinformationen
Der Hauptverantwortliche für das Projekt ist Dimitri Feuerstein. Kontakt: E-Mail: dipa1001@stud.hs-kl.de.

## Zukünftige Entwicklungen
Zukünftige Entwicklungen umfassen die Verwendung von Passfiltern und Hanning-Fenstern sowie die Implementierung eines neuronalen Netzwerks mit drei Schichten.

## Dokumentierter Fortschritt

### Generierte Aufnahmen im Ready State 20-01-2025:

![Screenshot 2025-01-20 182436](https://github.com/user-attachments/assets/35cb7be9-e47e-43b1-91f2-6189be4fe2f5)

### Generierte Aufnahmen im Set und Ready State 25-01-2025:

![Screenshot 2025-01-24 210717](https://github.com/user-attachments/assets/48b15614-cc81-466c-a625-e5363f1a3868)

### Code Snippets

```cpp
//Codesnippet die in WhistleRecognizer.h hinzugefügt wurden um die detektierten Pfeifen zu Speichern und die Globale Variable für "unsere" Pfreife

std::vector<std::pair<std::string, int>> whistleTimes; /**< Stores whistle names and their detection times. */
std::string closestWhistle; /**< Stores the whistle closest to STATE_PLAYING time. */

//If-Statement für die Berechnung der "closestWhistle"
if (theGameInfo.state == STATE_PLAYING)
{
  // Find the whistle closest to the STATE_PLAYING time
  int playingTime = theFrameInfo.time;
  int minDiff = std::numeric_limits<int>::max();
  for (const auto& whistleTime : whistleTimes)
  {
    int diff = std::abs(whistleTime.second - playingTime);
    if (diff < minDiff)
    {
      minDiff = diff;
      closestWhistle = whistleTime.first;
    }
  }
  OUTPUT_TEXT("Whistle: " << closestWhistle << " found as closest Whistle");
  ANNOTATION("WhistleRecognizer", closestWhistle << " with difference off " << minDiff);
}
```
## Testen

Das Testen des Codes ist am einfachsten mit einem funktionierenden [Game Controller](https://github.com/AK-Smart-Machines-HS-KL/R2K-SPL/wiki/Real-RoboCup-Matches#gamecontroller). Beim Starten ist zu beachten, dass auf einer Seite das R2K-Team gewählt wird und als Verbindungsmodus WLAN ausgewählt wird. Die Verbindung kann vorher im Bus mit der Einstellung "Wifi" getestet werden - Einstellungen findet ihr [hier](https://github.com/AK-Smart-Machines-HS-KL/R2K-SPL/wiki/Real-RoboCup-Matches#roboter-deployenverbinden). Es wird empfohlen, den Bot im Bus auf den mittleren Platz #3 zu setzen.

Wenn der Nao im Entwicklungsmodus bereitgestellt wurde, kann gleichzeitig der Simulator gestartet werden, um die gewünschten Konsolenausgaben zu sehen. Beim Starten der `GameController.exe` mit den oben genannten Einstellungen sollte ein Roboter einen "grünen" Punkt anzeigen. Nun klickt man auf den großen "Ready"-Knopf und gleich danach auf "Set", um zu vermeiden, dass sich der Nao bewegt. Ab hier kann die Whistle Detection getestet werden.

![Screenshot 2025-01-25 151718](https://github.com/user-attachments/assets/65895d56-c58e-40b7-bbdb-b8a334b3a208)
![Screenshot 2025-01-25 151736](https://github.com/user-attachments/assets/e82ef80b-2e30-443e-b542-1fd1f23eb7c7)
![Screenshot 2025-01-25 151748](https://github.com/user-attachments/assets/2107dd95-f273-486c-bf41-057d8b2a82d8)

Die Whistle Detection läuft auf ihrem eigenen Thread, das heißt, wenn die Eingangsvoraussetzungen – Spielstatus ist SET oder PLAYING – erfüllt sind, sollten, -falls der Code funktioniert- die Pfeifen erkannt werden.

## Derzeitige Blocker / Probleme
- [x] ~~Ausführung von: dr module:WhistleRecognizer:record~~
- [x] ~~Code Compelierung~~ // Lösung hier war, die Error Ausgaben von VisualStudioCode zu ignorieren und nur die Errors in der Konsole von oben nach unten zu beseitigen.
- [x] ~~nutzen des STREAMABLE Makros zum Schreiben des Spectrums von der "TrueWhistle"~~
- [x] ~~folgt aus dem Punkt darüber. NAO ERROR LOG: '11:50:45 /mnt/c/Development/R2K-SPL/Src/Platform/File.cpp:54: VERIFY(!eof()) failed'~~
- [x] ~~Verwendung eines Gamecontrollers hat im ersten Anlauf nicht funktioniert~~ // Lösung für das Problem war das eine der Einstellung der Gamecontroller Executable vor dem Start zwingenderweise WLAN sein musste. Diese Information wurde im Wiki nicht gefunden.
- [x] ~~Keine Ausgaben auf die Konsole nachdem der Gamecontroller gestartet wurde. Vermutung fehlerhafter Code, weil auch keine Annotations in die Logs geschrieben wurden~~ // Ein Logikfehler im Code hatte dazu geführt das die IF-Statements im Code in denen die Konsolenausgaben und Annotations enthalten waren nicht erreicht wurden.
