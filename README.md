ipvisualizer
============

Display activity for local addresses as squares in a grid for up to a /16 network

Run the server:
 * `ipvisualizer_flowgen -c <configfile>`
 * Must specify source (interface or pcap)
 * must specify network

Run the client:
 * `ipvisualizer_display -c <configfile>`
 * must specify server
 * must specify network
 
 Packages:
 * `deb http://mirror.usu.edu/usu-security [codename]-security-unstable`
 * sid, jessie, wheezy
 * someone remind me to build for ubuntu again
 * install `usu-archive-keyring` package and apt-get update
