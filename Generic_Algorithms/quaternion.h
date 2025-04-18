/***********************************************************************//**
 * @file		quaternion.h
 * @brief		quaternion for 3d attitude representation (template)
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

#ifndef QUATERNION_H
#define QUATERNION_H

#include "embedded_math.h"
#include "vector.h"
#include "float3matrix.h"
#include "float3vector.h"
#include "euler.h"

//! maintain attitude and provide coordinate transformation services
template <class datatype > class quaternion: public vector <datatype, 4>
{
public:
	//! constructor from euler angle
	quaternion( vector <datatype, 3> &init)
	{
		from_euler( init[0], init[1], init[2]);
	}

	//! constructor from datatype[4]
	quaternion( )
	: vector <datatype, 4>({0})
	  {
	    vector <datatype, 4>::e[0]=1;
	  };

	//! normalize quaternion absolute value to ONE
	inline void normalize(void)
	{
	  datatype tmp = this->e[0] * this->e[0]
		       + this->e[1] * this->e[1]
		       + this->e[2] * this->e[2]
		       + this->e[3] * this->e[3] ;
	  tmp = ONE / tmp;
	  tmp = SQRT( tmp);
	  for( int i = 0; i < 4; ++i)
		  this->e[i] *= tmp;
	};

	//! quaternion -> euler angle transformation
	operator eulerangle <datatype> () const
	{
		datatype e0 = this->e[0];
		datatype e1 = this->e[1];
		datatype e2 = this->e[2];
		datatype e3 = this->e[3];

		eulerangle <datatype> _euler;

		//! formula from roenbaeck p34
		_euler.roll  = ATAN2(  TWO * (e0*e1 + e2*e3) , e0*e0 - e1*e1 - e2*e2 + e3*e3 );
		_euler.pitch = ASIN(   TWO * (e0*e2 - e3*e1));
		_euler.yaw   = ATAN2(  TWO * (e0*e3 + e1*e2) , e0*e0 + e1*e1 - e2*e2 - e3*e3 );
		return _euler;
	}

	//! quaternion chaining, multiplication
	quaternion<datatype> operator *( const quaternion<datatype> right) const
	{
	  quaternion<datatype> result;
	  datatype w1 = vector<datatype, 4>::e[0];
	  datatype x1 = vector<datatype, 4>::e[1];
	  datatype y1 = vector<datatype, 4>::e[2];
	  datatype z1 = vector<datatype, 4>::e[3];

	  datatype w2 = right[0];
	  datatype x2 = right[1];
	  datatype y2 = right[2];
	  datatype z2 = right[3];

	  result[0] = w1*w2 - x1*x2 - y1*y2 - z1*z2;
	  result[1] = w1*x2 + x1*w2 + y1*z2 - z1*y2;
	  result[2] = w1*y2 - x1*z2 + y1*w2 + z1*x2;
	  result[3] = w1*z2 + x1*y2 - y1*x2 + z1*w2;

	  return result;
	}

	//! get north component of attitude
	inline datatype get_north( void) const
	{
		datatype e0 = this->e[0];
		datatype e1 = this->e[1];
		datatype e2 = this->e[2];
		datatype e3 = this->e[3];
		return e0*e0 + e1*e1 - e2*e2 - e3*e3;
	}

	//! get east component of attitude
	inline datatype get_east( void) const
	{
		datatype e0 = this->e[0];
		datatype e1 = this->e[1];
		datatype e2 = this->e[2];
		datatype e3 = this->e[3];
		return TWO * (e0*e3 + e1*e2);
	}

	//! get down component of attitude
	inline datatype get_down( void) const
	{
		datatype e0 = this->e[0];
		datatype e1 = this->e[1];
		datatype e2 = this->e[2];
		datatype e3 = this->e[3];
		return TWO * (e1*e3-e0*e2);
	}

	//! get heading component of attitude
	datatype get_heading( void) const
	{
		datatype e0 = vector<datatype, 4>::e[0];
		datatype e1 = vector<datatype, 4>::e[1];
		datatype e2 = vector<datatype, 4>::e[2];
		datatype e3 = vector<datatype, 4>::e[3];

		return ATAN2(  TWO * (e0*e3 + e1*e2) , e0*e0 + e1*e1 - e2*e2 - e3*e3 );
	}

	//! quaternion update using rotation vector
	void rotate( datatype p, datatype q, datatype r)
	{
		datatype e0 = this->e[0];
		datatype e1 = this->e[1];
		datatype e2 = this->e[2];
		datatype e3 = this->e[3];

		//! R.Rogers formula 2.92
		this->e[0] += (-e1*p -e2*q -e3*r);
		this->e[1] += ( e0*p +e2*r -e3*q);
		this->e[2] += ( e0*q -e1*r +e3*p);
		this->e[3] += ( e0*r +e1*q -e2*p);

		normalize();
	}

	//! euler angle -> quaternion transformation
	void from_euler( datatype p, datatype q, datatype r)
	{
	// 3-2-1: yaw, then pitch, then roll
		p *= HALF; q *= HALF; r *= HALF;
		datatype sinphi   = SIN( p);
		datatype cosphi   = COS( p);
		datatype sintheta = SIN( q);
		datatype costheta = COS( q);
		datatype sinpsi   = SIN( r);
		datatype cospsi   = COS( r);

		this->e[0] = cosphi*costheta*cospsi + sinphi*sintheta*sinpsi;
		this->e[1] = sinphi*costheta*cospsi - cosphi*sintheta*sinpsi;
		this->e[2] = cosphi*sintheta*cospsi + sinphi*costheta*sinpsi;
		this->e[3] = cosphi*costheta*sinpsi - sinphi*sintheta*cospsi;
	}

	//! quaternion -> rotation matrix transformation
	void get_rotation_matrix (matrix<datatype, 3> &m) const
	{
		//! R.Rogers formula 2.90
		datatype e0=this->e[0];
		datatype e1=this->e[1];
		datatype e2=this->e[2];
		datatype e3=this->e[3];

	    m.e[0][0] = TWO * (e0*e0+e1*e1) - ONE;
	    m.e[0][1] = TWO * (e1*e2-e0*e3);
	    m.e[0][2] = TWO * (e1*e3+e0*e2);

	    m.e[1][0] = TWO * (e1*e2+e0*e3);
	    m.e[1][1] = TWO * (e0*e0+e2*e2) - ONE;
	    m.e[1][2] = TWO * (e2*e3-e0*e1);

	    m.e[2][0] = TWO * (e1*e3-e0*e2);
	    m.e[2][1] = TWO * (e2*e3+e0*e1);
	    m.e[2][2] = TWO * (e0*e0+e3*e3) - ONE;
	}
	quaternion <datatype> operator * ( const quaternion <datatype> & right) const //!< quaternion multiplication
		{
		quaternion <datatype> result;
		datatype e0=this->e[0];
		datatype e1=this->e[1];
		datatype e2=this->e[2];
		datatype e3=this->e[3];
		datatype re0=right.e[0];
		datatype re1=right.e[1];
		datatype re2=right.e[2];
		datatype re3=right.e[3];
		result.e[0] = e0 * re0 - e1 * re1 - e2 * re2 + e3 * re3;
		result.e[1] = e1 * re0 + e2 * re3 - e3 * re2 + e0 * re1;
		result.e[2] = e3 * re1 + e0 * re2 - e1 * re3 + e2 * re0;
		result.e[3] = e1 * re2 - e2 * re1 + e0 * re3 + e3 * re0;
		return result;
		}

	void from_rotation_matrix( matrix<datatype, 3> &rotm) //!< rotation matrix -> quaternion transformation
		{
		float tmp;
		tmp = ONE + rotm.e[0][0] + rotm.e[1][1] + rotm.e[2][2];
		//! formula from roenbaeck p35
		tmp = SQRT( tmp);
		tmp *= HALF;
		this->e[0] = tmp;
		tmp = QUARTER / tmp;
		this->e[1] = tmp * (rotm.e[2][1] - rotm.e[1][2]);
		this->e[2] = tmp * (rotm.e[0][2] - rotm.e[2][0]);
		this->e[3] = tmp * (rotm.e[1][0] - rotm.e[0][1]);
		normalize(); // compensate computational inaccuracies
		};
};
#endif // QUATERNION_H
