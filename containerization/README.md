Docker file to build a container to perform the VR conversion webservices
* Check ../webserver/README.md

* copy the Dockerfile to .. (VR-demos -folder)
* install docker
* add your NOMAD username and password in webserver/pass
* build container
   sudo docker build -t nomadvr .
* run container
   sudo docker run -d -p 8080:8080 nomadvr
* Test e.g. with you.rip.add.res:8080/cgi-bin/NOMAD/material?109702
* stop container
  sudo docker stop <container-name>

