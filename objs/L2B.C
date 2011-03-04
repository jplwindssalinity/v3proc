//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l2b_c[] =
    "@(#) $Id$";

#include <memory.h>
#include <string.h>
#include <hdf.h>
#include <mfhdf.h>
#include "L2B.h"
#include "TlmHdfFile.h"
#include "NoTimeTlmFile.h"
#include "ParTab.h"
#include "L1AExtract.h"
#include "WindVector.h"
#include "Wind.h"
#include "Misc.h"
#include "HdfHelp.h"
#include "Sds.h"

#include "GSparameters.h"

#define HDF_ACROSS_BIN_NO    76
#define HDF_NUM_AMBIGUITIES  4

//===========//
// L2BHeader //
//===========//

L2BHeader::L2BHeader()
:   crossTrackResolution(0.0), alongTrackResolution(0.0), zeroIndex(0),
    inclination(0.0)
{
    return;
}

L2BHeader::~L2BHeader()
{
    return;
}

//-----------------//
// L2BHeader::Read //
//-----------------//

int
L2BHeader::Read(
    FILE*  fp)
{
	//---start of mods by AGF to add version IDs to L2B header (6/3/2010)
    fpos_t pos;
    char   l2b_ascii_header[80];
    
    // get file position
    if( fgetpos(fp,&pos) != 0 ) 
    	return(0);    
    // Attempt to read ASCII header with version ID (if present).
    if( fread((void *)&l2b_ascii_header[0], sizeof(char), 80, fp) != 80) 
    	return(0);
    
    if( strncmp( l2b_ascii_header, "QSCATSIM_L2B_VERSION_ID", 23 ) == 0 )
    {
    	//printf( "In L2BHeader::Read: ASCII Header: %s\n", l2b_ascii_header );
        char* str_version_id       = strstr( l2b_ascii_header, "==" ) + 2;
        char* str_version_id_minor = strstr( str_version_id, "." )    + 1;
    
        version_id_major = int( floor( atof( str_version_id ) ) );
        version_id_minor = atoi( str_version_id_minor );
    }
    else
    {
    	//printf("In L2BHeader::Read: No ASCII Header found\n");
    	version_id_major = 1;
    	version_id_minor = 0;
    	if( fsetpos(fp,&pos) !=0 ) return(0);
    }
    printf("In L2BHeader::Read, L2B version ID: %d.%d\n",
           version_id_major, version_id_minor);
    //---end of AGF 6/3/2010 mods.
    
    if (fread(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&zeroIndex, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//------------------//
// L2BHeader::Write //
//------------------//

int
L2BHeader::Write(
    FILE*  fp)
{
    char l2b_ascii_header[80] = "QSCATSIM_L2B_VERSION_ID==4.0";
    
    if (fwrite((void *)&l2b_ascii_header[0], sizeof(char), 80, fp) != 80 ||
        fwrite(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&zeroIndex, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//-----------------------//
// L2BHeader::WriteAscii //
//-----------------------//

int
L2BHeader::WriteAscii(
    FILE*  fp)
{
    fprintf(fp, "############################################\n");
    fprintf(fp, "##                L2B DataFile            ##\n");
    fprintf(fp, "############################################\n");
    fprintf(fp, "L2B VERSION ID == %d.%d\n", version_id_major, version_id_minor );
    fprintf(fp,"\n\nCrossTrackRes %g AlongTrackRes %g ZeroIndex %d\n\n",
    crossTrackResolution,alongTrackResolution,zeroIndex);
    return(1);
}

//==========//
// L2BFrame //
//==========//

L2BFrame::L2BFrame()
{
    return;
}

L2BFrame::~L2BFrame()
{
    return;
}

//=====//
// L2B //
//=====//

L2B::L2B()
:   _status(OK)
{
    return;
}

L2B::~L2B()
{
    return;
}

//-----------------//
// L2B::ReadHeader //
//-----------------//
int L2B::ReadHeader()
{
  // AGF moved this method from L2B.h to L2B.C so I can also copy
  // over the version ids from the header to the swath structures.
  // 6/3/2010
  if( !header.Read(_inputFp) ) return(0);
  frame.swath.version_id_major = header.version_id_major;
  frame.swath.version_id_minor = header.version_id_minor;
  return (1);
}

//----------------//
// L2B::SmartRead //
//----------------//

int
L2B::SmartRead(
    const char*  filename,
    int          unnormalize_mle_flag)
{
    // see if the file is HDF
    if (HdfHelp::IsHdfFile(filename)) {
        return(ReadPureHdf(filename, unnormalize_mle_flag));
    } else {
        return(Read(filename));
    }
    return(0);
}

//-----------//
// L2B::Read //
//-----------//

int
L2B::Read(
    const char*  filename)
{
    SetInputFilename(filename);
    if (! OpenForReading())
    {
        fprintf(stderr, "L2B::Read: error opening file %s for reading\n",
            filename);
        return(0);
    }
    if (! ReadHeader())
    {
        fprintf(stderr, "L2B::Read: error reading header from file %s\n",
            filename);
        return(0);
    }
    
    if (! ReadDataRec())
    {
        fprintf(stderr, "L2B::Read: error reading data record from file %s\n",
            filename);
        return(0);
    }
    return(1);
}

//------------------//
// L2B::ReadRETDAT //
//------------------//
int L2B::ReadRETDAT(
    const char*  filename,
    int read_nudge_vectors_flag)
{
  FILE* fid;

  int atibins;
  int ctibins;

  float* lat_arr;
  float* lon_arr;
  float* sel_speed;
  float* sel_dir;
  float* true_speed;
  float* true_dir;
  float* model_speed;
  float* model_dir;
	
  int* num_ambigs;
  int* wvc_selection;
  unsigned int* retdat_flags;
	
  float* amb_obj;
  float* amb_speed;
  float* amb_dir; 
  float* liquid;
  
  float* amb_speed_ridge_amp;
  float* amb_speed_ridge_phase;	
  
  float* amb_speed_ridge_azi_left;
  float* amb_speed_ridge_azi_right;

  fid = fopen( filename, "r" );
  if( fid == NULL ) {
      fprintf(stderr, "L2B::ReadRETDAT Unable to open file %s\n",filename);
	  return (0);
  }
	  
  if( fread( &atibins, sizeof(int), 1, fid ) != 1 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading atibins from %s\n",filename);  
  if( fread( &ctibins, sizeof(int), 1, fid ) != 1 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading ctibins from %s\n",filename);   
      
  printf("L2B::ReadRETDAT atibins: %d ctibins: %d\n",atibins,ctibins);
  
  unsigned int num_wvc = ctibins*atibins;
  
  int n_spec_fit_terms;
  
  lat_arr       = new float[atibins*ctibins];
  lon_arr       = new float[atibins*ctibins];
  sel_speed     = new float[atibins*ctibins];
  sel_dir       = new float[atibins*ctibins];
  model_speed   = new float[atibins*ctibins];
  model_dir     = new float[atibins*ctibins];
  true_speed    = new float[atibins*ctibins];
  true_dir      = new float[atibins*ctibins];
  liquid        = new float[atibins*ctibins];
  wvc_selection = new   int[atibins*ctibins];
  num_ambigs    = new   int[atibins*ctibins];
	
  retdat_flags  = new unsigned int[atibins*ctibins];
	
  amb_obj       = new float[atibins*ctibins*4];
  amb_dir       = new float[atibins*ctibins*4];
  amb_speed     = new float[atibins*ctibins*4];  

  amb_speed_ridge_azi_left  = new float[atibins*ctibins*4];
  amb_speed_ridge_azi_right = new float[atibins*ctibins*4];

  if( fread( lat_arr, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading lat_arr from %s\n",filename);
      
  if( fread( lon_arr, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading lon_arr from %s\n",filename);

  if( fread( sel_speed, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading sel_speed from %s\n",filename);

  if( fread( sel_dir, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading sel_dir from %s\n",filename);

  if( fread( true_speed, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading true_speed from %s\n",filename);

  if( fread( true_dir, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading true_dir from %s\n",filename);    

  if( fread( model_speed, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading model_speed from %s\n",filename);

  if( fread( model_dir, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading model_dir from %s\n",filename);      

  if( fread( liquid, sizeof(float), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading liquid from %s\n",filename);      

  if( fread( retdat_flags, sizeof(unsigned int), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading retdat_flags from %s\n",filename);     
 
  if( fread( wvc_selection, sizeof(int), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading wvc_selection from %s\n",filename);     

  if( fread( num_ambigs, sizeof(int), num_wvc, fid ) != num_wvc )
      fprintf(stderr, "L2B::ReadRETDAT Error reading num_ambigs from %s\n",filename);     

  if( fread( amb_obj, sizeof(float), num_wvc*4, fid ) != num_wvc*4 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading amb_obj from %s\n",filename);     

  if( fread( amb_speed, sizeof(float), num_wvc*4, fid ) != num_wvc*4 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading amb_speed from %s\n",filename);     
      
  if( fread( amb_dir, sizeof(float), num_wvc*4, fid ) != num_wvc*4 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading amb_dir from %s\n",filename);     
  
  if( fread( amb_speed_ridge_azi_left, sizeof(float), num_wvc*4, fid ) != num_wvc*4 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading amb_speed_ridge_azi_left from %s\n",filename);     
      
  if( fread( amb_speed_ridge_azi_right, sizeof(float), num_wvc*4, fid ) != num_wvc*4 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading amb_speed_ridge_azi_right from %s\n",filename);     
  
  // Read n_spec_fit_terms...
  if( fread( &n_spec_fit_terms, sizeof(float), 1, fid ) != 1 )
      fprintf(stderr, "L2B::ReadRETDAT Error reading n_spec_fit_terms from %s\n",filename);     

  amb_speed_ridge_amp   = new float[atibins*ctibins*n_spec_fit_terms];
  amb_speed_ridge_phase = new float[atibins*ctibins*n_spec_fit_terms];  

  // Read amb_speed_ridge_amp
  if( fread( amb_speed_ridge_amp, sizeof(float), num_wvc*n_spec_fit_terms, fid ) 
        != num_wvc*n_spec_fit_terms )
      fprintf(stderr, "L2B::ReadRETDAT Error reading amb_speed_ridge_amp from %s\n", filename);
      
  // Read amb_speed_ridge_phase
  if( fread( amb_speed_ridge_phase, sizeof(float), num_wvc*n_spec_fit_terms, fid ) 
        != num_wvc*n_spec_fit_terms )
      fprintf(stderr, "L2B::ReadRETDAT Error reading amb_speed_ridge_phase from %s\n", filename);
  
  fclose(fid);

  // prepare
  frame.swath.DeleteEntireSwath();

  // allocate
  if( !frame.swath.Allocate(ctibins, atibins) ) return(0);
  
  for( int cti = 0; cti < ctibins; ++cti )
  {
    for( int ati = 0; ati < atibins; ++ati )
    {
      unsigned int flat_index = ati+cti*atibins;
      
      if( num_ambigs[flat_index] == 0 ) 
        continue; // no ambiguities => no winds retrived
      
      // Create the WVC
      WVC* wvc = new WVC();
      
      if ( wvc == NULL )
      {
        fprintf(stderr, "L2B::ReadRETDAT: error allocating\n");
        return(0);
      }
      
      // Set lon,lat for WVC
      wvc->lonLat.longitude = lon_arr[flat_index] * dtr;
      wvc->lonLat.latitude  = lat_arr[flat_index] * dtr;
      
      // Set Nudge Vectors for WVC
      if( read_nudge_vectors_flag )      
      {
        wvc->nudgeWV = new WindVectorPlus();
        if ( wvc->nudgeWV == NULL )
        {
          fprintf(stderr, "L2B::ReadRETDAT: error allocating\n");
          return(0);
        }        
        wvc->nudgeWV->SetSpdDir( model_speed[flat_index], 
                               gs_deg_to_pe_rad( model_dir[flat_index] ) );
      }
      
      // Loop over the number of ambiguities and attach them to the 
      // WVC.
      for( int i_amb = 0; i_amb < num_ambigs[flat_index]; ++i_amb )
      {
        WindVectorPlus* wvp = new WindVectorPlus();
        if (wvp == NULL)
        {
          fprintf(stderr, "L2B::ReadRETDAT: error allocating\n");
          return(0);
        }
        
        wvp->SetSpdDir( amb_speed[4*flat_index+i_amb], 
                        gs_deg_to_pe_rad( amb_dir[4*flat_index+i_amb] ) );
        
        wvp->obj = amb_obj[4*flat_index+i_amb];
        
        if (! wvc->ambiguities.Append(wvp))
        {
          fprintf(stderr, "L2B::ReadRETDAT: error appending\n");
          return(0);
        }        
      }  // end i_amb loop
      
      // Set the selected wind vector in the WVC
      wvc->selected = wvc->ambiguities.GetByIndex( wvc_selection[flat_index]-1 );
      wvc->selected_allocated = 0;

      WindVector* wv = new WindVector();        
      if (wv == NULL)
      {
        fprintf(stderr, "L2B::ReadRETDAT: error allocating\n");
        return(0);
      }
      wv->SetSpdDir( sel_speed[flat_index], gs_deg_to_pe_rad( sel_dir[flat_index] ) );        
      wvc->specialVector = wv;
      
      // Set the rainFlagBits and landiceFlagBigs.
      wvc->rainProb        = 0; // ? what to do here??
      wvc->rainFlagBits    = (char)((0x7000 & retdat_flags[flat_index]) >> 12);
      wvc->landiceFlagBits = (char)((0x0180 & retdat_flags[flat_index]) >> 7 );
      
      // Add speed ridge left and right azimuth bounds for each ambiguity.
      for( int i_amb = 0; i_amb < 4; ++i_amb )
      {
        AngleInterval* alist = new AngleInterval;
        alist->SetLeftRight( amb_speed_ridge_azi_left[4*flat_index+i_amb],
                            amb_speed_ridge_azi_right[4*flat_index+i_amb] );

        wvc->directionRanges.Append( alist );
      }
      
      int   n_azi = 360.0 / WIND_DIR_INTV_INIT; // From GSparameters.h include...
        
      if( wvc->directionRanges.bestSpd != NULL || wvc->directionRanges.bestObj != NULL )
      {
        printf("L2B::ReadRETDAT: wvc->directionRanges.bestSpd != NULL\n");
        exit(1);
      }
      wvc->directionRanges.dirIdx.SpecifyWrappedCenters(0, two_pi, n_azi);
      wvc->directionRanges.bestSpd = (float*)malloc(sizeof(float)*n_azi);
      wvc->directionRanges.bestObj = (float*)malloc(sizeof(float)*n_azi);

      // amb_speed_ridge_amp[n_spec_fit_terms*flat_index + 0] should be less
      // than zero if wind speed ridge info was not available.
      // Also, if n_spec_fit_terms == 0 then skip this stuff.

      if( amb_speed_ridge_amp[n_spec_fit_terms*flat_index] >= 0 && 
          n_spec_fit_terms > 0 )
      {
        float wind_speed[n_azi];
  
        for( int i_azi = 0; i_azi < n_azi; ++i_azi )
	    {
          float azimuth = i_azi * WIND_DIR_INTV_INIT;
          wind_speed[i_azi] = 0;
          
          for( int i_term = 0; i_term < n_spec_fit_terms; ++i_term )
	      {
            float phase = amb_speed_ridge_phase[n_spec_fit_terms*flat_index +i_term ];
            float amp   = amb_speed_ridge_amp  [n_spec_fit_terms*flat_index +i_term ];
            wind_speed[i_azi] += amp * cos( azimuth + phase );
          }  // end i_term loop
          
          wvc->directionRanges.bestSpd[i_azi] = wind_speed[i_azi];
          wvc->directionRanges.bestObj[i_azi] = -1;
          
        }    // end i_azi loop
      }      // end conditional on reading speed ridge fits
      
      // Add this WVC to the swath.
      if ( !frame.swath.Add(cti, ati, wvc) )
      {
	    fprintf(stderr, "L2B::ReadRETDAT: error adding WVC\n");
        return(0);
      }
      
    }  // end ati loop
  }    // end cti loop
  
  // If we were asked to read them, set this flag...
  // I don't check that the bytes I read are sensible....
  if( read_nudge_vectors_flag ) frame.swath.nudgeVectorsRead = 1;
  
  // Free up the stuff I allocated...
  delete[] lat_arr;
  delete[] lon_arr;
  delete[] sel_speed;
  delete[] sel_dir;
  delete[] model_speed;
  delete[] model_dir;
  delete[] true_speed;
  delete[] true_dir;
  delete[] liquid;
  delete[] wvc_selection;
  delete[] num_ambigs;
  delete[] retdat_flags;
  delete[] amb_obj;
  delete[] amb_speed;
  delete[] amb_dir;
  delete[] amb_speed_ridge_amp;
  delete[] amb_speed_ridge_phase;
  delete[] amb_speed_ridge_azi_left;
  delete[] amb_speed_ridge_azi_right;
  
  return 1;
}


//------------------//
// L2B::WriteRETDAT //
//------------------//
int L2B::WriteRETDAT(
    const char*  filename,
    int write_speed_ridges_flag )
{
	FILE* fid;
	int atibins = frame.swath.GetAlongTrackBins();
	int ctibins = frame.swath.GetCrossTrackBins();

	float pi=3.141592653589793;
	
    int n_spec_fit_terms = 9; // # of terms in Fourier series to keep.
                              // see function specfit in objs/Misc.C

	if( write_speed_ridges_flag == 0 ) n_spec_fit_terms = 0;
		
    
	float* lat_arr;
	float* lon_arr;
	float* sel_speed;
	float* sel_dir;
	float* true_speed;
	float* true_dir;
	float* model_speed;
	float* model_dir;
	
	int* num_ambigs;
	int* wvc_selection;
	unsigned int* retdat_flags;
	
	float* amb_obj;
	float* amb_dir;
	float* amb_speed;
	float* liquid;
	
	float* amb_speed_ridge_azi_left;
	float* amb_speed_ridge_azi_right;
	float* amb_speed_ridge_amp;
	float* amb_speed_ridge_phase;	
	
	
	printf("\n");
	printf("L2B::WriteRETDAT: atibins: %d ctibins: %d\n",atibins,ctibins);
	
	lat_arr       = new float[atibins*ctibins];
	lon_arr       = new float[atibins*ctibins];
	sel_speed     = new float[atibins*ctibins];
	sel_dir       = new float[atibins*ctibins];
	model_speed   = new float[atibins*ctibins];
	model_dir     = new float[atibins*ctibins];
	true_speed    = new float[atibins*ctibins];
	true_dir      = new float[atibins*ctibins];
	liquid        = new float[atibins*ctibins];
	wvc_selection = new   int[atibins*ctibins];
	num_ambigs    = new   int[atibins*ctibins];
	
	retdat_flags  = new unsigned int[atibins*ctibins];
	
	amb_obj       = new float[atibins*ctibins*4];
	amb_dir       = new float[atibins*ctibins*4];
	amb_speed     = new float[atibins*ctibins*4];

	amb_speed_ridge_azi_left  = new float[atibins*ctibins*4];
	amb_speed_ridge_azi_right = new float[atibins*ctibins*4];
	amb_speed_ridge_amp       = new float[atibins*ctibins*n_spec_fit_terms];
	amb_speed_ridge_phase     = new float[atibins*ctibins*n_spec_fit_terms];
	
	
	// Loop over swath and populate the output file buffers.
	
    for ( int cti = 0; cti < ctibins; ++cti )
	{	  
  	  for( int ati = 0; ati < atibins; ++ati )
	  {
	    unsigned int flat_index = ati+cti*atibins;
	    
	    WVC* wvc = frame.swath.swath[cti][ati];
	    
	    if( wvc == NULL ) continue;  
	    
	    // Set (lon,lat) buffers
        lat_arr[flat_index]      = wvc->lonLat.latitude  * 180 / pi;
        lon_arr[flat_index]      = wvc->lonLat.longitude * 180 / pi;
        
        if( wvc->selected != NULL )
        {
          sel_speed[flat_index]    = wvc->selected->spd;
          sel_dir[flat_index]      = pe_rad_to_gs_deg( wvc->selected->dir );
        }
        else
          fprintf(stderr,"L2B::WriteRETDAT: wvc->selected == NULL\n");
        
        if( wvc->nudgeWV != NULL )
        {
          model_speed[flat_index]  = wvc->nudgeWV->spd;
          model_dir[flat_index]    = pe_rad_to_gs_deg( wvc->nudgeWV->dir );
        }
        else
        {
          //fprintf(stderr,"L2B::WriteRETDAT: wvc->nudgeWV == NULL\n");
          model_speed[flat_index]  = -999;
          model_dir[flat_index]    = 0;
        } 
        // Placeholder bytes for now...
        liquid[flat_index]       = 0;
        retdat_flags[flat_index] = (unsigned int)(0);
        true_speed[flat_index]   = 0;
        true_dir[flat_index]     = 0;
        
        // Pass through the rainFlagBits and landiceFlagBits to the RETDAT file.
        // least-significant 16 bits then have a similar bit pattern to 
        // wvc_quality_flag in the HDF files...
        retdat_flags[flat_index] = (unsigned int)( wvc->rainFlagBits    << 12 )
                                 + (unsigned int)( wvc->landiceFlagBits << 7  );
        
        // Initialize to zeroes.
        int i_amb   = 0;
        for ( i_amb = 0; i_amb < 4; ++i_amb )
        {
          amb_obj[4*flat_index+i_amb]   = 0;
          amb_speed[4*flat_index+i_amb] = 0;
          amb_dir[4*flat_index+i_amb]   = 0;
        }
        
        // Loop over the ambiguities of each WVC
        i_amb = 0;
        for ( WindVectorPlus* wvp = wvc->ambiguities.GetHead();
                      wvp;    wvp = wvc->ambiguities.GetNext()  )
        {
           // Set the ambigity obj function, speed, and direction.
           amb_obj  [4*flat_index+i_amb] = wvp->obj;
           amb_speed[4*flat_index+i_amb] = wvp->spd;
           amb_dir  [4*flat_index+i_amb] = pe_rad_to_gs_deg( wvp->dir );
           
           ++i_amb;
           
           // Populate the wvc_selection array.
           if( wvc->selected == wvp ) wvc_selection[flat_index] = i_amb;
        }
        
        // Populate the num_ambigs array
        num_ambigs[flat_index] = i_amb;
        
        // Initialize the amb_speed_ridge arrays to zeros
        for( int i_term = 0; i_term < n_spec_fit_terms; ++i_term)
        {  
          // Set speed to be negative so we know it is not physical.
          amb_speed_ridge_amp[n_spec_fit_terms*flat_index+i_term]   = -999;
          amb_speed_ridge_phase[n_spec_fit_terms*flat_index+i_term] = 0;
        }

        // Only do this stuff if the directionRanges.bestSpd is not null pointer.
        if( wvc->directionRanges.bestSpd != NULL )
        {
          // Get the left and right interval for each ambiguity...
          for( int i_amb = 0; i_amb < 4; ++i_amb)
          {
            AngleInterval* alist=wvc->directionRanges.GetByIndex(i_amb);
            if( !alist )
            {
              amb_speed_ridge_azi_left [4*flat_index+i_amb] = 0;
              amb_speed_ridge_azi_right[4*flat_index+i_amb] = 0;
            }
            else
            {
              amb_speed_ridge_azi_left [4*flat_index+i_amb] = alist->left  * 180 / pi;
              amb_speed_ridge_azi_right[4*flat_index+i_amb] = alist->right * 180 / pi;
            }
          }
          // WIND_DIR_INTV_INIT is from GSparameters.h include...
          // Caution: WIND_DIR_INTV_INIT may not control the population of 
          // wvc->directionRanges.bestSpd array. 8-4-2009
          int n_azi = 360.0 / WIND_DIR_INTV_INIT;
          double azimuth[n_azi];
          double wind_speed_ridge[n_azi];
          double amplitude[n_spec_fit_terms];
          double phase[n_spec_fit_terms];
          
          for( int i_azi = 0; i_azi < n_azi; ++i_azi )
          {
            azimuth[i_azi] = WIND_DIR_INTV_INIT * i_azi;
            wind_speed_ridge[i_azi] = wvc->directionRanges.bestSpd[i_azi];
          }
          if( ! specfit( azimuth, wind_speed_ridge, NULL, n_azi,
                         0, n_spec_fit_terms, amplitude, phase ) )
          {
            fprintf(stderr, "L2B::WriteRETDAT: Error in specfit\n");
          }
          else
          {  
          // Copy amplitude, phase arrays to output buffers if no
          // error in specfit.
            for( int i_term = 0; i_term < n_spec_fit_terms; ++i_term)
            {
          
              amb_speed_ridge_amp[n_spec_fit_terms*flat_index+i_term]   = 
                                       float(amplitude[i_term]);
                                       
              amb_speed_ridge_phase[n_spec_fit_terms*flat_index+i_term] = 
                                       float(phase[i_term]);
            }
          }
        }  // if( wvc->directionRanges.bestSpd != NULL ) conditional
        
      } // ati loop
	}   // cti loop
	
	fid = fopen(filename,"w");
	if( fid == NULL ) {
	  fprintf(stderr, "L2B::WriteRETDAT Unable to open file %s\n",filename);
	  return -1;
    }	
    
    unsigned int num_wvc = ctibins*atibins;
    
//--Write stuff to the RETDAT file-----
     //--1st 4 bytes are atibins	
	if( fwrite(&atibins, sizeof(unsigned int), 1, fid ) != 1 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing atibins to %s\n",filename);

     //--Next 4 bytes are ctibins
	if( fwrite(&ctibins, sizeof(unsigned int), 1, fid ) != 1 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing ctibins to %s\n",filename);	  

     //--Next 4*ctibins*atibins bytes are latitudes for the cells.	  
	if( fwrite(lat_arr, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing lat_arr to %s\n",filename);

     //--Next 4*ctibins*atibins bytes are longitudes for the cells.	  
	if( fwrite(lon_arr, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing lon_arr to %s\n",filename);	

     // sel_speed
	if( fwrite(sel_speed, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing sel_speed to %s\n",filename);	

     // sel_dir
	if( fwrite(sel_dir, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing sel_dir to %s\n",filename);	

     // Placeholder bytes for true_speed
	if( fwrite(true_speed, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing true_speed to %s\n",filename);	

     // Placeholder bytes for true_dir
	if( fwrite(true_dir, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing true_dir to %s\n",filename);

	 // model_speed  
	if( fwrite(model_speed, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing model_speed to %s\n",filename);	

	 // model_dir 
	if( fwrite(model_dir, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing model_dir to %s\n",filename);
	
	// Placeholder bytes for liquid
	if( fwrite(liquid, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing liquid to %s\n",filename);

	// retdat_flags
	if( fwrite(retdat_flags, sizeof(float), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing retdat_flags to %s\n",filename);

	// wvc_selection
	if( fwrite(wvc_selection, sizeof(int), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing wvc_selection to %s\n",filename);

	// num_ambigs
	if( fwrite(num_ambigs, sizeof(int), ctibins*atibins, fid ) != num_wvc )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing num_ambigs to %s\n",filename);
	  
	// amb_obj
	if( fwrite(amb_obj, sizeof(int), ctibins*atibins*4, fid ) != num_wvc*4 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing amb_obj to %s\n",filename);
	  
	// amb_speed
	if( fwrite(amb_speed, sizeof(int), ctibins*atibins*4, fid ) != num_wvc*4 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing amb_speed to %s\n",filename);
	  
	// amb_dir
	if( fwrite(amb_dir, sizeof(int), ctibins*atibins*4, fid ) != num_wvc*4 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing amb_dir to %s\n",filename);

	// amb_speed_ridge_azi_left
	if( fwrite(amb_speed_ridge_azi_left, sizeof(int), ctibins*atibins*4, fid ) != num_wvc*4 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing amb_speed_ridge_azi_left to %s\n",filename);
	  
	// amb_speed_ridge_azi_right
	if( fwrite(amb_speed_ridge_azi_right, sizeof(int), ctibins*atibins*4, fid ) != num_wvc*4 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing amb_speed_ridge_azi_right to %s\n",filename);

    // n_spec_fit_terms
	if( fwrite(&n_spec_fit_terms, sizeof(int), 1, fid ) != 1 )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing n_spec_fit_terms to %s\n",filename);
    
	// amb_speed_ridge_amp
	if( fwrite(amb_speed_ridge_amp, sizeof(float), ctibins*atibins*n_spec_fit_terms, fid ) 
	    != num_wvc*n_spec_fit_terms )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing amb_speed_ridge_amp to %s\n",filename);

    // amb_speed_ridge_phase
	if( fwrite(amb_speed_ridge_phase, sizeof(float), ctibins*atibins*n_spec_fit_terms, fid ) 
	    != num_wvc*n_spec_fit_terms )
	  fprintf(stderr, "L2B::WriteRETDAT Error writing amb_speed_ridge_phase to %s\n",filename);
	
	fclose(fid);
	
	// Free up the stuff I allocated...
	delete[] lat_arr;
	delete[] lon_arr;
	delete[] sel_speed;
	delete[] sel_dir;
	delete[] model_speed;
	delete[] model_dir;
	delete[] true_speed;
	delete[] true_dir;
	delete[] liquid;
	delete[] wvc_selection;
	delete[] num_ambigs;
	delete[] retdat_flags;
	delete[] amb_obj;
	delete[] amb_speed;
	delete[] amb_dir;
    delete[] amb_speed_ridge_azi_left;
    delete[] amb_speed_ridge_azi_right;
    delete[] amb_speed_ridge_amp;
    delete[] amb_speed_ridge_phase;
	
    return 1;
}


//------------------//
// L2B::ReadPureHdf //
//------------------//

/*
#define ROW_WIDTH  76
*/
#define AMBIGS      4

int
L2B::ReadPureHdf(
    const char*  filename,
    int          unnormalize_mle_flag)
{
    //---------//
    // prepare //
    //---------//

    frame.swath.DeleteEntireSwath();

    //--------------------------//
    // start access to the file //
    //--------------------------//

    int32 sd_id = SDstart(filename, DFACC_READ);
    if (sd_id == FAIL) {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDstart\n");
        return(0);
    }

    //-------------------------------------//
    // determine the actual number of rows //
    //-------------------------------------//

    int32 attr_index_l2b_actual_wvc_rows = SDfindattr(sd_id,
        "l2b_actual_wvc_rows");
    if (attr_index_l2b_actual_wvc_rows == FAIL) {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDfindattr\n");
        return(0);
    }

    char data[1024];
    if (SDreadattr(sd_id, attr_index_l2b_actual_wvc_rows, data) == FAIL) {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDreadattr\n");
        return(0);
    }

    int l2b_actual_wvc_rows = 0;
    if (sscanf(data, " %*[^\n] %*[^\n] %d", &l2b_actual_wvc_rows) != 1) {
        fprintf(stderr, "L2B::ReadPureHdf: error parsing header\n");
        return(0);
    }
    int along_track_width = l2b_actual_wvc_rows;

    //-------------------------------//
    // get all the necessary SDS IDs //
    //-------------------------------//

    int32 wvc_row_sds_id = SDnametoid(sd_id, "wvc_row");
    int32 wvc_lat_sds_id = SDnametoid(sd_id, "wvc_lat");
    int32 wvc_lon_sds_id = SDnametoid(sd_id, "wvc_lon");
    int32 wvc_index_sds_id = SDnametoid(sd_id, "wvc_index");
    int32 num_in_fore_sds_id = SDnametoid(sd_id, "num_in_fore");
    int32 num_in_aft_sds_id = SDnametoid(sd_id, "num_in_aft");
    int32 num_out_fore_sds_id = SDnametoid(sd_id, "num_out_fore");
    int32 num_out_aft_sds_id = SDnametoid(sd_id, "num_out_aft");
    int32 wvc_quality_flag_sds_id = SDnametoid(sd_id, "wvc_quality_flag");
//    int32 atten_corr_sds_id = SDnametoid(sd_id, "atten_corr");
    int32 model_speed_sds_id = SDnametoid(sd_id, "model_speed");
    int32 model_dir_sds_id = SDnametoid(sd_id, "model_dir");
    int32 num_ambigs_sds_id = SDnametoid(sd_id, "num_ambigs");
    int32 wind_speed_sds_id = SDnametoid(sd_id, "wind_speed");
    int32 wind_dir_sds_id = SDnametoid(sd_id, "wind_dir");
    int32 max_likelihood_est_sds_id = SDnametoid(sd_id, "max_likelihood_est");
    int32 wvc_selection_sds_id = SDnametoid(sd_id, "wvc_selection");
    int32 wind_speed_selection_sds_id = SDnametoid(sd_id,
        "wind_speed_selection");
    int32 wind_dir_selection_sds_id = SDnametoid(sd_id, "wind_dir_selection");
    int32 mp_rain_probability_sds_id = SDnametoid(sd_id,
        "mp_rain_probability");

    //---------------------------------//
    // determine the cross track width //
    //---------------------------------//

    char name[66];
    int32 rank, dim_sizes[8], data_type, n_attrs;
    if (SDgetinfo(wvc_lat_sds_id, name, &rank, dim_sizes, &data_type,
        &n_attrs) == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDgetinfo\n");
        return(0);
    }
    int cross_track_width = dim_sizes[1];

    //-------------------------//
    // guess at the resolution //
    //-------------------------//
    // this is a total hack, but the resolution isn't explicitly given
    // assume a 1900 km swath
    // 100.0 km   19 wvc across (  0 -  28)
    //  50.0 km   38 wvc across ( 29 -  56)
    //  25.0 km   76 wvc across ( 57 -  85)
    //  20.0 km   95 wvc across ( 86 - 110)
    //  15.0 km  127 wvc across (111 - 139)
    //  12.5 km  152 wvc across (140 -   ?)

    float resolution = 0.0;
    if (cross_track_width < 29) {
        resolution = 100.0;
    } else if (cross_track_width < 57) {
        resolution = 50.0;
    } else if (cross_track_width < 86) {
        resolution = 25.0;
    } else if (cross_track_width < 111) {
        resolution = 20.0;
    } else if (cross_track_width < 140) {
        resolution = 15.0;
    } else {
        resolution = 12.5;
    }

    header.crossTrackResolution = resolution;
    header.alongTrackResolution = resolution;
    header.zeroIndex = cross_track_width / 2;

    //----------------------------------------//
    // some generic HDF start and edge arrays //
    //----------------------------------------//

    // the HDF read routine should only access as many dimensions as needed
    int32 generic_start[3] = { 0, 0, 0 };
    int32 generic_edges[3] = { 1, cross_track_width, AMBIGS };

    //----------//
    // allocate //
    //----------//

    if (! frame.swath.Allocate(cross_track_width, along_track_width))
        return(0);

    //--------------//
    // for each row //
    //--------------//

    for (int ati = 0; ati < along_track_width; ati++)
    {
        //--------------//
        // read the row //
        //--------------//

        generic_start[0] = ati;

        int16 wvc_row;
        if (SDreaddata(wvc_row_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)&wvc_row) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_row)\n");
            return(0);
        }
        if (wvc_row != ati + 1)
        {
            fprintf(stderr, "L2B::ReadPureHdf: mismatched index and row\n");
            fprintf(stderr, "    ati = %d, wvc_row = %d\n", ati, wvc_row);
            return(0);
        }

        int16 wvc_lat[cross_track_width];
        if (SDreaddata(wvc_lat_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_lat) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_lat)\n");
            return(0);
        }

        uint16 wvc_lon[cross_track_width];
        if (SDreaddata(wvc_lon_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_lon) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_lon)\n");
            return(0);
        }

        int8 wvc_index[cross_track_width];
        if (SDreaddata(wvc_index_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_index) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_index)\n");
            return(0);
        }

        int8 num_in_fore[cross_track_width];
        if (SDreaddata(num_in_fore_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_in_fore) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_in_fore)\n");
            return(0);
        }

        int8 num_in_aft[cross_track_width];
        if (SDreaddata(num_in_aft_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_in_aft) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_in_aft)\n");
            return(0);
        }

        int8 num_out_fore[cross_track_width];
        if (SDreaddata(num_out_fore_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_out_fore) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_out_fore)\n");
            return(0);
        }

        int8 num_out_aft[cross_track_width];
        if (SDreaddata(num_out_aft_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_out_aft) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_out_aft)\n");
            return(0);
        }

        uint16 wvc_quality_flag[cross_track_width];
        if (SDreaddata(wvc_quality_flag_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_quality_flag) == FAIL)
        {
            fprintf(stderr,
              "L2B::ReadPureHdf: error with SDreaddata (wvc_quality_flag)\n");
            return(0);
        }

/*
        int16 atten_corr[cross_track_width];
        if (SDreaddata(atten_corr_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)atten_corr) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (atten_corr)\n");
            return(0);
        }
*/

        int16 model_speed[cross_track_width];
        if (SDreaddata(model_speed_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)model_speed) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (model_speed)\n");
            return(0);
        }

        uint16 model_dir[cross_track_width];
        if (SDreaddata(model_dir_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)model_dir) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (model_dir)\n");
            return(0);
        }

        int8 num_ambigs[cross_track_width];
        if (SDreaddata(num_ambigs_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_ambigs) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_ambigs)\n");
            return(0);
        }

        int16 wind_speed[cross_track_width][AMBIGS];
        if (SDreaddata(wind_speed_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_speed) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wind_speed)\n");
            return(0);
        }

        uint16 wind_dir[cross_track_width][AMBIGS];
        if (SDreaddata(wind_dir_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_dir) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wind_dir)\n");
            return(0);
        }

        int16 max_likelihood_est[cross_track_width][AMBIGS];
        if (SDreaddata(max_likelihood_est_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)max_likelihood_est) == FAIL)
        {
            fprintf(stderr,
            "L2B::ReadPureHdf: error with SDreaddata (max_likelihood_est)\n");
            return(0);
        }

        int8 wvc_selection[cross_track_width];
        if (SDreaddata(wvc_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_selection) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_selection)\n");
            return(0);
        }

        int16 wind_speed_selection[cross_track_width];
        if (SDreaddata(wind_speed_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_speed_selection) == FAIL)
        {
            fprintf(stderr,
          "L2B::ReadPureHdf: error with SDreaddata (wind_speed_selection)\n");
            return(0);
        }

        uint16 wind_dir_selection[cross_track_width];
        if (SDreaddata(wind_dir_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_dir_selection) == FAIL)
        {
            fprintf(stderr,
            "L2B::ReadPureHdf: error with SDreaddata (wind_dir_selection)\n");
            return(0);
        }

        int16 mp_rain_probability[cross_track_width];
        if (SDreaddata(mp_rain_probability_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)mp_rain_probability) == FAIL)
        {
            fprintf(stderr,
            "L2B::ReadPureHdf: error with SDreaddata (mp_rain_probability)\n");
            return(0);
        }

        //---------------------------------//
        // assemble into wind vector cells //
        //---------------------------------//

        for (int cti = 0; cti < cross_track_width; cti++)
        {
            // if there are no ambiguities, wind was not retrieved
            if (num_ambigs[cti] == 0)
                continue;

            //----------------//
            // create the WVC //
            //----------------//

            WVC* wvc = new WVC();

            //--------------------------------//
            // set the longitude and latitude //
            //--------------------------------//

            wvc->lonLat.longitude = wvc_lon[cti] * HDF_WVC_LON_SCALE * dtr;
            wvc->lonLat.latitude = wvc_lat[cti] * HDF_WVC_LAT_SCALE * dtr;

            //----------------------//
            // set the nudge vector //
            //----------------------//

            wvc->nudgeWV = new WindVectorPlus();
            if (wvc->nudgeWV == NULL)
            {
                fprintf(stderr, "L2B::ReadPureHdf: error allocating\n");
                return(0);
            }

            float nudge_edir = gs_deg_to_pe_rad(double(model_dir[cti] * HDF_MODEL_DIR_SCALE));
            float nudge_speed = model_speed[cti] * HDF_MODEL_SPEED_SCALE;

            wvc->nudgeWV->SetSpdDir(nudge_speed * NWP_SPEED_CORRECTION,
                nudge_edir);
            

            //--------------------------------//
            // set the number of measurements //
            //--------------------------------//

            wvc->numInFore = (unsigned char)num_in_fore[cti];
            wvc->numInAft = (unsigned char)num_in_aft[cti];
            wvc->numOutFore = (unsigned char)num_out_fore[cti];
            wvc->numOutAft = (unsigned char)num_out_aft[cti];

            //---------------------------//
            // create the ambiguity list //
            //---------------------------//

            int sigma0_count = num_in_fore[cti] + num_in_aft[cti]
                + num_out_fore[cti] + num_out_aft[cti];

            for (int ambig_idx = 0; ambig_idx < num_ambigs[cti]; ambig_idx++)
            {
                WindVectorPlus* wvp = new WindVectorPlus();
                if (wvp == NULL)
                {
                    fprintf(stderr, "L2B::ReadPureHdf: error allocating\n");
                    return(0);
                }

                float edir = gs_deg_to_pe_rad(wind_dir[cti][ambig_idx]
                    * HDF_WIND_DIR_SCALE);
                float spd = wind_speed[cti][ambig_idx] * HDF_WIND_SPEED_SCALE;
                wvp->SetSpdDir(spd, edir);

                wvp->obj = max_likelihood_est[cti][ambig_idx]
                    * HDF_MAX_LIKELIHOOD_EST_SCALE;
                if (unnormalize_mle_flag)
                {
                    wvp->obj *= sigma0_count;
                }

                if (! wvc->ambiguities.Append(wvp))
                {
                    fprintf(stderr, "L2B::ReadPureHdf: error appending\n");
                    return(0);
                }
            }

            //----------------------------//
            // set the selected ambiguity //
            //----------------------------//

            wvc->selected = wvc->ambiguities.GetByIndex(wvc_selection[cti]
                - 1);
            wvc->selected_allocated = 0;

            //------------------------//
            // set the special vector //
            //------------------------//

            WindVector* wv = new WindVector();
            if (wv == NULL)
            {
                fprintf(stderr, "L2B::ReadPureHdf: error allocating\n");
                return(0);
            }

            float special_dir = gs_deg_to_pe_rad(wind_dir_selection[cti]
                * HDF_WIND_DIR_SELECTION_SCALE);
            float special_spd = wind_speed_selection[cti]
                * HDF_WIND_SPEED_SELECTION_SCALE;
            wv->SetSpdDir(special_spd, special_dir);
            wvc->specialVector = wv;

            //----------------------------//
            // set the rain "probability" //
            //----------------------------//

            wvc->rainProb = mp_rain_probability[cti]
                * HDF_MP_RAIN_PROBABILITY_SCALE;
                
            //--------------------//
            // read quality flags //
            //--------------------//                
                
            wvc->rainFlagBits     = (char)((0x7000 & wvc_quality_flag[cti]) >> 12);
            wvc->landiceFlagBits  = (char)((0x0180 & wvc_quality_flag[cti]) >> 7 );
            
            // Set qualFlag variable from wvc_quality_flag 12/16/2010 AGF
            wvc->qualFlag = 0;
            if( L2B_HDF_QUAL_FLAG_LAND           & wvc_quality_flag[cti]) 
              wvc->qualFlag += L2B_QUAL_FLAG_LAND;
            if( L2B_HDF_QUAL_FLAG_ICE            & wvc_quality_flag[cti]) 
              wvc->qualFlag += L2B_QUAL_FLAG_ICE;
            if( L2B_HDF_QUAL_FLAG_RAIN_UNUSABLE  & wvc_quality_flag[cti]) 
              wvc->qualFlag += L2B_QUAL_FLAG_RAIN_UNUSABLE;
            if( L2B_HDF_QUAL_FLAG_RAIN           & wvc_quality_flag[cti]) 
              wvc->qualFlag += L2B_QUAL_FLAG_RAIN;
            if( L2B_HDF_QUAL_FLAG_AVAILABLE_DATA & wvc_quality_flag[cti])
              wvc->qualFlag += L2B_QUAL_FLAG_RAIN_LOCATION;
            
            //------------------//
            // add WVC to swath //
            //------------------//

            if (! frame.swath.Add(cti, ati, wvc))
            {
                fprintf(stderr, "L2B::ReadPureHdf: error adding WVC\n");
                return(0);
            }
        }
    }

// AGF added this since we have read the nudge vectors 7/21/2009
    frame.swath.nudgeVectorsRead = 1;

    if (SDendaccess(wvc_row_sds_id) == FAIL ||
        SDendaccess(wvc_lat_sds_id) == FAIL ||
        SDendaccess(wvc_lon_sds_id) == FAIL ||
        SDendaccess(wvc_index_sds_id) == FAIL ||
        SDendaccess(num_in_fore_sds_id) == FAIL ||
        SDendaccess(num_in_aft_sds_id) == FAIL ||
        SDendaccess(num_out_fore_sds_id) == FAIL ||
        SDendaccess(num_out_aft_sds_id) == FAIL ||
        SDendaccess(wvc_quality_flag_sds_id) == FAIL ||
//        SDendaccess(atten_corr_sds_id) == FAIL ||
        SDendaccess(model_speed_sds_id) == FAIL ||
        SDendaccess(model_dir_sds_id) == FAIL ||
        SDendaccess(num_ambigs_sds_id) == FAIL ||
        SDendaccess(wind_speed_sds_id) == FAIL ||
        SDendaccess(wind_dir_sds_id) == FAIL ||
        SDendaccess(max_likelihood_est_sds_id) == FAIL ||
        SDendaccess(wvc_selection_sds_id) == FAIL ||
        SDendaccess(wind_speed_selection_sds_id) == FAIL ||
        SDendaccess(wind_dir_selection_sds_id) == FAIL ||
        SDendaccess(mp_rain_probability_sds_id) == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDendaccess\n");
        return(0);
    }

    //------------------------//
    // end access to the file //
    //------------------------//

    if (SDend(sd_id) == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDend\n");
        return(0);
    }

    // success!
    return(1);
}

//---------------//
// L2B::WriteHdf //
//---------------//

// attributes
Attribute* l2b_actual_wvc_rows = new Attribute("l2b_actual_wvc_rows", "int",
    "1", "<missing>");

// attribute table
Attribute* g_l2b_attribute_table[] =
{
    l2b_actual_wvc_rows,
    NULL
};

// dimension sizes
int32 l2b_dim_sizes_frame[] = { SD_UNLIMITED, 76, 4 };

// dimension names
const char* l2b_dim_names_frame[] = { "Wind_Vector_Cell_Row",
    "Wind_Vector_Cell", "Ambiguity" };

// SDS's
SdsInt16* wvc_row = new SdsInt16("wvc_row", 1, l2b_dim_sizes_frame, "counts",
    1.0, 0.0, l2b_dim_names_frame, 1624, 1);
SdsInt16* wvc_lat = new SdsInt16("wvc_lat", 2, l2b_dim_sizes_frame, "degrees",
    0.01, 0.0, l2b_dim_names_frame, 9000, -9000);
SdsUInt16* wvc_lon = new SdsUInt16("wvc_lon", 2, l2b_dim_sizes_frame, "degrees",
    0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt8* wvc_index = new SdsInt8("wvc_index", 2, l2b_dim_sizes_frame, "counts",
    1.0, 0.0, l2b_dim_names_frame, 76, 1);
SdsInt8* num_in_fore = new SdsInt8("num_in_fore", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsInt8* num_in_aft = new SdsInt8("num_in_aft", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsInt8* num_out_fore = new SdsInt8("num_out_fore", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsInt8* num_out_aft = new SdsInt8("num_out_aft", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsUInt16* wvc_quality_flag = new SdsUInt16("wvc_quality_flag", 2,
    l2b_dim_sizes_frame, "n/a", 1.0, 0.0, l2b_dim_names_frame, 32643, 0);
SdsInt16* model_speed = new SdsInt16("model_speed", 2, l2b_dim_sizes_frame,
    "m/s", 0.01, 0.0, l2b_dim_names_frame, 7000, 0);
SdsUInt16* model_dir = new SdsUInt16("model_dir", 2, l2b_dim_sizes_frame,
    "deg", 0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt8* num_ambigs = new SdsInt8("num_ambigs", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 4, 0);
SdsInt16* wind_speed = new SdsInt16("wind_speed", 3, l2b_dim_sizes_frame,
    "m/s", 0.01, 0.0, l2b_dim_names_frame, 5000, 0);
SdsUInt16* wind_dir = new SdsUInt16("wind_dir", 3, l2b_dim_sizes_frame,
    "deg", 0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt16* max_likelihood_est = new SdsInt16("max_likelihood_est", 3,
    l2b_dim_sizes_frame, "n/a", 0.001, 0.0, l2b_dim_names_frame, -30000, 0);
SdsInt8* wvc_selection = new SdsInt8("wvc_selection", 2, l2b_dim_sizes_frame,
    "n/a", 1.0, 0.0, l2b_dim_names_frame, 4, 0);
SdsInt16* wind_speed_selection = new SdsInt16("wind_speed_selection", 2,
    l2b_dim_sizes_frame, "m/s", 0.01, 0.0, l2b_dim_names_frame, 7000, 0);
SdsUInt16* wind_dir_selection = new SdsUInt16("wind_dir_selection", 2,
    l2b_dim_sizes_frame, "deg", 0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt16* mp_rain_probability = new SdsInt16("mp_rain_probability", 2,
    l2b_dim_sizes_frame, "n/a", 0.001, 0.0, l2b_dim_names_frame, 1000, -3000);

// SDS table
Sds* g_l2b_sds_table[] =
{
    wvc_row,
    wvc_lat,
    wvc_lon,
    wvc_index,
    num_in_fore,
    num_in_aft,
    num_out_fore,
    num_out_aft,
    wvc_quality_flag,
    model_speed,
    model_dir,
    num_ambigs,
    wind_speed,
    wind_dir,
    max_likelihood_est,
    wvc_selection,
    wind_speed_selection,
    wind_dir_selection,
    mp_rain_probability,
    NULL
};

int
L2B::WriteHdf(
    const char*  filename,
    int          unnormalize_mle_flag)
{
    WindSwath* swath = &(frame.swath);

    //----------------------//
    // open sds for writing //
    //----------------------//

    int32 sds_output_file_id = SDstart(filename, DFACC_CREATE);
    if (sds_output_file_id == FAIL)
    {
        fprintf(stderr, "L2B::WriteHdf: error with SDstart\n");
        return(0);
    }

    //-----------------------//
    // create the attributes //
    //-----------------------//

    for (int idx = 0; g_l2b_attribute_table[idx] != NULL; idx++)
    {
        if (! (g_l2b_attribute_table[idx])->Write(sds_output_file_id))
        {
            return(0);
        }
    }

    //------------------//
    // create the SDS's //
    //------------------//

    for (int idx = 0; g_l2b_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_l2b_sds_table[idx];
        if (! sds->Create(sds_output_file_id))
        {
            fprintf(stderr, "L2B::WriteHdf: error creating SDS %d\n", idx);
            return(0);
        }
    }

    //-----------------------//
    // set up the attributes //
    //-----------------------//

    char buffer[1024];

    int at_max = swath->GetAlongTrackBins();
    sprintf(buffer, "%d", at_max);
    l2b_actual_wvc_rows->ReplaceContents(buffer);

    //----------------------//
    // write the attributes //
    //----------------------//

    for (int idx = 0; g_l2b_attribute_table[idx] != NULL; idx++)
    {
        if (! (g_l2b_attribute_table[idx])->Write(sds_output_file_id))
        {
            return(0);
        }
    }

    for (int ati = 0; ati < at_max; ati++)
    {
        //--------------------//
        // set the swath data //
        //--------------------//

        int16 wvc_row_value = ati + 1;
        wvc_row->SetWithInt16(&wvc_row_value);

        float wvc_lat_value[HDF_ACROSS_BIN_NO];
        float wvc_lon_value[HDF_ACROSS_BIN_NO];
        int8 wvc_index_value[HDF_ACROSS_BIN_NO];
        int8 num_in_fore_value[HDF_ACROSS_BIN_NO];
        int8 num_in_aft_value[HDF_ACROSS_BIN_NO];
        int8 num_out_fore_value[HDF_ACROSS_BIN_NO];
        int8 num_out_aft_value[HDF_ACROSS_BIN_NO];
        uint16 wvc_quality_flag_value[HDF_ACROSS_BIN_NO];
        float model_speed_value[HDF_ACROSS_BIN_NO];
        float model_dir_value[HDF_ACROSS_BIN_NO];
        int8 num_ambigs_value[HDF_ACROSS_BIN_NO];
        float wind_speed_value[HDF_ACROSS_BIN_NO * HDF_NUM_AMBIGUITIES];
        float wind_dir_value[HDF_ACROSS_BIN_NO * HDF_NUM_AMBIGUITIES];
        float max_likelihood_est_value[HDF_ACROSS_BIN_NO*HDF_NUM_AMBIGUITIES];
        int8 wvc_selection_value[HDF_ACROSS_BIN_NO];
        float wind_speed_selection_value[HDF_ACROSS_BIN_NO];
        float wind_dir_selection_value[HDF_ACROSS_BIN_NO];
        float mp_rain_probability_value[HDF_ACROSS_BIN_NO];
        for (int cti = 0; cti < HDF_ACROSS_BIN_NO; cti++)
        {
            // init in case there is no WVC
            wvc_lat_value[cti] = 0.0;
            wvc_lon_value[cti] = 0.0;
            num_in_fore_value[cti] = 0;
            num_in_aft_value[cti] = 0;
            num_out_fore_value[cti] = 0;
            num_out_aft_value[cti] = 0;
            wvc_quality_flag_value[cti] = 32387;
            model_speed_value[cti] = 0.0;
            model_dir_value[cti] = 0.0;
            num_ambigs_value[cti] = 0;
            for (int idx = 0; idx < HDF_NUM_AMBIGUITIES; idx++)
            {
                wind_speed_value[cti * HDF_NUM_AMBIGUITIES + idx] = 0.0;
                wind_dir_value[cti * HDF_NUM_AMBIGUITIES + idx] = 0.0;
                max_likelihood_est_value[cti*HDF_NUM_AMBIGUITIES + idx] = 0.0;
            }
            wvc_selection_value[cti] = 0;
            wind_speed_selection_value[cti] = 0.0;
            wind_dir_selection_value[cti] = 0.0;
            mp_rain_probability_value[cti] = 0.0;

            // set the obvious
            wvc_index_value[cti] = cti + 1;

            // get the wvc
            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            // set the rest
            wvc_lat_value[cti] = wvc->lonLat.latitude * rtd;
            wvc_lon_value[cti] = wvc->lonLat.longitude * rtd;
            num_in_fore_value[cti] = wvc->numInFore;
            num_in_aft_value[cti] = wvc->numInAft;
            num_out_fore_value[cti] = wvc->numOutFore;
            num_out_aft_value[cti] = wvc->numOutAft;
            
            wvc_quality_flag_value[cti] = wvc->rainFlagBits << 12 + 
                wvc->landiceFlagBits << 7;
            
            if (wvc->nudgeWV != NULL)
            {
                 model_speed_value[cti] = wvc->nudgeWV->spd
                     / NWP_SPEED_CORRECTION;
                 model_dir_value[cti] = pe_rad_to_gs_deg(wvc->nudgeWV->dir);
            }
            num_ambigs_value[cti] = wvc->ambiguities.NodeCount();

            int num_meas = wvc->numInFore + wvc->numInAft + wvc->numOutFore
                + wvc->numOutAft;
            int idx = 0;
            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext())
            {
                wind_speed_value[cti * HDF_NUM_AMBIGUITIES + idx] = wvp->spd;
                wind_dir_value[cti * HDF_NUM_AMBIGUITIES + idx] =
                    pe_rad_to_gs_deg(wvp->dir);
                float mle = wvp->obj;
                if (unnormalize_mle_flag)
                {
                    mle /= num_meas;
                }
                max_likelihood_est_value[cti * HDF_NUM_AMBIGUITIES + idx] =
                    mle;
                idx++;
            }

            wvc_selection_value[cti] =
                wvc->ambiguities.GetIndexOf(wvc->selected) + 1;
            wind_speed_selection_value[cti] = wvc->selected->spd;
            wind_dir_selection_value[cti] =
                pe_rad_to_gs_deg(wvc->selected->dir);
            mp_rain_probability_value[cti] = wvc->rainProb;
        }

        // set the sds
        wvc_lat->SetFromFloat(wvc_lat_value);
        wvc_lon->SetFromFloat(wvc_lon_value);
        wvc_index->SetWithChar(wvc_index_value);
        num_in_fore->SetWithChar(num_in_fore_value);
        num_in_aft->SetWithChar(num_in_aft_value);
        num_out_fore->SetWithChar(num_out_fore_value);
        num_out_aft->SetWithChar(num_out_aft_value);
        wvc_quality_flag->SetWithUnsignedShort(wvc_quality_flag_value);
        model_speed->SetFromFloat(model_speed_value);
        model_dir->SetFromFloat(model_dir_value);
        num_ambigs->SetWithChar(num_ambigs_value);
        wind_speed->SetFromFloat(wind_speed_value);
        wind_dir->SetFromFloat(wind_dir_value);
        max_likelihood_est->SetFromFloat(max_likelihood_est_value);
        wvc_selection->SetWithChar(wvc_selection_value);
        wind_speed_selection->SetFromFloat(wind_speed_selection_value);
        wind_dir_selection->SetFromFloat(wind_dir_selection_value);
        mp_rain_probability->SetFromFloat(mp_rain_probability_value);

        //-----------------//
        // write the SDS's //
        //-----------------//

        for (int idx = 0; g_l2b_sds_table[idx] != NULL; idx++)
        {
            Sds* sds = g_l2b_sds_table[idx];
            if (! sds->Write(ati))
            {
                fprintf(stderr, "L1AH::WriteSDSs: error writing SDS %s\n",
                    sds->GetName());
                return(0);
            }
        }
    }

    //----------------//
    // end sds access //
    //----------------//

    for (int idx = 0; g_l2b_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_l2b_sds_table[idx];
        if (! sds->EndAccess())
        {
            fprintf(stderr, "L1AH::EndSDSOutput: error ending SDS access\n");
            return(0);
        }
    }
    if (SDend(sds_output_file_id) != SUCCEED)
    {
        fprintf(stderr, "L1AH::EndSDSOutput: error with SDend\n");
        return(0);
    }

    return(1);
}

//--------------------//
// L2B::InsertPureHdf //
//--------------------//
// right now, this just inserts the selection

#define ROW_WIDTH  76

int
L2B::InsertPureHdf(
    const char*  input_filename,
    const char*  output_filename,
    int          unnormalize_mle_flag)
{
    //----------------------------------------//
    // copy the input file to the output file //
    //----------------------------------------//

    FILE* ifp = fopen(input_filename, "r");
    if (ifp == NULL)
        return(0);

    FILE* ofp = fopen(output_filename, "w");
    if (ofp == NULL)
        return(0);

    char c;
    while (fread(&c, sizeof(char), 1, ifp) == 1)
    {
        fwrite(&c, sizeof(char), 1, ofp);
    }
    if (! feof(ifp))
        return(0);    // error, not EOF

    fclose(ifp);
    fclose(ofp);

    //----------------------------------------//
    // some generic HDF start and edge arrays //
    //----------------------------------------//
    // the HDF write routines should only access as many dimensions as needed
    int32 generic_start[3] = { 0, 0, 0 };
    int32 generic_edges[3] = { 1, ROW_WIDTH, AMBIGS };

    //--------------------------//
    // start access to the file //
    //--------------------------//

    int32 sd_id = SDstart(output_filename, DFACC_WRITE);
    if (sd_id == FAIL)
    {
        fprintf(stderr, "L2B::InsertPureHdf: error with SDstart\n");
        return(0);
    }

    //-------------------------------//
    // get all the necessary SDS IDs //
    //-------------------------------//

    int32 wvc_selection_sds_id = SDnametoid(sd_id, "wvc_selection");

    //--------------//
    // for each row //
    //--------------//

    for (int ati = 0; ati < frame.swath.GetAlongTrackBins(); ati++)
    {
        //-----------------//
        // prepare the row //
        //-----------------//

        generic_start[0] = ati;

        //----------------------------------------//
        // assemble wind vector cells into arrays //
        //----------------------------------------//

        int8 wvc_selection[ROW_WIDTH];

        for (int cti = 0; cti < frame.swath.GetCrossTrackBins(); cti++)
        {
            //-------------------------------//
            // initialize the array elements //
            //-------------------------------//

            wvc_selection[cti] = 0;

            //-------------//
            // get the WVC //
            //-------------//

            WVC* wvc = frame.swath.GetWVC(ati, cti);
            if (wvc == NULL)
            {
                continue;
            }

            //----------------------------//
            // set the selected ambiguity //
            //----------------------------//

            int index = wvc->ambiguities.GetIndexOf(wvc->selected);
            if (index != -1)
                wvc_selection[cti] = index;
        }

        //---------------------------//
        // write the selection array //
        //---------------------------//

        if (SDwritedata(wvc_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_selection) == FAIL)
        {
            fprintf(stderr, "L2B::InsertPureHdf: error with SDwritedata\n");
            return(0);
        }
    }

    if (SDendaccess(wvc_selection_sds_id) == FAIL)
    {
        fprintf(stderr, "L2B::InsertPureHdf: error with SDendaccess\n");
        return(0);
    }

    //------------------------//
    // end access to the file //
    //------------------------//

    if (SDend(sd_id) == FAIL)
    {
        fprintf(stderr, "L2B::InsertPureHdf: error with SDend\n");
        return(0);
    }

    // success!
    return(1);
}

//----------------//
// L2B::WriteVctr //
//----------------//

int
L2B::WriteVctr(
    const char*  filename,
    const int    rank)
{
    return(frame.swath.WriteVctr(filename, rank));
}

//-----------------//
// L2B::WriteAscii //
//-----------------//

int
L2B::WriteAscii()
{
    if (! header.WriteAscii(_outputFp))
        return(0);
    return(frame.swath.WriteAscii(_outputFp));
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    int  unnormalize_mle)
{
    //------//
    // read //
    //------//

    if (! ReadHDF(_inputFilename, unnormalize_mle))
        return(0);

    return(1);
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    const char*  filename,
    int          unnormalize_mle)
{
    //--------------------------------//
    // convert filename to TlmHdfFile //
    //--------------------------------//

    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile l2b_hdf_file(filename, SOURCE_L2B, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

    //------//
    // read //
    //------//

    if (! ReadHDF(&l2b_hdf_file, unnormalize_mle))
        return(0);

    return(1);
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    TlmHdfFile*  tlmHdfFile,
    int          unnormalize_mle)
{
    printf("NOTICE: If you would like to try a cleaner Level 2B HDF\n");
    printf("        reader (i.e. not based on Sally's EA code), then\n");
    printf("        change L2B::ReadHDF to L2B::ReadPureHdf in your code\n");

    WindSwath* swath = &(frame.swath);
    swath->DeleteEntireSwath();    // just in case

    // along bin number comes from WVC_ROW
    const char* rowSdsName = ParTabAccess::GetSdsNames(SOURCE_L2B, WVC_ROW);
    if (rowSdsName == 0)
        return(0);

    // determine the inclination from the header (this is such a hack!)
    const char* filename = tlmHdfFile->GetFileName();
    int32 sd_id = SDstart(filename, DFACC_RDONLY);
    int32 attr_index = SDfindattr(sd_id, "orbit_inclination");
    if (attr_index == SUCCEED)
    {
        char attr_name[512];
        int32 adata_type, count;
        SDattrinfo(sd_id, attr_index, attr_name, &adata_type, &count);
        char inc[32];
        SDreadattr(sd_id, attr_index, inc);
        float inclination;
        char* ptr = strchr(inc, (int)'\n');
        ptr = strchr(ptr+1, (int)'\n');
        sscanf(ptr, " %f", &inclination);
        header.inclination = inclination * dtr;
    }
    SDend(sd_id);

    // continue on with whatever...
    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 rowSdsId = tlmHdfFile->SelectDataset(rowSdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (rowSdsId == HDF_FAIL)
        return(0);

    // open all needed datasets
    if (! _OpenHdfDataSets(tlmHdfFile))
        return(0);

    //--------------//
    // set up swath //
    //--------------//

    int along_track_bins = dataLength;
    int cross_track_bins = HDF_ACROSS_BIN_NO;

// hack in high resolution
#ifdef HIRES12
    cross_track_bins = 152;
#endif

    if (! swath->Allocate(cross_track_bins, along_track_bins))
        return(0);

    //---------------//
    // create arrays //
    //---------------//

    unsigned char*  numambigArray = new unsigned char[cross_track_bins];
    float*          lonArray = new float[cross_track_bins];
    float*          rainArray = new float[cross_track_bins];
    float*          latArray = new float[cross_track_bins];
    float*          speedArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    float*          dirArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    float*          mleArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    char*           selectArray = new char[cross_track_bins];
    float*          modelSpeedArray = (float *) new float[cross_track_bins];
    float*          modelDirArray = (float *) new float[cross_track_bins];
    char*           tmpArray = new char[cross_track_bins];
    int*            numArray = (int *) new int[cross_track_bins];
    unsigned short int*  qualArray =
                      (unsigned short int *) new int[cross_track_bins];
    int32           sdsIds[1];

    for (int32 ati = 0; ati < along_track_bins; ati++)
    {
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] = 0;

        sdsIds[0] = _lonSdsId;
        if (ExtractData2D_76_uint2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            lonArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _latSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            latArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _speedSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            speedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _dirSdsId;
        if (ExtractData3D_76_4_uint2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            dirArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _mleSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            mleArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _selectSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, selectArray) == 0)
            return(0);

        sdsIds[0] = _numambigSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1,
            numambigArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelSpeedSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelSpeedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelDirSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelDirArray) == 0)
        {
            return(0);
        }
        sdsIds[0] = _mpRainProbSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            rainArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _qualSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1,
            qualArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _numInForeSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numInAftSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numOutForeSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numOutAftSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        for (int cti = 0; cti < cross_track_bins; cti++)
        {
            WVC* wvc = new WVC();
            wvc->rainProb=rainArray[cti];
            wvc->rainFlagBits    = (char)((0x7000 & qualArray[cti]) >> 12);
            wvc->landiceFlagBits = (char)((0x180  & qualArray[cti]) >> 7 );
            
            wvc->lonLat.longitude = lonArray[cti] * dtr;
            wvc->lonLat.latitude = latArray[cti] * dtr;
            wvc->nudgeWV = new WindVectorPlus();
            float nudge_edir = gs_deg_to_pe_rad(modelDirArray[cti]);
            wvc->nudgeWV->SetSpdDir(modelSpeedArray[cti] * NWP_SPEED_CORRECTION,
                nudge_edir);

            for (int k = 0; k < numambigArray[cti]; k++)
            {
                WindVectorPlus* wvp = new WindVectorPlus();
                float edir = gs_deg_to_pe_rad(dirArray[cti
                    * HDF_NUM_AMBIGUITIES + k]);
                wvp->SetSpdDir(speedArray[cti * HDF_NUM_AMBIGUITIES + k], edir);
                if (unnormalize_mle)
                {
                    wvp->obj = mleArray[cti * HDF_NUM_AMBIGUITIES + k] *
                        numArray[cti];
                }
                else
                {
                    wvp->obj = mleArray[cti * HDF_NUM_AMBIGUITIES + k];
                }
                wvc->ambiguities.Append(wvp);
            }
            if (selectArray[cti] > 0 && numambigArray[cti] > 0)
            {
                wvc->selected =
                    wvc->ambiguities.GetByIndex(selectArray[cti] - 1);
                swath->Add(cti, ati, wvc);
            }
            else
            {
                delete wvc;
            }
        }
    }

    delete [] modelDirArray;
    delete [] modelSpeedArray;
    delete [] numambigArray;
    delete [] lonArray;
    delete [] latArray;
    delete [] speedArray;
    delete [] dirArray;
    delete [] mleArray;
    delete [] selectArray;
    delete [] numArray;
    delete [] tmpArray;

    // close all needed datasets
    _CloseHdfDataSets();

    swath->nudgeVectorsRead = 1;
    header.crossTrackResolution = 25.0;
    header.alongTrackResolution = 25.0;
    header.zeroIndex = 38;

    return(1);
}

//-------------------------------------//
// L2B::ReadLandIceRainFlagsFromHdfL2B //
//-------------------------------------//
int L2B::ReadLandIceRainFlagsFromHdfL2B(
    const char*  filename,
    int read_land_ice_flags,
    int read_rain_flags)
{
  // Rewritten to load directly from HDF file instead of copying from
  // another L2B object that was read in using ReadPureHDF.  That one will not
  // have flag values in WVCs where numambigs == 0.   2/3/2011  AGF
  WindSwath* swath = &(frame.swath);
  
  // read flags directly from HDF 
  int32 sd_id = SDstart(filename, DFACC_READ);
  if (sd_id == FAIL) {
    fprintf(stderr, "L2B::ReadLandIceRainFlagsFromHdfL2B: error with SDstart\n");
    return(0);
  }

  int32 wvc_quality_flag_sds_id = SDnametoid(sd_id, "wvc_quality_flag");
  
  char name[66];
  int32 rank, dim_sizes[8], data_type, n_attrs;
  if (SDgetinfo(wvc_quality_flag_sds_id, name, &rank, dim_sizes, &data_type,
      &n_attrs) == FAIL) {
    fprintf(stderr, "L2B::ReadLandIceRainFlagsFromHdfL2B: error with SDgetinfo\n");
    SDend(sd_id);
    return(0);
  }
  //printf( "name: %s; rank: %d; dim_sizes: %6d %6d\n", name, rank, dim_sizes[0], dim_sizes[1] ); 
  
  // compare # of cross-track bins and along-track bins
  if( dim_sizes[0] != swath->GetAlongTrackBins() || 
      dim_sizes[1] != swath->GetCrossTrackBins() ) {
    fprintf(stderr, "L2B::ReadLandIceRainFlagsFromHdfL2B: size mismatch: %6d %6d %6d %6d\n",
      dim_sizes[0], dim_sizes[1], swath->GetAlongTrackBins(), swath->GetCrossTrackBins() );
    SDend(sd_id);
    return(0);
  }
  
  int32  start[2] = { 0, 0 };
  int32  edges[2] = { dim_sizes[0], dim_sizes[1] };
  uint16 wvc_quality_flags[dim_sizes[0]][dim_sizes[1]];
  
  if( SDreaddata( wvc_quality_flag_sds_id, start, NULL, edges, 
        &wvc_quality_flags[0][0] ) == FAIL ) {
    fprintf(stderr, "L2B::ReadLandIceRainFlagsFromHdfL2B: error with SDreaddata\n");
    SDend(sd_id);
    return(0);
  }
  
  for( int ati = 0; ati < swath->GetAlongTrackBins(); ++ati ) {
    for( int cti = 0; cti < swath->GetCrossTrackBins(); ++cti ) {

      WVC* wvc = swath->GetWVC(cti, ati);
      if( !wvc ) // no retrieval 
        continue;
      
      // new_flag = ( flag & ~flag_mask ) | flag_state;
      // flag & ~flag_mask returns flag with the bits that are set in flag_mask unset.
      // then we bitwise or this with the state we want these bits to be in.
      if( read_land_ice_flags ) {

        // test for land (2^7 in hex == 0x80)
        wvc->landiceFlagBits = ( wvc->landiceFlagBits & ~LAND_ICE_FLAG_COAST ) |
                               ( (wvc_quality_flags[ati][cti]&0x00080) != 0  ) * LAND_ICE_FLAG_COAST;

        wvc->qualFlag        = ( wvc->qualFlag & ~L2B_QUAL_FLAG_LAND ) |
                               ( (wvc_quality_flags[ati][cti]&0x00080) != 0  ) * L2B_QUAL_FLAG_LAND;

        // test for ice (2^8 in hex == 0x100)
        wvc->landiceFlagBits = ( wvc->landiceFlagBits & ~LAND_ICE_FLAG_ICE ) |
                               ( (wvc_quality_flags[ati][cti]&0x00100) != 0  ) * LAND_ICE_FLAG_ICE;
        
        wvc->qualFlag        = ( wvc->qualFlag & ~L2B_QUAL_FLAG_ICE ) |
                               ( (wvc_quality_flags[ati][cti]&0x00100) != 0  ) * L2B_QUAL_FLAG_ICE;
      }
      if( read_rain_flags ) {
        // test for rain flag unusable (2^12 in hex  == 0x1000)
        wvc->rainFlagBits = ( wvc->rainFlagBits & ~RAIN_FLAG_UNUSABLE ) |
                            ( (wvc_quality_flags[ati][cti]&0x01000) != 0  ) * RAIN_FLAG_UNUSABLE;

        wvc->qualFlag     = ( wvc->qualFlag & ~L2B_QUAL_FLAG_RAIN_UNUSABLE ) |
                            ( (wvc_quality_flags[ati][cti]&0x01000) != 0  ) * L2B_QUAL_FLAG_RAIN_UNUSABLE;

        // test for rain flag rain (2^13 in hex  == 0x2000)
        wvc->rainFlagBits = ( wvc->rainFlagBits & ~RAIN_FLAG_RAIN ) |
                            ( (wvc_quality_flags[ati][cti]&0x02000) != 0  ) * RAIN_FLAG_RAIN;
        
        wvc->qualFlag     = ( wvc->qualFlag & ~L2B_QUAL_FLAG_RAIN ) |
                            ( (wvc_quality_flags[ati][cti]&0x02000) != 0  ) * L2B_QUAL_FLAG_RAIN;                            
      }
    }
  }
  
  if (SDend(sd_id) == FAIL) {
    fprintf(stderr, "L2B::ReadLandIceRainFlagsFromHdfL2B: error with SDend\n");
    return(0);
  }
  return(1);
}

//---------------------------------//
// L2B::ReadNudgeVectorsFromHdfL2B //
//---------------------------------//

int L2B::ReadNudgeVectorsFromHdfL2B(
    const char*  filename,
    int read_wvc_flags_flag )
{
    L2B n_l2b;

    if (n_l2b.ReadPureHdf(filename,1) == 0)  // Use ReadPureHDF, old one broken.
    {
        fprintf(stderr, "ReadNudgeVectorsFromHdfL2B: error reading HDF L2B file %s\n",
                filename);
        return 0;
    }

    WindSwath* swath = &(frame.swath);

    int crossTrackBins = n_l2b.frame.swath.GetCrossTrackBins();
#ifdef HIRES12
    crossTrackBins = 152;
#endif

    // check that the number of cross track bins in the thing just read match the
    // number in the current instance (where we'll be putting the data)
    if (crossTrackBins != swath->GetCrossTrackBins())
    {
        fprintf(stderr, "ReadNudgeVectorsFromHdfL2B: crosstrackbins mismatch\n");
        return(0);
    }

    int alongTrackBins = n_l2b.frame.swath.GetAlongTrackBins();

    // check the number of along track bins match
    if (alongTrackBins != swath->GetAlongTrackBins())
    {
        fprintf(stderr, "ReadNudgeVectorsFromHdfL2B: alongtrackbins mismatch\n");
        fprintf(stderr, "along track bins allocated: %d; along track bins needed: %d\n",
            swath->GetAlongTrackBins(), alongTrackBins);
        return(0);
    }

    for (int32 ati = 0; ati < alongTrackBins; ati++)
    {
        for (int cti = 0; cti < crossTrackBins; cti++)
        {
            WVC* n_wvc = n_l2b.frame.swath.GetWVC(cti, ati);
            WVC* o_wvc = swath->GetWVC(cti, ati);
            if (!n_wvc || !o_wvc)
                continue;
            float nudge_edir = n_wvc->nudgeWV->dir;
            float nudge_espd = n_wvc->nudgeWV->spd;
            if (o_wvc->nudgeWV != NULL)
                delete o_wvc->nudgeWV;
            o_wvc->nudgeWV = new WindVectorPlus();
            o_wvc->nudgeWV->SetSpdDir(nudge_espd, nudge_edir);
            
            if( read_wvc_flags_flag )
            {
              o_wvc->rainFlagBits   = n_wvc->rainFlagBits;
              o_wvc->landiceFlagBits = n_wvc->landiceFlagBits;
            }
        }
    }

    swath->nudgeVectorsRead = 1;

    return(1);
}

//-----------------------------------//
// L2B::GetArraysForUpdatingDirthHdf //
//-----------------------------------//

int
L2B::GetArraysForUpdatingDirthHdf(
    float**  spd,
    float**  dir,
    int**    num_ambig)
{
    WindSwath* swath = &(frame.swath);
    int along_track_bins = swath->GetAlongTrackBins();
    int cross_track_bins = swath->GetCrossTrackBins();

    for (int cti = 0; cti < cross_track_bins; cti++)
    {
        for (int ati = 0; ati < along_track_bins; ati++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (! wvc)
            {
                num_ambig[ati][cti] = 0;
            }
            else
            {
                int k = 0;
                num_ambig[ati][cti] = wvc->ambiguities.NodeCount();
                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    spd[ati][cti*HDF_NUM_AMBIGUITIES+k] = wvp->spd;
                    dir[ati][cti*HDF_NUM_AMBIGUITIES+k] = wvp->dir;
                    k++;
                }
            }
        }
    }
    return(1);
}

//-----------------------//
// L2B::_OpenHdfDataSets //
//-----------------------//

int
L2B::_OpenHdfDataSets(
    TlmHdfFile*  tlmHdfFile)
{
    if ((_numambigSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_AMBIGS)) == 0)
    {
        return(0);
    }
    if ((_lonSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LON)) == 0)
    {
        return(0);
    }
    if ((_latSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LAT)) == 0)
    {
        return(0);
    }
    if ((_speedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WIND_SPEED)) == 0)
    {
        return(0);
    }
    if ((_dirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WIND_DIR)) == 0)
    {
        return(0);
    }
    if ((_modelSpeedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_SPEED)) == 0)
    {
        return(0);
    }
    if ((_modelDirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_DIR)) == 0)
    {
        return(0);
    }
    if ((_mleSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MAX_LIKELIHOOD_EST)) == 0)
    {
        return(0);
    }
    if ((_selectSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WVC_SELECTION)) == 0)
    {
        return(0);
    }
    if ((_numInForeSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_IN_FORE)) == 0)
    {
        return(0);
    }
    if ((_numInAftSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_IN_AFT)) == 0)
    {
        return(0);
    }
    if ((_numOutForeSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_OUT_FORE)) == 0)
    {
        return(0);
    }
    if ((_numOutAftSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_OUT_AFT)) == 0)
    {
        return(0);
    }
    if ((_mpRainProbSdsId = _OpenOneHdfDataSetCorrectly(tlmHdfFile,
        "mp_rain_probability")) == 0)
    {
        return(0);
    }

    if ((_qualSdsId = _OpenOneHdfDataSetCorrectly(tlmHdfFile,
        "wvc_quality_flag")) == 0)
    {
        return(0);
    }
    return(1);
}

//-------------------------//
// L2B::_OpenOneHdfDataSet //
//-------------------------//

int
L2B::_OpenOneHdfDataSet(
    TlmHdfFile*  tlmHdfFile,
    SourceIdE    source,
    ParamIdE     param)
{
    const char* sdsName = ParTabAccess::GetSdsNames(source, param);
    if (sdsName == 0)
        return(0);

    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 sdsId = tlmHdfFile->SelectDataset(sdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (sdsId == HDF_FAIL)
        return(0);
    else
        return(sdsId);
}

//-------------------//
// L2B::ReadHDFDIRTH //
//-------------------//

int
L2B::ReadHDFDIRTH(
    const char*  filename)
{
    //--------------------------------//
    // convert filename to TlmHdfFile //
    //--------------------------------//

    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile l2b_hdf_file(filename, SOURCE_L2B, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

    // Open Data Sets
    int32 dirId;
    int32 spdId;
    if ((dirId = _OpenOneHdfDataSetCorrectly(&l2b_hdf_file,
        "wind_dir_selection")) == 0)
    {
        return(0);
    }
    if ((spdId = _OpenOneHdfDataSetCorrectly(&l2b_hdf_file,
        "wind_speed_selection")) == 0)
    {
        return(0);
    }

    int along_track_bins=frame.swath.GetAlongTrackBins();
    int cross_track_bins=frame.swath.GetCrossTrackBins();

    // create arrays
    float* speed = new float[cross_track_bins];
    float* dir = new float[cross_track_bins];

    for (int32 i = 0; i < along_track_bins; i++)
    {
        if (ExtractData2D_76_int2_float(&l2b_hdf_file, &spdId, i, 1, 1,
            speed) == 0)
        {
            return(0);
        }
        if (ExtractData2D_76_uint2_float(&l2b_hdf_file, &dirId, i, 1, 1,
            dir) == 0)
        {
            return(0);
        }
        for (int j = 0; j < cross_track_bins; j++)
        {
            WVC* wvc= frame.swath.GetWVC(j, i);
            if (! wvc)
                continue;
            if (! wvc->selected)
                continue;
            wvc->selected->spd = speed[j];
            float edir = gs_deg_to_pe_rad(dir[j]);
            wvc->selected->dir = edir;
        }
    }
    delete [] speed;
    delete [] dir;
    (void)SDendaccess(dirId);
    (void)SDendaccess(spdId);
    return(1);
}

//----------------------------------//
// L2B::_OpenOneHdfDataSetCorrectly //
//----------------------------------//

int
L2B::_OpenOneHdfDataSetCorrectly(
    TlmHdfFile*  tlmHdfFile,
    const char*  sdsName)
{
    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 sdsId = tlmHdfFile->SelectDataset(sdsName, dataType, dataStartIndex,
        dataLength, numDimensions);
    if (sdsId == HDF_FAIL)
        return(0);
    else
        return(sdsId);
}

//------------------------//
// L2B::_CloseHdfDataSets //
//------------------------//

void
L2B::_CloseHdfDataSets(void)
{
    (void)SDendaccess(_numambigSdsId); _numambigSdsId = HDF_FAIL;
    (void)SDendaccess(_lonSdsId); _lonSdsId = HDF_FAIL;
    (void)SDendaccess(_latSdsId); _latSdsId = HDF_FAIL;
    (void)SDendaccess(_speedSdsId); _speedSdsId = HDF_FAIL;
    (void)SDendaccess(_dirSdsId); _dirSdsId = HDF_FAIL;
    (void)SDendaccess(_mleSdsId); _mleSdsId = HDF_FAIL;
    (void)SDendaccess(_selectSdsId); _selectSdsId = HDF_FAIL;
    (void)SDendaccess(_numInForeSdsId); _numInForeSdsId = HDF_FAIL;
    (void)SDendaccess(_numInAftSdsId); _numInAftSdsId = HDF_FAIL;
    (void)SDendaccess(_numOutForeSdsId); _numOutForeSdsId = HDF_FAIL;
    (void)SDendaccess(_numOutAftSdsId); _numOutAftSdsId = HDF_FAIL;
    (void)SDendaccess(_modelSpeedSdsId); _modelSpeedSdsId = HDF_FAIL;
    (void)SDendaccess(_modelDirSdsId); _modelDirSdsId = HDF_FAIL;
    (void)SDendaccess(_mpRainProbSdsId); _mpRainProbSdsId = HDF_FAIL;
    (void)SDendaccess(_qualSdsId); _qualSdsId = HDF_FAIL;
    return;
}

//--------------------------//
// L2B::GetNumCellsSelected //
//--------------------------//

int
L2B::GetNumCellsSelected()
{
    return(frame.swath.GetNumCellsSelected());
}

//---------------------------------//
// L2B::GetNumCellsWithAmbiguities //
//---------------------------------//

int
L2B::GetNumCellsWithAmbiguities()
{
    return(frame.swath.GetNumCellsWithAmbiguities());
}
