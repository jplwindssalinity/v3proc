#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
import datetime
import sys
import os
import rdf
import util.time
import smtplib
import shelve
import pdb
import numpy as np

# key in shelved dict to use
LAST_GSE_NOTIFY_KEY = 'LAST_GSE_NOTIFY'

ANCIENT_HISTORY = datetime.datetime(1990,1,1)

# How long to wait for GSE data before commencement of sending annoying emails.
GSE_LAG_TOLERANCE = datetime.timedelta(hours=3)

MAIL_FROM = "fore@pawpaw"
PEOPLE_TO_ANNOY = [
        "fore@jpl.nasa.gov", "douglas.j.equils@jpl.nasa.gov",
        "wallace.hu@jpl.nasa.gov", "bryan.w.stiles@jpl.nasa.gov"]
# PEOPLE_TO_ANNOY = PEOPLE_TO_ANNOY[0]

# How long to wait before complaining about the same thing again.
ANNOY_INTERVAL = datetime.timedelta(days=1)

def send_message(subject, body, sender, recipients):
    smtpObj = smtplib.SMTP('localhost')
    message = 'To: %s\nSubject: %s\n\n%s' %(recipients, subject, body)
    smtpObj.sendmail(sender, recipients, message) 

def OldGSE(config_file):
    """
    Sends annoying emails to people who should fix the NRT GSE data
    stream.
    """
    MAIL_SUBJECT="GSE DATA IS %f HOURS OLD"
    MAIL_BODY="Last GSE filename: %s\nLast GSE Time: %s"
    
    if not config_file or not os.path.isfile(config_file):
        print>>sys.stderr, '%s not right' % config_file
        return 0
    try:
        rdf_data = rdf.parse(config_file)
        gsetimes = rdf_data["GSE_TIMES"]
        notify_dbfile = rdf_data["GSE_NOTIFY_DB"]
    except KeyError:
        print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
        return 0
    
    gsefile_names = np.genfromtxt(
        gsetimes, delimiter=',', usecols=(0,), dtype=None )
    gsefile_t_end = np.loadtxt(gsetimes, delimiter=',', usecols=(2,))
    
    maxidx = np.argmax(gsefile_t_end)
    last_gse_time = util.time.datetime_from_sim(gsefile_t_end[maxidx])
    last_gse_file = gsefile_names[maxidx]
    
    if (datetime.datetime.utcnow()-last_gse_time) > GSE_LAG_TOLERANCE:
        notify_shelf = shelve.open(notify_dbfile)
        try:
            last_notify = notify_shelf[LAST_GSE_NOTIFY_KEY]
        except KeyError:
            last_notify = {'file': '', 'time': ANCIENT_HISTORY}
        
        print last_notify
        
        if not (last_notify['file']==last_gse_file and 
               (datetime.datetime.now()-last_notify['time'])<ANNOY_INTERVAL):
            message_body = MAIL_BODY % (
                os.path.basename(last_gse_file), 
                util.time.ToCodeB(last_gse_time))
                
            hours_old = (
                datetime.datetime.utcnow()-last_gse_time).total_seconds()/60/60
            
            send_message(
                MAIL_SUBJECT%hours_old, message_body, MAIL_FROM, 
                PEOPLE_TO_ANNOY)
            
            last_notify['file'] = last_gse_file
            last_notify['time'] = datetime.datetime.now()
            notify_shelf[LAST_GSE_NOTIFY_KEY] = last_notify
        notify_shelf.close()
    
if __name__ == "__main__":
    config_file = sys.argv[1]
    if not os.path.isfile(config_file):
        print>>sys.stderr, usage_string
        sys.exit(1)
    OldGSE(config_file)
    
    
    
    