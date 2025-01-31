# WAV to .DAT Converter c2Dat.py

## Funktionsbeschreibung

Dieses Skript konvertiert eine WAV-Datei in eine DAT-Datei. Es liest die WAV-Datei, überprüft die Abtastrate und den Datentyp, konvertiert die Audiodaten in 16-Bit-Integer, falls erforderlich, und speichert die Audiodaten in einer DAT-Datei. Das Skript stellt sicher, dass die Audiodaten mono sind, indem es die Kanäle mittelt, falls die Audiodaten stereo sind.

## Abhängigkeiten installieren

Dieses Skript benötigt die Bibliotheken `numpy` und `scipy`. Sie können diese Bibliotheken mit `pip` installieren:

```sh
pip install numpy scipy
```

## Skript verwenden

```sh
python c2Dat.py <input_wav_file> <output_dat_file>
```

Ersetze `<input_wav_file>` durch den Pfad zu WAV-Datei und `<output_dat_file>` durch den gewünschten Pfad für die DAT-Datei.

### Beispiel

```sh
python c2Dat.py input.wav output.dat
```

Dieser Befehl konvertiert die Datei `input.wav` in `output.dat`.

## Hinweise

- Das Skript überprüft, ob die Eingabedatei existiert, bevor es die Konvertierung durchführt.
- Falls die Audiodaten nicht im 16-Bit-Integer-Format vorliegen, werden sie entsprechend konvertiert.
- Stereo-Audiodaten werden in Mono-Audiodaten umgewandelt, indem die Kanäle gemittelt werden.