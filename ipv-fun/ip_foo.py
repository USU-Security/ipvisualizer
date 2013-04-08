from socket import *
import struct

def build_ip_header( ver=4, ihl=0x5, tos=0, len=0, id=0, flags=0, frag_offset=0, ttl=10, proto=6, checksum=0, src=0, dst=0, opts=0 ):
	IPHdrFmt = 'BBHHHBBHII'
	ip_hdr = struct.pack(IPHdrFmt, ver << 4 | ihl, tos, htons( len ), htons( id ),
			htons( flags << 13 | frag_offset ), ttl, proto, htons( checksum ),
			htonl( src ), htonl( dst ) )
	return ip_hdr

def get_tcp_checksum( data ):
	if len(data) % 2:
		data += '\x00'
	sum = 0
	i=0
	while i < len(data):
		sum += struct.unpack( 'H', data[i:i+2] )[0]
		i+=2
	while sum >> 16:
		sum = ( sum & 0xFFFF ) + ( sum >> 16 )
	return ntohs( ( ~sum ) & 0xFFFF )

def build_tcp_header( spt=0, dpt=0, seq=0, ack=0, offset=None, flags=2, win=0, checksum=0, urg=0, opts=None, src=None, dst=None, data='' ):
	TCPHdrFmt = 'HHIIBBHHH'
	if not offset:
		offset=5
		if opts:
			offset+=int( ( len(opts) + 3 ) / 4 )
	tcp_len = 5*4 # tcp header w/out options
	if opts:
		if len(opts) % 4:
			raise Exception("Options must be a multiple of 32 bits.")
		tcp_len += len(opts)
	tcp_len += len(data)
	tcp_hdr = struct.pack( TCPHdrFmt, htons(spt), htons(dpt), htonl(seq), htonl(ack), offset << 4,
			flags, htons(win), htons(checksum), htons(urg))
	if not checksum:
		checksum = get_tcp_checksum( tcp_hdr + opts + struct.pack('LLHH', htonl(src), htonl(dst), htons(6), htons( tcp_len ) ) + data )
		tcp_hdr = struct.pack( TCPHdrFmt, htons(spt), htons(dpt), htonl(seq), htonl(ack), offset << 4,
				flags, htons(win), htons(checksum), htons(urg))
	return tcp_hdr + opts

def build_udp_header( spt=0, dpt=0, len=8, checksum=0 ):
	UDPHdrFmt = 'HHHH'
	udp_hdr = struct.pack( UDPHdrFmt, htons(spt), htons(dpt), htons(len), htons(checksum) )
	return udp_hdr

def build_icmp_header( type=0, code=0, checksum=0, id=0, seq=0, data='', src=None, dst=None ):
	ICMPHdrFmt = 'BBHHH'
	icmp_len=0*4
	icmp_len += len(data)
	icmp_hdr = struct.pack(ICMPHdrFmt, type, code, htons(checksum),
			htons(id), htons(seq) )
	if not checksum:
		checksum=get_tcp_checksum( icmp_hdr + data )
		#checksum=get_tcp_checksum( icmp_hdr, struct.pack('LLHH', htonl(src), htonl(dst), htons(1), htons( icmp_len ) ) + data )
		icmp_hdr = struct.pack(ICMPHdrFmt, type, code, htons(checksum),
				htons(id), htons(seq) )
		
	return icmp_hdr


def build_packet_x( ):
	ip_hdr = build_ip_header( dst = 0x817b0761, proto=17 )
	#udp_hdr = build_udp_header( spt=12345, dpt=54321 )
	icmp_hdr = build_icmp_header( type=8, code=0, id=1337 )
	return ip_hdr + icmp_hdr

def build_icmp_echo_packet( src=0, dst=0, spt=0, dpt=0, ttl=10, seq=0, data='' ):
	ip_hdr = build_ip_header( src=src, dst=dst, flags=0, ttl=ttl, proto=1 )
	icmp_hdr = build_icmp_header( type=8, data=data )
	return ip_hdr + icmp_hdr

def build_tcp_syn_packet( src=0, dst=0, spt=0, dpt=0, ttl=10, seq=0, data='' ):
	#tcp_ip_hdr = build_ip_header( dst = 0x817b0000 | pixel[P], ttl = 3, proto = 6, flags=4 )
	#tcp_hdr = build_tcp_header( spt = 1234, dpt = 4321, flags=2, seq=random.getrandbits(32), win=2048 )
	ip_hdr = build_ip_header( src=src, dst=dst, flags=0, ttl=ttl, proto=6 )
	tcp_opts = "\x02\x04\x05\xb4"
	#tcp_opts = None
	tcp_hdr = build_tcp_header( spt=spt, dpt=dpt, seq=seq, opts=tcp_opts, flags=2, win=1024, src=src, dst=dst, data=data )
	#if data:
	#	return ip_hdr + tcp_hdr + data
	#return ip_hdr + tcp_hdr
	return ip_hdr + tcp_hdr + data


