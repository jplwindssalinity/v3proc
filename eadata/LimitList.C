#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "CommonDefs.h"
#include "LimitList.h"
#include "ParTab.h"
#include "PolyTable.h"
#include "LimitChecker.h"
#include "LimitState.h"
#include "L1AExtract.h"

static const char LimitList_c_rcsid[]="(@)# $Id$";

extern const char* enable_map[];

//**********************************************************************//
// FUNCTION: L1ATxtToLim
//   AUTHOR: James N. Huddleston
//   INPUTS: A file pointer
//  RETURNS: An L1ALimitChecker on success, NULL on failure
//  EFFECTS: Reads the limits from file and returns the
//           L1ALimitChecker (for one parameter)
//    NOTES: Belongs in the LimitList object
//**********************************************************************//

int find_match(char *string, char *array[], int count);

#define LINE_LENGTH 256

//***********/
// FUNCTION //
//***********/

LimitChecker*
LimitList::L1ATxtToLim(
LimitList&      obj,        // LimitList object
FILE *          ifp)
{
    char enable;
    char paramName[PARAM_NAME_LEN], unitName[PARAM_NAME_LEN];
    char enable_string[PARAM_NAME_LEN];
    char line[LINE_LENGTH];
    char mode_string[PARAM_NAME_LEN], twt_string[PARAM_NAME_LEN];
    char twta_string[PARAM_NAME_LEN], frame_string[PARAM_NAME_LEN];
    char *ptr;
    int retval, match_index;

    /*-----------------------------------*/
    /* read in the parameter information */
    /*-----------------------------------*/

    ptr = fgets(line, LINE_LENGTH, ifp);
    if (ptr != line)
    {
        if (feof(ifp))
        {
            obj._status = END_OF_LIMIT_FILE;
            return(0);
        }
        else
        {
            obj._status = READ_ERROR;
            return(0);
        }
    }

    retval = sscanf(line, " [%[^]]] [%[^]]] [%[^]]]",
            paramName, unitName, enable_string);
    if (retval != 3)
    {
        fprintf(stderr, "Error determining parameter name\n");
        fprintf(stderr, "Line: %s\n", line);
        return 0;
    }

    //*************************************************************
    // search the parameter entry in the L1A parameter table
    //*************************************************************
    Parameter* parameter = ParTabAccess::GetParameter(obj._source,
                            paramName, unitName);
    if (parameter == 0)
    {
        fprintf(stderr, "No such parameter\n");
        fprintf(stderr, "Line: %s\n", line);
        return 0;
    }

    //*************************************************************
    // check whether it is enable or disable
    //*************************************************************
    match_index = find_match(enable_string, (char **) enable_map, 2);
    if (match_index == -1)
    {
        fprintf(stderr, "Error matching enable string: %s\n", enable_string);
        enable = 1;     /* default is enabled */
    }
    else
        enable = match_index;

    //****************************************
    // create a L1A limit checker first
    //****************************************
    L1ALimitChecker* checker=0;
    if (obj._source == SOURCE_L1A_DERIVED)
        checker = new L1ADrvLimitChecker(parameter, enable);
    else
        checker = new L1ALimitChecker(parameter, enable);

    if (checker == 0)
        return 0;
    else if (checker->GetStatus() != LIMIT_OK)
        return 0;

    /*--------------------------*/
    /* read in the limit values */
    /*--------------------------*/
    ptr = fgets(line, LINE_LENGTH, ifp);
    if (ptr != line)
    {
        fprintf(stderr, "Error reading line\n");
        delete checker;
        return 0;
    }

    void* cl = checker->AllocOneValue();
    void* ch = checker->AllocOneValue();
    void* al = checker->AllocOneValue();
    void* ah = checker->AllocOneValue();
    do
    {
        //---------------------------------------------------
        // read the Caution:(xx, xx) Action:(xx, xx) line
        //---------------------------------------------------
        if (checker->_readLimitLine(line, cl, ch, al, ah) == 0)
        {
            fprintf(stderr, "Error reading limit values...\n");
            fprintf(stderr, "Line: %s\n", line);
            delete checker;
            return 0;
        }

        /*------------------------------*/
        /* read in the limit conditions */
        /*------------------------------*/
        do
        {
            ptr = fgets(line, LINE_LENGTH, ifp);
            //----------------------------------------------------
            // if EOF occured, or first character is pageBreak
            // done with this parameter
            //----------------------------------------------------
            if (ptr == NULL)
            {
                obj._status = END_OF_LIMIT_FILE;
                return checker;
            }
            if (line[0] == PAGE_BREAK_CHAR)
            {
                obj._status = GOOD;
                return checker;
            }

            // save this line's value
            retval = sscanf(line, " Mode:%s TWT:%s TWTA:%s Frame:%s",
                    mode_string, twt_string, twta_string, frame_string);
            // if the line is not "Mode ...", then it must be "Caution..."
            if (retval != 4)
                break;
            int mode = find_match(mode_string,
                                        (char **) mode_map, NSCAT_MODE_COUNT);
            int twt = find_match(twt_string,
                                        (char **) twt_map, HVPS_STATE_COUNT);
            int twta = find_match(twta_string,
                                        (char **) twta_map, TWTA_COUNT);
            int frame = find_match(frame_string,
                                        (char **) cmf_map, FRAME_TYPE_COUNT);
            if (mode == -1 || twt == -1 || twta == -1 || frame == -1)
            {
                fprintf(stderr, "%s: Invalid mode, twt or frame.\n", line);
                return 0;
            }
            L1ALimitStatePair l1State(mode, twt, twta, frame);
            checker->SetLimits(&l1State, cl, ch, al, ah);

        } while (1);

    } while (1);

}//LimitList::L1ATxtToLim

LimitChecker*
LimitList::HkTxtToLim(
LimitList&      obj,        // LimitList object
FILE *          ifp)
{
    char enable;
    char paramName[PARAM_NAME_LEN], unitName[PARAM_NAME_LEN];
    char enable_string[PARAM_NAME_LEN];
    char line[LINE_LENGTH];
    char mode_string[PARAM_NAME_LEN], twt_string[PARAM_NAME_LEN];
    char twta_string[PARAM_NAME_LEN];
    char *ptr;
    int retval, match_index;

    /*-----------------------------------*/
    /* read in the parameter information */
    /*-----------------------------------*/

    ptr = fgets(line, LINE_LENGTH, ifp);
    if (ptr != line)
    {
        if (feof(ifp))
        {
            obj._status = END_OF_LIMIT_FILE;
            return(0);
        }
        else
        {
            obj._status = READ_ERROR;
            return(0);
        }
    }
 
    retval = sscanf(line, " [%[^]]] [%[^]]] [%[^]]]",
            paramName, unitName, enable_string);
    if (retval != 3)
    {
        fprintf(stderr, "Error determining parameter name\n");
        fprintf(stderr, "Line: %s\n", line);
        return 0;
    }

    //*************************************************************
    // search the parameter entry in the HK parameter table
    //*************************************************************
    Parameter* parameter = ParTabAccess::GetParameter(obj._source,
                            paramName, unitName);
    if (parameter == 0)
    {
        fprintf(stderr, "No such parameter\n");
        fprintf(stderr, "Line: %s\n", line);
        return 0;
    }

    //*************************************************************
    // check whether it is enable or disable
    //*************************************************************
    match_index = find_match(enable_string, (char **) enable_map, 2);
    if (match_index == -1)
    {
        fprintf(stderr, "Error matching enable string: %s\n", enable_string);
        enable = 1;     /* default is enabled */
    }
    else
        enable = match_index;

    //****************************************
    // create a HK limit checker first
    //****************************************
    HK2LimitChecker* checker = new HK2LimitChecker(parameter, enable);
    if (checker == 0)
        return 0;
    else if (checker->GetStatus() != LIMIT_OK)
        return 0;

    /*--------------------------*/
    /* read in the limit values */
    /*--------------------------*/
    ptr = fgets(line, LINE_LENGTH, ifp);
    if (ptr != line)
    {
        fprintf(stderr, "Error reading line\n");
        delete checker;
        return 0;
    }

    void* cl = checker->AllocOneValue();
    void* ch = checker->AllocOneValue();
    void* al = checker->AllocOneValue();
    void* ah = checker->AllocOneValue();
    do
    {
        //---------------------------------------------------
        // read the Caution:(xx, xx) Action:(xx, xx) line
        //---------------------------------------------------
        if (checker->_readLimitLine(line, cl, ch, al, ah) == 0)
        {
            fprintf(stderr, "Error reading limit values...\n");
            fprintf(stderr, "Line: %s\n", line);
            delete checker;
            return 0;
        }

        /*------------------------------*/
        /* read in the limit conditions */
        /*------------------------------*/
        do
        {
            ptr = fgets(line, LINE_LENGTH, ifp);
            //----------------------------------------------------
            // if EOF occured, or first character is pageBreak
            // done with this parameter
            //----------------------------------------------------
            if (ptr == NULL)
            {
                obj._status = END_OF_LIMIT_FILE;
                return checker;
            }
            if (line[0] == PAGE_BREAK_CHAR)
            {
                obj._status = GOOD;
                return checker;
            }

            // save this line's value
            retval = sscanf(line, " Mode:%s TWT:%s TWTA:%s",
                    mode_string, twt_string, twta_string);
            // if the line is not "Mode ...", then it must be "Caution..."
            if (retval != 3)
                break;
            int mode = find_match(mode_string,
                                        (char **) ext_mode_map, EXT_MODE_COUNT);
            int twt = find_match(twt_string,
                                        (char **) twt_map, HVPS_STATE_COUNT);
            int twta = find_match(twta_string,
                                        (char **) twta_map, TWTA_COUNT);
            if (mode == -1 || twt == -1 ||  twta == -1)
            {
                fprintf(stderr, "%s: Invalid mode, twt or twta.\n", line);
                return 0;
            }
            HK2LimitStatePair hkdtState(mode, twt, twta);
            checker->SetLimits(&hkdtState, cl, ch, al, ah);

        } while (1);

    } while (1);

}//LimitList::HkTxtToLim

/* auxilliary function */

int
find_match(
char *string,
char *array[],
int count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (strcasecmp(string, array[i]) == 0) 
        {   /* match */
            return i;
        }
    }
    return -1;
}

LimitList::LimitList(
SourceIdE       source,
const char*     limitFilename,
const FILE*     logFP,
const int       keepDisabled)
:   _source(source), _limitFP(0), _logFP(0), _status(GOOD), _worst(OK),
    _readOneParamLimit(0)
{
    (void) strcpy(_limitFilename, limitFilename);
    if ((_limitFP = fopen(_limitFilename, "r")) == NULL)
    {
        _status = ERROR_OPENING_FOR_READING;
        return;
    }

    _logFP = (FILE *)logFP;

    switch (source)
    {
        case SOURCE_HK2:
            _readOneParamLimit = LimitList::HkTxtToLim;
            _limitState = new HK2LimitStatePair;
            break;
        case SOURCE_L1A:
        case SOURCE_L1AP:
        case SOURCE_L1A_DERIVED:
            _readOneParamLimit = LimitList::L1ATxtToLim;
            _limitState = new L1ALimitStatePair;
            break;
        default:
            break;
    }// switch

    LimitChecker* limitChecker=NULL;
    do
    {
        if ((limitChecker = (*_readOneParamLimit) (*this, _limitFP)) != NULL)
        {
            // need to check if the limit checker is constructed OK
            if (limitChecker->GetStatus() == LIMIT_OK)
            {
                if (keepDisabled || limitChecker->GetEnable())
                    Append(limitChecker);
            }
            else
            {
                cout << "Cannot not create limit checker" << endl;
                delete limitChecker;
            }
        }
        if (_status == END_OF_LIMIT_FILE)
        {
            _status = GOOD;
            break;
        }
    } while (limitChecker != NULL);

}//LimitList::LimitList

LimitList::~LimitList()
{
    // ~List() will empty the list

    delete _limitState;

    if (_limitFP)
        fclose(_limitFP);

}//LimitList::~LimitList

LimitList::StatusE
LimitList::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    for (LimitChecker* limitChecker = GetHead();
                           limitChecker; limitChecker = GetNext())
    {
        // open the data sets for the parameter in each limit checker
        if (limitChecker->OpenParamDataSets(tlmFile) == 0)
            return(_status = ERROR_OPEN_DATASETS);
    }

    // open datasets for mode, twt ...
    if ( ! _limitState->OpenParamDataSets(tlmFile))
        return(_status = BAD_PARAM_IN_LIMIT_CHECKER);

    return(_status = GOOD);

}//LimitList::OpenParamDataSets

LimitList::StatusE
LimitList::CloseParamDataSets(
TlmHdfFile*      tlmFile)
{
    IotBoolean closeOK = 1;
    for (LimitChecker* limitChecker = GetHead();
                           limitChecker; limitChecker = GetNext())
    {
        // close the data sets for the parameter in each limit checker
        if (limitChecker->CloseParamDataSets(tlmFile) == 0)
            closeOK = 0;
    }

    // close datasets for mode, twt ...
    if ( ! _limitState->CloseParamDataSets(tlmFile))
        closeOK = 0;

    if (closeOK)
        return(_status = GOOD);
    else
        return(_status = ERROR_CLOSE_DATASETS);

}//LimitList::CloseParamDataSets

LimitStatusE
LimitList::CheckFrame(
PolynomialTable*  polyTable,
TlmHdfFile*       tlmFile,
int32             startIndex,
int               firstDataOnly)
{
    LimitChecker* limit;
    limit = GetHead();

    while (limit)
    {
        (void) _limitState->ApplyNewFrame(tlmFile, startIndex);
        LimitStatusE frameStatus = limit->CheckFrame(polyTable, tlmFile,
                          startIndex, _logFP, _limitState, firstDataOnly);
        switch(frameStatus)
        {
            case LIMIT_OK:
                break;
            case CAUTION_LOW:
            case CAUTION_HIGH:
            case ACTION_LOW:
            case ACTION_HIGH:
                if ((frameStatus == CAUTION_LOW ||
                           frameStatus == CAUTION_HIGH) && _worst == OK)
                {
                    _worst = CAUTION;
                }
                if (frameStatus == ACTION_LOW || frameStatus == ACTION_HIGH)
                {
                    _worst = ACTION;
                }
                break;
            default:
                fprintf(stderr, "Limit Check for '%s' (%s) failed\n",
                         limit->GetParamName(), limit->GetUnitName());
                return(frameStatus);
        }
        limit = GetNext();
    }
    return(LIMIT_CHECK_OK);

}//LimitList::CheckFrame

LimitList::StatusE
LimitList::WriteLimitText(void)
{
    // close the old file pointer, re-open as write only
    if (_limitFP)
        fclose(_limitFP);

    if ((_limitFP = fopen(_limitFilename, "w")) == NULL)
        return (_status = ERROR_OPENING_FOR_WRITING);

    // allocate the space for the limit report text
    char limitText[STRING_LEN * EXT_MODE_COUNT * HVPS_STATE_COUNT *
                            TWTA_COUNT];
    limitText[0] = '\0';

    // write out all the limit checker entries
    LimitChecker* limit;
    limit = GetHead();
    while (limit)
    {
        if (limit->PrintText(limitText) == 0)
            break;
        limit = GetNext();

        // put a page break char between parameters
        char temp[SHORT_STRING_LEN];
        if (limit != 0)
        {
            temp[0] = '\n';
            temp[1] = PAGE_BREAK_CHAR;
            temp[2] = '\n';
            temp[3] = '\0';
            (void) strcat (limitText, temp);
        }
        else
        {   // put a new line at the end of file so vi will not complain
            temp[0] = '\n';
            temp[1] = '\0';
            (void) strcat (limitText, temp);
        }
        if (fprintf(_limitFP, "%s", limitText) == EOF)
        {
            return (_status = WRITE_TO_LIMIT_FILE_FAILED);
        }
    }

    // close the file and reopen for reading
    fclose(_limitFP);
    if ((_limitFP = fopen(_limitFilename, "r")) == NULL)
        return(_status = ERROR_OPENING_FOR_READING);

    return (GOOD);

}//LimitList::WriteLimitText

//*************************************************
// returns LimitChecker* on success, returns 0 on failure
//*************************************************
LimitChecker*
LimitList::GetLimitChecker(
SourceIdE   source,     // source
char*       param,      // parameter name
char*       unit)       // unit name
{
    // search the limit list and compare with each limit checker's 
    // source, param and unit
    LimitChecker* limit;
    limit = GetHead();
    while (limit)
    {
        // if match, return this limit checker
        if (limit->GetSource() == source &&
                strcmp (limit->GetParamName(), param) == 0 &&
                strcmp (limit->GetUnitName(), unit) == 0)
            return (limit);
        limit = GetNext();

    }
    return (0);

}//LimitList::GetLimitChecker

//*************************************************
// returns TRUE on success, returns FALSE on failure
//*************************************************
LimitChecker*
LimitList::DeleteLimitChecker(
LimitChecker*       delLimit)
{
    // remove the limit checker from the list
    for (LimitChecker* current=GetHead(); current != 0; current=GetNext())
    {
        if (*current == *delLimit)
            return ((LimitChecker*)RemoveCurrent());
    }
    return 0;

}//LimitList::DeleteLimitChecker
