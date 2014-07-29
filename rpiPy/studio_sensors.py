
from __future__ import division
import spidev
import time
import pigpio
import urllib2

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

def read_light():
	NUM_CYCLES = 10
	start = time.time()
	for impulse_count in range(NUM_CYCLES):
	    GPIO.wait_for_edge(25, GPIO.FALLING)
	duration = time.time() - start      #seconds to run for loop
	return NUM_CYCLES / duration   #in Hz


####################################################################################################
# now begin the actual loop
####################################################################################################

if __name__ == "__main__":

    pigpio.pi().setup(pir_pin, gpio.IN)         # activate input

    # Sensor should be set to Adafruit_DHT.DHT11,
    # Adafruit_DHT22, or Adafruit_AM2302.
    sensor = Adafruit_DHT.DHT22
    dht_pin = 23

    last_send = time.time()

    while 1:

        if (time.time() - last_send) > 60:

            send = ""

            if io.input(pir_pin):
                send += "&m=1"
            else:
                send += "&m=0"

            light_value = read_light()
            send += "&l=" + str(light_value)

            sound_value = readADC()
            send += "&s=" + str(sound_value)

            # Try to grab a sensor reading.  Use the read_retry method which will retry up
            # to 15 times to get a sensor reading (waiting 2 seconds between each retry).
            humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)

            # Note that sometimes you won't get a reading and
            # the results will be null (because Linux can't
            # guarantee the timing of calls to read the sensor).  
            # If this happens try again!
            if humidity is not None and temperature is not None:
            	send += "&t=" + str(temperature) + "&h=" + str(humidity)
            else:
            	humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)
                send += "&t=" + str(temperature) + "&h=" + str(humidity)

            urllib2.urlopen("http://example.com/foo/bar")
            urllib2.close()

            last_send = time.time()

