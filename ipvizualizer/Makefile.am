SUBDIRS=client server
	
dist-hook:
	echo "#the visualization server. Client only" >> $(distdir)/ipvisualizer.conf
	echo "server = localhost" >> $(distdir)/ipvisualizer.conf
	echo "" >> $(distdir)/ipvisualizer.conf
	echo "#the listening port" >> $(distdir)/ipvisualizer.conf
	echo "port = 17843" >> $(distdir)/ipvisualizer.conf
	echo "" >> $(distdir)/ipvisualizer.conf
	echo "#the local network " >> $(distdir)/ipvisualizer.conf
	echo "localnet = 10.1.0.0/16" >> $(distdir)/ipvisualizer.conf
	echo "" >> $(distdir)/ipvisualizer.conf
	echo "#the path to the binary subnet specification." >> $(distdir)/ipvisualizer.conf
	echo "#Server only" >> $(distdir)/ipvisualizer.conf
	echo "#subnetpath = " >> $(distdir)/ipvisualizer.conf
	echo "" >> $(distdir)/ipvisualizer.conf
	echo "#the authorization for the subnet specification (user:pass). Server only" >> $(distdir)/ipvisualizer.conf
	echo "#auth = user:password" >> $(distdir)/ipvisualizer.conf
	echo "" >> $(distdir)/ipvisualizer.conf
	echo "#which interface to sniff on. defaults to eth0. Server only" >> $(distdir)/ipvisualizer.conf
	echo "#interface = eth0" >> $(distdir)/ipvisualizer.conf
	echo "" >> $(distdir)/ipvisualizer.conf
	echo "" >> $(distdir)/ipvisualizer.conf
	echo "#the following commands are executed when the user interacts with the program" >> $(distdir)/ipvisualizer.conf
	echo "#a %s should be used to indicate where an argument should be stored" >> $(distdir)/ipvisualizer.conf
	echo "#ex:" >> $(distdir)/ipvisualizer.conf
	echo "#rightclickcmd = firefox http://www.networksolutions.com/whois/results.jsp?ip=%s" >> $(distdir)/ipvisualizer.conf
	echo "#" >> $(distdir)/ipvisualizer.conf
	echo "#This command executes with a left click on a single ip as its argument" >> $(distdir)/ipvisualizer.conf
	echo "#leftclickcmd = echo User left clicked on ip %s" >> $(distdir)/ipvisualizer.conf
	echo "#this command executes with a right click on a single ip as its argument" >> $(distdir)/ipvisualizer.conf
	echo "#rightclickcmd = echo User right clicked on ip %s" >> $(distdir)/ipvisualizer.conf
	echo "#This command executes when a network is selected with the right mouse button" >> $(distdir)/ipvisualizer.conf
	echo "#rightclicknet = echo User selected the network %s with the right mouse button" >> $(distdir)/ipvisualizer.conf

install-exec-hook:
	if test -e /etc/ipvisualizer.conf; then echo "Existing Configuration already found"; else cp $(srcdir)/ipvisualizer.conf /etc/; fi
