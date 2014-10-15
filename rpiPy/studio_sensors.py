
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
DHT22_pin = 17
sensor_id = -1

def bitstring(n):
    s = bin(n)[2:]
    return '0'*(8-len(s)) + s

def readADC(adc_channel=0, spi_channel=0):
	conn = spidev.SpiDev(0, spi_channel)
	conn.max_speed_hz = 500000 # 1.2 MHz
	conn.mode = 0
	cmd = 192
	if adc_channel != 0:
		cmd += 32
	reply_bytes = conn.xfer2([cmd, 0])
	reply_bitstring = ''.join(bitstring(n) for n in reply_bytes)
	reply = reply_bitstring[5:15]
	return int(reply, 2)


#def readADC(adc_channel=0, spi_channel=0):
#    conn = spidev.SpiDev(0, spi_channel)
#    conn.max_speed_hz = 1200000 # 1.2 MHz
#    cmd = 128
#    if adc_channel:
#        cmd += 32
#    reply_bytes = conn.xfer2([cmd, 0])
#    reply_bitstring = ''.join(bitstring(n) for n in reply_bytes)
#    reply = reply_bitstring[5:15]
#    return int(reply, 2) / 2**10

####################################################################################################
# now begin the actual loop
####################################################################################################

if __name__ == "__main__":

	pir_pin = 18
	pi = pigpio.pi()
	pi.set_mode(pir_pin, pigpio.INPUT)         # activate input

	sensor = Adafruit_DHT.DHT22

	last_send = time.time()
	last_sound_sample = last_send
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
			except urllib2.HTTPException, e:
				 call(["ifdown", "wlan0"])
				 sleep(10)
				 call(["ifup", "wlan0"])
				 print('HTTPException')
			except Exception:
				 import traceback
				 print('generic exception: ' + traceback.format_exc())
				
			sleep(10)

	if os.path.isfile('studio_sensor_id') == True:
		f = open('studio_sensor_id', 'r')
		sensor_id = f.read()        
		f.close()    

	sound_values = [0] * 13
	sound_index = 0

	while 1:
		sleep(0.1)
		if pi.read(pir_pin) == False:
			pir_triggered = True
		else:
			pir_triggered = False

		if( time.time() - last_sound_sample) > 5:
			sound_values[sound_index] = readADC()
			sound_index += 1
			last_sound_sample = time.time()


		if (time.time() - last_send) > 60:
			print( " sending ")
			send = "i=" + str(sensor_id)
			if pir_triggered:
				send += "&m=1"
			else:
				send += "&m=0"
			light_value = readADC(1) #light is now just on MCP3002, tsl235 can stuff it
			send += "&l=" + str(light_value)
			
			sound_sum = 0
			for val in sound_values:
				sound_sum += val

			send += "&s=" + str(sound_sum / sound_index)
			# all done with sounds until next send
			sound_index = 0
			humidity, temperature = Adafruit_DHT.read_retry(sensor, DHT22_pin)
			print( str(temperature) + " " + str(humidity))
			send += "&t=" + str(temperature) + "&h=" + str(humidity)

			print( send )

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
			except urllib2.HTTPException, e:
				call(["ifdown", "wlan0"])
				sleep(10)
				call(["ifup", "wlan0"])
				print('HTTPException')
			except Exception:
				import traceback
				print('generic exception: ' + traceback.format_exc())
				call(["ifdown", "wlan0"])
				sleep(10)
				call(["ifup", "wlan0"])
				print( " http error " )

			last_send = time.time()
# end
