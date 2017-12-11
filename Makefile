default:
	g++ whoisd.cpp -I./ -o whoisd

install:
	@mkdir -p /etc/whois/
	@mkdir -p /etc/whois/db/
	@cp ./whoisd /usr/bin/
	@echo "[Unit]\nDescription=Whois Server\n\n[Service]\nExecStart=/usr/bin/whoisd\nPIDFile=/run/whoisd.pid\n\n[Install]\nWantedBy=multi-user.target\n" > /lib/systemd/system/whoisd.service
	systemctl enable whoisd.service

uninstall:
	@systemctl stop whoisd.service
	systemctl disable whoisd.service
	@rm -f /lib/systemd/system/whoisd.service
	@rm -f /usr/bin/whoisd
