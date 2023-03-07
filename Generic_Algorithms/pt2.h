/***********************************************************************//**
 * @file		pt2.h
 * @brief		tunable second order IIR lowpass filter (butterworth)
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

#ifndef PT2_H_
#define PT2_H_

#include <ringbuffer.h>
#include "embedded_math.h"

// butterworth filter prototype parameters at Fcutoff/Fsampling = 0.25
// B coefficients -> nominator
// A coefficients -> DE-nominator, A0 = 1
#define B0 0.292893218813452
#define B1 0.585786437626905
#define B2 0.292893218813452
#define A1 ZERO
#define A2 0.171572875253810
#define DESIGN_FREQUENCY 0.25

//! Second order IIR filter
template <class datatype, class basetype> class pt2
{
public:
	pt2( basetype fcutoff) //! constructor taking Fc/Fs
	: input( datatype()),
	  output( datatype()),
	  old( datatype()),
	  very_old( datatype())
	{
		basetype delta = SIN( M_PI * (DESIGN_FREQUENCY - fcutoff)) / SIN( M_PI * (fcutoff + DESIGN_FREQUENCY));
		basetype a0x = A2 * SQR(delta) - A1 + ONE;
		basetype a1x = -2.0 * delta * A2 + (SQR(delta) + ONE) * A1 - 2.0 * delta;
		basetype a2x = A2 - delta * A1 + SQR(delta);

		basetype b0x = B2 * SQR( delta) - B1 * delta + B0;
		basetype b1x = - 2.0 * delta * B2 + (SQR(delta) + 1) * B1 - 2.0 * delta * B0;
		basetype b2x = B2 - delta * B1 + SQR( delta) * B0;

		// normalize denominator a0 = ONE
		a1 = a1x / a0x;
		a2 = a2x / a0x;
		b0 = b0x / a0x;
		b1 = b1x / a0x;
		b2 = b2x / a0x;

		// fine-tune DC-gain = 1.0
		delta = (ONE + a1 + a2) / (b0 + b1 + b2);
		b0 *= delta;
		b1 *= delta;
		b2 *= delta;
	}
	void settle( const datatype &present_input)
	{
		basetype tuning = ONE  / ( ONE + a1 + a2);
		very_old = old = present_input * tuning;
		input = output = present_input;
	}
	datatype respond( const datatype &input)
	{
		this->input=input;;
		datatype x = input - old * a1 - very_old * a2;
		output = x * b0 + old * b1 + very_old * b2;
		very_old = old;
		old = x;
		return output;
	}
	datatype get_output( void) const
	{
	  return output;
	}
	datatype get_last_input( void) const
	{
	  return input;
	}
private:
	datatype input;
	datatype output;
	datatype old;
	datatype very_old;
	basetype b0, b1, b2, a1, a2;    //!< z-transformed transfer-function (b=nominator)
};

#endif /* PT2_H_ */
