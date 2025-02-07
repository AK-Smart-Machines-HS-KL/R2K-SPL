

Momentaner Arbeiter des Branches 2025-03-IRB:

Name: 		Dennis Fuhrmann </br>
E-mail:		defu1001@stud.hs-kl.de </br>
Discord name:	PinkPummeler </br>

Dieser Branch befasst mit der Intercept Rolling Ball Challenge 2025, hier noch eine genauere Erklärung vom Teamleiter Wilhelm:

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
 

Die genutzte Card ist [ChallangeCard.cpp](Src/Modules/BehaviorControl/BehaviorControl/Cards/Experimental/ChallangeCard.cpp) </br>
game stack: fast alle Cards löschen, TeachIn auch gelöscht </br>
DefaultCard wurde bearbeitet -> Walk in Fällt weg der Roboter bleibt auf der Stelle stehen.   

## Zusamenfassung
Die bisherigen Kick Skills sind für die Challange ungeignet. Stadessen nutzen wir WalkToPoint um in den Ball reinzulaufen und somit einen Kick "Simulieren".
Dabei wird der angelaufene Punkt durch die Funktion `Vector2f calcInterceptPoint()` berechnet. Kann durch veränderung von `interceptFactor` angepasst werden
Die Reaktionszeitpunkt wird durch `float calcMinDistance()` berechnet. Kann durch veränderung von `minDistanceFactor` angepasst werden
Bei einem echten Roboter mussten die Werte ebenfalls invertiert werden -\_(´. .`)\_-.

## 1) SetUp um  Challenge nachbauen
`IRBChallangeFast.ros2` (Für ein Fast Game auf schwachen Maschinen) </br>
ODER </br>
`IRBCHallangeNormal.ros2` (Für ein besser simulierten Roboter) -> noch Probleme mit der Ödometrie (der Roboter weis nicht wo er ist) </br>
Hier wird der Ball direkt vor die Füße von Roboter 1 gespielt dieser sollte dann auch </br>
in den rollenden Ball reinlaufen </br>

## 2) corner kick, 
Ziele den Ball auf den Elfmeterpunkt ODER </br>
Pass-spiel direkt vor die Füße </br>
-> Ball läuft vor dem bot schnell vorbei </br>


### -----Testing Setup:------

### Simulator:
Öffne im Simulator die Scene `IRBChallangeFast.ros2`  oder `IRBCHallangeNormal.ros2` </br>
diese führt den nötigen setup automatisch durch </br>

Alternativ:
using OneTeamFast.ros2 ignoriere einfach ddie Dummies </br>
First `gc playing` (ansonsten wird der CornerKick nicht ausgefürht) </br>
then ´mvb -4300 2900 0´ </br>
then bewege robot 4 näher an den Ball damit er Diesen rechtzeigit erreicht </br>
then wähle robot 5 und benutze den Befehl ´mv -3550 0 300´ </br>
then ´gc cornerKickForFirstTeam´ </br>
die CornerKickCard wurde so modifizeirt das der Ball for die Füße des Roboters form Tor gespielt werden sollte</br>

### Real-Live Test
Deploy den Roboter auf der Nummer 3 in Release Mode (für schnellere Reaktionen) und Platziere ihn vor dem Mittelkreis (damit er besser seine Ödometrie anpassen kann) </br>
Warte bis der Roboter sicher steht und Plaziere den Ball sichtbar für ihn Rechts oder Links schräg (er schaut sich nach ihm langsam um). </br>
Nachdem er ihn gefunden hat, rolle den Ball dem Roboter vor die Füße. </br>
-> eine Markierung machen wo der Ball hingerollt werden soll und am besten auch wo der Roboter optimalerweise auch den Intercept macht. </br>
Jetzt sollte der Roboter den Ball abfangen. </br>

copy - paste - List:

gc playing </br>
mvb -4300 2900 0 </br>
mv -3550 0 300 </br>
gc cornerKickForFirstTeam </br>

## 3) Welcher Skill

### gotoBall: viele Paramter schussteuerung
Welche kombination bringen welchen erfolg: -> Schuss wirst bei Ball stopp ausgelöst bei allen getesteten Kombinationen

Rexamination mit größerer Rangea (20_deg -> 40_deg -> 80_deg -> 140_deg -> 180_deg) </br>
Result: Keine Änderung zu vorher

### walktToBall: obstacleAvoidance ausschalten
Obstacle avoidance kann ausgeschaltet werden -> Schuss immmer noch zu langsam nicht Wünchenstwert. </br> 
Gleiches Problem wie bei GotoBall versucht dem Ball hinterherzulaufen anstatt einfach direkt zu kicken </br>

### Dribble:
Gleiches problem wie bei WalktoBall </br>

### WalkToPoint:
Anstelle den Ball zu treten in den Ball rein laufen -> schlechter kick </br>
Ergebnis: Vielverschprechendes Ergebnis -> muss noch dynamischer angepasst werden </br>
Echte Naos: In echt sind die Naos langsamer -> Reaktion entspechend den Naos anpassen oder die Geschwindigkeit für die wenigen Schritte erhöhen </br>

Dynamisches Anlaufen: Im Simulator viel zuverlässiger -> muss noch an echten naos getestet werden </br>
Erstes Testergebnis: Vorzeichen Fehler beim InterceptPoint -> der Roboter ist nach hinten gelaufen </br>
-> Ein einfaches IF statement um dies zu vermeiden (Fehlschlag) </br> 
-> Stand 24.01 das Problem bleibt beim echten Roboter bestehen </br>
-> Stand 27.01 das Problem konnte im Simulator reproduzeirt werden wenn der Ball sich nicht bewegt aber unter dem Schwellwert liegt </br>

Dynamisches anlaufen:
Die Funktionen finden sich alle in [ChallangeCard.cpp](Src/Modules/BehaviorControl/BehaviorControl/Cards/Experimental/ChallangeCard.cpp) </br>
 Der Abstand zum Ball der unterschritten werden muss damit der Roboter reagiert, </br>
 wird anhand der Geschwindigkeit des Balls berechnet mit der Funktion calcMinDistance </br>
 Dabei wird die Geschwindigkeit des Balls sowie die Distanz zur Lauf-Linie des Balls berücksichtigt,   
 so dass bei höherer Geschwindikeit/Distanz der Roboter früher reagiert.
   
    float calcMinDistance() const
    {
     Vector2f temp1 = theBallModel.estimate.velocity;
     float temp2 = temp1.x() * temp1.x();
     float temp3 = temp1.y() * temp1.y();
     float result = std::sqrt(temp2 + temp3);
     float distance = 0.f;
       if (result >= 1) {
         distance = Geometry::getDistanceToLine(Geometry::Line(theFieldBall.recentBallPositionRelative(), temp1), Vector2f::Zero());
       }
     return (result + distance) * minDistanceFactor;
     }


 der InterceptPoint (Der Punkt der angelaufen wird), </br>
 wird ebenfalls anhand der geschwindigkeit des Balls berechnet mit der Funktion calcInterceptPoint
    
        //relative InterceptPoint wird berechnet durch propagateBallPosition und einem Festen Offset für einen besseren Schritt in den Ball, ifdef weil es unterschiedliche ergebnisse beim Simulator und im echten Roboter gibt
        Vector2f calcInterceptPoint() const
        {
          Vector2f temp = BallPhysics::propagateBallPosition(theFieldBall.recentBallPositionOnField(), theBallModel.estimate.velocity, interceptFactor, theBallSpecification.friction);
          Vector2f result = Vector2f::Zero();
          //Für den Fehler beim echten Roboter (die Werte sind invertiert)
    #ifdef NAO
          result = Vector2f(-(temp.x() + 100.f), -(temp.y() + 100.f));
    #else
          result = Vector2f((temp.x() + 100.f), (temp.y() + 100.f));
    #endif //Simulator
          return result;
        }
        
die oben berechneten Werte werden nochmal um jeweils einen eigenen Faktor multipliziert: </br>
Diese können in dem Code je nach Test erfolgen angepassst werden -> in eine Config umlegen für schnelleres anpassen

### Beispiel Video:

- für Fast: </br>


https://github.com/user-attachments/assets/f2cd8e37-bab2-4473-a884-26c7c3786432

- für Normal: </br>


https://github.com/user-attachments/assets/d67f9cb2-a863-4b5b-a6e3-0eb94ae47f5f

- Feld Test:

  

https://github.com/user-attachments/assets/c21c4225-807b-42c4-83b5-e52f1ef872ec


https://github.com/user-attachments/assets/e8de8c33-9848-424c-bbe2-e5bb06a3cf0d


https://github.com/user-attachments/assets/8e3c0286-4033-4ac3-b017-8374896d450c

## Zukünftige Entwicklung

Momentan funktioniert der Test mit echten Robotern nicht. </br>
- ~~Sie gehen nicht auf den Ball zu sondern machen nur ein Paar Schritte Rückwärtz (Stand 27.01). </br>~~ -> Stand 29.01 x und y-Werte mussten invertiert werden beim echten Roboter
- Es muss noch überprüft werden wie sehr das momentan Programmierte tatsächlich mit dem Regelwerk der Challenge übereinstimmt. </br>
- Eine Card für die suche nach dem Ball. Aus dem Regelwerk liest es sich heraus das die Rampe einen Tag immer an der gleichen Stelle steht </br>
  -> Der Suchwinkel kann stark reduziert werden. </br>
- Bei dem Real-Live Test wird der Roboter von Penelized zu unPenlized gewechselt, dies updatet seine Position an den Rand des Spielfeldes. Dies stört sehr bei vorführungen bei denen der Roboter an bestimmten stellen Plaziert wird. </br>
  -> Schalte dies durch z.Bsp duch eine Testing Flag aus. </br>
- Real-Live laufen die Roboter nicht auf maximaler Geschwindigkeit um die gelenke zu schonen, dies ist allerdings für die wenigen Schrtitte die wir hier machen nicht Relevant.
  -> Default Geschwindigkeit anpassen auf wenn möglich 100%.


## Relevante Klassen
- [ChallangeCard.cpp](Src/Modules/BehaviorControl/BehaviorControl/Cards/Experimental/ChallangeCard.cpp)
- [FieldBall.h](Src/Representations/BehaviorControl/FieldBall.h)
- [BallModel.h](Src/Representations/Modeling/BallModel.h)

