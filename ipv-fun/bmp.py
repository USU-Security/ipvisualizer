import struct
import array

# Apparently a .bmp is little-endian
BMPFileHeaderFmt = '<HLHHL'
BMPInfoHeaderFmt = '<LLLHHLLLLLL'

class BMPWrapper:
	def __init__(self, bmp_filename):
		bmpfile = open( bmp_filename, 'rb' )
		packed_file_hdr = bmpfile.read(struct.calcsize(BMPFileHeaderFmt))
		file_hdr = struct.unpack( BMPFileHeaderFmt, packed_file_hdr )
		del packed_file_hdr
		self.bfType = file_hdr[0]
		self.bfSize = file_hdr[1]
		self.bfReserved1 = file_hdr[2]
		self.bfReserved2 = file_hdr[3]
		self.bfOffBits = file_hdr[4]
		del file_hdr
		packed_info_hdr = bmpfile.read(struct.calcsize(BMPInfoHeaderFmt))
		info_hdr = struct.unpack( BMPInfoHeaderFmt, packed_info_hdr )
		del packed_info_hdr
		self.biSize = info_hdr[0]
		self.biWidth = info_hdr[1]
		self.biHeight = info_hdr[2]
		self.biPlanes = info_hdr[3]
		self.biBitCount = info_hdr[4]
		self.biCompression = info_hdr[5]
		self.biSizeImage = info_hdr[6]
		self.biXPelsPerMeter = info_hdr[7]
		self.biYPelsPerMeter = info_hdr[8]
		self.biClrUsed = info_hdr[9]
		self.biClrImportant = info_hdr[10]
		del info_hdr
		bmpfile.seek( self.bfOffBits )
		if( self.biSizeImage ):
			len = self.biSizeImage
		else:
			len = -1
		self.str_rgbdata = bmpfile.read( len )
		# FIXME: I for 64-bit os, L for 32-bit os
		if( self.biBitCount == 32):
			BMPDataFmt = 'I'
		#elif( self.biBitCount == 24):
		#	BMPDataFmt = ''
		elif( self.biBitCount == 16 ):
			BMPDataFmt = '<H'
		elif( self.biBitCount == 8 ):
			BMPDataFmt = '<B'
		if( BMPDataFmt ):
			self.rgbdata = array.array( BMPDataFmt,self.str_rgbdata )


	
