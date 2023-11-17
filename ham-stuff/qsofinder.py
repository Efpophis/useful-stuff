#!/usr/bin/env python

from datetime import date, datetime, timedelta

# use this to generate ADIF records of a qso with the given
# call sign, band, mode every 14 minutes in order to find
# a missing (from your log) qso on clublog. Output is 
# to stdout, but you can redirect it to a file,
# ie: ./qsofinder.py > log.adi

# set this to the start of the range of date and time
# where you think you worked the station
# datetime(year, month, day, hour, minute, second)
st=datetime(2023,10,27,0,0,0)

# set this to the end of the range where you think you
# worked the station. Same parameters as above.
et=datetime(2023,11,14,0,0,0)

# set this to the max time difference allowed by clublog
# which is 14 minutes.
dt=timedelta(minutes=14)

# set the QSO parameters
myCall='WK2X'
dxCall='P5RS7'
qMode='SSB'
qFreq='14.195'
qBand='20m'

def adif_item(item="",payload=""):
    adif= "<{0}:{1}>{2}".format(item, len(payload),payload)
    return adif

print(adif_item('ADIF_VER', '3.1.0'))
print(adif_item('CREATED_TIMESTAMP', datetime.now().strftime("%Y%m%d %H%M%S")))
print(adif_item('PROGRAMID', 'qsofinder'))

print('<EOH>')


while st<=et:
    qsodate=st.strftime("%Y%m%d")
    qsotime=st.strftime("%H%M")
    qso=adif_item('QSO_DATE',qsodate) + adif_item('TIME_ON',qsotime) + \
        adif_item('STATION_CALLSIGN', myCall ) + adif_item('CALL', dxCall) + \
        adif_item('MODE', qMode) + adif_item('FREQ', qFreq) + adif_item('BAND', qBand) + \
        adif_item('RST_SENT', '599') + adif_item('RST_RCVD', '599')
    print(qso)
    print('<EOR>')   
    
    st+=dt



