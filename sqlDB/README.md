studo-sensors postgresqlDB setup
=============================

This is based on the postgresql setup outlined in [here].

Create the folder that will be mapped into the docker container

* `mkdir -p /var/docker/postgresql/data`

Build and run the docker image
* `docker build -t postgresql .'
* `docker run -d -i -v /var/docker/postgresql/data:/data -p 5432:5432 -t postgresql`

Bash into DB:
* `sudo docker run -rm -i -link ##container_name##:pg -v /var/docker/postgresql/data:/data -t postgresql bash`
* `psql -h $PG_PORT_5432_TCP_ADDR -p $PG_PORT_5432_TCP_PORT -d docker -U docker --password`
* `note: the password is docker`

[here]: http://docs.docker.io/en/latest/examples/postgresql_service/


create DB and table:

create database sensordata;

create table readings ( id serial, sensor_id int not null, time timestamp not null, light int not null, sound int not null, movement int not null, temp int not null, humidity int not null, brightness int not null);

