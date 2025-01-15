# Audio Beeps Communication
Anmerkungen für den Test am 15.01  
in [Beep Test Sounds](BeepTestSounds) sind mehrere aufgenommene beeps  
wenn bei encoded bits 1 eingestellt ist wird unabhägig von message die basisfrequenz kodiert(500,700...)  
ich habe auch testweise die config auf 4 bit gestellt die ergebniise davon sind in [Beep Test Sounds.txt](Beep Test Sounds/Beep Test Sounds .txt)  
in [BeepBroadcaster.cpp](Src/Modules/Communication/BeepComms/BeepBroadcaster.cpp)sind ab zeile 98 simple SystemCall::say zum testen von R1M15,R1M10,R5M1,R3M1
für den rest ist in zeile 68 auskommentiert eine OUTPUT_TEXT ausgabe hier kann man auch sehen ob mehrere dinge erkannt werden  

Namenschema R1M1 - Roboter 1 Message 1

## Kurzzusammenfassung
Aufbauend auf den (Teil-) Projekten der Studien-Kollegen Fortune, Hobelsberger und Simus, soll die Audio-Kommunikation erweitert werden. Bisher ist es dem Team gelungen erfolgreich Sender und Empfänger für ein Audio-Kommunikationsprotokoll in einer "Proof of Concept"-Variante umzusetzen. Der nächste Schritt ist, dem Ganzen ein Protokoll zuzuweisen, über welches die Bots ihren Nachrichten Sinn verleihen und welches im Spiel eingesetzt werden kann. Gleichzeitig dient dies als Robustness-Test für den bisherigen Code, da die Roboter darin bisher immer nur das gleiche Signal erkannt haben.

Entnommen aus Wiki

## Aktuelles Setup
Auf diesem Setup kann der Code aktuell gebaut sowie deployed werden.

Windows 10 

Visual Studio Version 17.2.2

Nao Nummer 10(Bisher nur Audioausgabe aber keine Audioeingabe)

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
Falls man nun die config auf 4 bit ändert beginnt er in 50Hz Schritten zu kodieren und mehrere davon anscheinend überlagerte.(siehe Spektogramme in "Beep Test Sounds"). Bisher ist unbekannt ob diese auch erkannt werden.

Die Ein sowie Ausgabe basiert hier auf einem int 0-15, diese nennen wir message.

Ein Beep kann ausgelöst werden mit Betätigen des vorderen Kopfschalters. Kann in der [BeepBroadcaster.cfg](Config\Scenarios\Default/BeepBroadcaster.cfg) als "headButtonMessage" geändert werden. Hier können ebenso die Anzal der Bänder(Zahl der Roboter), sowie die Basisfrequenz sowie die Bandbreite.

### Regognizer
Hier wird mittels fftw(Fourier-Transformation) dekodiert und dann in Form eines std::vector messages(siehe [Beep.h](Src\Representations\Communication\Beep.h)) abgespeichert. Hier sind mit dem Index der Roboter und dem Inhalt die messages beschrieben Bsp. bei 5 Robotern und 4 bit   
|0|0|0|0|0| - keine message  |1|0|0|0|0| - message 1 bei Roboter 1(R1M1) |0|0|0|0|15| - message 15 bei Roboter 5(R5M15)


Bisher wurde nur |1|0|0|0|0| gesendet und als Antwort eines Roboters der nicht die Nummer 1 trägt eine message 1 gesendet Bsp.(Roboter 2) |0|1|0|0|0|

## Zugriff auf OUTPUT_TEXT mit Simulator
Solange ein Nao in der bush verbunden ist, kann der Simulator genutzt werden um Daten des Roboter im laufenden Betrieb auszulesen. Hierzu zählen OUTPUT_TEXT() Statements die zum Debuggen bzw. einfachen Verifizieren des Codes verwendet werden kann. Hierzu muss bei verbundenem Roboter einfach nur in der bush, in der unteren Leiste der "Simulator" gestartet werden.

Mit: << können Variabeln ausgegeben werden. Bsp.  
OUTPUT_TEXT("Irgendwas: " << x);

Merke nach jedem Deploy muss der Simulator neugestartet werden


## Alte Problme
### Verbindung
Es wurden mehrere Wege zu Verbindung getesten vovon letztenendes nur der oben genante Weg über eine Direkte Verbindung per Ethernet erfolgreich war. Im folgenden kurz eine übersicht was probiert wurde und woran es vermutlich gescheitert ist.

Handy als Hotspot - Roboter wollte sich nicht verbinden

Fritz Box - egal ob Wlan oder Ethernet keine IPv4 Adresse vergebbar 

### Compile und Deploy
Hier gab es auch öfter Probleme mit Fehlermeldungen wie "Fehler    MSB8066    Der benutzerdefinierte Build für "C:\Users\Sandro\Desktop\FH\Robotik\R2K\R2K-SPL\Build\Windows\CMake\CMakeFiles\725006d56f053ab6e6f68c9c497d7a26\Nao.rule" wurde mit dem Code 1 beendet.    Nao    C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\Microsoft.CppCommon.targets    245" die mit einer Kombination aus Build Ordner löschen, ./generate ausführen sowie Systemneustarts irgendwann gelöst waren.

Weiterhin gab es ein Problem bei dem ./generate nichtmehr ausgeführt werden konne weil Virtualisierung im Bios nicht aktiviert war. Hierführ muss bei meinem Gigabyte AM4 Mainboard "SVM" im Bios aktiviert sein.



## Aktuelle Problme
Mit Nao Nummer 10 aktuell keine Dekodirung möglich, mit Verdacht auf defektes Mikro.Folgend kurzer Test hierfür: Bei Ausgabe der Volume Variable in der [BeepRecognizer](Src/Modules/Communication/BeepComms/BeepRecognizer.cpp) Klasse ist ein Großteil der ausgegeben Werte 0.000518799 unabhängig davon ob ein externes Audiosignal anliegt oder nicht. In [Volume_FehlerDaten](Volume_FehlerDaten.txt) ist ein lägerer Auschnitt der Ausgabe hinterlegt.

Bei Unterhaltungen die während der Analyse des bestehenden Codes geführt wurden hat der Roboter mehrfach eine Message erkannt. Daraus folgt eine möglicherweise problematische Rate an falsch positiven Erkennungen.   

Bei manchen Frequenzen werden mehr als nur ein Roboter als Sender erkannt. Hierfür nocheinmal Wilhem fragen,da das Problem schon seit damals bekannt ist. Vieleicht hielft hier [SpektrumanalyseR1M1.PNG](BeepTestSounds/SpektrumanalyseR1M1.PNG)
Bsp.(Zahlen müssen erst genauer bestimmt werden und dienen hier nur zu einfacheren veranschlichung)
|1|0|13|0|0|

## Zukünftige Arbeiten
Menge der kodierten Bits erhöhen(aktuell 4) sowie Bandbreite erhöhen(aktuell 200 HZ) bzw. herausfinden ob es möglich ist.




## Verlinkung relevanter Klassen  
[BeepBroadcaster.h](Src/Modules/Communication/BeepComms/BeepBroadcaster.h)  
[BeepBroadcaster.cpp](Src/Modules/Communication/BeepComms/BeepBroadcaster.cpp)  
[BeepRecognizer.h](Src/Modules/Communication/BeepComms/BeepRecognizer.h)  
[BeepRecognizer.cpp](Src/Modules/Communication/BeepComms/BeepRecognizer.cpp)  
[Beep.h](Src\Representations\Communication\Beep.h)  
[BeepCommData.h](Src\Representations\Infrastructure\BeepCommData.h)  
[BeepBroadcaster.cfg](Config\Scenarios\Default/BeepBroadcaster.cfg)  
[BeepRecognizer.cfg](Config\Scenarios\Default/BeepRecognizer.cfg)  
