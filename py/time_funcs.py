import datetime

gps_epoch = datetime.datetime(1980,1,6)
sim_epoch = datetime.datetime(1970,1,1)
gs_epoch  = datetime.datetime(1993,1,1)

def date_time_from_gs( tt ):
  return(gs_epoch+datetime.timedelta(0,tt))

def date_time_from_sim( tt ):
  return(sim_epoch+datetime.timedelta(0,tt))

def date_time_from_gps( tt ):
  return(gps_epoch+datetime.timedelta(0,tt))

def gps_to_sim( gps_tt ):
  delta = gps_epoch - sim_epoch
  return(gps_tt+delta.days*86400.0+delta.seconds)

def sim_to_gps( sim_tt ):
  delta = sim_epoch - gps_epoch
  return(sim_tt+delta.days*86400.0+delta.seconds)

def ToCodeB( dt ):
  return(dt.strftime('%Y-%jT%H:%M:%S.%f')[:-3])
