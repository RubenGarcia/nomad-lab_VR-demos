FROM bitnami/apache:2.4
COPY ./webserver/htdocs/NOMAD/  /opt/bitnami/apache/htdocs/NOMAD/
COPY ./webserver/cgi-bin/NOMAD/ /opt/bitnami/apache/cgi-bin/NOMAD/
