# Audio Beeps Communication

## Projektübersicht

Dieses Projekt erweitert die Audio-Kommunikation der NAO-Roboter, basierend auf den Vorarbeiten von Fortune, Hobelsberger und Simus. Ziel ist es, ein robustes Audio-Kommunikationsprotokoll zu entwickeln, bei dem die Roboter Nachrichten über unterschiedliche Frequenzen übertragen können. Der aktuelle Stand ist, dass die Roboter erfolgreich Audio-Sender und -Empfänger für ein Kommunikationsprotokoll implementiert haben, wobei bisher nur Signale von 32 bit ohne spezielle Bedeutung gesendet und empfangen wurden. Der nächste Schritt besteht darin, dieses Protokoll zu verbessern und den Robotern die Fähigkeit zu verleihen, Nachrichten mit höherer Bandbreite und Pulsbreite zu übertragen.

## Aktuelles Setup

- **Betriebssystem**: Windows 11
- **Entwicklungsumgebung**: Visual Studio Version 17.2.2
- **Roboter**: NAO Nummer 11
- **Verbindung**: Direkt über Ethernet
- **IP-Adresse des Roboters**: 192.168.50.13  
- **Subnetzpräfix**: 24  
- **Standardgateway**: 192.168.50.1  
- **DNS-Server**: Bevorzugt: 1.1.1.1, Alternativ: 0.0.0.0

## Nutzung des Simulators

Solange der NAO-Roboter über das Netzwerk verbunden ist, kann der Simulator verwendet werden, um während des Betriebs Daten des Roboters zu überwachen und zu debuggen. Dies umfasst insbesondere **OUTPUT_TEXT()**-Statements, die helfen, den Code zu verifizieren und Fehler zu identifizieren.

1. **Starten des Simulators**: 
   - Öffnen Sie den "Simulator" in der unteren Leiste der "bush"-Umgebung, um den Output im "Console"-Fenster zu sehen.
   
2. **Ausgabe von Variablen**: 
   - Sie können Variablen mit `<<` ausgeben, z. B.:
     ```cpp
     OUTPUT_TEXT("Variable x: " << x);
     ```

3. **Hinweis**: Nach jedem Deployment muss der Simulator neu gestartet werden.

## Code-Überblick

### Broadcaster (Senden von Beeps)

Jeder Roboter hat eine eigene Basisfrequenz, welche sich aus der in [Config](Config\Scenarios\Default/BeepBroadcaster.cfg) konfigurierbaren "baseFrequency" und  "bandWidth" über die Formel:

baseFrequency= bandWidth*Roboternummer. 

berechnet. Die Roboternummer wird verwendet, um eine eindeutige Frequenz für jede Nachricht zu erzeugen:

- **Berechnung der Basisfrequenz**: baseFrequency= bandWidth x RoboterID(1-5)

**Beispiel**: bandwidth=1000
- **Roboter 1**: Basisfrequenz = 1500 Hz
- **Roboter 2**: Basisfrequenz = 2500 Hz
- **Roboter 3**: Basisfrequenz = 2500 Hz

Die Anzahl der zu übertragenden Bits wird durch die **encodedBits**-Variable bestimmt. Diese legt fest, wie viele Bits pro Übertragung gesendet werden können, indem das Frequenzband in diese Anzahl von Bits unterteilt wird.

**Beispiel für eine Nachrichtenübertragung**:
- Roboter 1 sendet die Nachricht 1 bei 1500 Hz.
- Roboter 1 sendet die Nachricht 7 bei 1500, 1700 und 1900 Hz.
- Roboter 1 sendet die Nachricht 31 bei 1500, 1700, 1900, 2100 und 2300 Hz.

Dies wird auch als Debug Code bei jedem Senden auf der Console ausgegeben.
Beep-Signale können durch Drücken des Kopfschalter ausgelöst werden. Die gesendete Nachricht ist über die **headButtonMessage** in der Konfiguration [BeepBroadcaster.cfg](Config\Scenarios\Default/BeepBroadcaster.cfg) anpassbar.


Mit:  
beepCommData.broadcastQueue.push_back(x);  
kann im Code ein Beep erzeugt werden, wobei x ein Integer von 1 bis 2^encodedBits-1 ist.

### Recognizer
Hier wird mittels fftw(Fourier-Transformation) dekodiert und in Form eines std::vector messages(siehe [Beep.h](Src\Representations\Communication\Beep.h)) abgespeichert. Hier sind mit dem Index der Roboter und dem Inhalt die messages beschrieben Bsp. bei 5 Robotern und 4 bit   

|0|0|0|0|0| - keine message   
|1|0|0|0|0| - message 1 bei Roboter 1(R1M1)  
|0|0|0|0|15| - message 15 bei Roboter 5(R5M15)  


## Änderungen am Code
### Config BeepRecognizer.cfg
averagingBufferSize = 15  
averagingThreshold = 11   
baseFrequency = 1500  
bandWidth = 1000  
encodedBits = 5  
signalBaseline = 3  

Mit dieser Config ist zumindest in Tests ohne zusätzliche Hintergrundgeräusche eine eindeutige Erkennung am eigenen Roboter möglich und in ersten Tests mit Hintergrundgeräusche auch. Bei Tests mit mehreren Robotern treten manchmal Fehlerkennungen auf. Message 31 wird bisher immer nicht eindeutig oder sogar falsch erkannt. Lösungsvorschlag hier die 31 einfach zu ignorieren. Testausgaben hierfür finden sich [BeepTestSounds](BeepTestSounds). 

Bisherige Tests fanden mit 5 und 3 Robotern statt. Es gibt allerdings keinen Grund anzunehmmen warum 1,2 oder 3 nicht funktionieren würde.

Die baseFrequency kann theoretisch auch tiefer erkannt werden allerdings wird bei tieferen Frequenzen der Faktor der Menschlichen Stimme Relevant und sorgt vermehrt zu Fehlerkennungen.

### Broadcaster
Hier wurde hinzugefügt, dass der theGroundContactState(das gleiche was für "High" und "Ground" zuständig ist) ausgelesen wird und ein Beep erzeugt falls der Roboter hochgehoben bzw. "High" sagen würde.

In Zeile 47 startet die Demo und basiert auf dem im Regognizer verwendeten Code der say ausgabe(siehe unten).

### Recognizer
Für die DEMO: Ab Zeile 155 wird Mittels SystemCall::say wird für jeden Roboter und jede mögliche message im Muster: "message"+ nummer der message + "robot" + nummer des sendenden roboters ausgegeben.
Aktuell werden hier die messages 1-10 von jedem Roboter erkannt, die messages 11-30 nur von Spezifischen(siehe Muster unten) dies ist ein experimentelles Übertragunsprotokoll(jeder Roboter erkennt weiterhin jedes Signal reagiert aber nur auf die ihm zugewissenen) und aktuell fest für 5bit pogrammiert sprich nicht von der Config abhängig.

Aufteilung der Messages/Payload auf die Unterschiedlichen Roboter  
1-------------------------------------------------------------------------------------------------------------31  
1-10 Broadcast - 11-14 Roboter 1 - 15-18 Roboter 2 - 19-22 Roboter 3 - 23-26 Roboter 4 - Roboter 5

Wie sinnvoll diese aufteilung ist muss noch erprobt werden.

## Beep Test Sounds(Ordner)
in [BeepTestSounds](BeepTestSounds) sind mehrere aufgenommene beeps sowie Testergebnisse. 

beepbraodcaster.cpp, in der .. findet sich eine Demo
## Alte Probleme
### Verbindung
Eine Analyse zeigt, dass nur die direkte Verbindung per Ethernet zielführend ist. Im Folgenden sind die verschiedenen Ansätze und deren Problematiken aufgeführt.

- Verbindung über ein Handy als Hotspot
    - Der Roboter erscheint nicht in der Geräteliste, folglich ist keine Verbindung möglich.
- Verbindung über eine Fritz Box
    - sowohl über WLAN als auch Ethernet wird keine IPv4 Adresse vergeben 

Der NAO Nummer 10 erlaubt aktuell keine Dekodirung, möglichweise aufgrund eines defekten Mikrofons. 
Dies lässt sich testen : Bei Ausgabe der Volume Variable in der [BeepRecognizer](Src/Modules/Communication/BeepComms/BeepRecognizer.cpp) Klasse ist ein Großteil der ausgegeben Werte 0.000518799 unabhängig davon ob ein externes Audiosignal anliegt oder nicht. In [Volume_FehlerDaten](Volume_FehlerDaten.txt) ist ein lägerer Auschnitt der Ausgabe hinterlegt.

Keine Lösung wird für NAO 10 gefunden, folglich wird mit dem funktionierenden NAO 11 fortgefahren.


### Compile und Deploy
Es treten folgende Fehlermeldungen auf: "Fehler    MSB8066    Der benutzerdefinierte Build für "C:\Users\Sandro\Desktop\FH\Robotik\R2K\R2K-SPL\Build\Windows\CMake\CMakeFiles\725006d56f053ab6e6f68c9c497d7a26\Nao.rule" auf; Es wird mit dem Code 1 beendet.
Nao    C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\Microsoft.CppCommon.targets    245" die mit einer Kombination aus Build Ordner löschen, ./generate ausführen sowie Systemneustarts irgendwann gelöst waren.

Weiterhin gab es ein Problem bei dem ./generate nichtmehr ausgeführt werden konne weil Virtualisierung im Bios nicht aktiviert war. Hierführ muss bei meinem Gigabyte AM4 Mainboard "SVM" im Bios aktiviert sein.



## Aktuelle Limitationen der Audio Beep Communication
In der aktuellen Version erkennt ein Roboter mehrfach die selbe Nachricht. Daraus folgt eine potentielle Fehlerquelle durch falsch positiven Erkennungen. 

Bei manchen Frequenzen kann der sendende Robotor nicht eindeutig differenziert werden. Trotz Configänderungen kann dieser Kommunikationsfehler bisher nicht vermieden werden.
Insbesondere wenn eine Nachricht sich aus mehreren Frequenzen zusammensetzt, wird die Nachricht nicht korrekt entschlüsselt, wie folgendes Beispiel demonstriert:

Message 12 ist eine Überlagerung der zeitgleich gesendeten Messages 4 und Messages 8. Es wird beobachtet, dass für niedrigere Frequenzen eher die 4 und für höheren Frequenzen eher die 8 registriert wird. 
## Potentielle Fehlerquelle und Lösungsansatz
Eine pausible Erklärung könnte sein, dass eine minimale Verzögerung in der Sendung bzw. in dem Empfang der beiden Messages (4 oder 8) besteht. Daraus resultiert die Detektion einer einzelnen Nachricht anstelle der zusammengesetzten Message 12. Eine Lösung könnte darin liegen, den hinteren Teil der Nachricht abzuschneiden und so die Messages auf die identische Länge zu kürzen. Alternativ könnten von den ca. 30 Erkennungen[(Beispiele)](BeepTestSounds\BeepTestSounds(5bit,bf1500,bw1000,signalBaeline3)hintergrundgeräsche.txt), die bei einem Signal vorliegen, dasjenige auszuwählen, das am häufigsten detektiert wird.    

## Notwenidige nächste Schritte
Die aktuellen Schwierigkeiten der Audio Beep Communication lösen, insbesondere durch das Erstellen einer Configabhängigen API um eine Interaktion mit den restlichen Modulen des Roboters zu ermöglichen. 

## Potentielle Anwendungsgebiete für die Audio Beep Communication
- einem Roboter mitteilen das der Ball sich hinter ihm befindet während er selber ihn nicht sieht    
- dem Torwart mitteilen das er falsch steht  
- einem Roboter mitteilen das er die Schusslinie blockiert  


## Verlinkung relevanter Klassen  
[BeepBroadcaster.h](Src/Modules/Communication/BeepComms/BeepBroadcaster.h)  
[BeepBroadcaster.cpp](Src/Modules/Communication/BeepComms/BeepBroadcaster.cpp)  
[BeepRecognizer.h](Src/Modules/Communication/BeepComms/BeepRecognizer.h)  
[BeepRecognizer.cpp](Src/Modules/Communication/BeepComms/BeepRecognizer.cpp)  
[Beep.h](Src/Representations/Communication/Beep.h)  
[BeepCommData.h](Src/Representations/Infrastructure/BeepCommData.h)  
[beepBroadcaster.cfg](Config/Scenarios/Default/beepBroadcaster.cfg)  
[beepRecognizer.cfg](Config/Scenarios/Default/beepRecognizer.cfg)  
