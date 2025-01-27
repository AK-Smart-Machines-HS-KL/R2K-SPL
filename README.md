

Momentaner Arbeiter des Branches 2025-03-IRB:

Name: 		Dennis Fuhrmann </br>
E-mail:		defu1001@stud.hs-kl.de </br>
Discord name:	PinkPummeler </br>

Dieser Branch befasst mit der Intercept Rolling Ball Challenge 2025, hier noch eine genauere erklärung vom Teamleiter Wilhelm:

3. IRB - Intercept Rolling Ball (Technical Challenge 2025) </br></br>

Eine technische Challenge beim RoboCup2025 wird sein einen von einer Rampe rollenden Ball abzufangen und direkt zu spielen.</br> 
Bestenfalls wird dabei erwartet, dass der Ball angefangen und dann zu einem Mitspieler gepasst wird, sodass wir direkt die Indirect-Kick-Rule beachten und das Verhalten somit auch ins Turnierspiel integrieren können.</br></br>

Die Idee hierbei ist die Ballannahme beim Pass, sowie das Abfangen schneller Bälle vom Gegner zu optimieren.</br></br>

Studierende, die mit an der Challenge arbeiten, erhalten einen Einblick in die aktuellen Challenges im RoboCup und somit in die Hürden,</br>
 die der Robotik softwareseitig noch auferlegt sind. Es gilt sich in die Verhaltens-, aber auch einen Schritt tiefer, in die Körpersteuerung einzuarbeiten,</br>
 um aus dem Roboter eine erhöhte Reaktionszeit und ein angemessenes Abfangverhalten herauszuholen.</br></br>

Die ersten Test von B-Huamn die uns als Inpiration dienten:</br>

[![Inspiration von B-human](https://img.youtube.com/vi/ufiUQ-02DWk/0.jpg)](https://www.youtube.com/watch?v=ufiUQ-02DWk)


Hier findet man auch das Rule Book für die Technichal Challange 2025 wo auch der tatsächliche Setup genauer erklärt wird:

[Downlaod Seite für das Regelwerk](https://spl.robocup.org/downloads/)
 

Die genutzte Card ist ChallangeCard </br>
game stack: fast alle Cards löschen, TeachIn auch gelöscht </br>

## 1) SetUp um  Challenge nachbauen
IRBChallange.ros2 (Für ein Fast Game auf schwachen Maschinen) </br>
ODER </br>
IRBCHallangeOraclePerceptor.ros2 (Für ein besser simulierten Roboter) -> noch Probleme mit der Ödometrie (der Roboter weis nicht wo er ist) </br>
Hier wird der Ball direkt vor die Füße von Roboter 1 gespielt dieser sollte dann auch </br>
in den rollenden Ball reinlaufen </br>

## 2) corner kick, 
ziele für den ball elfmeter punkt ODER </br>
pass-spiel direkt vor die Füße </br>
ball läuft vor dem bot schnell vorbei </br>


### -----Testing Setup:------

Öffne im Simulator die Scene IRBChallange.ros2 </br>
diese führt den nötigen setup automatisch durch </br>

Alternativ:
using OneTeamFast.ros2 ignoriere einfach ddie Dummies </br>
First `gc playing` (ansonsten wird der CornerKick nicht ausgefürht) </br>
then ´mvb -4300 2900 0´ </br>
then bewege robot 4 näher an den Ball damit er Diesen rechtzeigit erreicht </br>
then wähle robot 5 und benutze den Befehl ´mv -3550 0 300´ </br>
then ´gc cornerKickForFirstTeam´ </br>
die CornerKickCard wurde so modifizeirt das der Ball for die Füße des Roboters form Tor gespielt werden sollte</br>

copy - paste - List:

gc playing </br>
mvb -4300 2900 0 </br>
mv -3550 0 300 </br>
gc cornerKickForFirstTeam </br>

## 3) Welcher Skill

### gotoBall: viele Paramter schussteuerung
welche kombination bringen welchen erfolg: -> schuss wirst bei Ball stopp ausgelöst bei alln Kombinationen

Rexamination mit größerer Rangea (20_deg -> 40_deg -> 80_deg -> 140_deg -> 180_deg) </br>
Result: Keine Änderung zu vorher

### walktToBall: obstacleAvoidance ausschalten
obstacle avoidance kann nicht ausgeschaltet werden? -> schuss zu langsam nicht wünchenstwert </br> 
gleiches Problem wie bei GotoBall versucht dem Ball hinterherzulaufen anstatt einfach zu kicken </br>

### Dribble:
gleiches problem wie bei WalktoBall </br>

### WalkToPoint:
Anstelle den Ball zu treten in den Ball rein laufen -> schlechter kick </br>
Ergebnis: Vielverschprechendes Ergebnis -> muss noch dynamischer angepasst werden </br>
Echte Naos: In echt sidn die Naos langsamer -> Reaktion entspechend den Naos anpassen oder die Geschwindigkeit für die wenigen Schritte erhöhen </br>

Dynamisches Anlaufen: Im Simulator viel zuverlässiger -> muss noch an echten naos getestet werden </br>
Erstes Testergebnis: Vorzeichen Fehler beim InterceptPoint -> der Roboter ist nach hinten gelaufen </br>
-> ein einfaches IF statement um dies zu vermeiden </br>
-> Stand 24.01 das Problem bleibt beim echten Roboter bestehen </br>

Dynamisches anlaufen:
 - der Abstand zum Ball der unterschritten werden muss damit der Roboter reagiert, </br>
 wird anhand der Geschwindigkeit des Balls berechnet mite der Funktion calcMinDistance
 - der InterceptPoint (Der Punkt der angelaufen wird), </br>
 wird ebenfalls anhand der geschwindigkeit des Balls berechnet mit der Funktion calcInterceptPoint

die oben berechneten Werte werden nochmal um jeweils einen eigenen Faktor multipliziert: </br>
Diese können in dem Code je nach Test erfolgen angepassst werden -> in eine Config umlegen für schnelleres anpassen

### Beispiel Video:

- für Fast: </br>


https://github.com/user-attachments/assets/f2cd8e37-bab2-4473-a884-26c7c3786432

- für Normal: </br>


https://github.com/user-attachments/assets/d67f9cb2-a863-4b5b-a6e3-0eb94ae47f5f





