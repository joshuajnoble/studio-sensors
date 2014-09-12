
from __future__ import division
import spidev
import time
import pigpio
import urllib2
import Adafruit_DHT
import datetime

TSL235_pin = 7
DHT22_pin = 23

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

    while 1:
    	
    	time.sleep(0.1)
    	
    	if pi.read(pir_pin):
            pir_triggered = True
        else:
            pir_triggered = False

        if (time.time() - last_send) > 60:

            send = "i=1"
            if pir_triggered:
                send += "&m=1"
            else:
                send += "&m=0"

            light_value = readADC(1) #light is now just on MCP3002, tsl235 can stuff it
            send += "&l=" + str(light_value)

            sound_value = readADC()
            send += "&s=" + str(sound_value)

            # Try to grab a sensor reading.  Use the read_retry method which will retry up
            # to 15 times to get a sensor reading (waiting 2 seconds between each retry).
            humidity, temperature = Adafruit_DHT.read_retry(sensor, DHT22_pin)

            # Note that sometimes you won't get a reading and
            # the results will be null (because Linux can't
            # guarantee the timing of calls to read the sensor).  
            # If this happens try again!
            if humidity is not None and temperature is not None:
            	send += "&t=" + str(temperature) + "&h=" + str(humidity)
            else:
            	humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)
                send += "&t=" + str(temperature) + "&h=" + str(humidity)

            request = urllib2.Request('162.242.237.33:3000/?'+send)
            try: 
                response = urllib2.urlopen(request)
            except urllib2.HTTPError, e:
                print('HTTPError = ' + str(e.code))
            except urllib2.URLError, e:
                print('URLError = ' + str(e.reason))
            except httplib.HTTPException, e:
                print('HTTPException')
            except Exception:
                import traceback
                print('generic exception: ' + traceback.format_exc())
            
            print send
            urllib2.close()
            last_send = time.time()

# end
