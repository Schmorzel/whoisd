#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>
#include <algorithm>

void error(const char *msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

int main(int argc, char** argv) {
	std::fstream f;
	f.open("/run/whoisd.pid", std::ios::out);
	f << getpid();
	f << "\n";
	f.close();
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in6 serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi("43");
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}
	while(true) {
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			error("ERROR on accept");
		}
		bzero(buffer,256);
		n = read(newsockfd,buffer,255);
		if (n < 0) {
			error("ERROR reading from socket");
		}
		std::string buffer_str = std::string(buffer);
		std::string file_path = "/etc/whois/db/" + buffer_str;
		std::transform(file_path.begin(), file_path.end(), file_path.begin(), ::tolower);
		boost::replace_all(file_path, " ", "");
		boost::replace_all(file_path, "\n", "");
		boost::replace_all(file_path, "\r", "");
		std::string response = "Domain or IP not found in Whois Database\n";
		f.open(file_path.c_str(), std::fstream::in);
		getline(f, response, '\0');
		f.close();
		int response_length = response.length();
		char response_char[response_length];
		strncpy(response_char, response.c_str(), response_length);
		n = write(newsockfd, response_char, response_length);
		if (n < 0) {
			error("ERROR writing to socket");
		}
		close(newsockfd);
	}
	close(sockfd);
	return 0;
}
