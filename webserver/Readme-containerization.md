# Overview of container set up

* install Docker 
* add your NOMAD username and password in webserver/pass 
* build container 
```
 sudo docker build -t nomadvr . 
```
* run container 
```
 sudo docker run -d -p 8080:8080 nomadvr 
```
* Test e.g. with 
```
 http://you.rip.add.res:8080/cgi-bin/NOMAD/material?109702 
```
* the json files in the zip should not be zero size, if they are, it's likely authentication issue
* stop container
```
 sudo docker stop 
```
