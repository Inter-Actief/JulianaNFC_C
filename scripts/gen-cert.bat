@ECHO OFF

WHERE openssl >nul
IF %ERRORLEVEL% == 0 (
	openssl req -new -x509 -nodes -days 3650 -keyout juliana.key -out juliana.crt -subj "/C=NL/ST=Overijssel/L=Enschede/O=Stichting Borrelbeheer Zilverling/CN=localhost"
) ELSE (
	echo OpenSSL was not found on your system.
)
