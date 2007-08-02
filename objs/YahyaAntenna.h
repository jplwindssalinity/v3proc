//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef YAHYAANTENNA_H
#define YAHYAANTENNA_H

static const char rcs_id_yahyaantenna_h[] =
	"@(#) $Id$";

#include <stdio.h>


//======================================================================
// CLASSES
//		YahyaAntenna
//======================================================================

//======================================================================
// CLASS
//		YahyaAntenna
//
// DESCRIPTION
//		The Yahyaantenna object contains antenna information.
//======================================================================

class YahyaAntenna
{
public:

	//--------------//
	// construction //
	//--------------//

	YahyaAntenna();
	~YahyaAntenna();

	//---------------------//
	// setting and getting //
	//---------------------//

	float	GetAlam() { return(_alam); };
	void	SetAlam(float alam) { _alam = alam; };
	float	GetDiam() { return(_diam); };
	void	SetDiam(float diam) { _diam = diam; };
	float	GetFlen() { return(_flen); };
	void	SetFlen(float flen) { _flen = flen; };
	float	GetBw1() { return(_bw1); };
	void	SetBw1(float bw1) { _bw1 = bw1; };
	float	GetBw2() { return(_bw2); };
	void	SetBw2(float bw2) { _bw2 = bw2; };
	float	GetTheta0() { return(_theta0); };
	void	SetTheta0(float theta0) { _theta0 = theta0; };
	float	GetDisp() { return(_disp); };
	void	SetDisp(float disp) { _disp = disp; };
	float	GetBlockage() { return(_blockage); };
	void	SetBlockage(float blockage) { _blockage = blockage; };

protected:

	//-----------//
	// variables //
	//-----------//

        float _alam; 				// wavelength (m) 
        float _diam; 				// diameter (m) 
        float _flen; 				// focal length (m) 
        float _bw1;  				// feed beamwidth (deg) 
        float _bw2;  				// feed beamwidth (deg) 
        float _theta0; 				// feed offset angle (deg) 
        float _disp; 				// feed displacement (m) 
        float _blockage; 			// feed blockage diameter (m) 

};

#endif
