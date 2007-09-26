from bmp import BMPWrapper
from ip_foo import *
from socket import *
import time

debug = 1

def get_ip( number ):
	return "129.123.%d.%d" % (number%256,(number/256).__int__())


def fwd_netblock( num ):
	x_inner = num % 16
	x_outer = (num / 16).__int__() % 16
	y_inner = (num / 256 ).__int__() % 16
	y_outer = (num / 4096 ).__int__() % 16
	return y_outer * 4096 + x_outer * 256 + y_inner * 16 + x_inner

def rev_netblock( num ):
	x_inner = num % 16
	y_inner = (num / 16).__int__() % 16
	x_outer = (num / 256 ).__int__() % 26
	y_outer = (num / 4096 ).__int__() % 26
	return y_outer * 4096 + x_outer * 256 + y_inner * 16 + x_inner

def draw_bmp( filename ):
	x = BMPWrapper( filename )
	i = 0
	pixels = []
	while i < x.biWidth*x.biHeight:
		if x.rgbdata[i]:
			a = x.rgbdata[i] >> 24 & 0xFF
			r = x.rgbdata[i] >> 16 & 0xFF
			g = x.rgbdata[i] >>  8 & 0xFF
			b = x.rgbdata[i]       & 0xFF
			pixel = fwd_netblock(i)
			#address = i
			# FIXME: separate r/g/b?
			# FIXME: deal with intensity?
			red =   int( 4/255.0 * r )
			green = int( 4/255.0 * g )
			blue =  int( 4/255.0 * b )
			if a:
				pixels.append( (pixel,red,green,blue) )
		i += 1
	s = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)
	f = open('packets.hex','w')
	port = 4321
	P=0
	R=1
	G=2
	B=3
	for pixel in pixels:
		if( pixel[R] ):
			udp_ip_hdr = build_ip_header( dst = 0x817b0000 | pixel[P], ttl = 3, proto = 17 )
			udp_hdr = build_udp_header( spt = 1234, dpt = 4321, len = 8 )
			if debug:
				f.write( udp_ip_hdr + udp_hdr )
			i = 0
			while i < pixel[R] : 
				s.sendto( udp_ip_hdr + udp_hdr, (get_ip(pixel[P]),port) )
				i+=1
		
		if( pixel[G] ):
			tcp_ip_hdr = build_ip_header( dst = 0x817b0000 | pixel[P], ttl = 3, proto = 6 )
			tcp_hdr = build_tcp_header( spt = 1234, dpt = 4321, len = 0 )
			if debug:
				f.write( tcp_ip_hdr + tcp_hdr )
			i=0
			while i < pixel[G] : 
				s.sendto( tcp_ip_hdr + tcp_hdr, (get_ip(pixel[P]),port) )
				i+=1

		if( pixel[B] ):
			icmp_ip_hdr = build_ip_header( dst = 0x817b0000 | pixel[P], ttl = 3, proto = 1 )
			icmp_hdr = build_icmp_header(type=8)
			if debug:
				f.write( icmp_ip_hdr + icmp_hdr )
			i = 0
			while i < pixel[B] : 
				s.sendto( icmp_ip_hdr + icmp_hdr, (get_ip(pixel[P]),port) )
				i+=1

	del x

#draw_bmp('ayb.bmp')
#draw_bmp('ayb.bmp')
#draw_bmp('ayb.bmp')

#time.sleep(10)

#draw_bmp('/home/esk/stickmiles.bmp')
#draw_bmp('/home/esk/stickmiles.bmp')
#draw_bmp('/home/esk/stickmiles.bmp')

#time.sleep(10)

#draw_bmp('/home/esk/obey.bmp')
#draw_bmp('/home/esk/obey.bmp')
#draw_bmp('/home/esk/obey.bmp')

draw_bmp('/home/esk/panic.bmp')



