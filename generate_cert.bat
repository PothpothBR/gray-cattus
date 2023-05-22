openssl genrsa -out CAKey.pem 2048
openssl req -config "./openssl.cnf" -x509 -sha256 -new -nodes -key CAKey.pem -out CACert.pem