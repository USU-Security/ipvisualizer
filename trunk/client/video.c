/*
    Copyright 2008 Utah State University    

    This file is part of ipvisualizer.

    ipvisualizer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ipvisualizer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ipvisualizer.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * file:	video.c
 * purpose:	to create a video file 
 * created:	03-15-2007
 * creator:	rian shelley
 */
#include "../config.h"
#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT

#include <stdlib.h>
#include <stdio.h>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <string.h>
#include "../shared/flowdata.h"
#include "video.h"

/* store our global data in here */
struct {
	/* encoding stuff */
	AVCodec* codec;
	AVCodecContext * c;
	FILE* f;
	AVFrame* picture;
	uint8_t * outbuf, *picture_buf;
	int outbuf_size;

	/* file formatting foo */
	AVOutputFormat *fmt;
	AVFormatContext *oc;
	AVStream *video_st;
	double video_pts;
} vid;

/* this is declared in main */
void displaymsg(const char* s);

/*
 * function:	videoinit()
 * purpose:		to initialize the library. needs called only once in the program
 */
void videoinit()
{
	av_register_all();
/*	avcodec_init();
	avcodec_register_all(); */
}


/*
 * function:	startvideo()
 * purpose:		to start a video stream
 * recieves:	the path of the file to create
 */
int startvideo(const char* moviefile, int imgwidth, int imgheight)
{
	displaymsg("Beginning Encoding");

	vid.fmt = guess_format(NULL, moviefile, NULL);
	if (!vid.fmt) {
		vid.fmt = guess_format("mpeg", NULL, NULL);
	}
	if (!vid.fmt) {
		displaymsg("Unable to use output format");
		return 0;
	}

	if (!(vid.oc = av_alloc_format_context()))
	{
		displaymsg("Unable to allocate file formatting resources");
		return 0;
	}
	vid.oc->oformat = vid.fmt;
	snprintf(vid.oc->filename, sizeof(vid.oc->filename), "%s", moviefile);
	
	/* TODO: fmt->video_codec will have a default codec. perhaps you should use the default
	 * if you can't allocate the one you want */
/*	vid.fmt->video_codec = CODEC_ID_RAWVIDEO; */
	vid.fmt->video_codec = CODEC_ID_FFV1; 
//	vid.fmt->video_codec = CODEC_ID_HUFFYUV;
	

	

	
	if (!(vid.video_st = av_new_stream(vid.oc, 0))) 
	{
		displaymsg("Unable to create stream");
		return 0;
	}
	
	vid.c = vid.video_st->codec;

	vid.c->codec_id = vid.fmt->video_codec;
	vid.c->codec_type = CODEC_TYPE_VIDEO;
	
	vid.c->bit_rate = 1000000;
	vid.c->width = imgwidth;
	vid.c->height = imgheight;

	vid.c->time_base = (AVRational){1, 24};
	vid.c->gop_size = 12;
/*	vid.c->max_b_frames = 1; */
#if COLORDEPTH == 4
	vid.c->pix_fmt = PIX_FMT_RGBA32;
#else
	vid.c->pix_fmt = PIX_FMT_BGR24;
#endif


	vid.codec = avcodec_find_encoder(vid.c->codec_id);
	if (!vid.codec) {
		displaymsg("Cannnot find desired codec");
		return 0;
	}




	vid.picture = avcodec_alloc_frame();


	/* open the encoder */
	if (avcodec_open(vid.c, vid.codec) < 0) {
		displaymsg("Cannot open codec");
		return 0;
	}

	if (url_fopen(&(vid.oc->pb), vid.oc->filename, URL_WRONLY))
	{
		displaymsg("Unable to open file");
		return 0;
	}

	/* prepare a buffer */
	vid.outbuf_size = imgwidth*imgheight*COLORDEPTH;
	vid.outbuf = malloc(vid.outbuf_size);

	//vid.picture_buf = malloc(imgwidth*imgheight*4);
	//memset(vid.picture_buf+(3*imgwidth*imgheight), 0, imgwidth*imgheight);
	//vid.picture->data[0] = vid.picture_buf;
	//vid.picture->data[1] = vid.picture_buf + (imgwidth*imgheight);
	//vid.picture->data[2] = vid.picture_buf + (2 * imgwidth*imgheight);
	//vid.picture->data[3] = vid.picture_buf + (3 * imgwidth*imgheight);
	vid.picture->linesize[0] = imgwidth * COLORDEPTH;
//	vid.picture->linesize[1] = imgwidth;
//	vid.picture->linesize[2] = imgwidth;
//	vid.picture->linesize[3] = imgwidth;
	av_set_parameters(vid.oc, NULL);
	dump_format(vid.oc, 0, vid.oc->filename, 1);
	av_write_header(vid.oc);
	
	/* set negative so that when it reads each line, it will progress *backward* 
	 * this is done so that the image is oriented correctly, as it is displayed 
	 * bottom up, but encoded top down */
	vid.picture->linesize[0] *= -1;
	return 1;
}

/*
 * function:	writeframe()
 * purpose:		to write a frame to a file
 * recieves:	a reference to the imgdata
 */
void writeframe(uint8_t * imgdata, int imgwidth, int imgheight) 
{
	vid.video_pts = (double)vid.video_st->pts.val * vid.video_st->time_base.num / vid.video_st->time_base.den;
	
	/* *sigh* another slow copy. if i need to convert to yuv, here is the place */
	/* silly me. this only needs to be done for planar (YUV) formats, rgb is ok
	int i;
	for (i = 0; i < imgwidth * imgheight; i++) 
	{
		vid.picture->data[0][i] = imgdata[i][0];
		vid.picture->data[1][i] = imgdata[i][1];
		vid.picture->data[2][i] = imgdata[i][2];
	}
	*/
	/* since it reads it backwards, set it to the last data element */
	vid.picture->data[0] = &imgdata[(imgheight-1)*imgwidth];
	int outsize = avcodec_encode_video(vid.c, vid.outbuf, vid.outbuf_size, vid.picture);

	if (outsize > 0) /* codec might buffer image for later processing */
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		/* TODO: try without this first, i believe they are one-to-one */
		//pkt.pts = av_rescale_q(vid.c->coded_frame->pts, vid.c->time_base, vid.video_st->time_base);
		if (vid.c->coded_frame->key_frame)
			pkt.flags |= PKT_FLAG_KEY;
		pkt.stream_index = vid.video_st->index;
		pkt.data = vid.outbuf;
		pkt.size = outsize;

		/* finally. write the packet */
		/* TODO: handle error return here */
		av_write_frame(vid.oc, &pkt);
	}
}

/*
 * function:	endvideo()
 * purpose:		to stop encoding a video stream
 */
void endvideo()
{
	/* grab whats left */
	unsigned int outsize, i;
	while ((outsize = avcodec_encode_video(vid.c, vid.outbuf, vid.outbuf_size, NULL)))
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		/* TODO: try without this first, i believe they are one-to-one */
		//pkt.pts = av_rescale_q(vid.c->coded_frame->pts, vid.c->time_base, vid.video_st->time_base);
		if (vid.c->coded_frame->key_frame)
			pkt.flags |= PKT_FLAG_KEY;
		pkt.stream_index = vid.video_st->index;
		pkt.data = vid.outbuf;
		pkt.size = outsize;

		/* finally. write the packet */
		/* TODO: handle error return here */
		av_write_frame(vid.oc, &pkt);
	}
	av_write_trailer(vid.oc);

	for (i = 0; i < vid.oc->nb_streams; i++) {
		av_freep(&(vid.oc->streams[i]->codec));
		av_freep(&(vid.oc->streams[i]));
	}
	
	url_fclose(&(vid.oc->pb));

	//free(vid.picture_buf);
	free(vid.outbuf);
	avcodec_close(vid.c);
	av_free(vid.picture);
	av_free(vid.oc);
	displaymsg("Done Writing avi file");
}

#endif /* HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT */
