/**
 * @file HuetchenSpielerCard.cpp
 * @author Laura Hammerschmidt
 * @brief Card for Huetchenspieler behavior
 * @version 1.0
 * @date 2025-12-13
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
#include "Representations/Infrastructure/GroundTruthWorldState.h"
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
        REQUIRES(GroundTruthWorldState),

        DEFINES_PARAMETERS(
             {,
                // Parameter hier definieren
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
    ConeID lastBallPosition = ConeID::MIDDLE;
    int lastPlayerWithBall = -1;
    unsigned int waitStartTime = 0;

    unsigned int lastOutputTime = 0;  
    const unsigned int outputInterval = 1000;  // Ausgabe immer nach 1 sekunde




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

        if(shouldOutput)
        {
            OUTPUT_TEXT("HuetchenspielerCard active for robot " << theRobotInfo.number);
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
                    OUTPUT_TEXT("BALL WIRD GESEHEN");
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
                        
                        if(shouldOutput) OUTPUT_TEXT("BALL IST RECHTS");
                        lastBallPosition = ConeID::RIGHT;

                    } 
                    else if(y_ball > 200)
                    {
                        if (shouldOutput) OUTPUT_TEXT("BALL IST LINKS");
                        lastBallPosition = ConeID::LEFT;
                    } 
                    else 
                    {
                        if(shouldOutput) OUTPUT_TEXT("BALL IST MITTIG");
                        lastBallPosition = ConeID::MIDDLE;
                    }
                }
                // Ball nicht mehr sichtbar?
                else
                {
                    OUTPUT_TEXT("BALL WURDE VERSTECKT");

                    // Spieler mit Ball finden und merken
                    float minDist = std::numeric_limits<float>::max(); //minimaler abstand, startet mit max wert
                    for(const auto& player : theGroundTruthWorldState.firstTeamPlayers) // alle spieler der einen mannschaft durchlaufen
                    {
                        float dist = (player.pose.translation - theFieldBall.positionOnField).norm(); //abstand zwischen spieler und ball
                        if(dist < minDist)
                        {
                            minDist = dist;
                            lastPlayerWithBall = player.number;
                        }
                    } 
                
                    OUTPUT_TEXT("SPIELER MIT BALL: " << lastPlayerWithBall);
                        
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
                OUTPUT_TEXT("REVEALING_POSITION...");
               /* OUTPUT_TEXT("BALL IST: ");
                switch(lastBallPosition)
                {
                    case ConeID::LEFT:
                        OUTPUT_TEXT("LINKS");
                        break;
                    case ConeID::MIDDLE:
                        OUTPUT_TEXT("MITTIG");
                        break;
                    case ConeID::RIGHT:
                        OUTPUT_TEXT("RECHTS");
                        break;
                }*/
                
                
                for(const auto& player : theGroundTruthWorldState.firstTeamPlayers)
                {
                    if(player.number == lastPlayerWithBall)
                    {   
                        float y_player = player.pose.translation.y();
                        if(y_player < -200) //mittig definieren: [-200, 200]
                        {
                            OUTPUT_TEXT("SPIELER" << player.number <<" MIT BALL IST RECHTS");
                            //lastBallPosition = ConeID::RIGHT;

                        } 
                        else if(y_player > 200)
                        {
                            OUTPUT_TEXT("SPIELER" << player.number <<" MIT BALL IST LINKS");
                            //lastBallPosition = ConeID::LEFT;
                        } 
                        else 
                        {
                            OUTPUT_TEXT("SPIELER" << player.number <<" MIT BALL IST MITTIG");
                            //lastBallPosition = ConeID::MIDDLE;
                        }

                        //OUTPUT_TEXT("SPIELER " << player.number << " STEHT JETZT BEI X=" << player.pose.translation.x() << " Y=" << player.pose.translation.y());
                    }
                }
                //reset
                currentState = WAITING_FOR_SETUP;
                break;
            }
                
        }

        

    }
};

MAKE_CARD(HuetchenSpielerCard);