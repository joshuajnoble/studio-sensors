
frog studio sensor

================================================================================

Let's keep it simple at first: there's a server and you can pull info from it. Here's what some of that info looks like:

```
[{"id":134,"sensor_id":1,"time":"2014-03-24T23:48:55.442Z","light":1,"sound":114,"movement":1,"temp":18,"humidity":147,"brightness":0},
{"id":135,"sensor_id":1,"time":"2014-03-24T23:49:20.120Z","light":1347,"sound":562,"movement":0,"temp":34,"humidity":3,"brightness":0}]
```

Whoa. That's JSON!

The frog studio sensors log all their data up there to my server every 15 seconds. 

Ok, so we have a few things in there:

* sensor_id: what sensor is it?
* studio: what studio is it in?
* zone: what part of the studio is it in?
* time: that's a timestamp (ignore the Z, it just means it's using the current timezone, which, yeah, it is)
* sound: scale from 50 - 400 (afaict)
* movement: has anything moved in that zone in the last 30 seconds?
* humidity: what's the relative humidity (percent)
* temperature: Celsius
* light: light frequency in values from 100-2000 with the relative color temperature of the light sensor
* brightness: not being used right now

The raspberry pi just logs its data using a URL like:

`http://OUR_URL:3000/?i=1&l=343&s=232&t=22&h=33&m=0&&studio=sea&zone=1`

You can make fake (test) posts all you want.

===============================================
VISUALIZATION
===============================================

Where's the visualization? Good question. Have you made one yet? No? Well there's your answer. This is meant to look different to each studio. There are a few basic pattern goes like so:

`/view?params`

Let's look at some more stuff in here. You're *probably* going to want to just use the following:

`URL:3000/view?studio=SEA&begindate=24032014&begintime=1600&enddate=24032014&endtime=1700`

That gives you all the data from Seattle from 24032014:1600 to 24032014:1700 thats DDMMYY:24HR

There is more though, be warned though, these datasets can get massive and pulling them all is *slow*

All studios by date:

`URL:3000/view?begindate=24032014&begintime=1600&enddate=24032014&endtime=1700`

The last reading by zone

`URL:3000/view?studio=SEA&zone=1&last=1`

All readings by zone

`URL:3000/view?studio=SEA&zone=1`

All readings by sensorid (good for debugging)

`URL:3000/view?id=1`

Timeboxed view of a particular zone:

`URL:3000/?i=1&l=343&s=232&t=22&h=33&m=0&&studio=sea&zone=1`

All readings from the last hour

`URL:3000/view?recent=1&zone=1&studio=SEA`

The last reading of a studio

`URL:3000/view?now=1&studio=SEA`

Want more? It's very easy to add. Just take a look at studio-sensors/server.js for tips on how to make it better.

===============================================
DEVICE INFO
===============================================

The device is a Raspberry pi with some sensors on it. The first thing the device does is start up, report its IP, studio (within frog), location (zone), and then get a sensor id that will follow it as long as it (it, in this case being the SD card) exists.

This is accessed like so:

`$URL/get_id?studio=SEA&zone=1&ip=172.189.128.112`

When the machine is kicked off the network and hops back on that it updates its IP. This is done like so:

`$URL/update_id?id=1&ip=172.189.128.112`

The id you see there is the same ID that is retrieved when the machine is first started up. 

Why is this "id => IP" mapping important? Well, so that you can always SSH into your device and see what's up with it, if it crashed, if the DHT22 burnt out, etc. That's what makes it worthwhile to have everything on the RPI.

To configure the device to run the studio sensor program you need to do a few different things first.

1) Install the drivers for the Edimax EW-7811Un USB WiFi chip

2) Install pigpio

3) Install Adafruit DHT22 driver

===============================================
SERVER INFO
===============================================

I have my server set up to use upstart to start up our node application, it's logging to :"/etc/init/studioapi.conf" at the moment so you can see what's going on with it.

I'm using ec2 ubuntu.

To get things set up I just did the following:

`sudo apt-get install postgres node`

then 

`sudo vim /etc/postgresql/9.3/main/postgresql.conf`

uncomment the line that says: `#listen_addresses = 'localhost' `
then
`sudo vim /etc/postgresql/9.3/main/pg_hba.conf`

then

`sudo service postgresql start`

then

```
sudo -i -u postgres
ubuntu@ip-172-31-30-171:~$ sudo -i -u postgres
postgres@ip-172-31-30-171:~$ psql
psql (9.3.5)
Type "help" for help.

ALTER USER postgres WITH PASSWORD '$password';
```

Create a job:

`sudo vim /etc/init/studioapi.conf`

then add:

```
description "node.js server"
author     "josh"

# used to be: start on startup
# until we found some mounts weren't ready yet while booting:
start on started mountall
stop on shutdown

# Automatically Respawn:
respawn
respawn limit 99 5

script
    # Not sure why $HOME is needed, but we found that it is:
    export HOME="/root"

    exec /usr/bin/node /home/joshuajnoble/studio-sensors/server.js >> /var/log/node.log 2>&1
end script

post-start script
   # Optionally put a script here that will notifiy you node has (re)started
   # /root/bin/hoptoad.sh "node.js has started!"
end script
```

===============================================
DB INFO
===============================================

Creating the DB is pretty simple:

create database sensordata;

create table sensors ( studio varchar(3), zone varchar(10), id integer, constraint uid unique(id), ip varchar(16) );

create table readings ( id serial, sensor id int not null, time timestamp with time zone not null, light int not null, sound int not null, movement int not null, temp int not null, humidity int not null, brightness int not null, zone varchar(30) not null, studio varchar(30) not null);


