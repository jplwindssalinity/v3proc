//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_seapac_c[] =
    "@(#) $Id$";

//-------//
// ijbin //
//-------//

#include "Constants.h"

#define EARTH_A           6.3781363E6    // meters
#define EPSILON           1E-30
#define L2A_ATRACK_GRIDS  1624
#define L2A_XTRACK_GRIDS  76

int
ijbin(
    double  orb_smaj_axis,
    double  orb_eccen,
    double  orb_inclination,
    double  long_asc_node,
    double  arg_lat,
    double  nodal_period,
    int     joffset,
    double  atrack_bin_const,
    double  xtrack_bin_const,
    double  meas_lon,
    double  meas_lat)
{
    //--------------------------------------------------------//
    // Set some common conversions of orbit element variables //
    //--------------------------------------------------------//

    double aa = orb_smaj_axis;
    double ecc = orb_eccen;
    double inc = orb_inclination * dtr;
    double cosi = cos(inc);
    double sini = sin(inc);
    double lzero = long_asc_node * dtr;
    double arglat = arg_lat * dtr;
    double arglmod = arglat + pi_over_two;
    if (arglmod > two_pi)
        arglmod = arglmod - two_pi;
 
    //--------------------------------------------//
    // Compute orbit and nodal precession periods //
    //--------------------------------------------//

    double arat = aa / EARTH_A;   // axis to radius ratio
    double slr = arat * (1.0 - ecc*ecc);    // normalized orbit ellipse
 
    if (fabs(nodal_period) < EPSILON)
    {
        fprintf(stderr, "Nodal period too small\n");
        return(0);
    }
 
    if (fabs(slr) < EPSILON)
    {
        fprintf(stderr, "SLR too small (divide by zero)\n");
        return(0);
    }

    double pnode = -1.5 * two_pi * rj2 * cosi / (nodal_period / (slr * slr));

    if (fabs(wa - pnode) < EPSILON)
    {
        fprintf(stderr, "Nodal quantity too small\n");
        return(0);
    }

    double p1 = two_pi / (wa - pnode);    // earth period
    double prat = nodal_period / p1;    // period ratio
 
    //-----------------------------------------------//
    // Compute bin coordinates for each cell in beam //
    //-----------------------------------------------//

    double mlon = meas_lon * dtr;    // cell longitude in radians
    double mlat = meas_lat * dtr;    // cell latitude in radians
    double snlat = sin(mlat);
    double cslat = cos(mlat);

    if (fabs(cslat) < EPSILON)
    {
        fprintf(stderr, "cslat too small\n");
        return(0);
    }

    double tnlat = snlat / cslat;
 
    //------------------------------------------------------------------//
    // Get a trial value of along-track longitude to seed the iteration //
    //------------------------------------------------------------------//

    int ascend = 1;
    double lnode = lzero;    // ascending node longitude
    double lit0 = mlon - lnode;  // nadir node longitude difference
    double cslit0 = cos(lit0);
    double snlit0 = sin(lit0);

    if (fabs(cslit0) <  EPSILON)
    {
        fprintf(stderr, "cslit0 too small\n");
        return(0);
    }

    if (cslit0 < 0.0)
        ascend = 0;
 
    // along-track longitude
    double lip = atan((cosi*snlit0 + sini*tnlat) / cslit0);
 
    if (! ascend)
        lip = lip + pi;

    if (lip < 0.0)
        lip = lip + two_pi;

    lip = lip + pi_over_two;

    if (lip > two_pi)
        lip = lip - two_pi;

    //-----------------------------------------------//
    // Begin iteration to get along-track longitude  //
    // Compute new value for along-track `longitude' //
    //-----------------------------------------------//

    double diff = 1.0;    // set initial value greater than tolerance
    double slt = 0.0;

    while (diff > 1.0e-5)
    {
        diff = lip - arglmod;    // angular difference
        double d = pi - fabs(pi - fabs(diff));
 
        double ddif = 0.0;
        if (fabs(diff) > pi)
        {
            if (diff < -pi)
                ddif = d;    // angular difference minimum
            if (diff > pi)
                ddif = -d;
        }
        else
        {
            ddif = diff;
        }

        lnode = lzero - prat*ddif;    // ascending node longitude
        double lit = mlon - lnode;    // cell node longitude difference
        slt = sin(lit);
        double clt = cos(lit);
        ascend = 1;
 
        if (fabs(clt) <  EPSILON)
        {
            fprintf(stderr, "abs_clt too small\n");
            return(0);
        }

        if (clt < 0.0)
            ascend = 0;

        double lip1 = atan((cosi*slt + (1.0 - e2) * sini*tnlat) / clt);
  
        if (! ascend)
            lip1 = lip1 + pi;

        if (lip1 < 0.0)
            lip1 = lip1 + two_pi;

        lip1 = lip1 + pi_over_two;
  
        if (lip1 > two_pi)
            lip1 = lip1 - two_pi;
 
        //--------------------------------//
        // Check convergence of iteration //
        //--------------------------------//

        diff = fabs(lip - lip1);    // angular difference
        lip = lip1;    // along-track longitude
    }

    //----------------------------------------------------//
    // Iteration to get along-track longitude is complete //
    // Now compute cross-track `latitude'                 //
    //----------------------------------------------------//
 
    double sphipi = (1.0 - e2)*cosi*snlat - sini*cslat*slt;
    sphipi = sphipi / sqrt(1.0 - e2*snlat*snlat);
    double phipi = asin(sphipi);    // cross-track latitude
 
    //-----------------------------------------------------//
    // Compute bin coordinates from binning formulae       //
    // Compute modified WVC_I for south polar rev boundary //
    //-----------------------------------------------------//

    double dat = lip / atrack_bin_const;    // along-track distance
    // int wvc_i = nint(dat + 0.5);
    int wvc_i = (int)floor(dat + 1.0);    // along-track coordinate

    if (wvc_i > L2A_ATRACK_GRIDS)
        wvc_i = wvc_i - L2A_ATRACK_GRIDS;

    //------------------------------------------------------------//
    // Compute modified WVC_J for uniform grid                    //
    // JPRIME = cell coordinate with respect to subtrack "cell 0" //
    //------------------------------------------------------------//

    double dct = phipi / xtrack_bin_const;    // cross-track distance
    // wvc_j = nint(dct - 0.5);
    int wvc_j = (int)floor(dct);

    //-------------------------------------------------------------//
    // Final cross-track coordinate                                //
    // Reverse sign to conform to SeaWinds swath coordinate system //
    //-------------------------------------------------------------//

    wvc_j = -wvc_j + joffset;    // cross-track coordinate
 
    //-------------------------------------------//
    // Make sure that the cross-track coordinate //
    // is within 1,L2A_XTRACK_GRIDS range        //
    //-------------------------------------------//

    if (wvc_j < 1)
    {
        wvc_j = 1;
    }
    else if (wvc_j > L2A_XTRACK_GRIDS)
    {
        wvc_j = L2A_XTRACK_GRIDS;
    }
    return(1);
}

/*
subroutine SWS_IJBIN(   &
      glb,              &
      meas_lat,         &
      meas_lon,         &
      arg_lat,          &
      long_asc_node,    &
      nodal_period,     &
      orb_eccen,        & 
      orb_smaj_axis,    &
      orb_inclination,  &
      atrack_grid,      &
      xtrack_grid,      &
      atrack_bin_const, &
      xtrack_bin_const, &
      joffset,          & 
      return_status,    &
      wvc_i,            &
      wvc_j)

   use  Sws_Smf 
   use  Sws_Sws_45090  
   use Proc_Tables

   implicit none            
 
!
! Input Parameters
!

   TYPE (Global_Constants_Real_Type), intent(in) ::  glb  

   real,    intent(in) :: meas_lat     
   real,    intent(in) :: meas_lon     
   real,    intent(in) :: arg_lat          
   real,    intent(in) :: long_asc_node    
   real,    intent(in) :: nodal_period     
   real,    intent(in) :: orb_eccen        
   real,    intent(in) :: orb_smaj_axis    
   real,    intent(in) :: orb_inclination
   integer, intent(in) :: atrack_grid
   integer, intent(in) :: xtrack_grid
   real,    intent(in) :: atrack_bin_const
   real,    intent(in) :: xtrack_bin_const
   integer, intent(in) :: joffset
 
!
! Input/Output Parameters
!
   integer, intent(inout) :: return_status 
 
!
! Output Parameters
!
   integer, intent(out) :: wvc_i      
   integer, intent(out) :: wvc_j     
 
!
! Local Declarations
!

   real, parameter :: EPSILON = 1e-30 ! value used for error testing
                                      ! needs to be a very small number
 
   integer                 :: handler_status   ! message handler status
   character(MAX_MSG_SIZE) :: message          ! message string

! calling subroutine
   character(32)           :: module_name = 'SWS_IJBIN'
 
   logical :: ascend           ! ascending node flag
   real    :: aa               ! orb_smaj_axis
   real    :: ecc              ! orb_eccen
   real    :: lzero            ! long_asc_node   (radians)
   real    :: arglat           ! arg_lat         (radians)
   real    :: inc              ! orb_inclination (radians)

   real    :: cosi             ! cos(inc)
   real    :: sini             ! sin(inc)
 
   real    :: arglmod          ! modified argument of latitude
 
   real    :: arat             ! axis to radius ratio
   real    :: slr              ! normalized orbit ellipse
 
   real    :: prat             ! period ratio
   real    :: p1               ! earth period wrt to node
   real    :: pnode            ! ascending node rate

   real    :: mlat             ! meas_lat (radians)
   real    :: mlon             ! meas_lon (radians)
 
   real    :: snlat            ! sin(mlat)
   real    :: cslat            ! cos(mlat)
   real    :: tnlat            ! tan(mlat)
 
   real    :: lnode            ! ascending node longitude
   real    :: lit0             ! nadir node difference longitude
 
   real    :: cslit0           ! cos(lit0)
   real    :: snlit0           ! sin(lit0)
 
   real    :: lip              ! along-track longitude
   real    :: lip1             ! along-track longitude temp variable
 
   real    :: diff             ! angular difference
   real    :: ddif             ! angular difference min
   real    :: d                ! angular difference temp variable
 
   real    :: lit              ! cell node longitude difference
   real    :: slt              ! sin(lit)
   real    :: clt              ! cos(lit)
 
   real    :: phipi            ! cross-track latitude
   real    :: sphipi           ! cross-track latitiude temp variable
 
   real    :: dat              ! along-track distance
   real    :: dct              ! cross-track distance
 
!
! Function Declarations
!

   integer :: SWS_Get_Msg      ! SeaWinds error handling function
   integer :: SWS_Dynamic_Msg  ! SeaWinds error handling function
   logical :: SWS_Status_Test  ! SeaWinds error handling function
 
!
!     Initialize variables.
!
   return_status =  S_SUCCESS

 
!
!     Set some common conversions of orbit element variables.
!

   aa      = orb_smaj_axis

   ecc     = orb_eccen

   inc     = orb_inclination*glb%deg_to_rad

   cosi    = cos(inc)

   sini    = sin(inc)

   lzero   = long_asc_node*glb%deg_to_rad

   arglat  = arg_lat*glb%deg_to_rad    ! NEW 9/29/93

   arglmod = arglat + glb%half_pi      ! to correspond to S.P.rev 10/4/93

   if (arglmod > glb%two_pi) arglmod = arglmod - glb%two_pi
 
!
! Compute orbit and nodal precession periods.
!

   arat  = aa / glb%earth_a         ! axis to radius ratio
   slr   = arat*(1.e0 - ecc*ecc)    ! normalized orbit ellipse
 
   nodal_period_too_small:  if (abs(nodal_period) < EPSILON) then

      handler_status = SWS_Get_Msg(SWS_E_DIVIDE_BY_ZERO,             &
          message,module_name,return_status)

      call SWS_Fill_Char_Field(handler_status,message,message,       &
          'nodal period')

      handler_status = SWS_Dynamic_Msg(                              &
          handler_status,return_status,message,module_name)

   endif   nodal_period_too_small
 
   chk_nodal_period_status: if (.not. SWS_Status_Test(return_status, &
                                                      MASK_LEV_E)) then

      chk_slr_too_small: if (abs(slr) <  EPSILON) then

         handler_status = SWS_Get_Msg(SWS_E_DIVIDE_BY_ZERO,          &
            message,module_name,return_status)

         call SWS_Fill_Char_Field(handler_status,message,message,    &
            'normalized orbit ellipse')

         handler_status = SWS_Dynamic_Msg(                           &
            handler_status,return_status,message,module_name)

      endif chk_slr_too_small

   
      chk_slr_status: if (.not. SWS_Status_Test(return_status,       &
                                                MASK_LEV_E)) then
 
         pnode = -1.5*glb%two_pi*glb%earth_j2_coeff*cosi  &
                 /(nodal_period/slr*slr)

         nodal_quantity_too_small: if (abs(glb%earth_angular_spd_rad     &
                                       - pnode) <   EPSILON)    then

            handler_status = SWS_Get_Msg(SWS_E_DIVIDE_BY_ZERO,          &
                message,module_name,return_status)

            call SWS_Fill_Char_Field(handler_status,message,message,    &
                'quantity used to calculate earth period wrt to node')

            handler_status = SWS_Dynamic_Msg(                           &
                handler_status,return_status,message,module_name)
  
         endif  nodal_quantity_too_small
 
         chk_nodal_quantity_status: if (.not. SWS_Status_Test(return_status,  &
                                                              MASK_LEV_E)) then

            p1 = glb%two_pi/(glb%earth_angular_spd_rad - pnode) ! earth period

            prat  = nodal_period/p1                             ! period ratio
 
!
! Compute bin coordinates for each cell in beam.
!

            mlon  = meas_lon * glb%deg_to_rad  ! cell longitude in radians

            mlat  = meas_lat * glb%deg_to_rad  ! cell latitude in radians

            snlat = sin(mlat)

            cslat = cos(mlat)
 
            cslat_too_small: if (abs(cslat) < EPSILON)    then

               handler_status = SWS_Get_Msg(SWS_E_DIVIDE_BY_ZERO,              &
                                            message,module_name,return_status)

               call SWS_Fill_Char_Field(handler_status,message,message,        &
                                       'cosine of cell latitude')

               handler_status = SWS_Dynamic_Msg(handler_status,return_status,  &
                                                message,module_name)

            endif  cslat_too_small
 
            chk_cslat_status: if (.not. SWS_Status_Test(return_status,         &
                                                        MASK_LEV_E)) then

               tnlat = snlat/cslat
 
!
! Get a trial value of along-track longitude to seed the iteration.
!

               ascend = .true.

               lnode  = lzero                ! ascending node longitude

               lit0   = mlon - lnode         ! nadir node longitude difference

               cslit0 = cos(lit0)

               snlit0 = sin(lit0)
 
               cslit0_too_small: if (abs(cslit0) <  EPSILON) then

                  handler_status = SWS_Get_Msg(SWS_E_DIVIDE_BY_ZERO,           &
                                              message,module_name,return_status)
 
                  call SWS_Fill_Char_Field(handler_status,message,message,     &
                                    'cosine of nadir node longitude difference')

                  handler_status = SWS_Dynamic_Msg(handler_status,             &
                                                   return_status,              &
                                                   message,module_name)

               endif  cslit0_too_small
 
               chk_cslit0_status: if (.not. SWS_Status_Test(return_status, &
                                                             MASK_LEV_E)) then

                  if (cslit0 < 0.0)  ascend = .false.
 
                  lip = atan((cosi*snlit0 + sini*tnlat)/cslit0) ! along-track
                                                                ! longitude
 
                  if (.not. ascend) lip = lip + glb%pi

                  if (lip <  0.0)   lip = lip + glb%two_pi

                  lip = lip + glb%half_pi

                  if (lip >  glb%two_pi)  lip = lip - glb%two_pi

!
! Begin iteration to get along-track longitude.
! Compute new value for along-track `longitude'.
!

                  diff = 1.e0               ! set initial value greater
                                            ! than tolerance

                  atrack_longitude_loop: do while (diff > 1.0e-5)
 
                     diff = lip - arglmod   ! angular difference

                     d = glb%pi - abs(glb%pi - abs(diff))
 

! Scott Dunbar and Vincent Hsiao made changes from here 
! through abs_clt_too_small:, in order to properly handle
! problems at beginning and end of rev.

                     if (abs(diff) > glb%pi) then

                        if (diff < -glb%pi) ddif = d            ! angular difference minimum
                        if (diff >  glb%pi) ddif = -d

                     else  

                        ddif = diff

                     endif  

                     lnode  = lzero - prat*ddif ! ascending node longitude

                     lit    = mlon - lnode      ! cell node longitude difference

                     slt    = sin(lit)

                     clt    = cos(lit)

                     ascend = .true.
 
                     abs_clt_too_small: if (abs(clt) <  EPSILON)   then

                        handler_status = SWS_Get_Msg(SWS_E_DIVIDE_BY_ZERO,   &
                                                     message,module_name,    &
                                                     return_status)

                        call SWS_Fill_Char_Field(handler_status,message,     &
                                                 message,                    &
                            'cosine of cell node longitude difference')

                        handler_status = SWS_Dynamic_Msg(handler_status,     &
                                                         return_status,      &
                                                         message,            &
                                                         module_name)

                     endif  abs_clt_too_small
 
!
! Exit loop on error else continue.
!


                     if( SWS_Status_Test(return_status, MASK_LEV_E))  &
                                                    exit atrack_longitude_loop
  
                     if (clt < 0.0) ascend = .false.

                     lip1 = atan((cosi*slt  + (1.e0 - glb%earth_e_sq)  &
                                            * sini*tnlat)/clt)
  
                     if (.not. ascend) lip1 = lip1 + glb%pi

                     if (lip1 < 0.0) lip1 = lip1 + glb%two_pi

                     lip1 = lip1 + glb%half_pi
  
                     if (lip1 > glb%two_pi) lip1 = lip1 - glb%two_pi
 
!
! Check convergence of iteration.
!

                     diff = abs(lip - lip1) ! angular difference

                     lip  = lip1            ! along-track longitude
 
                  end do  atrack_longitude_loop
 
!
! Iteration to get along-track longitude is complete.
! Now compute cross-track `latitude'.
!

              
                 chk_err_status: if(.not. SWS_Status_Test(return_status,   &
                                                        MASK_LEV_E))  then

                    sphipi = (1.e0 - glb%earth_e_sq)*cosi*snlat - sini*cslat*slt

                    sphipi = sphipi/sqrt (1.e0 - glb%earth_e_sq*snlat*snlat)

                     phipi = asin(sphipi)         ! cross-track latitude
 
!
! Compute bin coordinates from binning formulae.
! Compute modified WVC_I for south polar rev boundary.
!

                     dat   = lip/atrack_bin_const   ! along-track distance

                     wvc_i = nint(dat + 0.5)        ! along-track coordinate

                     if (wvc_i > atrack_grid) wvc_i = wvc_i - atrack_grid 

!
! Compute modified WVC_J for uniform grid.
! JPRIME = cell coordinate with respect to subtrack "cell 0".
!
 
                     dct = phipi/xtrack_bin_const    ! cross-track distance

                     wvc_j = nint(dct - 0.5)

!
! Final cross-track coordinate 
! Reverse sign to conform to SeaWinds swath coordinate system.
!

                     wvc_j = - wvc_j + joffset         ! cross-track coordinate
 
!
! Make sure that the cross-track coordinate is within 1,L2A_XTRACK_GRIDS range.
!

                     small_wvc_j:  if (wvc_j < 1) then

                        wvc_j = 1

                     else if (wvc_j > xtrack_grid) then

                        wvc_j = xtrack_grid   

                     endif   small_wvc_j

                  endif chk_err_status

               endif  chk_cslit0_status

            endif  chk_cslat_status
 
         endif  chk_nodal_quantity_status

      endif chk_slr_status

   endif   chk_nodal_period_status   

   return

end  subroutine SWS_IJBIN
*/
