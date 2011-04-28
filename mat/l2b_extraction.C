/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION: 
 *
 * File Name:     l2b_extraction.C
 * Creation Date: 26 Apr 2011
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 * Copyright 2009-2011, by the California Institute of Technology.
 * ALL RIGHTS RESERVED.  United States Government Sponsorship
 * acknowledged. Any commercial use must be negotiated with the Office 
 * of Technology Transfer at the California Institute of Technology.
 *
 * This software may be subject to U.S. export control laws and
 * regulations.  By accepting this document, the user agrees to comply
 * with all U.S. export laws and regulations.  User has the
 * responsibility to obtain export licenses, or other export authority
 * as may be required before exporting such information to foreign
 * countries or providing access to foreign persons.
 ********************************************************************/

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

#include <stdlib.h>

#include "Misc.h"
#include "L2B.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "Constants.h"

#include "mex.h"

#define FOPEN_ERROR "Could not open file: "

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;
template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<long>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

enum {
    FILENAME  = 0,
    PARAMETER = 1,
    FILLVAL   = 2
};

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {

    char *filename;
    char *parameter;

    L2B l2b;
    WVC *wvc;

    int at_num, xt_num;
    int i, j, n;

    /* Pick off the input variables */
    n = mxGetN(prhs[FILENAME]);
    filename = (char *)calloc(n + 1, sizeof *filename);
    mxGetString(prhs[FILENAME], filename, n + 1);

    n = mxGetN(prhs[PARAMETER]);
    parameter = (char *)calloc(n + 1, sizeof *filename);
    mxGetString(prhs[PARAMETER], parameter, n + 1);

    if (l2b.OpenForReading(filename) == 0) {
        char *errmsg = (char *)calloc(strlen(FOPEN_ERROR) + 
            strlen(filename) + 1, sizeof *errmsg);
        /* These are safe because errmsg is built-to-size */
        strcpy(errmsg, FOPEN_ERROR);
        strcat(errmsg, filename);
        
        mexErrMsgTxt(errmsg);
    }

    l2b.ReadHeader();
    l2b.ReadDataRec();

    at_num = l2b.frame.swath.GetAlongTrackBins(); 
    xt_num = l2b.frame.swath.GetCrossTrackBins();

    if (strcmp("speedBias", parameter) == 0) {
        double *data;
        double fill_val = *mxGetPr(prhs[FILLVAL]);

        plhs[0] = mxCreateDoubleMatrix(xt_num, at_num, mxREAL);
        data = mxGetPr(plhs[0]);

        for (i = 0; i < at_num; i++) {
            for (j = 0; j < xt_num; j++) {
                wvc = l2b.frame.swath.swath[j][i];

                if ((wvc != NULL) && (wvc->selected != NULL)) {
                    data[i*xt_num + j] = wvc->speedBias;
                } else {
                    data[i*xt_num + j] = fill_val;
                }
            }
        }
    }

    l2b.Close();

    free(filename);
    free(parameter);
}

