pro l00_view, datafile

close,1




@l00def		; get record structure defined



LOOP:
	read,'Frame number: ',n
	if (n le 0) then goto,STOP 
	rec=rec00
        openr,unit,datafile,/get_lun
        for rr=0,n-1 do begin
        	readu,unit,rec
	endfor
        free_lun, unit

        Time = rec.Time
        Instr_Ticks = rec.Instr_Ticks
        Orbit_Ticks = rec.Orbit_Ticks
        Pri_Orbit_Ticks= rec.Pri_Orbit_Ticks
   
;	==================================================================================
	print
	print,'Time:',Time, ' Instr_Ticks:',Instr_Ticks,' Orbit_Ticks:',Orbit_Ticks
;	==================================================================================
;
;	==================================================================================
	print
	print,'          Alt        Lat          Lon'
	print,'---------------------------------------------------------------------------'
        print,rec.gcAltitude,rec.gcLatitude*180/!pi,rec.gcLongitude*180/!pi
	print,'---------------------------------------------------------------------------'
	print
        print,'          Roll       Pitch       Yaw       PtGr'
	print,'---------------------------------------------------------------------------'
	print,rec.Roll*180/!pi, rec.Pitch*180/!pi, rec.Yaw*180/!pi, rec.PtGr
	print,'---------------------------------------------------------------------------'
	print

;	==================================================================================

SPOT_LOOP:
		nSPOT = 0
		read,'Spot number to view: ',nSPOT
		if (nSPOT eq 0) then goto,LOOP
		if (nSPOT gt spotsPerFrame) then goto,SPOT_LOOP
	
		ii = nSPOT - 1
	
;	==================================================================================
	print
	print,'      SPOT# ',nSPOT,' AntennaPosition= ',rec.antennaPosition(ii),'   spotNoise= ',rec.spotNoise(ii)
	print
	print,'      Slice#    P(s+n)'
	print,'--------------------------------------------------------------------------'

	for jj = 0,slicesPerSpot-1 do begin
		print, jj, rec.science(jj,ii)
	endfor
	print,'--------------------------------------------------------------------------'
	print
;	==================================================================================

		goto,SPOT_LOOP

	goto,LOOP


STOP:	close,1
	end
