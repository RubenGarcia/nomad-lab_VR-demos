# note, copy this file to VR-demos folder first
FROM bitnami/apache:2.4

# copy NOMAD webservices
COPY ./webserver/htdocs/NOMAD/  /opt/bitnami/apache/htdocs/NOMAD/
COPY ./webserver/cgi-bin/NOMAD/ /opt/bitnami/apache/cgi-bin/NOMAD/
COPY ./webserver/pass /opt/bitnami/apache/

# allow cgi-bin
RUN sed -i 's%#LoadModule cgid_module modules/mod_cgid.so%LoadModule cgid_module modules/mod_cgid.so%' /opt/bitnami/apache/conf/httpd.conf

# add wget. gzip, curl, tar are there by default
USER root
RUN apt-get update \
    && apt-get install -y wget \
    && apt-get install -y zip \
    && rm -rf /var/lib/apt/lists/*


