from __future__ import division
import struct
import socket
import fcntl
import getopt
import spidev
import time
import sys
from time import sleep
import pigpio
import urllib2
import Adafruit_DHT
import datetime
import os
import contextlib

import subprocess
from subprocess import call

zone = ""
studio = ""

TSL235_pin = 7
DHT22_pin = 17
sensor_id = -1

IP = "54.191.189.58"
PORT = "3000"

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
	conn.close()
	return int(reply, 2)

def get_ip_address(ifname):
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	return socket.inet_ntoa( fcntl.ioctl( s.fileno(), 0x8915, struct.pack('256s', ifname[:15]) )[20:24] )

def downUp():
	subprocess.Popen(["ifdown", "wlan0"],close_fds=True)
	sleep(10)
	subprocess.Popen(["ifup", "wlan0"],close_fds=True)
	sleep(20)


def rebootWlan0():
	downUp()
	hasIP = False
	while(hasIP == False):
		try:
			ip = get_ip_address("wlan0")
			hasIP = True
		except IOError:
			downUp()


	request = urllib2.Request('http://'+IP+':'+PORT+'/update_ip?id='+sensor_id+'&ip=' + ip)
	# recursively calls itself, could be bad
	try:
		response = urllib2.urlopen(request)
	except urllib2.HTTPError, e:
		#response.close()
		print(str(datetime.datetime.now()) + 'HTTPError = ' + str(e.code) )
		rebootWlan0()
	except urllib2.URLError, e:
		#response.close()
		rebootWlan0()
		print(str(datetime.datetime.now()) + 'URLError = ' + str(e.reason))
	except Exception:
		#response.close()
		import traceback
		print(str(datetime.datetime.now()) + 'generic exception: ' + traceback.format_exc())
		rebootWlan0()
		print(str(datetime.datetime.now()) +  " http error " )


def getHwAddr(ifname):
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	info = fcntl.ioctl(s.fileno(), 0x8927,  struct.pack('256s', ifname[:15]))
	return ''.join(['%02x' % ord(char) for char in info[18:24]])



# now begin the actual loop
####################################################################################################

if __name__ == "__main__":

	# Read command line args
	#myopts, args = getopt.getopt(sys.argv[1:],"s:z:")
 
	###############################
	# o == option
	# a == argument passed to the o
	###############################
	#for o, a in myopts:
	#	if o == '-s':
	#		studio=a
	#	elif o == '-z':
	#		zone=a
	#	else:
	#		print(str(datetime.datetime.now()) + "Usage: %s -s studio -z zone" % sys.argv[0])


	# FIRST
	# do we know our zone and studio?

	has_studio_zone = False

	if os.path.isfile('studio_sensor_studio_zone') == False:
		print(str(datetime.datetime.now()) + " no studio + zone, getting one from server ")
		mac = getHwAddr("wlan0")
		print(str(datetime.datetime.now()) + " my mac is " + str(mac))
		while has_studio_zone == False:
			print(str(datetime.datetime.now()) +  " going to mac request from " + IP + ":" + PORT)
			request = urllib2.Request("http://"+str(IP)+":"+str(PORT)+"/get_studio_zone?mac="+str(mac))
			try:
				response = urllib2.urlopen(request)
				studio_zone = response.read()
				print(str(datetime.datetime.now()) + " got studio + zone " + str(studio_zone).split(':')[0] + " " + str(studio_zone).split(':')[1])
				f = open('studio_sensor_studio_zone', 'w')
				f.write(str(studio_zone))
				f.close()
				response.close()
				has_studio_zone = True

			except urllib2.HTTPError, e:
				 if(e.code == 409):
				 	print(str(datetime.datetime.now()) + ' DB error configure MAC ' + str(e.code))
					sys.exit()
				 print(str(datetime.datetime.now()) + 'HTTPError = ' + str(e.code))
				 #print(str(datetime.datetime.now()) + 'HTTPError = ' + str(e.code))
				 downUp();
			except urllib2.URLError, e:
				 downUp();
				 print(str(datetime.datetime.now()) + 'URLError = ' + str(e.reason))
			except urllib2.HTTPException, e:
				 downUp();
				 print(str(datetime.datetime.now()) + 'HTTPException')
			except Exception:
				 import traceback
				 print(str(datetime.datetime.now()) + 'generic exception: ' + traceback.format_exc())
				
			sleep(10)

	# we know our zone and studio, lets load it in

	if os.path.isfile('studio_sensor_studio_zone') == True:
		f = open('studio_sensor_studio_zone', 'r')
		studio_zone = f.read()
		studio = str(studio_zone).split(":")[0]
		zone = str(studio_zone).split(":")[1]
		f.close()    

	print (studio + " " + zone)

	pir_pin = 18
	pi = pigpio.pi()
	pi.set_mode(pir_pin, pigpio.INPUT)         # activate input

	sensor = Adafruit_DHT.DHT22

	pir_triggered = False
	has_id = False

	# if you have an ID file then you don't need to do this

	if os.path.isfile('studio_sensor_id') == False:
		print(str(datetime.datetime.now()) + " no ID, getting one from server ")
		ip = get_ip_address("wlan0")
		print(str(datetime.datetime.now()) + " my IP is " + str(ip))
		while has_id == False:
			print(str(datetime.datetime.now()) +  " going to request from " + IP + ":" + PORT)
			request = urllib2.Request("http://"+str(IP)+":"+str(PORT)+"/get_id?studio="+str(studio)+"&zone="+str(zone)+"&ip="+str(ip))
			#request = urllib2.Request("http://54.191.189.58:3000/get_id?studio=SEA&zone=1&ip=127.0.0.1")

			try:
				response = urllib2.urlopen(request)
				sensor_id = response.read()
				print(str(datetime.datetime.now()) + " got ID " + str(sensor_id))
				f = open('studio_sensor_id', 'w')
				f.write(str(sensor_id))
				f.close()
				response.close()
				has_id = True
			except urllib2.HTTPError, e:
				 print(str(datetime.datetime.now()) + 'HTTPError = ' + str(e.code))
				 downUp();
			except urllib2.URLError, e:
				 downUp();
				 print(str(datetime.datetime.now()) + 'URLError = ' + str(e.reason))
			except urllib2.HTTPException, e:
				 downUp();
				 print(str(datetime.datetime.now()) + 'HTTPException')
			except Exception:
				 import traceback
				 print(str(datetime.datetime.now()) + 'generic exception: ' + traceback.format_exc())
				
			sleep(10)

	if os.path.isfile('studio_sensor_id') == True:
		f = open('studio_sensor_id', 'r')
		sensor_id = f.read()        
		f.close()    

	sound_values = [0] * 150
	sound_index = 0
	last_send = time.time()
	last_sound_sample = last_send

	while 1:
		sleep(0.05)
		if pi.read(pir_pin) == False:
			pir_triggered = True
		else:
			pir_triggered = False

		if( time.time() - last_sound_sample) > 5:
			sound_values[sound_index] = readADC()
			sound_index += 1
			last_sound_sample = time.time()

		# every 5 minutes?
		if (time.time() - last_send) > 300:
			print(str(datetime.datetime.now()) +  " sending ")
			send = "i=" + str(sensor_id) + "&studio="+ studio+ "&zone="+ zone
			if pir_triggered:
				send += "&m=1"
			else:
				send += "&m=0"
			light_value = readADC(1) #light is now just on MCP3002, tsl235 can stuff it
			send += "&l=" + str(light_value)
			
			sound_sum = 0
			for val in range(sound_index):
				sound_sum += abs( 512 - sound_values[val] )

			send += "&s=" + str(sound_sum / sound_index)
			# all done with sounds until next send
			sound_index = 0
			humidity, temperature = Adafruit_DHT.read_retry(sensor, DHT22_pin)
			print(str(datetime.datetime.now()) +  str(temperature) + " " + str(humidity))
			if temperature != None:
				send += "&t=" + str(int(temperature))
			else:
				send += "&t=0"

			if humidity != None:
				send += "&h=" + str(int(humidity))
			else:
				send += "&h=0"

			print(str(datetime.datetime.now()) +  send )

			# we're going to want this to allow us to pull this out of the
			# config file so it can be set from the desktop I'd think
			request = urllib2.Request('http://'+IP+':'+PORT+'/?'+send)
			try: 
				response = urllib2.urlopen(request)
				response.close()
			except urllib2.HTTPError, e:
				#response.close()
				print(str(datetime.datetime.now()) + " HTTP ERROR = " + str(e.code))
				if( e.code == 409 ):
					print(str(datetime.datetime.now()) + " We're getting a 409 which means a misconfigured request and resulting DB error ")
				else:
					rebootWlan0()
			except urllib2.URLError, e:
				#response.close()
				rebootWlan0()
				print(str(datetime.datetime.now()) + 'URLError = ' + str(e.reason))
			except Exception:
				#response.close()
				import traceback
				print(str(datetime.datetime.now()) + 'generic exception: ' + traceback.format_exc())
				rebootWlan0()
				print(str(datetime.datetime.now()) +  " http error " )

			last_send = time.time()
# end
