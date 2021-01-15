#!/bin/bash

USER="vrshell"
PASS="EBDzGIIRYgKe"

SAMLURL=https://encyclopedia.nomad-coe.eu/api/v1.0/saml/

wget --save-cookies cookies.txt \
     --keep-session-cookies \
        --no-check-certificate $SAMLURL -O login.html >/dev/null 2>&1
URL=`grep "form action" login.html |cut -f 2 -d \" |head -n 1`
wget --save-cookies cookies2.txt --keep-session-cookies --load-cookies cookies.txt --no-check-certificate \
        "https://idp.nomad-coe.eu$URL&j_username=$USER&j_password=$PASS&_eventId_proceed" -O a.html >/dev/null 2>&1
URL=`grep "form action" a.html |cut -f 2 -d \" |head -n 1`
wget --save-cookies cookies3.txt --keep-session-cookies --load-cookies cookies2.txt --no-check-certificate \
        "https://idp.nomad-coe.eu$URL&_shib_idp_consentOptions=_shib_idp_globalConsent&_eventId_proceed" \
        -O b.html >/dev/null 2>&1

#now continue, as we don't support javascript :o)
URL2=`echo $URL |cut -f 1 -d \?`
DEST=`grep action b.html |cut -f 2 -d \"`
RELAY=`grep RelayState b.html |cut -f6 -d \"`
SAML=`grep SAMLResponse b.html |cut -f6 -d \"`

RELAY=$SAMLURL

curl -L -c cookies3.txt -X GET -F "RelayState=$RELAY" -F "SAMLResponse=$SAML" "$SAMLURL?acs" -o resp.json >/dev/null 2>&1
TOKEN=`grep data resp.json |cut -f 4 -d \"`

echo $TOKEN
echo $TOKEN > token.txt

rm -f cookies3.txt b.html a.html cookies2.txt login.html cookies.txt resp.json
