# Pfeifen Visualisierung WhistleRecognizer.py

## Nutzung

Um das Skript zu verwenden, stellt sicher, dass die erforderlichen Abhängigkeiten installiert sind und die `.dat`-Datei an dem angegebenen `file_path` verfügbar ist. Führt das Skript aus, um die Audiodaten zu verarbeiten und die Analysen durchzuführen.

## Installationsanleitung (Linux)

Um die erforderlichen Pakete auf Linux zu installieren, verwenden Sie die folgenden Befehle:

```bash
sudo apt-get update
sudo apt-get install python3-numpy python3-matplotlib python3-sounddevice python3-pydub
```

## Programmstart

Um das Programm zu starten:

```bash
python WhistleRecognizer.py
```

## Kurzbeschreibung

Dieser Code verarbeitet Audiodaten aus einer `.dat`-Datei, normalisiert sie und führt Analysen durch, einschließlich des Plotten des Wellenform, des Plotten des Frequenzbereichs und des Erkennens von Pfeiftönen innerhalb eines bestimmten Frequenzbereichs.