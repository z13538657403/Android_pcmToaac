//
// Created by 张涛 on 17/9/21.
//
#include <stdio.h>
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

extern "C"
{
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libavutil/log.h"
#include "include/libswscale/swscale.h"
#include "include/libavutil/opt.h"
#include "include/libavutil/imgutils.h"
#include "include/libavutil/frame.h"
#include "include/libswresample/swresample.h"
#include "include/libavutil/channel_layout.h"
}

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG,"Native",__VA_ARGS__)
#define nullptr (void *)0

extern "C"
JNIEXPORT jstring JNICALL Java_com_imooc_ffmpegpcmencode_MainActivity_helloFFmpeg
  (JNIEnv *env, jobject obj)
{
    char info[10000] = { 0 };
    sprintf(info, "%s\n", avcodec_configuration());
    return env->NewStringUTF(info);
}

extern "C"
JNIEXPORT jint JNICALL Java_com_imooc_ffmpegpcmencode_MainActivity_pcmToaac
        (JNIEnv *env, jobject obj, jstring pcmPath, jstring aacPath)
{
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx = NULL;
    int i , ret , got_output;
    FILE *fp_in;
    FILE *fp_out;

    AVFrame *pFrame;
    uint8_t* frame_buf;
    int size = 0;

    AVPacket pkt;
    int y_size;
    int framecnt = 0;

    char *in_path = (char*) env->GetStringUTFChars(pcmPath , (unsigned char*)nullptr);
    char *out_path = (char*) env->GetStringUTFChars(aacPath , (unsigned char*)nullptr);

    AVCodecID codec_id = AV_CODEC_ID_AAC;
    int framenum = 100000;
    av_register_all();

    pCodec = avcodec_find_encoder(codec_id);
    if(!pCodec)
    {
        LOG("Codec not found\n");
        return -1;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(!pCodecCtx)
    {
        LOG("could not allocate video codec context\n");
        return -1;
    }

    pCodecCtx->codec_id = codec_id;
    pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
    pCodecCtx->sample_rate = 44100;
    pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);

    if((ret = avcodec_open2(pCodecCtx , pCodec , NULL)) < 0)
    {
        LOG("could not open avcodec\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrame->nb_samples = pCodecCtx->frame_size;
    pFrame->format = pCodecCtx->sample_fmt;
    pFrame->channels = 2;

    size = av_samples_get_buffer_size(NULL , pCodecCtx->channels , pCodecCtx->frame_size , pCodecCtx->sample_fmt , 0);
    frame_buf = (uint8_t *)av_malloc(size);

    ret = avcodec_fill_audio_frame(pFrame , pCodecCtx->channels, pCodecCtx->sample_fmt , (const uint8_t*)frame_buf, size, 0);
    if(ret < 0)
    {
        LOG("avcodec fill audio frame error\n");
        return -1;
    }

    fp_in = fopen(in_path , "rb");
    if (!fp_in)
    {
        LOG("Could not open %s\n", in_path);
        return -1;
    }

    fp_out = fopen(out_path , "wb");
    if(!fp_out)
    {
        LOG("Could not open %s\n", out_path);
        return -1;
    }

    for(i = 0 ; i < framenum ; i++)
    {
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;

        if(fread(frame_buf , 1 , size , fp_in) <= 0)
        {
            LOG("Failed to read raw data!\n");
            return -1;
        }
        else if(feof(fp_in))
        {
            break;
        }

        pFrame->pts = i;
        ret = avcodec_encode_audio2(pCodecCtx , &pkt , pFrame , &got_output);
        if(ret < 0)
        {
            LOG("encoding error\n");
            return -1;
        }

        if(pkt.data == NULL)
        {
            av_free_packet(&pkt);
            continue;
        }

        if(got_output)
        {
            LOG("encode pcm success \n");
            framecnt++;
            fwrite(pkt.data, 1, pkt.size, fp_out);
            av_free_packet(&pkt);
        }
    }

    for (got_output = 1; got_output; i++)
    {
        ret = avcodec_encode_audio2(pCodecCtx, &pkt, NULL, &got_output);
        if (ret < 0)
        {
            LOG("Error encoding frame\n");
            return -1;
        }
        if (got_output)
        {
            LOG("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", pkt.size);
            fwrite(pkt.data, 1, pkt.size, fp_out);
            av_free_packet(&pkt);
        }
    }

    fclose(fp_out);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);

    return 1;
}