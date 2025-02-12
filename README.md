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

## Zugriff auf OUTPUT_TEXT mit Simulator
Solange ein Nao in der bush verbunden ist, kann der Simulator genutzt werden um Daten des Roboter im laufenden Betrieb auszulesen. Hierzu zählen OUTPUT_TEXT() Statements die zum Debuggen bzw. einfachen Verifizieren des Codes verwendet werden kann. Hierzu muss bei verbundenem Roboter einfach nur in der bush, in der unteren Leiste der "Simulator" gestartet werden. Hier wird dann im "Console" Fenster der Output angezeigt.

Mit: << können Variabeln ausgegeben werden. Bsp.  
OUTPUT_TEXT("Irgendwas: " << x);

Merke nach jedem Deploy muss der Simulator neugestartet werden

## Beschreibung bisheriger Code
### Broadcaster
Jeder Roboter besitz eine eigene Basisfrequenz. Diese berechnet sich aus den in der [Config](Config\Scenarios\Default/BeepBroadcaster.cfg) konfigurierbaren "baseFrequency" und  "bandWidth" mit folgender Formel:

baseFrequency x bandWidth x Roboternummer

Bsp.  
baseFrequency = 1500  
bandWidth = 1000  
Roboter 1 -> 1500  
Roboter 2 -> 2500

In der konfigurierbaren "encodedBits" wird bestimmt wie viele Bit übertragen werden. Hierfür wird das Frequenzband durch die Anzahl der Bits geteilt und die Kombinationen dieser gleichzeitig gesendeten Frequenzen stellen dann unsere Übertragung da. Diese  werden durch den Integer "message" identifiziert.

Bsp.  
Robot nr. 1 broadcasting message 1(1500Hz)  

Robot nr. 1 broadcasting message 7(1500Hz)  
Robot nr. 1 broadcasting message 7(1700Hz)  
Robot nr. 1 broadcasting message 7(1900Hz)  

Robot nr. 1 broadcasting message 31(1500Hz)  
Robot nr. 1 broadcasting message 31(1700Hz)  
Robot nr. 1 broadcasting message 31(1900Hz)  
Robot nr. 1 broadcasting message 31(2100Hz)  
Robot nr. 1 broadcasting message 31(2300Hz) 

Dies wird auch als Debug Code bei jedem Senden auf der Console ausgegeben.

Ein Beep kann ausgelöst werden mit Betätigen des vorderen Kopfschalters. Die gesendete message kann in der [BeepBroadcaster.cfg](Config\Scenarios\Default/BeepBroadcaster.cfg) als "headButtonMessage" geändert werden.

### Regognizer
Hier wird mittels fftw(Fourier-Transformation) dekodiert und in Form eines std::vector messages(siehe [Beep.h](Src\Representations\Communication\Beep.h)) abgespeichert. Hier sind mit dem Index der Roboter und dem Inhalt die messages beschrieben Bsp. bei 5 Robotern und 4 bit   

|0|0|0|0|0| - keine message   
|1|0|0|0|0| - message 1 bei Roboter 1(R1M1)  
|0|0|0|0|15| - message 15 bei Roboter 5(R5M15)  

Bisher falls |1|0|0|0|0| von einem Roboter dedektiert wurde, welchernicht Nummer 1 trägt eine message 1 als Antwort gesendet gesendet.


## Änderungen am Code
### Config
averagingBufferSize = 15
averagingThreshold = 11 
baseFrequency = 1500
bandWidth = 1000
encodedBits = 5
signalBaseline = 3

Mit dieser Config ist zumindest in Tests ohne zusätzliche Hintergrundgeräusche eine genaue erkennung möglich und in ersten Tests mit Hintergrundgeräusche auch. Die einzige Ausnahmme hier ist die message 31 welche nicht eindeutig oder sogar falsch erkannt wird. Lösungsvorschlag hier die 31 einfach zu ignorieren. Testausgaben hierfür finden sich [BeepTestSounds](BeepTestSounds). 

### Broadcaster
Hier wurde hinzugefügt, dass der theGroundContactState(das gleiche was für "High" und "Ground" zuständig ist) ausgelesen wird und ein Beep erzeugt falls der Roboter hochgehoben bzw. "High" sagen würde.

### Regognizer
Mittels SystemCall::say wird für jeden Roboter und jede mögliche message im Muster: "message"+ nummer der message + "robot" + nummer des sendenden roboters ausgegeben.
Aktuell werden hier die messages 1-10 von jedem Roboter erkannt, die messages 11-30 nur von Spezifischen(siehe Muster unten) dies ist unser Übertragunsprotokoll(jeder Roboter erkennt weiterhin jedes Signal reagiert aber nur auf die ihm zugewissenen).

Aufteilung der Messages/Payload auf die Unterschiedlichen Roboter  
1-------------------------------------------------------------------------------------------------31  
1-10 Broadcast - 11-14 Roboter 1 - 15-18 Roboter 2 - 19-22 Roboter 3 - 23-26 Roboter 4 - Roboter 5


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

Bei manchen Frequenzen werden mehr als nur ein Roboter als Sender erkannt. Nach Configänderungen bisher nicht beobachtet allerdings immernoch möglich. 

## Zukünftige Arbeiten
Bisherige Ideen für eine Zukünftige praktische Anwendung:  
-einem Roboter mitteilen das der Ball sich hinter ihm befindet während er selber ihn nicht sieht    
-dem Torwart mitteilen das er falsch steht  
-einem Roboter mitteilen das er die Schusslinie blockiert  


## Verlinkung relevanter Klassen  
[BeepBroadcaster.h](Src/Modules/Communication/BeepComms/BeepBroadcaster.h)  
[BeepBroadcaster.cpp](Src/Modules/Communication/BeepComms/BeepBroadcaster.cpp)  
[BeepRecognizer.h](Src/Modules/Communication/BeepComms/BeepRecognizer.h)  
[BeepRecognizer.cpp](Src/Modules/Communication/BeepComms/BeepRecognizer.cpp)  
[Beep.h](Src/Representations/Communication/Beep.h)  
[BeepCommData.h](Src/Representations/Infrastructure/BeepCommData.h)  
[beepBroadcaster.cfg](Config/Scenarios/Default/beepBroadcaster.cfg)  
[beepRecognizer.cfg](Config/Scenarios/Default/beepRecognizer.cfg)  