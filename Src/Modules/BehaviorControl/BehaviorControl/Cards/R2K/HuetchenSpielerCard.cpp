/**
 * @file HuetchenSpielerCard.cpp
 * @author Laura Hammerschmidt
 * @brief Card for Huetchenspieler behavior
 * @version 1.0
 * @date 2025-01-23
 * !!! Nicht Stresssave
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

        


        LOADS_PARAMETERS(
             {,
             (float) coneThresholdY,
             (unsigned int) mixingDuration,
             (unsigned int) revealTimeout,
             }),
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
    State lastOutputState = WAITING_FOR_SETUP; // Track last output state
    ConeID lastBallPosition = ConeID::MIDDLE;
    ConeID lastOutputBallPosition = ConeID::MIDDLE; // Track last output ball position
    ConeID lastOutputPlayerPosition = ConeID::MIDDLE; // Track last output player position

    int trackedJerseyColor = TEAM_RED; 
    Vector2f savedOpponentPosition = Vector2f::Zero(); 
    
    unsigned int waitStartTime = 0;
    unsigned int revealingStartTime = 0; 

    bool hasOutputWelcome = false;  //nur einmal HuetchenspielerCard active ausgeben
    bool hasOutputBallPosition = false; // Track if any ball position was output yet
    bool hasOutputBallHidden = false; // Track if "Ball versteckt" was output
    bool hasOutputMixingStarted = false; // Track if mixing message was output
    bool hasOutputSearchStart = false; // Track if search start was output


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

        if(!hasOutputWelcome)
        {
            OUTPUT_TEXT("HuetchenspielerCard active for robot " << theRobotInfo.number);
            OUTPUT_TEXT("WILLKOMMEN BEIM HUETCHENSPIEL!");
            hasOutputWelcome = true;
        }


        //OUTPUT_TEXT("Ball position global: x=" << theFieldBall.positionOnField.x() << " y=" << theFieldBall.positionOnField.y());
       // OUTPUT_TEXT("Ball position relative: x=" << theFieldBall.positionRelative.x() << " y=" << theFieldBall.positionRelative.y());

        //Zustandsautomat
        switch(currentState)
        {
            case WAITING_FOR_SETUP:
            {
                // Output state nur beim ersten Mal oder bei Zustandswechsel
                if(lastOutputState != currentState)
                {
                    OUTPUT_TEXT("WAITING_FOR_SETUP...");
                    lastOutputState = currentState;
                }
                
                bool ballSeen = theFieldBall.ballWasSeen();

                if(ballSeen)
                {
                    OUTPUT_TEXT("Ball wird gesehen");
                    currentState = OBSERVING_HIDE;
                    hasOutputBallHidden = false; // Reset for next cycle
                    hasOutputBallPosition = false; // Reset to output initial ball position
                }
                break;
            }

            case OBSERVING_HIDE:
            {   
                // Output state nur beim ersten Mal oder bei Zustandswechsel
                if(lastOutputState != currentState)
                {
                    OUTPUT_TEXT("OBSERVING_HIDE...");
                    lastOutputState = currentState;
                }

                bool ballSeen = theFieldBall.ballWasSeen(); //ball wird in kamera gesehen
                float y_ball = theFieldBall.positionRelative.y();
               
                //Ball erkannt?
                if(ballSeen)
               {    
                    ConeID currentBallPosition;
                    
                    if(y_ball < -coneThresholdY)
                    {
                        currentBallPosition = ConeID::RIGHT;
                    } 
                    else if(y_ball > coneThresholdY)
                    {
                        currentBallPosition = ConeID::LEFT;
                    } 
                    else 
                    {
                        currentBallPosition = ConeID::MIDDLE;
                    }
                    
                    // Output beim ersten Mal oder wenn sich Position geändert hat
                    if(!hasOutputBallPosition || currentBallPosition != lastOutputBallPosition)
                    {
                        if(currentBallPosition == ConeID::RIGHT)
                            OUTPUT_TEXT("Ball ist rechts");
                        else if(currentBallPosition == ConeID::LEFT)
                            OUTPUT_TEXT("Ball ist links");
                        else
                            OUTPUT_TEXT("Ball ist mittig");
                        
                        lastOutputBallPosition = currentBallPosition;
                        hasOutputBallPosition = true;
                    }
                    
                    lastBallPosition = currentBallPosition;
                }
                // Ball nicht mehr sichtbar?
                else
                {
                    // Nur einmal ausgeben
                    if(!hasOutputBallHidden)
                    {
                        OUTPUT_TEXT("BALL WURDE VERSTECKT");
                        hasOutputBallHidden = true;
                    }

                    // nur weitermachen wenn Obstacles sichtbar sind
                    if(theObstaclesFieldPercept.obstacles.empty())
                    {   
                        // bleibt in OBSERVING_HIDE und versucht es im nächsten frame erneut
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
                        ConeID playerPosition;

                        if(y_pos < -coneThresholdY)
                        {
                            playerPosition = ConeID::RIGHT;
                            OUTPUT_TEXT("SPIELER "<<getColorName(trackedJerseyColor)<<" MIT BALL IST RECHTS");
                        } 
                        else if(y_pos > coneThresholdY)
                        {
                            playerPosition = ConeID::LEFT;
                            OUTPUT_TEXT("SPIELER "<<getColorName(trackedJerseyColor)<<" MIT BALL IST LINKS");
                        } 
                        else 
                        {
                            playerPosition = ConeID::MIDDLE;
                            OUTPUT_TEXT("SPIELER "<<getColorName(trackedJerseyColor)<<" MIT BALL IST MITTIG");
                        }
                        
                        lastOutputPlayerPosition = playerPosition;
                    }

                    currentState = TRACKING_BALL;
                    hasOutputMixingStarted = false; // Reset for next state
                }       
                
                break;
            }

            case TRACKING_BALL:
            {
                // Output state nur beim ersten Mal oder bei Zustandswechsel
                if(lastOutputState != currentState)
                {
                    OUTPUT_TEXT("TRACKING_BALL...");
                    lastOutputState = currentState;
                }

                if(waitStartTime == 0)
                {
                    waitStartTime = Time::getCurrentSystemTime();

                    if(!hasOutputMixingStarted)
                    {
                        OUTPUT_TEXT("Pause gestartet, jetzt mischen!");
                        hasOutputMixingStarted = true;
                    }
                }
                
                if(Time::getCurrentSystemTime() - waitStartTime >= mixingDuration)
                {
                    waitStartTime = 0; //reset
                    currentState = REVEALING_POSITION;
                    hasOutputSearchStart = false; // Reset for next state
                }
                break;
            }   

            case REVEALING_POSITION:
            {
                // Output state nur beim ersten Mal oder bei Zustandswechsel
                if(lastOutputState != currentState)
                {
                    OUTPUT_TEXT("REVEALING_POSITION...");
                    lastOutputState = currentState;
                }
                
                // init Timeout beim ersten Mal
                if(revealingStartTime == 0)
                {
                    revealingStartTime = Time::getCurrentSystemTime();
                    
                    if(!hasOutputSearchStart)
                    {
                        OUTPUT_TEXT("Starte Suche nach Farbe: " << getColorName(trackedJerseyColor));
                        hasOutputSearchStart = true;
                    }
                }

                // Timeout überschritten?
                unsigned int elapsedTime = Time::getCurrentSystemTime() - revealingStartTime;
                if(elapsedTime > revealTimeout)
                {
                    OUTPUT_TEXT("TIMEOUT: Farbe " << getColorName(trackedJerseyColor) << " nicht gefunden nach " << (revealTimeout/1000) << " Sekunden ");
                    revealingStartTime = 0;
                    trackedJerseyColor = -1;
                    currentState = WAITING_FOR_SETUP;
                    break;
                }

                // WICHTIG: nur weitermachen wenn Obstacles sichtbar 
                if(theObstaclesFieldPercept.obstacles.empty())
                {
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
                            foundOpponent = &obstacle;
                            break;
                        }
                    }
                }
                
                if(foundOpponent != nullptr)
                {
                    OUTPUT_TEXT("Spieler mit Farbe " << getColorName(trackedJerseyColor) << " gefunden!");
                    revealingStartTime = 0; // Reset timeout
                    
                    float y_pos = foundOpponent->center.y();
                    ConeID foundPlayerPosition;
                    
                    if(y_pos < -coneThresholdY)
                    {
                        foundPlayerPosition = ConeID::RIGHT;
                    } 
                    else if(y_pos > coneThresholdY)
                    {
                        foundPlayerPosition = ConeID::LEFT;
                    } 
                    else 
                    {
                        foundPlayerPosition = ConeID::MIDDLE;
                    }
                    
                    // Vergleiche mit letzter gespeicherter Position
                    if(foundPlayerPosition != lastOutputPlayerPosition)
                    {
                        if(foundPlayerPosition == ConeID::RIGHT)
                            OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << " MIT BALL IST JETZT RECHTS");
                        else if(foundPlayerPosition == ConeID::LEFT)
                            OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << " MIT BALL IST JETZT LINKS");
                        else
                            OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << " MIT BALL IST JETZT MITTIG");
                    }
                    else
                    {
                        if(foundPlayerPosition == ConeID::RIGHT)
                            OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << " MIT BALL IST IMMER NOCH RECHTS");
                        else if(foundPlayerPosition == ConeID::LEFT)
                            OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << " MIT BALL IST IMMER NOCH LINKS");
                        else
                            OUTPUT_TEXT("SPIELER "<< getColorName(trackedJerseyColor) << " MIT BALL IST IMMER NOCH MITTIG");
                    }
                }
                else
                {
                    // Keine Ausgabe mehr bei jedem Frame - warten still
                    break;
                }

                // Nur hier ankommen wenn gefunden
                  
                //reset
                currentState = WAITING_FOR_SETUP;
                trackedJerseyColor = TEAM_RED; // Reset
                savedOpponentPosition = Vector2f::Zero();


                break;
            }
                
        }

        

    }
};

MAKE_CARD(HuetchenSpielerCard);