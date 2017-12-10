whoisd  
Copyright (c) Tim Zimmermann <tizimmermann01@gmail.com>  
  
Description:   
whoisd is a server software compatible with the whois(1) command  
on most linux and unix systems.  It can serve domain name, IPv4    
and IPv6  whois records when queried and uses a linux / unix   
filesystem structure for storing all the records.   
  
Requirements:    
- Linux / Unix based operating system  
- g++  
- make  
- libboost-dev  
  
Usage:  
In order to use the server, simply run the whois(1) command  in  
any operating system with the -h flag followed by your server's  
IP Address and then the  query.  An  example  asking  localhost  
about 192.168.178.2 is: whois -h localhost 192.168.178.2.  
  
Storage Structure:  
The database that is served by whoisd is using the linux / unix  
filesystem in order to store content. All whois content is   
located in "/etc/whois/db/". The name of the files has to be   
the domain name/IPv4/IPv6 adress. An example for 192.168.178.2:  
"/etc/whois/db/192.168.178.2". The content of the file will be   
sent to the client as it is, it won't be modified by the server.   
You don't need to restart the whois server to load changes.    
  
