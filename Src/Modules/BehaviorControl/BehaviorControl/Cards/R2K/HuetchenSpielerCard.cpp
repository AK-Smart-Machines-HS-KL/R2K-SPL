/**
 * @file HuetchenSpielerCard.cpp
 * @author Laura Hammerschmidt
 * @brief Card for Huetchenspieler behavior
 * @version 1.0
 * @date 2025-01-20
 *
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Perception/ObstaclesPercepts/ObstaclesFieldPercept.h" 
#include "Representations/Modeling/ObstacleModel.h"
#include "Tools/Modeling/Obstacle.h"
#include "Representations/Communication/TeamInfo.h"

#include "Platform/Time.h"



#include "Tools/Debugging/Annotation.h"
#include "Tools/Debugging/DebugDrawings.h"

CARD(HuetchenSpielerCard,
     {
        ,
        CALLS(Activity),
        CALLS(Stand),
        CALLS(LookActive),
        REQUIRES(RobotInfo),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),
        REQUIRES(ObstacleModel),
        REQUIRES(ObstaclesFieldPercept), 

        


        DEFINES_PARAMETERS(
             {,}),
     });

enum class ConeID { LEFT, MIDDLE, RIGHT };

class HuetchenSpielerCard : public HuetchenSpielerCardBase
{

   enum State
   {
         WAITING_FOR_SETUP,
         OBSERVING_HIDE,
         TRACKING_BALL,
         REVEALING_POSITION

    };

    State currentState = WAITING_FOR_SETUP;
    ConeID lastBallPosition = ConeID::MIDDLE;

    int trackedJerseyColor = -1; 
    Vector2f savedOpponentPosition = Vector2f::Zero(); 
    
    int lastPlayerWithBall = -1;

    unsigned int waitStartTime = 0;
    unsigned int revealingStartTime = 0; 

    unsigned int lastOutputTime = 0;  
    const unsigned int outputInterval = 1000;  // Ausgabe immer nach 1 sekunde
    bool hasOutputWelcome = false;  //nur einmal HuetchenspielerCard active ausgeben


    std::string getColorName(int teamColor) const
    {
        switch(teamColor)
        {
            case TEAM_BLUE: return "BLAU";
            case TEAM_RED: return "ROT";
            case TEAM_YELLOW: return "GELB";
            case TEAM_GREEN: return "GRUEN";
            case TEAM_ORANGE: return "ORANGE";
            case TEAM_PURPLE: return "LILA";
            case TEAM_BROWN: return "BRAUN";
            default: return "UNBEKANNT";
        }
    }


   bool preconditions() const override
    {
        return true;
    }

    bool postconditions() const override
    {
        return false;
    }

    void execute() override
    {
        theLookActiveSkill();
        theActivitySkill(BehaviorStatus::defaultBehavior);
        theStandSkill();

        unsigned int currentTime = Time::getCurrentSystemTime();
        bool shouldOutput = (currentTime - lastOutputTime >= outputInterval); //ist 1 sekunde vergangen?

        if(!hasOutputWelcome)
        {
            OUTPUT_TEXT("HuetchenspielerCard active for robot " << theRobotInfo.number);
            OUTPUT_TEXT("WILLKOMMEN BEIM HUETCHENSPIEL!");
            hasOutputWelcome = true;
        }

        if(shouldOutput)
        {
            lastOutputTime = currentTime; //einmal setzen, damit die zeit im zyklus synchron bleibt
        }


        //OUTPUT_TEXT("Ball position global: x=" << theFieldBall.positionOnField.x() << " y=" << theFieldBall.positionOnField.y());
       // OUTPUT_TEXT("Ball position relative: x=" << theFieldBall.positionRelative.x() << " y=" << theFieldBall.positionRelative.y());

        //Zustandsautomat
        switch(currentState)
        {
            case WAITING_FOR_SETUP:
            {
                if(shouldOutput) OUTPUT_TEXT("WAITING_FOR_SETUP...");
                bool ballSeen = theFieldBall.ballWasSeen();


                if(ballSeen)
                {
                    OUTPUT_TEXT("Ball wird gesehen");
                    currentState = OBSERVING_HIDE;
                }
                break;
            }

            case OBSERVING_HIDE:
            {   
                if(shouldOutput) OUTPUT_TEXT("OBSERVING_HIDE...");

                bool ballSeen = theFieldBall.ballWasSeen(); //ball wird in kamera gesehen
                float y_ball = theFieldBall.positionRelative.y();
               
                //Ball erkannt?
                if(ballSeen)
               {    

                    if(y_ball < -200) //mittig definieren: [-200, 200]
                    {
                        
                        if(shouldOutput) OUTPUT_TEXT("Ball ist rechts");
                        lastBallPosition = ConeID::RIGHT;

                    } 
                    else if(y_ball > 200)
                    {
                        if (shouldOutput) OUTPUT_TEXT("Ball ist links");
                        lastBallPosition = ConeID::LEFT;
                    } 
                    else 
                    {
                        if(shouldOutput) OUTPUT_TEXT("Ball ist mittig");
                        lastBallPosition = ConeID::MIDDLE;
                    }
                }
                // Ball nicht mehr sichtbar?
                else
                {
                    OUTPUT_TEXT("BALL WURDE VERSTECKT");

                    // nur weitermachen wenn Obstacles sichtbar sind
                    if(theObstaclesFieldPercept.obstacles.empty())
                    {   
                        // bleibt in OBSERVING_HIDE und versucht es im nächsten frame erneut
                        OUTPUT_TEXT("Keine Obstacles gefunden,... Warte auf nächsten Frame");
                        
                        break;
                    }

                    // Spieler mit Ball finden und merken
                    float minDist = std::numeric_limits<float>::max(); //minimaler abstand, startet mit max wert
                    const ObstaclesFieldPercept::Obstacle* closestOpponent = nullptr; 

                    //mit obstaclefieldpercept
                    for(const auto& obstacle : theObstaclesFieldPercept.obstacles)
                    {
                        if(obstacle.type == ObstaclesFieldPercept::opponentPlayer)
                        {
                            float dist = (obstacle.center - theFieldBall.positionRelative).norm(); //abstand zwischen spieler und ball
                            if(dist < minDist)
                            {
                                minDist = dist;
                                closestOpponent = &obstacle;
                            }

                        }

                    }

                    if(closestOpponent != nullptr)
                    {   
                        savedOpponentPosition = closestOpponent->center;
                        trackedJerseyColor = closestOpponent->jerseyColor;

                        float y_pos = savedOpponentPosition.y();

                        if(y_pos < -200)
                        {
                            OUTPUT_TEXT("SPIELER "<<getColorName(trackedJerseyColor)<<" MIT BALL IST RECHTS");
                        } 
                        else if(y_pos > 200)
                        {
                            OUTPUT_TEXT("SPIELER "<<getColorName(trackedJerseyColor)<<" MIT BALL IST LINKS");
                        } 
                        else 
                        {
                            OUTPUT_TEXT("SPIELER "<<getColorName(trackedJerseyColor)<<" MIT BALL IST MITTIG");
                        }
                    }

                    currentState = TRACKING_BALL;
                }       
                
                break;
            }

            case TRACKING_BALL:
            {
                if(shouldOutput) OUTPUT_TEXT("TRACKING_BALL...");

                if(waitStartTime == 0)
                {
                    waitStartTime = Time::getCurrentSystemTime();

                    OUTPUT_TEXT("Pause gestartet, jetzt mischen!");
                    //jetzt spieler mischen
                }
                
                if(Time::getCurrentSystemTime() - waitStartTime >= 10000)//10 sek vergangen?
                {
                    waitStartTime = 0; //reset
                    currentState = REVEALING_POSITION;
                }
                break;
            }   

            case REVEALING_POSITION:
            {
                // init Timeout beim ersten Mal
                if(revealingStartTime == 0)
                {
                    revealingStartTime = Time::getCurrentSystemTime();
                    OUTPUT_TEXT("Starte Suche nach Farbe: " << getColorName(trackedJerseyColor));
                }
                    

                // sind 5sek vergangen?
                unsigned int elapsedTime = Time::getCurrentSystemTime() - revealingStartTime;
                if(elapsedTime > 5000)
                {
                    OUTPUT_TEXT("TIMEOUT: Farbe " << getColorName(trackedJerseyColor) << " nicht gefunden nach 5 Sekunden ");
                    revealingStartTime = 0;
                    trackedJerseyColor = -1;
                    currentState = WAITING_FOR_SETUP;
                    break;
                }
                

                OUTPUT_TEXT("REVEALING_POSITION... (" << (elapsedTime/1000.0f) << "s)");
                OUTPUT_TEXT("Suche Spieler mit Farbe: " << getColorName(trackedJerseyColor));

                // WICHTIG: nur weitermachen wenn Obstacles sichtbar 
                if(theObstaclesFieldPercept.obstacles.empty())
                {
                    OUTPUT_TEXT("WARNUNG: keine Obstacles gefunden,... Warte auf nächsten Frame");
                    // bleibt in REVEALING_POSITION und versucht es im nächsten Frame nochmal
                    break;
                }

                const ObstaclesFieldPercept::Obstacle* foundOpponent = nullptr; 
                // Suche Spieler mit der gespeicherten Jersey-Farbe
                for(const auto& obstacle : theObstaclesFieldPercept.obstacles)
                {
                    if(obstacle.type == ObstaclesFieldPercept::opponentPlayer)
                    {
                        if(obstacle.jerseyColor == trackedJerseyColor && trackedJerseyColor != -1)
                        {
                            OUTPUT_TEXT("Spieler mit Farbe " << getColorName(trackedJerseyColor) << " gefunden!");
                            foundOpponent = &obstacle;
                            break;
                        }
                    }
                }
                
                if(foundOpponent != nullptr)
                {
                    revealingStartTime = 0; // Reset timeout
                    OUTPUT_TEXT("Spieler mit Farbe " << getColorName(trackedJerseyColor) << " wiedergefunden!");

                    float y_pos = foundOpponent->center.y();
                    if(y_pos < -200) //mittig definieren: [-200, 200]
                    {
                        OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << "MIT BALL IST RECHTS");
                    } 
                    else if(y_pos > 200)
                    {
                        OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << " MIT BALL IST LINKS");
                    } 
                    else 
                    {
                        OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << "MIT BALL IST MITTIG");
                    }
                }
                else
                {
                    OUTPUT_TEXT("SPIELER MIT FARBE " << getColorName(trackedJerseyColor) << " IN DIESEM FRAME NICHT GEFUNDEN - warte weiter... ");
                    // bleibt in REVEALING_POSITION und versucht es weiter
                    break;
                }

                // Nur hier ankommen wenn gefunden
                  
                //reset
                currentState = WAITING_FOR_SETUP;
                trackedJerseyColor = -1; // Reset
                savedOpponentPosition = Vector2f::Zero();


                break;
            }
                
        }

        

    }
};

MAKE_CARD(HuetchenSpielerCard);