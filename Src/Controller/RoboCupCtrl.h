/**
 * @file Controller/RoboCupCtrl.h
 *
 * This file declares the class RoboCupCtrl.
 *
 * @author Thomas Röfer
 */

#pragma once

#include "SimulatedRobot.h"
#include "GameController.h"
#include "Tools/Settings.h"
#include <SimRobotCore2.h>
#include <SimRobotCore2D.h>

#include <QBrush>
#include <QList>
#include <QString>
#include <list>

class ControllerRobot;

/**
 * The class implements the SimRobot controller for RoboCup.
 */
class RoboCupCtrl : public SimRobot::Module, public SimRobotCore2::CollisionCallback, public SimRobotCore2D::CollisionCallback
{
public:
  static RoboCupCtrl* controller; /**< A pointer to the SimRobot controller. */
  static SimRobot::Application* application; /**< The interface to the SimRobot GUI. */
  GameController gameController;
  Settings::TeamColor firstTeamColor = static_cast<Settings::TeamColor>(-1); /**< Color of the first team. */
  Settings::TeamColor secondTeamColor = static_cast<Settings::TeamColor>(-1); /**< Color of the second team. */
  bool is2D = false; /**< Whether the controller is loaded in the 2D simulator (otherwise it is 3D simulation). */
  SimRobot::Object* ballGeometry = nullptr; /**< The collision geometry belonging to the ball. */
  float simStepLength; /**< The length of one simulation step (in ms). */

protected:
  std::list<ControllerRobot*> robots; /**< The list of all robots. */
  float delayTime = 0.f; /**< Delay simulation to reach this duration of a step. */
  float lastTime = 0.f; /**< The last time execute was called. */
  QString statusText; /**< The text to be printed in the status bar. */
  std::string location = "Default"; /**< The location for simulated robots. */
  std::array<std::string, 2> scenarios = {"Default", "Default"}; /**< The scenarios for simulated robots per team. */

private:
  QList<SimRobot::Object*> views; /**< List of registered views */

public:
  RoboCupCtrl(SimRobot::Application& application);

protected:
  ~RoboCupCtrl();

public:
  /**
   * Return the alternate background color for views. It is used when
   * view separate different rows by switching between two different
   * background colors.
   * @return The color.
   */
  QBrush getAlternateBackgroundColor() const;

  /**
   * Adds a scene graph object to the scene graph displayed in SimRobot
   * @param object The scene graph object to add
   * @param parent The parent scene graph object (e.g. a category or null)
   * @param flags Some flags for registering the scene graph object (see SimRobot::Flag)
   */
  void addView(SimRobot::Object* object, const SimRobot::Object* parent = nullptr, int flags = 0);

  /**
   * Adds a scene graph object to the scene graph displayed in SimRobot
   * @param object The scene graph object to add
   * @param categoryName The full name of the parent categroy
   * @param flags Some flags for registering the scene graph object (see SimRobot::Flag)
   */
  void addView(SimRobot::Object* object, const QString& categoryName, int flags = 0);

  /**
   * Removes a view that was previously added with addView(). Note that no parent object
   * is required here, due to asymmetrical registerObject()/unregisterObject() definitions
   * @param object View to remove from scene graph
   */
  void removeView(SimRobot::Object* object);

  /**
   * Adds a category to the scene graph displayed in SimRobot that can be used for grouping views
   * @param name The name of the category
   * @param parent The parent scene graph object (e.g. a category or null)
   * @param icon The icon used to list the category in the scene graph
   * @return The category
   */
  SimRobot::Object* addCategory(const QString& name, const SimRobot::Object* parent = nullptr, const char* icon = nullptr);

  /**
   * Adds a category to the scene graph displayed in SimRobot that can be used for grouping views
   * @param name The name of the category
   * @param parentName The name of the parent scene graph object (e.g. a category)
   * @return The category
   */
  SimRobot::Object* addCategory(const QString& name, const QString& parentName);

  /**
   * Removes a category that was previously added with addCategory(). Note that no parent object
   * is required here, due to asymmetrical registerObject()/unregisterObject() definitions
   * @param object Category to remove from scene graph
   */
  void removeCategory(SimRobot::Object* object);

protected:
  /**
   * The function is called to initialize the module.
   */
  bool compile() override;

  /**
   * The function is called in each simulation step.
   */
  void update() override;

  /**
   * The callback function.
   * Called whenever the geometry at which this interface is registered collides with another geometry.
   * @param geom1 The geometry at which the interface has been registered
   * @param geom2 The other geometry
   */
  void collided(SimRobotCore2::Geometry& geom1, SimRobotCore2::Geometry& geom2) override;

  /**
   * The callback function for the 2D core.
   * Called whenever the geometry at which this interface is registered collides with another geometry.
   * @param geom1 The geometry at which the interface has been registered
   * @param geom2 The other geometry
   */
  void collided(SimRobotCore2D::Geometry& geom1, SimRobotCore2D::Geometry& geom2) override;

  /**
   * Has to be called by derived class to start threads.
   */
  void start();

  /**
   * Has to be called by derived class to stop threads.
   */
  void stop();
};
