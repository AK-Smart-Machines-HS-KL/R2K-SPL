# Whistle Identification Optimization

## Beschreibung
Bei Turnieren im Rahmen der vergangenen RoboCup-Wettbewerbe ist aufgefallen, dass unsere Roboter zwar recht gut darin sind die verwendeten Whistles zu erkennen (wenige bis gar keine False Negatives, Erkennung von 60-70% aller Whistles im Game), aber einige Geräusche zu viel als solche interpretieren (einiges an False Positives).

Um False Positives zu vermeiden, soll in diesem Projekt die Signatur verwendeter Whistles (mit FFTs) untersucht werden, um festzustellen, ob und wenn ja welche weiteren Optimierungen nötig/möglich sind, damit Störgeräusche (z.B. Kinderjauchzen) gefiltert werden können.

## Projektziel
Das Hauptziel dieses Projekts ist die Optimierung der Pfeiffenerkennung. Spezifische Herausforderungen, die gelöst werden sollen, sind zu viele falsch-positive Erkennungen, z.B. Pfeifen von Nachbarfeldern. Als erster Schritt wird das Abfangen von falsch-positiver Erkennungen durch die Dynamik des Gamecontrollers umgesezt.

## Funktionalitäten
Die Hauptfunktionen des Projekts umfassen die Funktionen der FFTW3 Library sowie die bereits vorhandene Update- und Correlete-Funktion aus WhistleRecognizer.cpp und .h . Spezielle Algorithmen, die verwendet werden, sind die Fast Fourier Transformation (FFT) und die Diskrete Fourier Transformation (DFT).

## Technologien und Tools
Die verwendete Programmiersprache ist C++. Das Framework Qt wird ebenfalls genutzt. Als Codebasis dient das R2 Kickers GitHub Repository. Der grundlegende Code für den Roboter ist für eine Ubuntu Linux Plattform. Der Simulator ist auf mehreren Plattformen nutzbar, z.B. Windows und Linux.

## Spezielle Voraussetzungen
Wie bei jeder akademischen Prüfung werden keine Plagiate akzeptiert. Die Bearbeitung des Projekts erfolgt ausschließlich durch den Studenten.

## Lizenz
Dieses Projekt wird unter der GNU-Lizenz veröffentlicht.

## Kontaktinformationen
Der Hauptverantwortliche für das Projekt ist Dimitri Feuerstein. Kontakt: E-Mail: dipa1001@stud.hs-kl.de, Alternative E-Mail: dpaprnc@gmail.com.

## Zukünftige Entwicklungen
Zukünftige Entwicklungen umfassen die Verwendung von Passfiltern und Hanning-Fenstern sowie die Implementierung eines neuronalen Netzwerks mit drei Schichten.

## Diagramm zum ersten Teilschritt
![Draw_io_Whistle_Recognizer_Process png](https://github.com/user-attachments/assets/33bd1934-cc9c-4eb0-b250-a3694255cb9e)

## aufgetretene Probleme
[x] ~~Problem mit dr module:WhistleRecognizer:record~~ // Wurde gelöst durch versetzen in penezile -> not penelized -> penelized 
[ ] Problem mit versetzen in STATE_PLYING um die Auswahl und Erstellung der Pfeife
