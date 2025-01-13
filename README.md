# R-ZWEI Kickers Codebase 2024

This is where the code of the team **R-ZWEI Kickers** lives. Its documentation can be found in the [Wiki of this repository](https://github.com/AK-Smart-Machines-HS-KL/R2K-SPL/wiki)

### Git

If you need a refreshment on your Git know-how, we recommend freshening it up with a tutorial, e.g. [this one in German](https://lerneprogrammieren.de/git/) or [this English one](https://www.w3schools.com/git/).

### B-Human Code Release

This code is based on the official 2021 B-Human code release. Documentation for it can be found in their [public wiki](https://wiki.b-human.de/coderelease2021/).

Previous code releases are tagged with "coderelease&lt;year&gt;", where &lt;year&gt; is the year in which the code was released (starting with 2013).

Please note that **before** you clone this repository on Windows, execute:

```
git config --global core.autocrlf input
git config --global core.eol lf
```

As this repository uses submodules, it must be cloned using `git clone --recursive`. Downloading the release as `zip` or `tar.gz` does not work.

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
 
using OneTeam.ros2
First `gc ready` (otherwise cornerKick wont work)
wait until playing
then ´mvb -4300 2900 0´
then ´gc cornerKickForFirstTeam´
then select one free nao of first team and ´mv -3400 0 300´
the CornerKickCard hs benn modified so that the Ball should be played right infront of the feet of the second robot

Testing results:
GoToBallAndKick is too slow for what we need here and after a discussion with the professor no other skill is viable
-> Programm a new Skill, with a faster Kick which also acts more dynamicly:
 Proposed name: waitForBallAndKick keeping with the previous naming convention 

