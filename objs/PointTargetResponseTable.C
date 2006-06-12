#include"PointTargetResponseTable.h"
#include"Constants.h"

PointTargetResponseTable::PointTargetResponseTable()
{
  nData = new int[NBEAMS];
  nAux = new int[NBEAMS];

  int ii, jj;

  time = new float*[NBEAMS];
  scanAngle = new float*[NBEAMS];
  nRngPixel = new int*[NBEAMS];
  nAzPixel = new int*[NBEAMS];

  for (ii=0; ii<NBEAMS; ii++) {
    time[ii] = new float[AUX_MAX_LINE];
    scanAngle[ii] = new float[AUX_MAX_LINE];
    nRngPixel[ii] = new int[AUX_MAX_LINE];
    nAzPixel[ii] = new int[AUX_MAX_LINE];
  }

  scPos = new float**[NBEAMS];
  tarPos = new float**[NBEAMS];
  rngUnit = new float**[NBEAMS];
  azUnit = new float**[NBEAMS];

  for (ii=0; ii<NBEAMS; ii++) {
    scPos[ii] = new float*[AUX_MAX_LINE];
    tarPos[ii] = new float*[AUX_MAX_LINE];
    rngUnit[ii] = new float*[AUX_MAX_LINE];
    azUnit[ii] = new float*[AUX_MAX_LINE];
  }

  for (ii=0; ii<NBEAMS; ii++) {
    for (jj=0; jj<AUX_MAX_LINE; jj++) {
      scPos[ii][jj] = new float[3];
      tarPos[ii][jj] = new float[3];
      rngUnit[ii][jj] = new float[3];
      azUnit[ii][jj] = new float[3];
    }
  }

  rngOffset = new float*[NBEAMS];
  azOffset = new float*[NBEAMS];
  semiMinorWidth = new float*[NBEAMS];
  semiMajorWidth = new float*[NBEAMS];

  for (ii=0; ii<NBEAMS; ii++) {
    rngOffset[ii] = new float[DATA_MAX_LINE];
    azOffset[ii] = new float[DATA_MAX_LINE];
    semiMinorWidth[ii] = new float[DATA_MAX_LINE];
    semiMajorWidth[ii] = new float[DATA_MAX_LINE];
  }
}


PointTargetResponseTable::~PointTargetResponseTable()
{
  delete [] nData;
  delete [] nAux;

  int ii, jj;

  for (ii=0; ii<NBEAMS; ii++) {
    delete [] time[ii];
    delete [] scanAngle[ii];
    delete [] nRngPixel[ii];
    delete [] nAzPixel[ii];
  }

  delete [] time;
  delete [] scanAngle;
  delete [] nRngPixel;
  delete [] nAzPixel;

  for (ii=0; ii<NBEAMS; ii++) {
    for (jj=0; jj<AUX_MAX_LINE; jj++) {
      delete [] scPos[ii][jj];
      delete [] tarPos[ii][jj];
      delete [] rngUnit[ii][jj];
      delete [] azUnit[ii][jj];
    }
  }

  for (ii=0; ii<NBEAMS; ii++) {
    delete [] scPos[ii];
    delete [] tarPos[ii];
    delete [] rngUnit[ii];
    delete [] azUnit[ii];
  }

  delete [] scPos;
  delete [] tarPos;
  delete [] rngUnit;
  delete [] azUnit;

  for (ii=0; ii<NBEAMS; ii++) {
    delete [] rngOffset[ii];
    delete [] azOffset[ii];
    delete [] semiMajorWidth[ii];
    delete [] semiMinorWidth[ii];
  }

  delete [] rngOffset;
  delete [] azOffset;
  delete [] semiMajorWidth;
  delete [] semiMinorWidth;
}


int PointTargetResponseTable::ReadAux(char* filename, int beam_num)
{
  int ii, bn;
  FILE *miscFileP;

  miscFileP = fopen(filename,"r");
  if (miscFileP==NULL) {
    fprintf(stderr,"PointTargetResponseTable: ReadAux : no aux file\n");
    return (0);
  }

  bn = beam_num-1;

  ii = 0;
  while (!feof(miscFileP)) {
    fscanf(miscFileP, "%f %f %d %d %f %f %f %f %f %f %f %f %f %f %f %f\n",
           &time[bn][ii], &scanAngle[bn][ii], &nRngPixel[bn][ii], &nAzPixel[bn][ii],
           &scPos[bn][ii][0], &scPos[bn][ii][1], &scPos[bn][ii][2],
           &tarPos[bn][ii][0], &tarPos[bn][ii][1], &tarPos[bn][ii][2],
           &rngUnit[bn][ii][0], &rngUnit[bn][ii][1], &rngUnit[bn][ii][2],
           &azUnit[bn][ii][0], &azUnit[bn][ii][1], &azUnit[bn][ii][2]);
           //cout << time[bn][ii] << endl;
           //cout << scanAngle[bn][ii] << endl;
           //cout << nRngPixel[bn][ii] << endl;
           //cout << nAzPixel[bn][ii] << endl;
           //cout << scPos[bn][ii][0] << endl;
           //cout << scPos[bn][ii][1] << endl;
           //cout << scPos[bn][ii][2] << endl;
           //cout << tarPos[bn][ii][0] << endl;
           //cout << tarPos[bn][ii][1] << endl;
           //cout << tarPos[bn][ii][2] << endl;
           //cout << rngUnit[bn][ii][0] << endl;
           //cout << rngUnit[bn][ii][1] << endl;
           //cout << rngUnit[bn][ii][2] << endl;
           //cout << azUnit[bn][ii][0] << endl;
           //cout << azUnit[bn][ii][1] << endl;
           //cout << azUnit[bn][ii][2] << endl;
           ii++;
  }

  nAux[bn] = ii;
  cout << "Number of line in misc file: " << nAux[bn] << endl;

  return 1;
}


int PointTargetResponseTable::ReadData(char* filename, int beam_num)
{
  int ii, bn;
  float dd[4];
  FILE *dataFileP;

  dataFileP = fopen(filename,"r");
  if (dataFileP==NULL) {
    fprintf(stderr,"PointTargetResponseTable: ReadData : no data file\n");
    return 0;
  }

  bn = beam_num - 1;

  ii = 0;
  while(!feof(dataFileP)) {
    fread(dd, sizeof(float), 4, dataFileP);
    rngOffset[bn][ii] = dd[0];
    azOffset[bn][ii] = dd[1];
    semiMinorWidth[bn][ii] = dd[2];
    semiMajorWidth[bn][ii] = dd[3];
    //cout << rngOffset[bn][ii] << endl;
    //cout << azOffset[bn][ii] << endl;
    //cout << semiMajorWidth[bn][ii] << endl;
    //cout << semiMinorWidth[bn][ii] << endl;
    ii++;
  }

  nData[bn] = ii-1;
  cout << "Number of line in data file: " << nData[bn] << endl;

  return(1);
}


float PointTargetResponseTable::GetSemiMinorWidth(
                        float range_km, float azimuth_km,
                        float scan_angle_rad, float orbit_time_in_rev_s,
                        int beam_num)
{
  int bn, timeIdx, angIdx, rngIdx, azIdx, selectIdx1, selectIdx2;
  float frac, intpWidth;

  bn = beam_num - 1;

  /* find index */

  scan_angle_rad=scan_angle_rad+pi/2;
  if(scan_angle_rad>two_pi) scan_angle_rad-=two_pi;

  timeIdx = int(orbit_time_in_rev_s/TIME_STEP);
  timeIdx = 0; // now, for appling result of one rev
  angIdx = int(scan_angle_rad*rtd/ANGLE_STEP);
  rngIdx = int(range_km/RNG_STEP_SIZE)+N_RNG_BINS/2;
  azIdx = int(azimuth_km/AZ_STEP_SIZE)+N_AZ_BINS/2;

  frac = scan_angle_rad*rtd/ANGLE_STEP - angIdx;

  selectIdx1 = timeIdx*N_ANG_STEPS*N_RNG_BINS*N_AZ_BINS
              + angIdx*N_RNG_BINS*N_AZ_BINS
              + rngIdx*N_AZ_BINS
              + azIdx;

  if (scan_angle_rad*rtd < 360.-ANGLE_STEP) { // next angle Idx
    selectIdx2 = selectIdx1 + N_RNG_BINS*N_AZ_BINS;
  } else { // use angle idx 0
    selectIdx2 = timeIdx*N_ANG_STEPS*N_RNG_BINS*N_AZ_BINS
                + rngIdx*N_AZ_BINS
                + azIdx;
  }

  //cout << selectIdx1 << " " << semiMinorWidth[bn][selectIdx1] << endl;
  //cout << selectIdx2 << " " << semiMinorWidth[bn][selectIdx2] <<endl;
  intpWidth = (1.-frac)*semiMinorWidth[bn][selectIdx1] + frac*semiMinorWidth[bn][selectIdx2];
  //cout << frac << " " << intpWidth << endl;
  //return semiMinorWidth[bn][selectIdx];
  return intpWidth;

}


float PointTargetResponseTable::GetSemiMajorWidth(
                        float range_km, float azimuth_km,
                        float scan_angle_rad, float orbit_time_in_rev_s,
                        int beam_num)
{
  int bn, timeIdx, angIdx, rngIdx, azIdx, selectIdx1, selectIdx2;
  float frac, intpWidth;

  bn = beam_num - 1;

  /* find index */

  scan_angle_rad=scan_angle_rad+pi/2;
  if(scan_angle_rad>two_pi) scan_angle_rad-=two_pi;

  timeIdx = int(orbit_time_in_rev_s/TIME_STEP);
  timeIdx = 0; // now, for appling result of one rev
  angIdx = int(scan_angle_rad*rtd/ANGLE_STEP);
  rngIdx = int(range_km/RNG_STEP_SIZE)+N_RNG_BINS/2;
  azIdx = int(azimuth_km/AZ_STEP_SIZE)+N_AZ_BINS/2;

  frac = scan_angle_rad*rtd/ANGLE_STEP - angIdx;

  selectIdx1 = timeIdx*N_ANG_STEPS*N_RNG_BINS*N_AZ_BINS
              + angIdx*N_RNG_BINS*N_AZ_BINS
              + rngIdx*N_AZ_BINS
              + azIdx;

  if (scan_angle_rad*rtd < 360.-ANGLE_STEP) { // next angle Idx
    selectIdx2 = selectIdx1 + N_RNG_BINS*N_AZ_BINS; 
  } else { // use angle idx 0
    selectIdx2 = timeIdx*N_ANG_STEPS*N_RNG_BINS*N_AZ_BINS
                + rngIdx*N_AZ_BINS
                + azIdx;
  }

  //cout << selectIdx1 << " " << semiMajorWidth[bn][selectIdx1] << endl;
  //cout << selectIdx2 << " " << semiMajorWidth[bn][selectIdx2] << endl;
  intpWidth = (1.-frac)*semiMajorWidth[bn][selectIdx1] + frac*semiMajorWidth[bn][selectIdx2];
  //cout << frac << " " << intpWidth << endl;
  //return semiMajorWidth[bn][selectIdx];
  return intpWidth;

}

