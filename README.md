

Momentaner Arbeiter des Branches 2025-03-IRB:

Name: 		Dennis Fuhrmann
E-mail:		defu1001@stud.hs-kl.de
Discord name:	PinkPummeler

Dieser Branch befasst mit der Intercept Rolling Ball Challenge 2025, hier noch eine genauere erklärung vom Teamleiter Wilhelm:

3. IRB - Intercept Rolling Ball (Technical Challenge 2025)

Eine technische Challenge beim RoboCup2025 wird sein einen von einer Rampe rollenden Ball abzufangen und direkt zu spielen. 
Bestenfalls wird dabei erwartet, dass der Ball angefangen und dann zu einem Mitspieler gepasst wird, sodass wir direkt die Indirect-Kick-Rule beachten und das Verhalten somit auch ins Turnierspiel integrieren können.

Die Idee hierbei ist die Ballannahme beim Pass, sowie das Abfangen schneller Bälle vom Gegner zu optimieren.

Studierende, die mit an der Challenge arbeiten, erhalten einen Einblick in die aktuellen Challenges im RoboCup und somit in die Hürden,
 die der Robotik softwareseitig noch auferlegt sind. Es gilt sich in die Verhaltens-, aber auch einen Schritt tiefer, in die Körpersteuerung einzuarbeiten,
 um aus dem Roboter eine erhöhte Reaktionszeit und ein angemessenes Abfangverhalten herauszuholen.

Die ersten Test von B-Huamn die uns als Inpiration dienten:

[![Inspiration von B-human](https://img.youtube.com/vi/ufiUQ-02DWk/0.jpg)](https://www.youtube.com/watch?v=ufiUQ-02DWk)


Hier findet man auch das Rule Book für die Technichal Challange 2025 wo auch der tatsächliche Setup genauer erklärt wird:

[Downlaod Seite für das Regelwerk](https://spl.robocup.org/downloads/)
 

Die genutzte Card ist ChallangeCard
game stack: fast alle Cards löschen, TeachIn auch gelöscht

1) SetUp um  Challenge nachbauen
IRBChallange.ros2 (Für ein Fast Game auf schwachen Maschinen)
ODER
IRBCHallangeOraclePerceptor.ros2 (Für ein besser simulierten Roboter) -> noch Probleme mit der Ödometrie (der Roboter weis nicht wo er ist)
Hier wird der Ball direkt vor die Füße von Roboter 1 gespielt dieser sollte dann auch
in den rollenden Ball reinlaufen 

2) corner kick, 
ziele für den ball elfmeter punkt ODER
pass-spiel direkt vor die Füße
ball läuft vor dem bot schnell vorbei


-----Testing Setup:------

Öffne im Simulator die Scene IRBChallange.ros2
diese führt den nötigen setup automatisch durch

Alternativ:
using OneTeamFast.ros2 ignoriere einfach ddie Dummies
First `gc playing` (ansonsten wird der CornerKick nicht ausgefürht)
then ´mvb -4300 2900 0´
then bewege robot 4 näher an den Ball damit er Diesen rechtzeigit erreicht
then wähle robot 5 und benutze den Befehl ´mv -3550 0 300´
then ´gc cornerKickForFirstTeam´
die CornerKickCard wurde so modifizeirt das der Ball for die Füße des Roboters form Tor gespielt werden sollte

copy - paste - List:

gc playing
mvb -4300 2900 0
mv -3550 0 300
gc cornerKickForFirstTeam


3) welche skill

gotoBall: viele Paramter schussteuerung
welche kombination bringen welchen erfolg: -> schuss wirst bei Ball stopp ausgelöst bei alln Kombinationen

Rexamination mit größerer Rangea (20_deg -> 40_deg -> 80_deg -> 140_deg -> 180_deg)
Result: Keine Änderung zu vorher

walktToBall: obstacleAvoidance ausschalten
obstacle avoidance kann nicht ausgeschaltet werden? -> schuss zu langsam nicht wünchenstwert 
gleiches Problem wie bei GotoBall versucht dem Ball hinterherzulaufen anstatt einfach zu kicken

Dribble:
gleiches problem wie bei WalktoBall

WalkToPoint:
Anstelle den Ball zu treten in den Ball rein laufen -> schlechter kick
Ergebnis: Vielverschprechendes Ergebnis -> muss noch dynamischer angepasst werden
Echte Naos: In echt sidn die Naos langsamer -> Reaktion entspechend den Naos anpassen oder die Geschwindigkeit für die wenigen Schritte erhöhen

Dynamisches Anlaufen: Im Simulator viel zuverlässiger -> muss noch an echten naos getestet werden 
Erstes Testergebnis: Vorzeichen Fehler beim InterceptPoint -> der Roboter ist nahc hinten gelaufen
-> ein einfaches IF statement um dies zu vermeiden

Dynamisches anlaufen:
 - der Abstand zum Ball der unterschritten werden muss damit der Roboter reagiert, wird anhand der Geschwindigkeit des Balls berechnet mite der Funktion calcMinDistance
 - der InterceptPoint (Der Punkt der angelaufen wird) wird ebenfalls anhand der geschwindigkeit des Balls berechnet mit der Funktion calcInterceptPoint

die oben berechneten Werte werden nochmal um jeweils einen eigenen Faktor multipliziert:
Diese können in dem Code je nach Test erfolgen angepassst werden -> in eine Config umlegen für schnelleres anpassen

Beispiel Video:

Findet man Hier im Ordner mit dem Namen: 2025-01-20 21-41-55.mp4


