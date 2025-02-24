# Whistle Identification Optimization

## Beschreibung
Bei Turnieren im Rahmen der vergangenen RoboCup-Wettbewerbe ist aufgefallen, dass unsere Roboter zwar recht gut darin sind, die verwendeten Whistles zu erkennen (wenige bis gar keine False Negatives, Erkennung von 60-70% aller Whistles im Spiel), aber einige Geräusche zu viel als solche interpretieren (einiges an False Positives).

Um False Positives zu vermeiden, soll in diesem Projekt die Signatur verwendeter Whistles (mit FFTs) untersucht werden, um festzustellen, ob und wenn ja, welche weiteren Optimierungen nötig/möglich sind, damit Störgeräusche (z.B. Kinderjauchzen) gefiltert werden können.

## Projektziel
Das Hauptziel dieses Projekts ist die Verbesserung der Identifikation der Pfeiffenerkennung. Spezifische Herausforderungen, die gelöst werden sollen, sind zu viele falsch-positive Erkennungen, z.B. Pfeifen von Nachbarfeldern. Als erster Schritt wird das Abfangen von falsch-positiven Erkennungen durch die Dynamik des Gamecontrollers umgesetzt.

## Ansatz
Der bestehende Whistle-Detection-Code (zu finden in den Dateien `WhistleRecognition.cpp` und `.h`) wird verwendet, um versehentliche Pfeifenaufnahmen von benachbarten Feldern von "unserer" Pfeife zu unterscheiden. Dies soll zunächst über die Dynamik des Spielablaufes geschehen.

Wie erkennen wir "unsere" Pfeife:
1. Es werden alle Pfeifen detektiert und deren Zeitstempel und Namen der erkannten Pfeifen (z.B. "Fox40") in einem Vector gespeichert und "unsere" in einem String (siehe Diagramm unten).
2. Durch die Spieldynamik gegeben ist, dass der GameController 15 Sekunden nachdem "unsere" Pfeife gepfiffen wurde eine Bestätigung rausschickt, damit jeder Roboter – auch die, die diese überhört oder nicht erkannt haben – mit dem Spiel starten können.
3. Ab dem Zeitpunkt, an dem der GameController offiziell den Spielstatus PLAYING ausruft, wird im Code zurückgerechnet, welche der detektierten Pfeifen die beste absolute Zeitdifferenz aufweist. Die Pfeife mit der geringsten Zeitdifferenz wird als String in `closestWhistle` gespeichert (siehe Diagramm unten).

## Diagramm zum Ansatz
Das Diagramm beschreibt den derzeitigen Programfluss der Whistle Detection. Für die zwei Teilschritte wurden die "grün" markierten Bereiche des Codes im WhistleRecognizer.cpp verändert. Die WhistleRecognizer.h wurde zum Speichern der Besten "Correlations" und der "ClosestWhistle" um den <string,int>Vectoer "WhistleTimes" und den string "closestWhistle" erweitert.

![diagram12 2 2025-15_03_09](https://github.com/user-attachments/assets/cd4a0b9c-43d8-4cd3-aeae-c215b1807311)

## Funktionalitäten
Die Hauptfunktionen des Projekts umfassen die Funktionen der FFTW3 Library sowie die bereits vorhandene Update- und Correlate-Funktion aus `WhistleRecognizer.cpp` und `.h`. Spezielle Algorithmen, die verwendet werden, sind die Fast Fourier Transformation (FFT) und die Diskrete Fourier Transformation (DFT).

## Kontaktinformationen
Der Hauptverantwortliche für das Projekt ist Dimitri Feuerstein. Kontakt: E-Mail: dipa1001@stud.hs-kl.de.



## Dokumentierter Fortschritt

### Teilschritt 1
Getestet wurde mit zwei unterschiedlichen Pfeifen durch zwei unterschiedliche Videos auf YouTube. Einmal Fox40 Classic und einmal Fox Black.
Bedingt durch den Testablauf und der Zeit die der NAO an ist ergeben sich unterschiedliche Werte für 'mindiff'.

#### Szenarien bedingt durch Testablauf
1. **Abspielen der Pfeife bevor "Playing" im Gamecontroller gestartet wird:**
   - Die ausgegebene 'mindiff' Zeit ist größer als im Fall 3.

2. **Abspielen der Pfeife nachdem "Playing" im Gamecontroller gestartet wird:**
   - Die ausgegebene 'mindiff' Zeit ist größer als im Fall 3.

3. **Abspielen der Pfeife zeitgleich mit dem Gamecontroller:**
   - Die minimale 'mindiff' Zeit ist sehr klein.

### Code Snippets Teilschritt 1

```cpp
//Codesnippet die in WhistleRecognizer.h hinzugefügt wurden um die detektierten Pfeifen zu Speichern und 
//die Globale Variable für "unsere" Pfreife
//Weil wir nicht wissen welche Pfeife "unsere ist müssen wir vor dem Spielbeginn alle Pfeifen aufnehmen. 
//Von jeder bestSignature Pfeife wird der Name als string und die Zeit als int gespeicher in einem vector gespeichert.

std::vector<std::pair<std::string, int>> whistleTimes; /**< Stores whistle names and their detection times. */
std::string closestWhistle; /**< Stores the whistle closest to STATE_PLAYING time. */

//If-Statement für die Berechnung der "closestWhistle" (string)
//Suche closestWhistle wenn im "STATE_PLAYING" und closestWhistle seid dem Start noch nicht gefunden wurde
// closestWhistle.empty() = die Whistle wird aktiv gesetzt 
if (theGameInfo.state == STATE_PLAYING && closestWhistle.empty())
{
  // Finde die Pfeife die am die kleine Zeit zwischen der zeit von STATE_PLAYING -15 Sekunden und 
  //den gespeicherten Pfeifen aufzeigt
  int playingTime = theFrameInfo.time - 15000; // time were game state playing - 15000 milliseconds = 15 seconds
  int minDiff = std::numeric_limits<int>::max();
  // Für jede vorher gespeicherte bestSignature wird nun geprüft welcher absolute 
  //Zeitunterschied der kleinste ist, diese wird als closestWhistle gespeichert
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

#### Bilder Teilschritt 1
![Screenshot 2025-01-30 181849](https://github.com/user-attachments/assets/61e4742f-5400-459d-96b2-245e2ee5f063)
![Screenshot 2025-01-30 180049](https://github.com/user-attachments/assets/5f48a547-0961-413e-92d7-935066cad062)

#### Ergebnisse
Die beiden Tests zeigen, dass die Pfeifen erkannt werden und die Pfeife mit der minimalsten Zeit zwischen Spielfreigabe des Gamecontrollers -15 Sekunden gewählt wird. Hier wurde zudem beim Testen mit [Video 1](https://www.youtube.com/watch?v=Q3EYBVUgcXo&ab_channel=fox40world) festgestellt, dass bei der Fox Black auch andere Pfeifen die Fox Blue oder Fox Silver erkannt werden, was auf sehr ähnliches Spektrum oder einen schlechtes Sample hinweisen könnte.

### Teilschritt 2
Das Ziel in diesem Teilschritt ist, dass nur noch die Pfeifen mit demselben Namen wie die in Teilschritt 1 gefundene `closestWhistle` vom NAO erkannt werden. Alle anderen sollen für die gesamte Zeit, in der der NAO nicht rebootet wird, erhalten bleiben.

#### Code Snippets Teilschritt 2

```cpp
for (auto& signature : signatures)
{// Nutze die funktion Correlate nur wenn der Name von closestWhistle gleich dem namen der aktuellen Pfeife und closestWhistle nich leer ist
    if (!closestWhistle.empty() && signature.name != closestWhistle)
    {
      continue;
    }
    //Wenn der Name mit closestWhistle übereinstimmt suche das beste Sample
    if (selectedIter == signatures.end() || &signature == &*selectedIter)
    {
```

#### Bild Teilschritt 2
![Screenshot 2025-01-30 185307](https://github.com/user-attachments/assets/2fe2734c-7ab7-42af-9728-a75dc7d0818b)


#### Ergebnisse
Ein großer Zeitsprung zwischen den beiden erkannten Pfeifen oberhalb und unterhalb der roten Linie ist zu erkennen. Hier wurde versucht, mit [Video 2](https://www.youtube.com/watch?v=99E9mi87XgM&ab_channel=WhistlePerformerMr.Kojyoro) (Fox Black) eine Pfeifenerkennung zu generieren. Erst als wieder auf Fox 40 Classic Sounds abgespielt wurde, wurden neue Pfeifen erkannt.

#### Schlussfolgerung
Der `WhistleRecognizer`-Modul wurde erfolgreich erweitert, um nur die Pfeifen mit demselben Namen wie die gefundene `closestWhistle` zu erkennen. Die Tests zeigen, dass das Modul korrekt funktioniert und die gewünschten Pfeifen erkennt.

## Testenaufbau

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


## Mögliche zukünftige Entwicklungen 

### Hanning-Fensterfunktionen
Hanning-Fensterfunktionen sind eine wichtige Technik in der Signalverarbeitung, die dazu beitragen kann, die Anzahl falsch positiver Erkennungen bei der Identifikation spezifischer Geräusche zu reduzieren.

Ein Hanning-Fenster ist eine spezielle Fensterfunktion, die verwendet wird, um die spektrale Leckage (spectral leakage) zu minimieren. Spektrale Leckage tritt auf, wenn die Frequenzkomponenten eines Signals aufgrund der endlichen Länge des Fensters in benachbarte Frequenzbänder "auslaufen". Dies kann zu falsch positiven Erkennungen führen, da Frequenzen, die nicht im ursprünglichen Signal vorhanden sind, fälschlicherweise als vorhanden erkannt werden.[Link](https://en.wikipedia.org/wiki/Window_function#Hann_and_Hamming_windows)

### Passfilter

Passfilter sind eine grundlegende Technik in der Signalverarbeitung, die dazu dient, bestimmte Frequenzbereiche eines Signals zu isolieren oder zu unterdrücken. In der Whistle Detection können Passfilter verwendet werden, um die relevanten Frequenzbereiche des Pfeifsignals zu isolieren und störende Frequenzen zu unterdrücken. Dies hilft, das Signal-Rausch-Verhältnis (SNR) zu verbessern und die Erkennung des Pfeifsignals zu erleichtern.

Passfilter arbeiten, indem sie das Eingangssignal durch eine mathematische Funktion leiten, die bestimmte Frequenzen durchlässt und andere unterdrückt. Ein Bandpassfilter ist nützlich für die Whistle Detection, da es nur die Frequenzen durchlässt, die typischerweise von einem Pfeifsignal erzeugt werden, und alle anderen Frequenzen unterdrückt. [Link](https://en.wikipedia.org/wiki/Band-pass_filter)

### Neuronale Netzwerke

Wie beim Team von BHuman zu sehen können für die Whistle Detection ([WhistleNet](https://github.com/bhuman/BHumanCodeRelease/tree/master/Config/NeuralNets/WhistleDetector) und [WhislteDetector.cpp](https://github.com/bhuman/BHumanCodeRelease/blob/master/Src/Modules/Modeling/WhistleDetector/WhistleDetector.cpp)) neuronale Netzwerke verwendet werden, um die Merkmale des Pfeifsignals zu lernen und zu erkennen, selbst in Anwesenheit von Störgeräuschen und Überlagerungen. Dies kann die Genauigkeit der Erkennung erheblich verbessern und die Anzahl der false positives reduzieren. Die Schwierigkeit liegt hier jedoch in der Erzeugung eines guten Trainings Datensatzes. [Link](https://www.tensorflow.org/tutorials/quickstart/beginner)