//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.4   21 Apr 1998 16:39:26   sally
// for L2B
// 
//    Rev 1.3   10 Apr 1998 14:04:10   daffer
//     Changed ConfigList to EAConfigList throughout
// 
//    Rev 1.2   06 Apr 1998 16:27:02   sally
// merged with SVT
// 
//    Rev 1.1   26 Feb 1998 09:55:32   sally
// to pacify GNU compiler
// 
//    Rev 1.0   04 Feb 1998 14:15:08   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:00  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_Drn_C[] =
    "@(#) $Header$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Drn.h"
#include "Itime.h"
#include "Mail.h"

const char* status_map[] =
{
    "Successful Retrieval", "Unused Target File",
    "FTP Error (Receiver Site Problem)", "FTP Error (Sender Site Problem)",
    "Spare", "Spare", "Spare", "Spare", "Spare", "Spare"
};

//=============
// Drn Methods 
//=============

Drn::Drn(
    const char*     config_filename,
    const char*     log_filename)   :
    _configList(0), _from(0), _to(0), _subject(0), _errorAddress(0),
    _receivingSite(0), _status(OK), _logFp(0)
{
    //-----------------------
    // open the log filename 
    //-----------------------

    _logFp = fopen(log_filename, "a");
    if (! _logFp)
    {
        _status = ERROR_OPENING_LOG_FILE;
        return;
    }

    //-----------------------------
    // read the configuration file 
    //-----------------------------

    if (! ReadConfigList(config_filename))
    {
        ReportError(log_filename);
        return;
    }

    //----------------------
    // read the mail header 
    //----------------------

    if (! ReadHeader())
    {
        ReportError(log_filename);
        return;
    }

    //------------------------------
    // read the message and process 
    //------------------------------

    if (! IsValidDrn())
        return;

    if (! ProcessBody(config_filename))
    {
        ReportError(log_filename);
        return;
    }

    return;
}

Drn::~Drn()
{
    //------------
    // free stuff 
    //------------

    if (_configList)
        delete _configList;
    if (_from)
        free(_from);
    if (_to);
        free(_to);
    if (_subject)
        free(_subject);

    //--------------------
    // close the log file 
    //--------------------

    if (_logFp)
        fclose(_logFp);

    return;
}

//----------------
// ReadConfigList 
//----------------

int
Drn::ReadConfigList(
    const char*     config_file)
{
    _configList = new EAConfigList;
    if (! _configList)
    {
        fprintf(_logFp,
            "%s: Error allocating configuration list (low memory?)\n",
            Itime::CurrentCodeA());
        return(0);
    }
    if ( ! _configList->Read(config_file))
    {
        fprintf(_logFp, "%s: Error reading configuration file\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  Configuration File: %s\n", config_file);
        return(0);
    }

    _errorAddress = _configList->Get(ERROR_ADDRESS_KEYWORD);
    if (! _errorAddress)
    {
        fprintf(_logFp, "%s: Missing keyword in configuration file\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  Keyword: %s\n", ERROR_ADDRESS_KEYWORD);
        fprintf(_logFp, "  Configuration File: %s\n", config_file);
        return(0);
    }

    _receivingSite = _configList->Get(RECEIVING_SITE_KEYWORD);
    if (! _receivingSite)
    {
        fprintf(_logFp, "%s: Missing keyword in configuration file\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  Keyword: %s\n", RECEIVING_SITE_KEYWORD);
        fprintf(_logFp, "  Configuration File: %s\n", config_file);
        return(0);
    }

    return(1);
}

//------------
// ReadHeader 
//------------

int
Drn::ReadHeader()
{
    while (! _from || ! _to || ! _subject)
    {
        // read a line
        char line[LINE_LENGTH];
        if (fgets(line, LINE_LENGTH, stdin) != line)
        {
            fprintf(_logFp, "%s: Incomplete mail message header\n",
                Itime::CurrentCodeA());
            fprintf(_logFp, "  Missing the following:");
            if (! _from)
                fprintf(_logFp, " \"%s\"", FROM_LABEL);
            if (! _to)
                fprintf(_logFp, " \"%s\"", TO_LABEL);
            if (! _subject)
                fprintf(_logFp, " \"%s\"", SUBJECT_LABEL);
            fprintf(_logFp, "\n");
            return(0);
        }

        if (strstr(line, FROM_LABEL) == line)
            _from = strdup(line);
        if (strstr(line, TO_LABEL) == line)
            _to = strdup(line);
        if (strstr(line, SUBJECT_LABEL) == line)
            _subject = strdup(line);
    }
    return(1);
}

//------------
// IsValidDrn 
//------------
// check the header fields for validity

int
Drn::IsValidDrn()
{
    // check the subject
    char mail_type_id[2];
    char mail_urgency_id[3];
    char breakout_of_mail_type_id[2];
    int retval = sscanf(_subject, "%*s %1s-%2s-%1s-%9[^-]-%9s",
        mail_type_id, mail_urgency_id, breakout_of_mail_type_id,
        _computerIdOfSender, _computerIdOfReceiver);
    if (retval != 5)
    {
        fprintf(_logFp, "%s: Mail not a File Transmission Notification\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  %s", _subject);
        return(0);
    }
    if (strcmp(breakout_of_mail_type_id, DRN_BREAKOUT) != 0)
    {
        fprintf(_logFp, "%s: Non-DRN File Transmission Notification\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  %s", _subject);
        fprintf(_logFp, "  Breakout of mail type should be %s\n",
            DRN_BREAKOUT);
        return(0);
    }

    return(1);
}

//-------------
// ProcessBody 
//-------------

int
Drn::ProcessBody(
    const char*     config_filename)
{
    char in_message_field = 0;
    char line[LINE_LENGTH];
    int count = 0;
    while (fgets(line, LINE_LENGTH, stdin) == line)
    {
        if (strstr(line, BEGINNING_OF_MESSAGE) == line)
        {
            in_message_field = 1;
            continue;
        }
        else if (strstr(line, END_OF_MESSAGE) == line)
        {
            in_message_field = 0;
            break;
        }
        else if (in_message_field)
        {
            // assume it is a content message and parse
            char file_name[81];
            int file_size;
            char dest_computer_id[10];
            int retval = sscanf(line, "CON>%80[^>]>%d>%9[^>]",
                file_name, &file_size, dest_computer_id);
            if (retval != 3)
            {
                fprintf(_logFp, "%s: Error parsing DRN message contents\n",
                    Itime::CurrentCodeA());
                fprintf(_logFp, "  Line:%s", line);
                return(0);
            }

            // check the destination
            if (strstr(dest_computer_id, _receivingSite) != dest_computer_id)
            {
                fprintf(_logFp, "%s: DRN received for site other than %s\n",
                    Itime::CurrentCodeA(), _receivingSite);
                fprintf(_logFp, "  Destination Computer ID: %s\n",
                    dest_computer_id);
                return(0);
            }

            // get the file and send an appropriate reponse
            GetFileSendRcn(file_name, file_size, dest_computer_id,
                config_filename);
            count++;
        }
    }

    if (in_message_field)
    {
        fprintf(_logFp, "%s: Message ended before %s marker\n",
            Itime::CurrentCodeA(), END_OF_MESSAGE);
        return(0);
    }

    if (count == 0)
    {
        fprintf(_logFp, "%s: No message found in DRN\n", Itime::CurrentCodeA());
        return(0);
    }

    return(1);
}

//----------------
// GetFileSendRcn 
//----------------

int
Drn::GetFileSendRcn(
    const char*     file_name,
    const int       file_size,
    const char*     dest_computer_id,
    const char*     config_filename)
{
    //-------------------------
    // get all the information 
    //-------------------------

    char* file_name_dup = strdup(file_name);
    char* file = strrchr(file_name_dup, '/');
    char* path;
    if (! file)
    {
        file = file_name_dup;
        path = ".";
    }
    else
    {
        *file = '\0';   // terminate the path
        file++;         // move to the start of the file
        path = file_name_dup;
    }

    int match = 0;
    EAStringPair* pair = 0;
    for (pair = _configList->GetHead(); pair;
        pair = _configList->GetNext())
    {
        if (strstr(file, pair->keyword) == file)
        {
            match = 1;
            break;
        }
    }
    if (! match)
    {
        fprintf(_logFp, "%s: No matching keyword for specified filename\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  Filename: %s\n", file_name);
        fprintf(_logFp, "  Configuration File: %s\n", config_filename);
        return(0);
    }

    //----------------------------------------
    // parse the matching configuration entry 
    //----------------------------------------

    char data_server[1024];
    char username[128];
    char password[32];
    char local_directory[1024];
    char rcn_address[1024];
    char cc_address[1024];

    if (sscanf(pair->value, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%s",
        data_server, username, password, local_directory, rcn_address,
        cc_address) != 6)
    {
        fprintf(_logFp, "%s: Error parsing configuration file\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  Keyword: %s  Value: %s\n", pair->keyword,
            pair->value);
        fprintf(_logFp, "  Configuration File: %s\n", config_filename);
        return(0);
    }

    char local_filename[1024];
    sprintf(local_filename, "%s/%s", local_directory, file);

    //--------------
    // start up ftp 
    //--------------

    FILE* ftpfp = popen("ftp -n", "w");
    if (! ftpfp)
    {
        fprintf(_logFp, "%s: Error opening pipe to ftp\n",
            Itime::CurrentCodeA());
        return(0);
    }

    //-------------------------------
    // connect to the remote machine 
    //-------------------------------

    fprintf(ftpfp, "open %s\n", data_server);

    //-------------------------------
    // provide username and password 
    //-------------------------------

    fprintf(ftpfp, "user %s %s\n", username, password);

    //--------------
    // get the file 
    //--------------

    fprintf(ftpfp, "cd %s\n", path);
    fprintf(ftpfp, "get %s %s\n", file, local_filename);

    //----------
    // quit ftp 
    //----------

    fprintf(ftpfp, "quit\n");
    pclose(ftpfp);

    //---------------------
    // determine file size 
    //---------------------

    int fd = open(local_filename, O_RDONLY);
    off_t size = 0;
    int status_code = 3;    // default as their fault (ha!)
    if (fd == -1)
    {
        fprintf(_logFp, "%s: Error opening local file to check size\n",
            Itime::CurrentCodeA());
        fprintf(_logFp, "  Local Filename: %s\n", local_filename);
        status_code = 3;    // assume it was their fault
    }
    else
    {
        size = lseek(fd, 0, SEEK_END);
        close(fd);

        //-----------------
        // check file size 
        //-----------------
 
        if (size == file_size)
            status_code = 0;
        else if (size == 0)
            status_code = 3;    // assume it was their fault
        else
            status_code = 2;    // assume disk full
    }

    return(SendRcn(file_name, local_filename, file_size, dest_computer_id,
        rcn_address, cc_address, status_code));
}

//---------
// SendRcn 
//---------

int
Drn::SendRcn(
    const char*     file_name,
    const char*     local_file_name,
    const int       file_size,
    const char*     dest_computer_id,
    const char*     rcn_address,
    const char*     cc_address,
    const int       status)
{
    //----------
    // send RCN 
    //----------

    char message[1024];
    sprintf(message, "%s\nCON>%s>%d>%s>%d>\n%s", BEGINNING_OF_MESSAGE,
         file_name, file_size, dest_computer_id, status, END_OF_MESSAGE);
    char subject[1024];
    sprintf(subject, "%1s-%2s-%1s-%s-%s", FILE_XMIT_MAIL_TYPE_ID,
        RCN_MAIL_URGENCY_ID, RCN_BREAKOUT, _computerIdOfReceiver,
        _computerIdOfSender);
    // computer Id's are reversed on purpose for the RCN
    SendMsg(rcn_address, subject, message);

    //-----------------------
    // log it in the logfile 
    //-----------------------

    fprintf(_logFp, "%s: Sent RCN [%s]\n", Itime::CurrentCodeA(),
        status_map[status]);
    fprintf(_logFp, "  Local File: %s\n", local_file_name);

    //-------------------------
    // send copy to cc address 
    //-------------------------

    if (cc_address)
    {
        char tag_line[1024];
        switch(status)
        {
        case 0:
            sprintf(tag_line, "%s\nLocal File: %s\n", status_map[status],
                local_file_name);
            break;
        case 2: case 3:
            sprintf(tag_line, "%s\n", status_map[status]);
            break;
        default:
            sprintf(tag_line, "Unknown Data Retrieval Error.\n");
            break;
        }
        strcat(message, "\n\n");
        strcat(message, TAG_LINE_DELIMETER);
        strcat(message, tag_line);
        SendMsg(cc_address, subject, message);
    }
    return(1);
}

//-------------
// ReportError 
//-------------

void
Drn::ReportError(
    const char*     log_file)
{
    if (_errorAddress)
    {
        char message[1024];
        sprintf(message, "%s\n%s\n%s\n",
            "*** Error processing DRN ***",
            "See the DRN Processing log file for details",
            log_file);
        SendMsg(_errorAddress, "process_drn: *** Error ***", message);
    }
    return;
}
