slicesPerSpot=6
spotsPerFrame=80
rec00 = {                               $
        Time:          DOUBLE(0.0),     $
	Instr_Ticks:		0L,     $
	Orbit_Ticks:		0L,     $
	Pri_Orbit_Ticks:    byte(0),    $
        gcAltitude:            0.0,     $
        gcLongitude:           0.0,     $
        gcLatitude:            0.0,     $
        gcX:                   0.0,     $
        gcY:                   0.0,     $
        gcZ:                   0.0,     $
        velX:                  0.0,     $
        velY:                  0.0,     $
        velZ:                  0.0,     $
	Roll:                  0.0,     $
        Pitch:                 0.0,     $
        Yaw:                   0.0,     $
	PtGr:                  0.0,     $
	antennaPosition:       intarr(spotsPerFrame), $
	science:               fltarr(slicesPerSpot,spotsPerFrame), $
        spotNoise:             fltarr(spotsPerFrame) }

        







