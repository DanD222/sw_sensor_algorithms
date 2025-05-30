/***********************************************************************//**
 * @file		atmosphere.h
 * @brief		computes properties of earth's atmosphere
 * @author		Dr. Klaus Schaefer
 * @copyright 		Copyright 2021 Dr. Klaus Schaefer. All rights reserved.
 * 			air density formula developed by Philipp Puetz
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

#ifndef APPLICATION_ATMOSPHERE_H_
#define APPLICATION_ATMOSPHERE_H_

#include "embedded_math.h"
#include <air_density_observer.h>
#include "NAV_tuning_parameters.h"
#include <pt2.h>

#define RECIP_STD_DENSITY_TIMES_2 1.632f

/*! The gas constant of dry air in J/kg/K */
#define GAS_CONST_DRY_AIR	287.058f
/*! The gas constant of water vapor in J/kg/K */
#define GAS_CONST_WATER_VAPOR	461.523f
/*! The ratio of the gas constant of dry air to the gas contant of water vapor */
#define ONE_MINUS_RATIO_GAS_CONSTANTS 0.378f
/*! The offest for the conversion from degree celsius to kelvin */
#define CELSIUS_TO_KELVIN_OFFSET 273.15f

//! Maintenance of atmosphere data like pressure, density etc.
class atmosphere_t
{
public:
  atmosphere_t( float p_abs)
  :
    have_ambient_air_data(false),
    pressure ( p_abs),
    temperature(20.0f),
    humidity( 0.0f),
    density_correction(1.0f),
    extrapolated_sea_level_pressure(101325),
    GNSS_altitude_based_density_available(false),
    GNSS_altitude_based_density(1.2255f),
    old_density_correction(1.0f),
    weight_sum(0.0f),
    density_factor_weighed_sum(0.0f)
  {
  }
  void update_density( float GNSS_altitude, bool valid)
  {
    if( valid)
      {
	GNSS_altitude_based_density = get_std_density( GNSS_altitude) * density_correction;
	GNSS_altitude_based_density_available = true;
      }
    else
      GNSS_altitude_based_density_available = false;
  }
  void initialize( float altitude)
  {
    air_density_observer.initialize(altitude);
  }
  void set_pressure( float p_abs)
  {
    pressure = p_abs;
  }
  float get_pressure( void) const
  {
    return pressure;
  }
  float get_std_density( float GNSS_altitude)
  {
    float std_density =
	0.000000003547494f * GNSS_altitude * GNSS_altitude
	-0.000115412739613f * GNSS_altitude + 1.224096628212817f;
    return std_density;
  }
  float get_pressure_density( float static_pressure)
  {
    return 1.0496346613e-5f * static_pressure + 0.1671546011f;
  }
  float get_density( void) const
  {
    if( GNSS_altitude_based_density_available)
      return GNSS_altitude_based_density;
    else
      return  (1.0496346613e-5f * pressure + 0.1671546011f) * density_correction;
  }
  float get_negative_pressure_altitude( void) const
  {
    float tmp = 8.104381531e-4f * pressure;
    return - tmp * tmp  + 0.20867299170f * pressure - 14421.43945f;
  }
  float get_TAS_from_dynamic_pressure( float dynamic_pressure) const
  {
    return SQRT( 2 * dynamic_pressure / get_density());
  }
  float get_IAS_from_dynamic_pressure( float dynamic_pressure) const
  {
    return SQRT( dynamic_pressure * RECIP_STD_DENSITY_TIMES_2);
  }
  void set_ambient_air_data( float temperature, float humidity)
  {
    this->temperature = temperature;
    this->humidity = humidity;
    have_ambient_air_data = true;
  }
  void disregard_ambient_air_data( void)
  {
    have_ambient_air_data = false;
  }

  float get_extrapolated_sea_level_pressure () const
  {
    return extrapolated_sea_level_pressure;
  }

  void air_density_metering( float pressure, float MSL_altitude)
    {
    air_data_result result = air_density_observer.feed_metering( pressure, MSL_altitude);
      if( result.valid)
	{
#if USE_AIR_DENSITY_LETHARGY
	  bool first_measurement = (weight_sum == 0.0f);

	  weight_sum =  weight_sum * AIR_DENSITY_LETHARGY + (AIR_DENSITY_LETHARGY -1) / result.density_variance;
	  density_factor_weighed_sum =  density_factor_weighed_sum * AIR_DENSITY_LETHARGY + (AIR_DENSITY_LETHARGY -1) * result.density_correction / result.density_variance;

	  // postpone update unless we have two measurements
	  if( ! first_measurement)
	    density_correction = density_factor_weighed_sum / weight_sum;
#else
	  density_correction = result.density_correction;
#endif
	  // todo patch: emergency-brake for implausible values
	  if( density_correction > 1.15 || density_correction < 0.85)
	    density_correction = ONE;
	}
    }

private:
  float calculateGasConstantHumAir(
      float humidity, float pressure, float temperature);
  float calculateAirDensity(
      float humidity, float pressure, float temperature);
  float calculateSaturationVaporPressure(float temp);
  bool have_ambient_air_data;
  float pressure;
  float temperature;
  float humidity;
  float density_correction;
  float extrapolated_sea_level_pressure;
  air_density_observer_t air_density_observer;
  bool GNSS_altitude_based_density_available;
  float GNSS_altitude_based_density;
  float old_density_correction;
  float weight_sum;
  float density_factor_weighed_sum;
};

#endif /* APPLICATION_ATMOSPHERE_H_ */
