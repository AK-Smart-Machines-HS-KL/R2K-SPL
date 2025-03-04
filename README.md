

Momentaner Arbeiter des Branches 2025-03-IRB:

Name: 		Dennis Fuhrmann </br>
E-mail:		defu1001@stud.hs-kl.de </br>
Discord name:	PinkPummeler </br>

Dieses Projekt beschäftigt sich mit der Intercept Rolling Ball Challenge 2025. Der Bericht besteht aus 2 Blöcken. Der erste Block befasst sich mit Dennis Projektarbeit, der zweite Block beschreibt die zukünftigen Bestrebungen.</br>

Beim RoboCup2025 ist eine Challange zu absolvieren bei der ein von einer Rampe rollender Ball abzufangen und direkt zu spielen ist.</br> 
Das Ziel ist es, im Spiel einen Pass anzunehmen, sowie das Abfangen schneller Bälle vom Gegner zu optimieren.</br></br>

Studierende, die mit an der Challenge arbeiten, erhalten einen Einblick in die aktuellen Challenges im RoboCup und in die Hürden,</br>
 die der Robotik softwareseitig derzeit auferlegt sind. Es gilt sich in die Verhaltens-, aber auch in die Körpersteuerung einzuarbeiten, um die Reaktionszeit der Roboter zu erhöhen und und ein angemessenes Abfangverhalten sicherzustellen.</br></br>

Folgendes Demonstrationsvideo von B-Human veranschaulicht unser Ziel:</br>

[![Inspiration von B-human](https://img.youtube.com/vi/ufiUQ-02DWk/0.jpg)](https://www.youtube.com/watch?v=ufiUQ-02DWk)


Das Rule Book für die Technichal Challange 2025 erklärt das tatsächliche Setup genauer:

[Download Seite für das Regelwerk](https://spl.robocup.org/downloads/)
 

Verwendet wird der Code, genannt Card: [ChallangeCard.cpp](Src/Modules/BehaviorControl/BehaviorControl/Cards/Experimental/ChallangeCard.cpp) </br> 
## 0) Projektidee und Lösungsansatz
Unser Ziel ist es den Ball direkt abzufangen und zu schießen.
Ein erster Ansatz verfolgt die normale Schusstechnuik der NAOs zu verwenden. Es stellt sich heraus, dass diese Technik zu langsam ist, da der NAO sich erst richtig ausrichten muss: Der Ball rollt am NAO vorbei bevor dieser die Schussbewegung ausführen kann.</br>
Daher liegt ein alternativer Ansatz nahe, der auf die Schussbewegung verzichtet: Wir lassen den NAO anstelle des Schusses in den rollenden Ball reinstolpern. So wird zwar nichtmehr die NAO Schussbewegung genutzt, allerdings erhöht sich durch diesen Ansatz die Rate der Torschüsse. 
## 1) SetUp zum Nachbau der Challenge
game stack: fast alle Cards löschen, TeachIn auch gelöscht </br>
DefaultCard wurde bearbeitet -> Walk in fällt weg und der Roboter bleibt auf der Stelle stehen.  
## Simulator:

Öffne im Simulator die Scene `IRBChallangeFast.ros2` für ein Fast Game auf schwachen Maschinen oder das SetUp`IRBCHallangeNormal.ros2`, welches sich für einen besser simulierten Roboter anbietet.</br>
Hier wird der Ball direkt vor die Füße des Roboters vor dem Tor gespielt und dieser soll in den rollenden Ball hineinlaufen.  </br>

Alternativ zum selber Nachbauen im Simulator:</br>
using OneTeamFast.ros2 ignoriere einfach die Dummies </br>
First `gc playing` (ansonsten wird der CornerKick nicht ausgeführt) </br>
then ´mvb -4300 2900 0´ </br>
then bewege robot 4 näher an den Ball damit er diesen rechtzeitig erreicht </br>
then wähle robot 5 und benutze den Befehl ´mv -3550 0 300´ </br>
then ´gc cornerKickForFirstTeam´ </br>
die CornerKickCard wurde so modifiziert  das der Ball vor die Füße des Roboters vorm Tor gespielt werden sollte</br>


copy - paste - List:

gc playing </br>
mvb -4300 2900 0 </br>
mv -3550 0 300 </br>
gc cornerKickForFirstTeam </br>
## Corner kick
Die [OwnCornerKickCard.cpp](Src\Modules\BehaviorControl\BehaviorControl\Cards\Gamestates\OwnCornerKickCard.cpp) wurde so angepasst, das der Ball nicht mehr an den Pfosten gespielt wird, sondern um einen festen Winkel versetzt vor das Tor gespielt wird, dabei wird die aktuelle Spiellage nicht berücksichtigt.
Der Empfänger steht auf einem festen Punkt, der durch ausprobieren ermittelt wurde, vor dem Tor und dieser soll dann die Challenge ausführen.
### Beispiel Video für Simulator:

https://github.com/user-attachments/assets/d67f9cb2-a863-4b5b-a6e3-0eb94ae47f5f


### Durchführung Feldtest
Für den Feldtest wird nur ein Roboter verwendet, den Pass spielt ein Mensch.</br>
Deploye den Roboter in Release Mode (für schnellere Reaktionen) und platziere ihn vor dem Mittelkreis, sodass er seine Odometrie besser anpassen kann. </br>
Warte bis der Roboter sicher steht und platziere den Ball für ihn sichtbar schräg Rechts oder schräg Links, denn er schaut sich nach ihm langsam um. </br>
Nachdem er ihn gefunden hat, rolle den Ball dem Roboter vor die Füße. Es ist sinnvoll eine Markierung dort zu setzen, wo der Ball hin gerollt werden soll.   </br>
Nun sollte der Roboter den Ball abfangen. </br>

### Hier sind drei Beispielvideos aus dem Feldtest:


https://github.com/user-attachments/assets/c21c4225-807b-42c4-83b5-e52f1ef872ec


https://github.com/user-attachments/assets/e8de8c33-9848-424c-bbe2-e5bb06a3cf0d


https://github.com/user-attachments/assets/8e3c0286-4033-4ac3-b017-8374896d450c
 

## Projekterkenntnisse: Welche Skills kommen in Frage?
### gotoBall: viele Parameter Schusssteuerung
Welche Kombination bringen welchen Erfolg: -> Der Schuss wird bei allen getesteten Kombinationen erst bei Ball stopp ausgelöst.

Die Reexamination mit größerer Range (20_deg -> 40_deg -> 80_deg -> 140_deg -> 180_deg) ergibt keine Änderung zu vorher.  

### walktToBall: obstacleAvoidance ausschalten
Die Obstacle avoidance kann ausgeschaltet werden, denn der Schuss ist immer noch zu langsam. Dies ist nicht wünschenswert. </br> 
Es besteht das gleiche Problem wie bei GotoBall: Der NAO versucht dem Ball hinterherzulaufen, anstatt diesen einfach direkt zu kicken. </br>

### Dribble:
Es ergibt sich das gleiche Problem wie bei WalktoBall. </br>

### WalkToPoint:
Anstatt den Ball zu treten in den Ball rein laufen -> Schwacher Kick </br>
Ergebnis: Vielversprechendes Ergebnis -> muss noch dynamischer angepasst werden </br>
Echte Naos: In echt sind die Naos langsamer -> Reaktion entsprechend den Naos anpassen oder die Geschwindigkeit für die wenigen Schritte erhöhen </br>

Dynamisches Anlaufen: Im Simulator viel zuverlässiger </br>
Erstes Testergebnis: Vorzeichen Fehler beim InterceptPoint -> der Roboter ist nach hinten gelaufen </br>
-> Ein einfaches IF statement um dies zu vermeiden (Fehlschlag) </br> 
-> Stand 24.01 das Problem bleibt beim echten Roboter bestehen </br>
-> Stand 27.01 das Problem konnte im Simulator reproduziert werden, wenn der Ball sich nicht bewegt aber unter dem Schwellwert liegt </br>
-> Stand 31.01 das Problem kommt durch Fehler bei propagateBallPosition, (invertierte x und y Werte) wurde durch ein #ifdef gefixt  
-> Der Roboter läuft richtig in den Ball  

Dynamisches Anlaufen:
Die Funktionen finden sich alle in [ChallangeCard.cpp](Src/Modules/BehaviorControl/BehaviorControl/Cards/Experimental/ChallangeCard.cpp) </br>
 Der Abstand zum Ball, der unterschritten werden muss, damit der Roboter reagiert, </br>
 wird anhand der Geschwindigkeit des Balls berechnet mit der Funktion calcMinDistance. </br>
 Dabei wird die Geschwindigkeit des Balls sowie die Distanz zur Lauf-Linie des Balls berücksichtigt,   
 sodass bei höherer Geschwindikeit/Distanz der Roboter früher reagiert.
   
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


 Der InterceptPoint (Der Punkt der angelaufen wird), </br>
 wird ebenfalls anhand der Geschwindigkeit des Balls berechnet mit der Funktion calcInterceptPoint   
 dabei muss durch ein ifdef zwischen dem Simulator und einem echten Roboter unterschieden werden,   
 weil beim echten Roboter die x und y Werte invertiert sind.  
    
        //relative InterceptPoint wird berechnet durch propagateBallPosition und einem Festen Offset für einen besseren Schritt in den Ball, ifdef weil es unterschiedliche Ergebnisse beim Simulator und im echten Roboter gibt
        Vector2f calcInterceptPoint() const
        {
          Vector2f temp = BallPhysics::propagateBallPosition(theFieldBall.recentBallPositionOnField(), theBallModel.estimate.velocity, interceptFactor, theBallSpecification.friction);
          Vector2f result = Vector2f::Zero();
          //Für den Fehler beim echten Roboter (die Werte sind invertiert)
    #ifdef TARGET_ROBOT
          result = Vector2f(-(temp.x() + 100.f), -(temp.y() + 100.f));
    #else
          result = Vector2f((temp.x() + 100.f), (temp.y() + 100.f));
    #endif //Simulator
          return result;
        }
        
Die oben berechneten Werte werden nochmal um jeweils einen eigenen Faktor multipliziert: </br>
Diese können in dem Code je nach Test erfolgen angepasst werden -> in eine Config umlegen für schnelleres anpassen


## Zukünftige Entwicklung: Rolling Ball Challenge
- Eine Card für die Suche nach dem Ball. Aus dem Regelwerk liest es sich heraus, das die Rampe einen Tag immer an der gleichen Stelle steht </br>
  -> Der Suchwinkel kann stark reduziert werden. </br>
- Bei dem Real-Live Test wird der Roboter von Penelized zu unPenlized gewechselt, dies updatet seine Position an den Rand des Spielfeldes. Dies stört sehr bei vorführungen bei denen der Roboter an bestimmten stellen Plaziert wird. </br>
  -> Schalte dies durch z.Bsp druch eine Testing Flag aus. </br>
-  Real-Live laufen die Roboter nicht auf maximaler Geschwindigkeit um die Gelenke zu schonen, dies ist allerdings für die wenigen Schritte die wir hier machen nicht Relevant.
  -> Default Geschwindigkeit anpassen auf wen möglich 100%.
   ## Zukünftige Entwicklung: Migration zum Corner Kick
- Anpassung von CornerKick für ein besseren Passspiel, mit berücksichtigung aktueller Spielage.
- Bestimmung der Empfänger-Pose beim Eckball (Corner Kick).
  -> Kommunikation zwichen Empfäanger und Passspieler realisieren.
- Precondition von Card für den Torschützen anpassen
- Feldtest: reicht die Corner Kick Zeitphse aus für die beiden Schüsse  



## Zusammenfassung
Die bisherigen Kick Skills sind für die Challange ungeeignet. Stattdessen nutzen wir WalkToPoint um in den Ball hineinzulaufen und somit einen Kick "Simulieren".
Dabei wird der angelaufene Punkt durch die Funktion `Vector2f calcInterceptPoint()` berechnet. Kann durch verändern von `interceptFactor` angepasst werden
Die Reaktionszeitpunkt wird durch `float calcMinDistance()` berechnet. Kann durch verändern von `minDistanceFactor` angepasst werden.
Bei einem echten Roboter mussten die Werte ebenfalls invertiert werden -\_(´. .`)\_-, dies wurde durch eine ifdef gemacht.

## Relevante Klassen
- [ChallangeCard.cpp](Src/Modules/BehaviorControl/BehaviorControl/Cards/Experimental/ChallangeCard.cpp)
- [FieldBall.h](Src/Representations/BehaviorControl/FieldBall.h)
- [BallModel.h](Src/Representations/Modeling/BallModel.h)

