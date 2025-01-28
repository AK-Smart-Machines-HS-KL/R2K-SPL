# Audio Beeps Communication

## Kurzzusammenfassung
Aufbauend auf den (Teil-) Projekten der Studien-Kollegen Fortune, Hobelsberger und Simus, soll die Audio-Kommunikation erweitert werden. Bisher ist es dem Team gelungen erfolgreich Sender und Empfänger für ein Audio-Kommunikationsprotokoll in einer "Proof of Concept"-Variante umzusetzen. Der nächste Schritt ist, dem Ganzen ein Protokoll zuzuweisen, über welches die Bots ihren Nachrichten Sinn verleihen und welches im Spiel eingesetzt werden kann. Gleichzeitig dient dies als Robustness-Test für den bisherigen Code, da die Roboter darin bisher immer nur das gleiche Signal erkannt haben.

Entnommen aus Wiki

## Aktuelles Setup
Auf diesem Setup kann der Code aktuell gebaut sowie deployed werden.

Windows 10 

Visual Studio Version 17.2.2

Nao Nummer 11

Direkte Verbindung über Ethernet

IP-Adresse: 192.168.50.13  
Subnetzpräfix: 24  
Standartgateway: 192.168.50.1  
Bevorzugter DNS 1.1.1.1  
Alternativer DNS 0.0.0.0  

## Beschreibung bisheriger Code
### Broadcaster
Bei der Übertragung werden mittels Frequenzen startend von 500 in 200 Schritten die Nummer des Roboters kodiert(1->500,2->700...).  
In diesen 200Hz Bändern werden aktuell in der Standartconfig nur 1 Bit Kodiert und damit immer die Basisfrequenz(1->500,2->700...) kodiert.
Falls man nun die config auf 4 bit ändert beginnt er in 50Hz(Spanne skaliert mit Anzahl kodierter Bits und bandWidth) Schritten zu kodieren und mehrere davon anscheinend überlagerte.(siehe Spektogramme in [BeepTestSounds](BeepTestSounds)). 

Die Ein sowie Ausgabe basiert hier auf einem int 0-15, diese nennen wir message.

Ein Beep kann ausgelöst werden mit Betätigen des vorderen Kopfschalters. Die message kann in der [BeepBroadcaster.cfg](Config\Scenarios\Default/BeepBroadcaster.cfg) als "headButtonMessage" geändert werden. Hier können ebenso die Anzal der Bänder(Zahl der Roboter), sowie die Basisfrequenz und Bandbreite geändert werden.

### Regognizer
Hier wird mittels fftw(Fourier-Transformation) dekodiert und in Form eines std::vector messages(siehe [Beep.h](Src\Representations\Communication\Beep.h)) abgespeichert. Hier sind mit dem Index der Roboter und dem Inhalt die messages beschrieben Bsp. bei 5 Robotern und 4 bit   
|0|0|0|0|0| - keine message  |1|0|0|0|0| - message 1 bei Roboter 1(R1M1) |0|0|0|0|15| - message 15 bei Roboter 5(R5M15)


Bisher falls |1|0|0|0|0| dedektiert wurde, wurde falls der Roboters nicht die Nummer 1 trägt eine message 1 als Antwort gesendet gesendet.

## Zugriff auf OUTPUT_TEXT mit Simulator
Solange ein Nao in der bush verbunden ist, kann der Simulator genutzt werden um Daten des Roboter im laufenden Betrieb auszulesen. Hierzu zählen OUTPUT_TEXT() Statements die zum Debuggen bzw. einfachen Verifizieren des Codes verwendet werden kann. Hierzu muss bei verbundenem Roboter einfach nur in der bush, in der unteren Leiste der "Simulator" gestartet werden. Hier wird dann im "Console" Fenster der Output angezeigt.

Mit: << können Variabeln ausgegeben werden. Bsp.  
OUTPUT_TEXT("Irgendwas: " << x);

Merke nach jedem Deploy muss der Simulator neugestartet werden

## Änderungen am Code
### Config
averagingBufferSize = 15; averagingThreshold = 11; baseFrequency = 1500; bandWidth = 1000; encodedBits = 5;

Durch obrige Änderungen zumindest in simplen Tests ohne zusätzliche Hintergrundgeräusche eine genaue erkennung möglich. Mit Ausnahme von message 31 welche nicht eindeutig oder sogar falsch erkannt wird. Lösungsvorschlag hier die 31 einfach zu ignorieren.

### Broadcaster
Neue Methode "BeepBroadcaster::requestBeep(int robot_number, int message)" die als Schnittstelle dient um im Code Beeps zu senden. Bisher noch Probleme bei der Nutzung

### Regognizer
Code zum aufrufen gewünschter Aktionen auf Basis der empfangenen messages. Aktuell noch Probleme mit merfach aufrufen.

## Beep Test Sounds(Ordner)
in [BeepTestSounds](BeepTestSounds) sind mehrere aufgenommene beeps sowie Testergebnisse. 


## Alte Problme
### Verbindung
Es wurden mehrere Wege zu Verbindung getesten vovon letztenendes nur der oben genante Weg über eine Direkte Verbindung per Ethernet erfolgreich war. Im folgenden kurz eine Übersicht was probiert wurde und woran es vermutlich gescheitert ist.

-Handy als Hotspot - Roboter wollte sich nicht verbinden bzw. ist nicht in der Geräteliste aufgetaucht

-Fritz Box - egal ob über Wlan oder Ethernet keine IPv4 Adresse vergebbar 



Mit Nao Nummer 10 aktuell keine Dekodirung möglich, mit Verdacht auf ein defektes Mikro.Folgend kurzer Test hierfür: Bei Ausgabe der Volume Variable in der [BeepRecognizer](Src/Modules/Communication/BeepComms/BeepRecognizer.cpp) Klasse ist ein Großteil der ausgegeben Werte 0.000518799 unabhängig davon ob ein externes Audiosignal anliegt oder nicht. In [Volume_FehlerDaten](Volume_FehlerDaten.txt) ist ein lägerer Auschnitt der Ausgabe hinterlegt.

Es ist immernoch keine Lösung bekannt, daher wurde Nao 10 gegen Nao 11 ausgetauscht bei dem diese Probleme nicht aufteten.


### Compile und Deploy
Hier gab es auch öfter Probleme mit Fehlermeldungen wie "Fehler    MSB8066    Der benutzerdefinierte Build für "C:\Users\Sandro\Desktop\FH\Robotik\R2K\R2K-SPL\Build\Windows\CMake\CMakeFiles\725006d56f053ab6e6f68c9c497d7a26\Nao.rule" wurde mit dem Code 1 beendet.    Nao    C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\Microsoft.CppCommon.targets    245" die mit einer Kombination aus Build Ordner löschen, ./generate ausführen sowie Systemneustarts irgendwann gelöst waren.

Weiterhin gab es ein Problem bei dem ./generate nichtmehr ausgeführt werden konne weil Virtualisierung im Bios nicht aktiviert war. Hierführ muss bei meinem Gigabyte AM4 Mainboard "SVM" im Bios aktiviert sein.



## Aktuelle Problme
Bei Unterhaltungen die während der Analyse des bestehenden Codes geführt wurden hat der Roboter mehrfach eine Message erkannt. Daraus folgt eine möglicherweise problematische Rate an falsch positiven Erkennungen. Durch Configänderungen abgeschwächt.   

Bei manchen Frequenzen werden mehr als nur ein Roboter als Sender erkannt. Hierfür nocheinmal Wilhem fragen,da das Problem schon seit damals bekannt ist. Vieleicht hielft hier [SpektrumanalyseR1M1.PNG](BeepTestSounds/SpektrumanalyseR1M1.PNG)
Bsp.(Zahlen müssen erst genauer bestimmt werden und dienen hier nur zu einfacheren veranschlichung)
|1|0|13|0|0|
Nach Configänderungen bisher nicht beobachtet allerdings immernoch möglich. 

## Zukünftige Arbeiten
Merfach ausgabe im Regognizer fixen

"BeepBroadcaster::requestBeep(int robot_number, int message)" Probleme fixen

Demovideo drehen




## Verlinkung relevanter Klassen  
[BeepBroadcaster.h](Src/Modules/Communication/BeepComms/BeepBroadcaster.h)  
[BeepBroadcaster.cpp](Src/Modules/Communication/BeepComms/BeepBroadcaster.cpp)  
[BeepRecognizer.h](Src/Modules/Communication/BeepComms/BeepRecognizer.h)  
[BeepRecognizer.cpp](Src/Modules/Communication/BeepComms/BeepRecognizer.cpp)  
[Beep.h](Src/Representations/Communication/Beep.h)  
[BeepCommData.h](Src/Representations/Infrastructure/BeepCommData.h)  
[beepBroadcaster.cfg](Config/Scenarios/Default/beepBroadcaster.cfg)  
[beepRecognizer.cfg](Config/Scenarios/Default/beepRecognizer.cfg)  