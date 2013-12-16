import datetime

def date_time_from_sim( sim_tt ):
  return(datetime.datetime(1970,1,1)+datetime.timedelta(0,sim_tt))

def date_time_from_gps( gps_tt ):
  return(datetime.datetime(1980,1,6)+datetime.timedelta(0,gps_tt))

def ToCodeB( dt ):
  millisecs = round(dt.microsecond/1000.0)
  dt_out = dt + datetime.timedelta(0,0,millisecs*1000-dt.microsecond)
  string = dt_out.strftime('%Y-%jT%H:%M:%S') + '.%3.3d' % millisecs
  return(string)
