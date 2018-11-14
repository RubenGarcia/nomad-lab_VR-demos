# Copyright 2018 CSC - IT Center for Science
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM centos/httpd-24-centos7

USER root
# add wget, zip and httpd. gzip, curl, tar are there by default
RUN yum update -y \
 && yum install -y zip wget


ARG SERVERROOT=/var/www/html
ARG CGIBIN=/var/www/cgi-bin

# copy NOMAD webservices
COPY ./htdocs/NOMAD/  $SERVERROOT/htdocs/NOMAD/
COPY ./cgi-bin/NOMAD/ $CGIBIN/NOMAD/
# copy NOMAD username and password
COPY ./pass $CGIBIN/../

USER 1001
