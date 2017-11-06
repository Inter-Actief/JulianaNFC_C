#!/bin/sh

if ! [ -x "$(command -v openssl)" ]; then
	echo "OpenSSL not found on your system." >&2
	exit 1
fi

openssl req -new -x509 -nodes -days 3650 -keyout juliana.key -out juliana.crt -subj "/C=NL/ST=Overijssel/L=Enschede/O=Stichting Borrelbeheer Zilverling/CN=localhost"

