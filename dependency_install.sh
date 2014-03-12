apt-get install -y gzip git-core curl python-software-properties python g++ make libssl-dev pkg-config build-essential
add-apt-repository ppa:chris-lea/node.js
echo "deb http://us.archive.ubuntu.com/ubuntu/ precise universe" >> /etc/apt/sources.list
apt-get update
apt-get install -y nodejs