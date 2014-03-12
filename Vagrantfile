Vagrant::configure("2") do |config|

  # use Ubunut 13.04
  config.vm.box = "raring"
  config.vm.box_url = "https://cloud-images.ubuntu.com/vagrant/raring/current/raring-server-cloudimg-amd64-vagrant-disk1.box"
  config.vm.network :forwarded_port, guest:80, host:3000

  config.vm.provider :virtualbox do |vb|
    vb.customize ["modifyvm", :id, "--ostype", "Ubuntu_64"]
  end

  # install Docker
  config.vm.provision "docker"

end