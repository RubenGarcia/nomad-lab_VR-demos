# Copyright 2018 Atte Silanpää
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

# note, copy this file to VR-demos folder first
FROM bitnami/apache:2.4

# copy NOMAD webservices
COPY ./webserver/htdocs/NOMAD/  /opt/bitnami/apache/htdocs/NOMAD/
COPY ./webserver/cgi-bin/NOMAD/ /opt/bitnami/apache/cgi-bin/NOMAD/
COPY ./webserver/pass /opt/bitnami/apache/

# allow cgi-bin
RUN sed -i 's%#LoadModule cgid_module modules/mod_cgid.so%LoadModule cgid_module modules/mod_cgid.so%' /opt/bitnami/apache/conf/httpd.conf

# add wget and zip. gzip, curl, tar are there by default
USER root
RUN apt-get update \
    && apt-get install -y wget \
    && apt-get install -y zip \
    && rm -rf /var/lib/apt/lists/*


