#!/usr/bin/python

from bmp import BMPWrapper
from ip_foo import *
from socket import *
import time
import sys
import random
import xkcd

import math

from PIL import Image

# IP address of our UEN box
src = 0xcd790082

debug = 0
#debug = 1

P=0
R=1
G=2
B=3

random.seed()

dims = { 'x':256, 'y':256 }

pktstore = []

def get_ip( number ):
	return "129.123.%d.%d" % ( int(number/dims['x']), number%dims['x'] )

def to_addr( int ):
	return "%d.%d.%d.%d" % ( int>>24 & 0xff, int>>16 & 0xff, int>>8 & 0xff, int & 0xff)

seq = 1234
spt = 1
#s = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)
s = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)
f = open('packets.hex','w')
port = 4321
tcp_data="Random data for a TCP packet..."

def ip_to_int( ip ):
	(a,b,c,d) = map(int,ip.split('.'))
	return a<<24|b<<16|c<<8|d

def red(ip, n=3):
	udp_ip_hdr = build_ip_header( dst = ip_to_int(ip), ttl = 3, proto = 17 )
	udp_hdr = build_udp_header( spt = 1234, dpt = 4321, len = 8 )
	if debug:
		f.write( udp_ip_hdr + udp_hdr )
	i = 0
	while i < n: 
		s.sendto( udp_ip_hdr + udp_hdr, (ip,port) )
		i+=1
		
def green(ip, n=3):
	tcp_pkt = build_tcp_syn_packet( src=src, dst=ip_to_int(ip), spt=spt, dpt=4321, seq=seq, ttl=3, data="meh " )
	if debug:
		f.write( tcp_pkt )
	i = 0
	while i < n: 
		s.sendto( tcp_pkt, (ip,port) )
		i+=1

def blue(ip, n=3):
	pkt=build_icmp_echo_packet(dst = ip_to_int(ip), ttl = 3 )
	if debug:
		f.write( pkt )
	i = 0
	while i < n : 
		s.sendto( pkt, (ip,port) )
		i+=1


def fwd_netblock( num ):
	x_inner = num % 16
	x_outer = (num / 16).__int__() % 16
	y_inner = (num / 256 ).__int__() % 16
	y_outer = (num / 4096 ).__int__() % 16
	return y_outer * 4096 + x_outer * 256 + y_inner * 16 + x_inner

def rev_netblock( num ):
	x_inner = num % 16
	y_inner = (num / 16).__int__() % 16
	x_outer = (num / 256 ).__int__() % 16
	y_outer = (num / 4096 ).__int__() % 16
	return y_outer * 4096 + y_inner * 256 + x_outer * 16 + x_inner

def fwd_xkcd( num ):
	return xkcd.forward_xkcd[num]

def rev_xkcd( num ):
	return xkcd.reverse_xkcd[num]

def get_rgb( pixel, img_type='rgba' ):
	if img_type == 'rgba':
		r = pixel >> 24 & 0xFF
		g = pixel >> 16 & 0xFF
		b = pixel >>  8 & 0xFF
		a = pixel       & 0xFF
	elif img_type == 'argb':
		a = pixel >> 24 & 0xFF
		r = pixel >> 16 & 0xFF
		g = pixel >>  8 & 0xFF
		b = pixel       & 0xFF
	else:
		raise Exception('Unrecognized pixel format: %s' % img_type)

	return (a,r,g,b)


def get_pixels_from_image( filename ):
	pixels = []

	im = Image.open(filename).convert('P').convert('RGB')

	def get_count(val):
		max_val = 5
		return int(val * max_val / 256)
	
	for y in xrange(256):
		for x in xrange(256):
			r,g,b = im.getpixel( (x,y) )
			if r or g or b:
				pixel = fwd_map(256*(256-y)+x)
				pixels.append( (pixel, get_count(r), get_count(g), get_count(b)) )

	return pixels


def get_addresses_from_bmp( filename ):
	x = BMPWrapper( filename )
	i = 0
	r_addr = []
	g_addr = []
	b_addr = []

	for k in x.__dict__.keys():
		if k[:2] in ('bf','bi'):
			print k, x.__dict__[k]

	while i < x.biWidth*x.biHeight:
		if x.rgbdata[i]:
			(a,r,g,b) = get_rgb(x.rgbdata[i], img_type)
			pixel = fwd_map(i)
			#address = i
			# FIXME: separate r/g/b?
			# FIXME: deal with intensity?
			if r:
				r_addr.append(to_addr(0x817b0000 |fwd_map(i)))
			if g:
				g_addr.append(to_addr(0x817b0000 |fwd_map(i)))
			if b:
				b_addr.append(to_addr(0x817b0000 |fwd_map(i)))
		i += 1
	return ( r_addr, g_addr, b_addr, )

def get_pixels_from_bmp( filename ):
	x = BMPWrapper( filename )
	i = 0
	pixels = []
	max_brightness = 4

	for k in x.__dict__.keys():
		if k[:2] in ('bf','bi'):
			print k, x.__dict__[k]

	while i < x.biWidth*x.biHeight:
		if x.rgbdata[i]:
			(a,r,g,b) = get_rgb(x.rgbdata[i], img_type)
			pixel = fwd_map(i)
			#address = i
			# FIXME: separate r/g/b?
			# FIXME: deal with intensity?
			red =   int( max_brightness/255.0 * r )
			green = int( max_brightness/255.0 * g )
			blue =  int( max_brightness/255.0 * b )
			if a and ( red or green or blue ):
				pixels.append( (pixel,red,green,blue) )
		i += 1
	del x
	return pixels

def draw_pixels( pixels ):
	seq = 1234
	spt = 1
	#s = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)
	s = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)
	f = open('packets.hex','w')
	port = 4321
	tcp_data="Random data for a TCP packet..."
	print time.time(), "\tpregenerating packets"
	for pixel in pixels:
		if( pixel[R] ):
			udp_ip_hdr = build_ip_header( dst = 0x817b0000 | pixel[P], ttl = 3, proto = 17 )
			udp_hdr = build_udp_header( spt = 1234, dpt = 4321, len = 8 )
			if debug:
				f.write( udp_ip_hdr + udp_hdr )
			i = 0
			#while i < pixel[R] : 
			#	s.sendto( udp_ip_hdr + udp_hdr, (get_ip(pixel[P]),port) )
			#	i+=1
			pktstore.append((pixel[R], udp_ip_hdr + udp_hdr, (get_ip(pixel[P]),port)))
		
		if( pixel[G] ):
			# send this packet pixel[G] times for brightness
			i=0
			tcp_pkt = build_tcp_syn_packet( src=src, dst=0x817b0000 | pixel[P], spt=spt, dpt=4321, seq=seq, ttl=3, data="meh " )
			if debug:
				f.write( tcp_pkt )
			#while i < pixel[G] : 
				#tcp_ip_hdr = build_ip_header( dst = 0x817b0000 | pixel[P], ttl = 3, proto = 6, flags=4 )
				#tcp_hdr = build_tcp_header( spt = 1234, dpt = 4321, flags=2, seq=random.getrandbits(32), win=2048 )
				#tcp_pkt = tcp_ip_hdr + tcp_hdr + tcp_data
			#	s.sendto( tcp_pkt, (get_ip(pixel[P]),port) )
			#	seq += 1
			#	spt += 1
			#	i+=1
			pktstore.append((pixel[G], tcp_pkt, (get_ip(pixel[P]),port)))

		if( pixel[B] ):
			#icmp_ip_hdr = build_ip_header( dst = 0x817b0000 | pixel[P], ttl = 3, proto = 1 )
			#icmp_hdr = build_icmp_header(type=8)
			#pkt = icmp_ip_hdr+ icmp_hdr
			pkt=build_icmp_echo_packet(dst = 0x817b0000 | pixel[P], ttl = 3 )
			if debug:
				f.write( pkt )
			i = 0
			#while i < pixel[B] : 
			#	s.sendto( pkt, (get_ip(pixel[P]),port) )
			#	i+=1
			pktstore.append((pixel[B], pkt, (get_ip(pixel[P]),port)))

	t0 = time.time()
	count = 0
	while count < 2:
		print time.time(), "\tsending packets, count:", count
		for sendpkt in pktstore:
			for numberpkt in range(sendpkt[0]):
				s.sendto(sendpkt[1], sendpkt[2])
		t1 = time.time()
		delta = t1-t0
		t0 = t1
		if delta < 10:
			print time.time(), "\tsleeping"
			time.sleep(10 - delta)
		count += 1

def shift_pixels( pixels, x, y, wrap=True ):
	newpixels = []
	for pos, r, g, b in pixels:
		new_pos_xy = fwd_map( pos ) + x + dims['x'] * y
		if new_pos_xy < ( dims['x'] * dims['y'] ) or wrap:
			new_pos = rev_netblock( new_pos_xy % ( dims['x'] * dims['y'] ) )
			newpixels.append( ( new_pos, r, g, b, ) )
	return newpixels

def draw_image( filename, ):
	print time.time(), "getting image data"
	pixels = get_pixels_from_image(filename)
	#print time.time(), "randomizing pixels"
	#random.shuffle(pixels)
	print time.time(), "drawing"
	draw_pixels( pixels )
	print time.time(), "done"

def set_map( fwd ):
	global fwd_map
	fwd_map = fwd

def alt_draw( filename ):
	r, g, b = get_addresses_from_bmp(filename)
	print type(r),len(r)
	for i in r:
		red(i)
	for i in g:
		green(i)
	for i in b:
		blue(i)

if __name__ == "__main__":
	print sys.argv
	global fwd_map
	fwd_map = fwd_netblock
	#time.sleep(10)
	if len(sys.argv) > 1:
		picture = sys.argv[1]
	else:
		picture = "panic.bmp"

	draw_image(picture)



