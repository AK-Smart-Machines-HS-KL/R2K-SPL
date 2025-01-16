

Worker of current Branch 2025-03-IRB:

Name: 		Dennis Fuhrmann
Hochschul ID: 	885725
E-mail:		defu1001@stud.hs-kl.de
Discord name:	PinkPummeler

This branch tackles the Technichal Challange 2025 kiking a rolling Ball how this challange Works in more Detail:

3. IRB - Intercept Rolling Ball (Technical Challenge 2025)

Eine technische Challenge beim RoboCup2025 wird sein einen von einer Rampe rollenden Ball abzufangen und direkt zu spielen. 
Bestenfalls wird dabei erwartet, dass der Ball angefangen und dann zu einem Mitspieler gepasst wird, sodass wir direkt die Indirect-Kick-Rule beachten und das Verhalten somit auch ins Turnierspiel integrieren können.

Die Idee hierbei ist die Ballannahme beim Pass, sowie das Abfangen schneller Bälle vom Gegner zu optimieren.

Studierende, die mit an der Challenge arbeiten, erhalten einen Einblick in die aktuellen Challenges im RoboCup und somit in die Hürden,
 die der Robotik softwareseitig noch auferlegt sind. Es gilt sich in die Verhaltens-, aber auch einen Schritt tiefer, in die Körpersteuerung einzuarbeiten,
 um aus dem Roboter eine erhöhte Reaktionszeit und ein angemessenes Abfangverhalten herauszuholen. 

Card used is ChallangeCard

Method for Testing GoToBallAndKick
Robot num 5 used for testing

Entscheidungen, Tests gewonnen

Entscheidungen, Tests gewonnen

1) SetUp um  Challenge nachbauen
2 vs dummies
impuls auf ball mvb geht nicht 
manuell: bloss nicht ;-)
Lsg: CornerKick Modofizieren
game stack: fast alle Cards löschen, TeachIn durch löschen


Skizze feld 

2) corner kick, 
ziele für den ball elfmeter punkt ODER
pass-spiel direkt vor die Füße
ball löuft vor dem bot schnell vorbei

3) welche skill

gotoBall: viele Paramter schussteuerung
welche kombination bringen welchen erfolg: -> schuss wirst bei Ball stopp ausgelöst

Rexamination mit größerer Rangea (20_deg -> 40_deg -> 80_deg -> 140_deg -> 180_deg)
Result: no visible change to before

walktToBall: obstacleAvoidance ausschalten
obstacle avoidance kann nicht ausgeschaltet werden? -> schuss zu langsam nicht wünchenstwert 
gleiches Problem wie bei GotoBall versucht dem Ball hinterherzulaufen anstatt einfach zu kicken

Dribble:
gleiches problem wie bei WalktoBall

WalkToPoint:
Anstelle den Ball zu treten in den Ball rein laufen -> schlechter kick
Ergebnis:


4) Lösungsversuch
MAKE_SKILL "waitForBallAndKick"
parameters targetAngle


-----Testing Setup:------

using OneTeamFast.ros2 whith manually removing the dummy team from the field
First `gc playing` (otherwise cornerKick wont work)
then ´mvb -4300 2900 0´
then move robot 4 closer to Ball so it can reach it in time
then select robot 5 and ´mv -3550 0 300´
then ´gc cornerKickForFirstTeam´
the CornerKickCard hs benn modified so that the Ball should be played right infront of the feet of the second robot

copy - paste - List:

gc playing
mvb -4300 2900 0
mv -3550 0 300
gc cornerKickForFirstTeam


Testing results:
GoToBallAndKick is too slow for what we need here and after a discussion with the professor no other skill is viable
-> Programm a new Skill, with a faster Kick which also acts more dynamicly:
 Proposed name: waitForBallAndKick keeping with the previous naming convention 

