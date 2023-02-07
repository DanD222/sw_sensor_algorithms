/***********************************************************************//**
 * @file		AHRS.h
 * @brief		attitude and heading reference system (interface)
 * @author		Dr. Klaus Schaefer
 * @copyright 		Copyright 2021 Dr. Klaus Schaefer. All rights reserved.
 * @license 		This project is released under the GNU Public License GPL-3.0

    <Larus Flight Sensor Firmware>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************************/

#ifndef AHRS_H_
#define AHRS_H_

#include "embedded_math.h"
#include "quaternion.h"
#include "float3vector.h"
#include "float3matrix.h"
#include "integrator.h"
#include "compass_calibration.h"
#include "HP_LP_fusion.h"
#include "induction_observer.h"

#include "pt2.h"

extern float3vector nav_induction;

enum { ROLL, NICK, YAW};
enum { FRONT, RIGHT, BOTTOM};
enum { NORTH, EAST, DOWN};

typedef enum  { STRAIGHT_FLIGHT, TRANSITION, CIRCLING} circle_state_t;

typedef integrator<float, float3vector> vector3integrator;

//! Attitude and heading reference system class
class AHRS_type
{
public:
	AHRS_type(float sampling_time);
	void attitude_setup( const float3vector & acceleration, const float3vector & induction);

	void update( const float3vector &gyro, const float3vector &acc, const float3vector &mag,
		const float3vector &GNSS_acceleration,
		float GNSS_heading,
		bool GNSS_heading_valid
		);

	inline void set_from_euler( float r, float n, float y)
	{
		attitude.from_euler( r, n, y);
		attitude.get_rotation_matrix( body2nav);
		euler = attitude;
	}
	inline eulerangle<ftype> get_euler(void) const
	{
		return euler;
	}
	inline quaternion<ftype> get_attitude(void) const
	{
		return attitude;
	}
	inline const float3vector &get_nav_acceleration(void) const
	{
		return acceleration_nav_frame;
	}
	inline const float3vector &get_nav_induction(void) const
	{
		return induction_nav_frame;
	}
	inline float get_lin_e0(void) const
	{
		return attitude.lin_e0();
	}
	inline float get_lin_e1(void) const
	{
		return attitude.lin_e1();
	}
	inline float get_e2(void) const
	{
		return attitude.get_e2();
	}
	inline float get_north(void) const
	{
		return attitude.get_north();
	}
	inline float get_east(void) const
	{
		return attitude.get_east();
	}
	inline float get_down(void) const
	{
		return attitude.get_down();
	}
	inline const float3vector get_orientation(void) const
	{
	  float3vector retv;
	  retv[0] = get_north();
	  retv[1] = get_east();
	  retv[2] = get_down();
	  return retv;
	}
	inline const float3vector &get_gyro_correction(void) const
	{
		return gyro_correction;
	}
	inline const float3matrix &get_body2nav( void)
	{
	  return body2nav;
	}
	inline const float3vector &get_nav_correction(void)
	{
	  return nav_correction;
	}
	inline const float3vector &get_gyro_correction(void)
	{
	  return gyro_correction;
	}
	circle_state_t get_circling_state(void) const
	{
	  return circle_state;
	}

  float
  getSlipAngle () const
  {
    return slip_angle_averager.get_output();
  }

  float
  getNickAngle () const
  {
    return nick_angle_averager.get_output();
  }

  float get_turn_rate( void ) const
  {
    return turn_rate_averager.get_output();
  }
  float get_G_load( void ) const
  {
    return G_load_averager.get_output();
  }
  void handle_magnetic_calibration( void) const;

  void update_compass(
		  const float3vector &gyro, const float3vector &acc, const float3vector &mag,
		  const float3vector &GNSS_acceleration); //!< rotate quaternion taking angular rate readings
#if 1 // SOFT_IRON_TEST
  void update_ACC_only(
		  const float3vector &gyro, const float3vector &acc, const float3vector &mag,
		  const float3vector &GNSS_acceleration); //!< rotate quaternion taking angular rate readings
#endif
  float
  getHeadingDifferenceAhrsDgnss () const
  {
    return heading_difference_AHRS_DGNSS;
  }

  float getMagneticDisturbance () const
  {
    return magnetic_disturbance;
  }

private:
  void update_magnetic_loop_gain( void)
  {
    magnetic_control_gain = M_H_GAIN / SQRT( SQR(expected_nav_induction[EAST])+SQR(expected_nav_induction[NORTH]));
  }
  quaternion<ftype>attitude;
  void feed_magnetic_induction_observer(const float3vector &mag_sensor);
  circle_state_t circle_state;
  circle_state_t update_circling_state( void);

  void update_diff_GNSS( const float3vector &gyro, const float3vector &acc, const float3vector &mag,
	  const float3vector &GNSS_acceleration,
	  float GNSS_heading);

  void update_attitude( const float3vector &acc, const float3vector &gyro, const float3vector &mag);

  float3vector nav_correction;
  float3vector gyro_correction;
  float3vector gyro_integrator;
  float3vector acceleration_nav_frame;
  float3vector induction_nav_frame; 	//!< observed NAV induction
  float3vector expected_nav_induction;	//!< expected NAV induction
  float3matrix body2nav;
  eulerangle<ftype> euler;
  float3vector control_body;
  ftype Ts;
  ftype Ts_div_2;
  unsigned circling_counter;
  pt2<float,float> slip_angle_averager;
  pt2<float,float> nick_angle_averager;
  pt2<float,float> turn_rate_averager;
  pt2<float,float> G_load_averager;
  linear_least_square_fit<float> mag_calibrator[3];
  compass_calibration_t compass_calibration;
  induction_observer_t induction_observer;
  float antenna_DOWN_correction;  //!< slave antenna lower / DGNSS base length
  float antenna_RIGHT_correction; //!< slave antenna more right / DGNSS base length
  float heading_difference_AHRS_DGNSS;
  float magnetic_disturbance; //!< abs( observed_induction - expected_induction)
  float magnetic_control_gain; //!< declination-dependent magnetic control loop gain
};

#endif /* AHRS_H_ */
