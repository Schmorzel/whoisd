#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <thread>

void error(const char *msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

void thread_task(int newsockfd) {
		int n;
		std::fstream f;
		char buffer[255];
		bzero(buffer,256);
		n = read(newsockfd,buffer,255);
		if (n < 0) {
			error("ERROR reading from socket");
		}
		std::string buffer_str = std::string(buffer);
		boost::replace_all(buffer_str, "/", "");
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

std::string shell_exec(std::string cmd) {
        std::array<char, 1024> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
                std::cerr << "ERROR executing " << cmd << std::endl;
                exit(1);
        }
        while (!feof(pipe.get())) {
                if (fgets(buffer.data(), 1024, pipe.get()) != nullptr) {
                        result += buffer.data();
                }
        }
        return result;
}

std::string read_config(std::string file_path, std::string config_key) {
        std::fstream f;
        std::string result;
        std::string config_value;
        f.open(file_path, std::fstream::in);
        int length = config_key.length();
        while(getline(f, config_value)) {
                if(config_value.substr(0, length) == config_key) {
                        boost::replace_all(config_value, config_key, "");
                        result = config_value;
                        break;
                }
        }
        f.close();
        return result;
}

int main(int argc, char** argv) {
	std::string version = "v1.2.5";
	std::string release_date = "17.1.2018";
	std::string config = "/etc/whois/whoisd.conf";
	std::string update_checker = read_config(config, "update_checker = ");
	if (update_checker == "true") {
		std::string remote_version = shell_exec("curl --silent https://raw.githubusercontent.com/Schmorzel/whoisd/master/.version");
		boost::replace_all(remote_version, "\n", "");
		boost::replace_all(remote_version, "\r", "");
		if (remote_version != version) {
    	            if (remote_version.length() == 0) {
        	                std::cout << "Could not check for updates!" << std::endl;
                	}
                	else {
                        	std::cout << "Update to version " << remote_version << " available!" << std::endl;
                        	std::cout << "To update clone the following Git Repository: https://github.com/Schmorzel/whoisd" << std::endl;
                	}
		}
	}
	if (argc > 1) {
		if (std::string(argv[1]) == "-version" || std::string(argv[1]) == "--version") {
			std::cout << "Whoisd " << version << " " << release_date << std::endl;
			exit(0);
		}
	}
	std::string config_version = read_config(config, "config_version = ");
	if (config_version != "1") {
		shell_exec("echo \"#Whoisd Configuration\n\n#Enable (true)/Disable (false) the Update Checker:\nupdate_checker = true\n\n#Do not touch:\nconfig_version = 1\n\" >/etc/whois/whoisd.conf");
	}
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
		std::thread t(thread_task, newsockfd);
		t.detach();
	}
	close(sockfd);
	return 0;
}
