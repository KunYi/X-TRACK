
#include "sdl.h"
#include "sdl_common_internal.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>  // Ensure this line is included
#include <libavutil/imgutils.h>  // Useful for image utilities
#include "monitor.h"
#include "ffmpeg.h"

// Define FFmpeg variables (global or within a struct)
static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *codec_ctx = NULL;
static AVStream *stream = NULL;
static struct SwsContext *sws_ctx = NULL;
static AVFrame *frame = NULL;
static AVPacket pkt;

// Example of FFmpeg initialization
void ffmpeg_init(int width, int height, const char* filename) {
    avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, filename);
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    codec_ctx = avcodec_alloc_context3(codec);

    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = (AVRational){1, 30};  // 30 fps
    codec_ctx->framerate = (AVRational){30, 1};
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    stream = avformat_new_stream(fmt_ctx, codec);
    avcodec_parameters_from_context(stream->codecpar, codec_ctx);

    avio_open(&fmt_ctx->pb, filename, AVIO_FLAG_WRITE);
    avformat_write_header(fmt_ctx, NULL);
    avcodec_open2(codec_ctx, codec, NULL);

    // Allocate frame
    frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;
    av_frame_get_buffer(frame, 0);

    // Initialize SwsContext for pixel conversion
    sws_ctx = sws_getContext(width, height, AV_PIX_FMT_BGRA,
    		width, height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
		NULL, NULL, NULL);
}

void ffmpeg_close(void) {
    avcodec_send_frame(codec_ctx, NULL);  // Flush encoder
    avcodec_receive_packet(codec_ctx, &pkt);  // Write remaining packets

    av_write_trailer(fmt_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_free_context(fmt_ctx);
    sws_freeContext(sws_ctx);
    av_frame_free(&frame);
}

void capture(int width, int height, monitor_t *m) {
    static int64_t frame_index = 0;

    // Allocate memory for the pixel data
    uint32_t* pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    if (!pixels) {
        fprintf(stderr, "Failed to allocate memory for pixels\n");
        return;
    }

    // Read pixels from SDL renderer into ARGB format
    SDL_RenderReadPixels(m->renderer, NULL, SDL_PIXELFORMAT_ARGB8888, pixels, width * sizeof(uint32_t));

    // Prepare input data for sws_scale (converting ARGB to YUV420P)
    uint8_t const* inData[1] = {(uint8_t*)pixels};  // RGB data from SDL
    int inLinesize[1] = {4 * width};                // RGB stride (4 bytes per pixel)

    // Convert ARGB to YUV420P
    sws_scale(sws_ctx, inData, inLinesize, 0, height, frame->data, frame->linesize);

    // Set frame PTS (increment frame index or calculate based on time base)
    frame->pts = frame_index++;  // Increment frame_index for each captured frame

    // Send frame to encoder
    int ret = avcodec_send_frame(codec_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending frame to encoder: %s\n", av_err2str(ret));
        free(pixels);
        return;
    }

    // Receive encoded packet
    while (ret >= 0) {
        ret = avcodec_receive_packet(codec_ctx, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;  // No more packets to encode, break out of loop
        } else if (ret < 0) {
            fprintf(stderr, "Error encoding frame: %s\n", av_err2str(ret));
            break;
        }

        // Set packet stream index and write to output
        pkt.stream_index = stream->index;
        pkt.pts = av_rescale_q(pkt.pts, codec_ctx->time_base, stream->time_base);
        pkt.dts = av_rescale_q(pkt.dts, codec_ctx->time_base, stream->time_base);
        pkt.duration = av_rescale_q(pkt.duration, codec_ctx->time_base, stream->time_base);

        // Write the packet to the file
        ret = av_write_frame(fmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error writing frame to output: %s\n", av_err2str(ret));
        }

        // Free the packet once it's written
        av_packet_unref(&pkt);
    }

    // Free allocated memory for pixels
    free(pixels);
}
