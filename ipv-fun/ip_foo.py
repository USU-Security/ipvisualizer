from socket import *
import struct

def build_ip_header( **kw ):
	if( kw.has_key('ver') ):
		ver = kw['ver']
	else:
		ver = 4
	if( kw.has_key('ihl') ):
		ihl = kw['ihl']
	else:
		ihl = 0x5
	if( kw.has_key('tos') ):
		tos = kw['tos']
	else:
		tos = 0
	if( kw.has_key('len') ):
		len = kw['len']
	else:
		len = 0 # minimum
	if( kw.has_key('id') ):
		id = kw['id']
	else:
		id = 0
	if( kw.has_key('flags') ):
		flags = kw['flags']
	else:
		flags = 0
	if( kw.has_key('frag_offset') ):
		frag_offset = kw['frag_offset']
	else:
		frag_offset = 0
	if( kw.has_key('ttl') ):
		ttl = kw['ttl']
	else:
		ttl = 10
	if( kw.has_key('proto') ):
		proto = kw['proto']
	else:
		proto = 6
	if( kw.has_key('checksum') ):
		checksum = kw['checksum']
	else:
		checksum = 0
	if( kw.has_key('src') ):
		src = kw['src']
	else:
		src = 0
	if( kw.has_key('dst') ):
		dst = kw['dst']
	else:
		dst = 0
	if( kw.has_key('opts') ):
		opts = kw['opts']
		print "options ignored"
	else:
		opts = 0
	IPHdrFmt = 'BBHHHBBHII'
	ip_hdr = struct.pack(IPHdrFmt, ver << 4 | ihl, tos, htons( len ), htons( id ),
			htons( flags << 13 | frag_offset ), ttl, proto, htons( checksum ),
			htonl( src ), htonl( dst ) )
	return ip_hdr

def build_tcp_header( **kw ):
	if( kw.has_key('spt') ):
		spt = kw['spt']
	else:
		spt = 0
	if( kw.has_key('dpt') ):
		dpt = kw['dpt']
	else:
		dpt = 0
	if( kw.has_key('seq') ):
		seq = kw['seq']
	else:
		seq = 0
	if( kw.has_key('ack') ):
		ack = kw['ack']
	else:
		ack = 0
	if( kw.has_key('offset') ):
		offset = kw['offset']
	else:
		offset = 0
	if( kw.has_key('flags') ):
		flags = kw['flags']
	else:
		flags = 0
	if( kw.has_key('win') ):
		win = kw['win']
	else:
		win = 0
	if( kw.has_key('checksum') ):
		checksum = kw['checksum']
	else:
		checksum = 0
	if( kw.has_key('urg') ):
		urg = kw['urg']
	else:
		urg = 0
	if( kw.has_key('opts') ):
		opts = kw['opts']
	else:
		opts = 0
	TCPHdrFmt = 'HHIIBBHHHI'
	tcp_hdr = struct.pack( TCPHdrFmt, htons(spt), htons(dpt), htonl(seq), htonl(ack), offset << 4,
			flags, htons(win), htons(checksum), htons(urg), htonl(opts) )
	return tcp_hdr

def build_udp_header( **kw ):
	if( kw.has_key('spt') ):
		spt = kw['spt']
	else:
		spt = 0
	if( kw.has_key('dpt') ):
		dpt = kw['dpt']
	else:
		dpt = 0
	if( kw.has_key('len') ):
		len = kw['len']
	else:
		len = 8
	if( kw.has_key('checksum') ):
		checksum = kw['checksum']
	else:
		checksum = 0
	UDPHdrFmt = 'HHHH'
	udp_hdr = struct.pack( UDPHdrFmt, htons(spt), htons(dpt), htons(len), htons(checksum) )
	return udp_hdr

def build_icmp_header( **kw ):
	if( kw.has_key('type') ):
		type = kw['type']
	else:
		type = 0
	if( kw.has_key('code') ):
		code = kw['code']
	else:
		code = 0
	if( kw.has_key('checksum') ):
		checksum = kw['checksum']
	else:
		checksum = 0
	if( kw.has_key('id') ):
		id = kw['id']
	else:
		id = 0
	if( kw.has_key('seq') ):
		seq = kw['seq']
	else:
		seq = 0
	ICMPHdrFmt = 'BBHHH'
	icmp_hdr = struct.pack(ICMPHdrFmt, type, code, htons(checksum),
			htons(id), htons(seq) )
	return icmp_hdr


def build_packet_x( ):
	ip_hdr = build_ip_header( dst = 0x817b0761, proto=17 )
	#udp_hdr = build_udp_header( spt=12345, dpt=54321 )
	icmp_hdr = build_icmp_header( type=8, code=0, id=1337 )
	return ip_hdr + icmp_hdr


