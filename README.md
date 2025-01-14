# Audio Beeps Communication;

### Kurzzusammenfassung
Aufbauend auf den (Teil-) Projekten der Studien-Kollegen Fortune, Hobelsberger und Simus, soll die Audio-Kommunikation erweitert werden. Bisher ist es dem Team gelungen erfolgreich Sender und Empfänger für ein Audio-Kommunikationsprotokoll in einer "Proof of Concept"-Variante umzusetzen. Der nächste Schritt ist, dem Ganzen ein Protokoll zuzuweisen, über welches die Bots ihren Nachrichten Sinn verleihen und welches im Spiel eingesetzt werden kann. Gleichzeitig dient dies als Robustness-Test für den bisherigen Code, da die Roboter darin bisher immer nur das gleiche Signal erkannt haben.

Entnommen aus Wiki

### Beschreibung bisheriger Code
Die Übertragung findet mittels Sinuswellen deren Frequenzen startend mit 500 in 200 Schritten die Nummer des Roboters kodiert(1->500,2->700...).
In den Frequenzen dazwichen werden messages als int von 0-15 kodiert(nicht verifieziert da im code bisher nur "msg 1" benutzt wird) und mittels fftw(Fourier-Transformation) dekodiert und in Form eines int Vektors(0-15) mit [x] Roboter nummer x-1  in "theBeeb" zurückgegeben.
Bisher kann ein msg1 beep mit betätigen des Kopftasters ausgelöst werden und Roboter die nicht die Nummer 1 sind antworten mit einem msg1 beep auf 500hz(msg1 beep roboter 1) beeps.


### Aktuelle Problme
-aufgrund von Rezonanzfrequenzen werden bei manchen Frequenzbändern mehr als ein sendenter Roboter erkannt. Beispiel bei 500 hz werden Roboter 1 und 3 erkannt(zahlen im Beispiel möglicherweise nicht richtig)
-oben genanntes im Labor verifiziert aber zuhause nicht reproduzierbar mit Verdacht auf Hardwareprobleme