from bmp import BMPWrapper
from ip_foo import *
from socket import *
import time

debug = 0

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
			address = fwd_netblock(i)
			#address = i
			# FIXME: separate r/g/b?
			# FIXME: deal with intensity?
			pixels.append(address)
		i += 1
	s = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)
	f = open('packets.hex','w')
	for pixel in pixels:
		ip_hdr = build_ip_header( dst = 0x817b0000 | pixel, ttl = 3, proto = 17 )
		udp_hdr = build_udp_header( spt = 1234, dpt = 4321, len = 8 )
		port = 4321
		#try:
		#	s.send( ip_hdr + udp_hdr )
		#except:
		#	print_ip(addr & 0x0000FFFF)
		if debug:
			f.write( ip_hdr + udp_hdr )
		s.sendto( ip_hdr + udp_hdr, (get_ip(pixel),port) )
	del x

draw_bmp('ayb.bmp')
draw_bmp('ayb.bmp')
#draw_bmp('ayb.bmp')

time.sleep(10)

draw_bmp('/home/esk/stickmiles.bmp')
draw_bmp('/home/esk/stickmiles.bmp')
#draw_bmp('/home/esk/stickmiles.bmp')

time.sleep(10)

draw_bmp('/home/esk/obey.bmp')
draw_bmp('/home/esk/obey.bmp')
#draw_bmp('/home/esk/obey.bmp')



