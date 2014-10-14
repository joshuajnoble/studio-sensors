
from __future__ import division
import spidev
import time
from time import sleep
import pigpio
import urllib2
import Adafruit_DHT
import datetime
import os

from subprocess import call

TSL235_pin = 7
DHT22_pin = 23
sensor_id = -1
def bitstring(n):
    s = bin(n)[2:]
    return '0'*(8-len(s)) + s

def readADC(adc_channel=0, spi_channel=0):
    conn = spidev.SpiDev(0, spi_channel)
    conn.max_speed_hz = 1200000 # 1.2 MHz
    cmd = 128
    if adc_channel:
        cmd += 32
    reply_bytes = conn.xfer2([cmd, 0])
    reply_bitstring = ''.join(bitstring(n) for n in reply_bytes)
    reply = reply_bitstring[5:15]
    return int(reply, 2) / 2**10

#def read_light(pi):
#    NUM_CYCLES = 10
#    start = time.time()
#    for impulse_count in range(NUM_CYCLES):
#    	pi.wait_for_edge(TSL235_pin, pigpio.FALLING_EDGE)
#        duration = time.time() - start      #seconds to run for loop
#    
#    return NUM_CYCLES / duration   #in Hz



####################################################################################################
# now begin the actual loop
####################################################################################################

if __name__ == "__main__":

    pir_pin = 18
    pi = pigpio.pi()
    pi.set_mode(pir_pin, pigpio.INPUT)         # activate input

    # Sensor should be set to Adafruit_DHT.DHT11,
    # Adafruit_DHT22, or Adafruit_AM2302.
    sensor = Adafruit_DHT.DHT22

    last_send = time.time()

    pir_triggered = False
    has_id = False

	# if you have an ID file then you don't need to do this
        
    if os.path.isfile('studio_sensor_id') == False:
		# maybe this is wonky?
		while has_id == False:
			request = urllib2.Request('http://162.242.237.33:3000/get_id')
			try:
				response = urllib2.urlopen(request)
				sensor_id = response.read()
				print(" got ID " + str(sensor_id))
				f = open('studio_sensor_id', 'w')
				f.write(str(sensor_id))
				f.close()
				has_id = True
			except urllib2.HTTPError, e:
				 print('HTTPError = ' + str(e.code))
				 call(["ifdown", "wlan0"])
				 sleep(10)
				 call(["ifup", "wlan0"])
			except urllib2.URLError, e:
				 call(["ifdown", "wlan0"])
				 sleep(10)
				 call(["ifup", "wlan0"])
				 print('URLError = ' + str(e.reason))
#			except urllib2.HTTPException, e:
#				 call(["ifdown", "wlan0"])
#				 sleep(10)
#				 call(["ifup", "wlan0"])
#				 print('HTTPException')
			except Exception:
				 import traceback
				 print('generic exception: ' + traceback.format_exc())
				
			sleep(10)

    if os.path.isfile('studo_sensor_id') == True:
        f = open('studio_sensor_id', 'r')
        sensor_id = f.read()        
        f.close()    

    while 1:
    	
    	sleep(0.1)
    	
    	if pi.read(pir_pin):
            pir_triggered = True
        else:
            pir_triggered = False

        if (time.time() - last_send) > 60:

            send = "i=" + str(sensor_id)
            if pir_triggered:
                send += "&m=1"
            else:
                send += "&m=0"

            light_value = readADC(1) #light is now just on MCP3002, tsl235 can stuff it
            send += "&l=" + str(light_value)

            sound_value = readADC()
            send += "&s=" + str(sound_value)

            humidity, temperature = Adafruit_DHT.read_retry(sensor, DHT22_pin)

            if humidity is not None and temperature is not None:
            	send += "&t=" + str(temperature) + "&h=" + str(humidity)
            else:
            	humidity, temperature = Adafruit_DHT.read_retry(sensor, DHT22_pin)
                send += "&t=" + str(temperature) + "&h=" + str(humidity)

            # we're going to want this to allow us to pull this out of the
            # config file so it can be set from the desktop I'd think
            request = urllib2.Request('http://162.242.237.33:3000/?'+send)
            try: 
                response = urllib2.urlopen(request)
            except urllib2.HTTPError, e:
                print('HTTPError = ' + str(e.code))
                call(["ifdown", "wlan0"])
                sleep(10)
                call(["ifup", "wlan0"])
                print( " http error " )
            except urllib2.URLError, e:
                call(["ifdown", "wlan0"])
                sleep(10)
                call(["ifup", "wlan0"])
                print('URLError = ' + str(e.reason))
#            except httplib.HTTPException, e:
#                call(["ifdown", "wlan0"])
#                sleep(10)
#                call(["ifup", "wlan0"])
#                print('HTTPException')
            except Exception:
				import traceback
				print('generic exception: ' + traceback.format_exc())
				call(["ifdown", "wlan0"])
				sleep(10)
				call(["ifup", "wlan0"])
				print( " http error " )


 
            print send
            urllib2.close()
            last_send = time.time()

# end
