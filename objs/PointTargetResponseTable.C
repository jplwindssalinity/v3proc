#include"PointTargetResponseTable.h"

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

  return 0;
}


int PointTargetResponseTable::ReadData(char* filename, int beam_num)
{
  int ii, bn;
  float dd[4];
  FILE *dataFileP;

  dataFileP = fopen(filename,"r");
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

  return(0);
}


float PointTargetResponseTable::GetSemiMajorWidth(
                        float range_km, float azimuth_km,
                        float scan_angle_rad, float orbit_time_in_rev_s,
                        int beam_num)
{
  int ii, timeIdx, angIdx, bn;
  float pi;

  pi = atan(1.)*4.;
  bn = beam_num - 1;

  /* search time from misc time data */

  float offset, minTimeOffset;

  timeIdx = -1000;
  minTimeOffset = 1.e10;

  for (ii=0; ii<nAux[bn]; ii++) {
    offset = fabs(orbit_time_in_rev_s-time[bn][ii]);
    if (offset<minTimeOffset) {
      minTimeOffset = offset;
      timeIdx = ii;
    }
  }
  //cout << timeIdx << endl;
  //cout << minTimeOffset << endl;

  /* search angle from misc angle data                            */
  /* searching area is around the index obtained from time search */

  int angMinIdx, angMaxIdx;
  float minAngOffset;

  angMinIdx = timeIdx - 10;
  angMaxIdx = timeIdx + 10;
  if (angMinIdx < 0) angMinIdx = 0;
  if (angMaxIdx > nAux[bn]) angMaxIdx = nAux[bn];

  angIdx = -1000;
  minAngOffset = 1.e10;

  for (ii=angMinIdx; ii<angMaxIdx; ii++) {
    offset = fabs(scan_angle_rad*180./pi-scanAngle[bn][ii]);
    if (offset<minAngOffset) {
      minAngOffset = offset;
      angIdx = ii;
    }
  }
  //cout << angIdx << endl;
  //cout << minAngOffset << endl;

  /* setup the index boundaries for seraching */

  int minIdx, maxIdx;

  minIdx = 0;
  for (ii=0; ii<angIdx; ii++) {
    minIdx += nRngPixel[bn][ii]*nAzPixel[bn][ii];
  }

  maxIdx = minIdx + nRngPixel[bn][angIdx]*nAzPixel[bn][angIdx];

  //cout << minIdx << endl;
  //cout << maxIdx << endl;

  float minDistOffset;

  minDistOffset = 1.e10;

  int selectIdx;

  selectIdx = -1;

  for (ii=minIdx; ii<maxIdx; ii++) {
    offset = (range_km-rngOffset[bn][ii])*(range_km-rngOffset[bn][ii]) +
             (azimuth_km-azOffset[bn][ii])*(azimuth_km-azOffset[bn][ii]);
    if (offset < minDistOffset) {
      minDistOffset = offset;
      selectIdx = ii;
    }
  }

  //cout << minDistOffset << endl;
  //cout << selectIdx << endl;

  return semiMajorWidth[bn][selectIdx];
}


float PointTargetResponseTable::GetSemiMinorWidth(
                        float range_km, float azimuth_km,
                        float scan_angle_rad, float orbit_time_in_rev_s,
                        int beam_num)
{
  int ii, timeIdx, angIdx, bn;
  float pi;
  
  pi = atan(1.)*4.;
  bn = beam_num - 1;
  
  /* search time from misc time data */
  
  float offset, minTimeOffset;
  
  timeIdx = -1000;
  minTimeOffset = 1.e10;
    
  for (ii=0; ii<nAux[bn]; ii++) {
    offset = fabs(orbit_time_in_rev_s-time[bn][ii]);
    if (offset<minTimeOffset) {
      minTimeOffset = offset;
      timeIdx = ii;
    }
  }
  //cout << timeIdx << endl;
  //cout << minTimeOffset << endl;

  /* search angle from misc angle data                            */
  /* searching area is around the index obtained from time search */

  int angMinIdx, angMaxIdx;
  float minAngOffset;

  angMinIdx = timeIdx - 10;
  angMaxIdx = timeIdx + 10;
  if (angMinIdx < 0) angMinIdx = 0;
  if (angMaxIdx > nAux[bn]) angMaxIdx = nAux[bn];

  angIdx = -1000;
  minAngOffset = 1.e10;

  for (ii=angMinIdx; ii<angMaxIdx; ii++) {
    offset = fabs(scan_angle_rad*180./pi-scanAngle[bn][ii]);
    if (offset<minAngOffset) {
      minAngOffset = offset;
      angIdx = ii;
    }
  }
  //cout << angIdx << endl;
  //cout << minAngOffset << endl;

  /* setup the index boundaries for seraching */

  int minIdx, maxIdx;

  minIdx = 0;
  for (ii=0; ii<angIdx; ii++) {
    minIdx += nRngPixel[bn][ii]*nAzPixel[bn][ii];
  }

  maxIdx = minIdx + nRngPixel[bn][angIdx]*nAzPixel[bn][angIdx];

  //cout << minIdx << endl;
  //cout << maxIdx << endl;

  float minDistOffset;

  minDistOffset = 1.e10;

  int selectIdx;

  selectIdx = -1;

  for (ii=minIdx; ii<maxIdx; ii++) {
    offset = (range_km-rngOffset[bn][ii])*(range_km-rngOffset[bn][ii]) +
             (azimuth_km-azOffset[bn][ii])*(azimuth_km-azOffset[bn][ii]);
    if (offset < minDistOffset) {
      minDistOffset = offset;
      selectIdx = ii;
    }
  }

  //cout << minDistOffset << endl;
  //cout << selectIdx << endl;

  return semiMinorWidth[bn][selectIdx];
}

