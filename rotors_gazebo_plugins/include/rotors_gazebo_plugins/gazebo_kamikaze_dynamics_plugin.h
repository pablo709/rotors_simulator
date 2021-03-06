/*
 */

#ifndef ROTORS_GAZEBO_PLUGINS_KAMIKAZE_DYNAMICS_PLUGIN_H
#define ROTORS_GAZEBO_PLUGINS_KAMIKAZE_DYNAMICS_PLUGIN_H

#include <gazebo/common/Plugin.hh>
#include <gazebo/common/common.hh>
#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <mav_msgs/default_topics.h>

#include "Actuators.pb.h"
#include "RollPitchYawrateThrust.pb.h"
#include "WindSpeed.pb.h"

#include "rotors_gazebo_plugins/common.h"
#include "rotors_gazebo_plugins/kamikaze_parameters.h"

namespace gazebo {

typedef const boost::shared_ptr<const gz_sensor_msgs::Actuators>
    GzActuatorsMsgPtr;
typedef const boost::shared_ptr<const gz_mav_msgs::RollPitchYawrateThrust>
    GzRollPitchYawrateThrustMsgPtr;
typedef const boost::shared_ptr<const gz_mav_msgs::WindSpeed>
    GzWindSpeedMsgPtr;

// Default values.
static constexpr bool kDefaultIsInputJoystick = false;

// Constants.
static constexpr double kAirDensity = 1.18;
static constexpr double kGravity = 9.81;
static constexpr double kMinAirSpeedThresh = 0.1;

class GazeboKamikazeDynamicsPlugin : public ModelPlugin {
 public:
  /// \brief    Constructor.
  GazeboKamikazeDynamicsPlugin();

  /// \brief    Destructor.
  virtual ~GazeboKamikazeDynamicsPlugin();

 protected:
  /// \brief    Called when the plugin is first created, and after the world
  ///           has been loaded. This function should not be blocking.
  void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf);

  /// \brief  	This gets called by the world update start event.
  void OnUpdate(const common::UpdateInfo&);

  /// \brief	Calculates the forces and moments to be applied to the
  ///           fixed-wing body.
  void UpdateForcesAndMoments();

  /// \brief    Convert control surface input that is normalized in range
  ///           [-1, 1] to a physical deflection angle value.
  double NormalizedInputToAngle(const ControlSurface& surface, double input);

 private:
  /// \brief    Are the input commands coming from a joystick (as opposed to
  ///           a remote control via HIL interface, for example)?
  bool is_input_joystick_;

  /// \brief    Flag that is set to true once CreatePubsAndSubs() is called,
  ///           used to prevent CreatePubsAndSubs() from be called on every
  ///           OnUpdate().
  bool pubs_and_subs_created_;

  /// \brief    Creates all required publishers and subscribers, incl. routing
  ///           of messages to/from ROS if required.
  /// \details  Call this once the first time OnUpdate() is called (can't be
  ///           called from Load() because there is no guarantee
  ///           GazeboRosInterfacePlugin has loaded and listening to
  ///           ConnectGazeboToRosTopic and ConnectRosToGazeboTopic messages).
  void CreatePubsAndSubs();

  /// \brief    Transport namespace.
  std::string namespace_;
  /// \brief    Topic name for actuator commands.
  std::string actuators_sub_topic_;
  /// \brief    Topic name for roll_pitch_yawrate_thrust commands.
  std::string roll_pitch_yawrate_thrust_sub_topic_;
  /// \brief    Topic name for wind speed readings.
  std::string wind_speed_sub_topic_;

  /// \brief    Handle for the Gazebo node.
  transport::NodePtr node_handle_;

  /// \brief    Subscriber for receiving actuator commands.
  gazebo::transport::SubscriberPtr actuators_sub_;
  /// \brief    Subscriber for receiving roll_pitch_yawrate_thrust commands.
  gazebo::transport::SubscriberPtr roll_pitch_yawrate_thrust_sub_;
  /// \brief    Subscriber ror receiving wind speed readings.
  gazebo::transport::SubscriberPtr wind_speed_sub_;

  /// \brief    Pointer to the world.
  physics::WorldPtr world_;
  /// \brief    Pointer to the model.
  physics::ModelPtr model_;
  /// \brief    Pointer to the link.
  physics::LinkPtr link_;
  /// \brief    Pointer to the update event connection.
  event::ConnectionPtr updateConnection_;

  /// \brief    Most current wind speed reading [m/s].
  ignition::math::Vector3d W_wind_speed_W_B_;

  /// \brief    The aerodynamic properties of the aircraft.
  KAMIKAZEAerodynamicParameters aero_params_;

  /// \brief    The physical properties of the aircraft.
  KAMIKAZEVehicleParameters vehicle_params_;

  /// \brief    Left aileron deflection [rad].
  double delta_aileron_left_;
  /// \brief    Right aileron deflection [rad].
  double delta_aileron_right_;
  /// \brief    Elevator deflection [rad].
  double delta_elevator_;
  /// \brief    Flap deflection [rad].
  double delta_flap_;
  /// \brief    Rudder deflection [rad].
  double delta_rudder_;
  /// \brief    Throttle input, in range from 0 to 1.
  double throttle_;

  /// \brief    Processes the actuator commands.
  /// \details  Converts control surface actuator inputs into physical angles
  ///           before storing them and throttle values for later use in
  ///           calculation of forces and moments.
  void ActuatorsCallback(GzActuatorsMsgPtr& actuators_msg);

  /// \brief    Process the roll_pitch_yawrate_thrust commands.
  /// \details  Converts the inputs into physical angles before storing them
  ///           and thrust values for later use in calculation of forces and
  ///           moments.
  void RollPitchYawrateThrustCallback(GzRollPitchYawrateThrustMsgPtr&
                                          roll_pitch_yawrate_thrust_msg);

  /// \brief    Processes the wind speed readings.
  /// \details  Stores the most current wind speed reading for later use in
  ///           calculation of forces and moments.
  void WindSpeedCallback(GzWindSpeedMsgPtr& wind_speed_msg);
};

}  // namespace gazebo

#endif // ROTORS_GAZEBO_PLUGINS_KAMIKAZE_DYNAMICS_PLUGIN_H
