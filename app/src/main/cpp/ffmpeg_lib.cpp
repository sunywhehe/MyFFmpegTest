#include <jni.h>
#include <string>
#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO, "ffmpeg_lib", FORMAT, ##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, "ffmpeg_lib", FORMAT, ##__VA_ARGS__);

extern "C" {
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libswscale/swscale.h"
}

/**
 * 拿到 ffmpeg 当前版本
 * @return
 */
const char *getFFmpegVer() {
    return av_version_info();
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_leosun_myffmpegtest_MainActivity_getFFmpegVersion(JNIEnv *env, jclass clazz) {
    return env->NewStringUTF(getFFmpegVer());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_leosun_myffmpegtest_MainActivity_videoDecode(JNIEnv *env, jclass clazz, jstring input_jstr,
                                                      jstring output_jstr) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, NULL);
    const char *output_cstr = env->GetStringUTFChars(output_jstr, NULL);

    //1. 注册所有组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //2. 打开输入视频文件，成功返回0，第三个参数为NULL，表示自动检测文件格式
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }

    //3. 获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("%s", "获取视频文件信息失败");
        return;
    }

    //查找视频流所在的位置
    //遍历所有类型的流（视频流、音频流可能还有字幕流），找到视频流的位置
    int video_stream_index = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    //编解码上下文
    AVCodecContext *pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;
    //4. 查找解码器 不能通过pCodecCtx->codec获得解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "查找解码器失败");
        return;
    }

    //5. 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("%s", "打开解码器失败");
        return;
    }

    LOGI("width: %d  height: %d", pCodecCtx->width, pCodecCtx->height);

    //编码数据
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pYuvFrame = av_frame_alloc();

    FILE *fp_yuv = fopen(output_cstr, "wb");

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *) pYuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width,
                   pCodecCtx->height);

    //srcW：源图像的宽
    //srcH：源图像的高
    //srcFormat：源图像的像素格式
    //dstW：目标图像的宽
    //dstH：目标图像的高
    //dstFormat：目标图像的像素格式
    //flags：设定图像拉伸使用的算法
    struct SwsContext *pSwsCtx = sws_getContext(
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL);

    int got_frame, len, frameCount = 0;
    //6. 从输入文件一帧一帧读取压缩的视频数据AVPacket
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        if (pPacket->stream_index == video_stream_index) {
            //7. 解码一帧压缩数据AVPacket ---> AVFrame，第3个参数为0时表示解码完成
            len = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, pPacket);

            if (len < 0) {
                LOGE("%s", "解码失败");
                return;
            }
            //AVFrame ---> YUV420P
            //srcSlice[]、dst[]        输入、输出数据
            //srcStride[]、dstStride[] 输入、输出画面一行的数据的大小 AVFrame 转换是一行一行转换的
            //srcSliceY                输入数据第一列要转码的位置 从0开始
            //srcSliceH                输入画面的高度
            sws_scale(pSwsCtx,
                      pFrame->data, pFrame->linesize, 0, pFrame->height,
                      pYuvFrame->data, pYuvFrame->linesize);

            //非0表示正在解码
            if (got_frame) {
                //图像宽高的乘积就是视频的总像素，而一个像素包含一个y，u对应1/4个y，v对应1/4个y
                int yuv_size = pCodecCtx->width * pCodecCtx->height;
                //写入y的数据
                fwrite(pYuvFrame->data[0], 1, yuv_size, fp_yuv);
                //写入u的数据
                fwrite(pYuvFrame->data[1], 1, yuv_size / 4, fp_yuv);
                //写入v的数据
                fwrite(pYuvFrame->data[2], 1, yuv_size / 4, fp_yuv);

                LOGI("解码第%d帧", frameCount++);
            }
            av_free_packet(pPacket);
        }
    }

    fclose(fp_yuv);
    av_frame_free(&pFrame);
    av_frame_free(&pYuvFrame);
    avcodec_free_context(&pCodecCtx);
    avformat_free_context(pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
    env->ReleaseStringUTFChars(output_jstr, output_cstr);
}