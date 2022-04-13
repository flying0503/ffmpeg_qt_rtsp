#include "videoplayer.h"
#include <QDebug>

VideoPlayer::VideoPlayer()
{
    pauseFlag = false;
    stopFlag = false;
}

VideoPlayer::~VideoPlayer()
{
    clear();
}

void VideoPlayer::startPlay()
{
    ///调用 QThread 的start函数 将会自动执行下面的run函数 run函数是一个新的线程
    this->start();
}

void VideoPlayer::stopPlay()
{
    if (QThread::isRunning())
    {
        stopFlag = true;
        condition.wakeAll();
        QThread::quit();
        QThread::wait();
    }
}

void VideoPlayer::pause()
{
    if (QThread::isRunning())
    {
        pauseFlag = true;
    }
}

void VideoPlayer::resume()
{
    if (QThread::isRunning())
    {
        pauseFlag = false;
        condition.wakeAll();
    }
}

void VideoPlayer::setFileName(QString name)
{
    this->mFileName = name;
}

QString VideoPlayer::getFileName()
{
    return mFileName;
}

State VideoPlayer::state()
{
    State s = Stoped;
    if (!QThread::isRunning())
    {
        s = Stoped;
    }
    else if (QThread::isRunning() && pauseFlag)
    {
        s = Paused;
    }
    else if (QThread::isRunning() && (!pauseFlag))
    {
        s = Running;
    }
    return s;

}

void VideoPlayer::run()
{
    qDebug() << "enter thread : " << QThread::currentThreadId();

    int ret = initFFMPEG();//初始化FFMPEG
    if(-1 == ret)   //初始化失败结束线程
        return;

    while (!stopFlag)
    {
        if (pauseFlag)
        {
            mutex.lock();
            condition.wait(&mutex);
            mutex.unlock();
        }
        int ret = process();
        if(-1 == ret)
            break;  //这里认为视频读取完了
    }
    clear();
    pauseFlag = false;
    stopFlag = false;

    qDebug() << "exit thread : " << QThread::currentThreadId();
}

int VideoPlayer::initFFMPEG()
{
    int numBytes;

    avformat_network_init();   ///初始化FFmpeg网络模块，2017.8.5---lizhen
    av_register_all();         //初始化FFMPEG  调用了这个才能正常适用编码器和解码器


    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();

    //设置获取方式
    AVDictionary *pAvdic=NULL;
    char option_key[]="rtsp_transport";
    char option_value[]="tcp";
    av_dict_set(&pAvdic,option_key,option_value,0);
    char option_key2[]="max_delay";
    char option_value2[]="100";
    av_dict_set(&pAvdic,option_key2,option_value2,0);

    ///rtsp地址，根据传入QString转换
    char *url=this->mFileName.toLatin1().data();

    if(avformat_open_input(&pFormatCtx,url,NULL,&pAvdic)!=0)
    {
        printf("can't open the url \n");
        return -1;
    }

    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        printf("Conld't find stream infomation \n");
        return -1;
    }

    videoStream = -1;

    ///遍历视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 保存到videoStream变量中
    ///这里我们现在只处理视频流  音频流先不管他

    for(unsigned int k=0;k<pFormatCtx->nb_streams;k++)
    {
        if(pFormatCtx->streams[k]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = k;
        }
    }

    //如果videoStream为-1 说明没有找到视频流
    if(videoStream == -1)
    {
        printf("Didn't find a video stream\n");
        return -1;
    }

    ///查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    pCodecCtx->bit_rate = 0;        //帧率初始化为零
    pCodecCtx->time_base.num =1;    ///时间戳精准设置（分子）
    pCodecCtx->time_base.den =10;   ///时间戳精准设置（分母）
    pCodecCtx->frame_number =1;     //帧计数器

    if(pCodec == NULL)
    {
        printf("Codec not found \n");
        return -1;
    }

    //打开解码器
    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0)
    {
        printf("Could not open codec \n");
        return -1;
    }

    //分配帧空间
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    ///这里我们改成了 将解码后的YUV数据转换成RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
            pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
            AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width,pCodecCtx->height);
    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer, AV_PIX_FMT_RGB32,
            pCodecCtx->width, pCodecCtx->height);
    int y_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
    av_new_packet(packet, y_size); //分配packet的数据

    return 0;
}

int VideoPlayer::process()
{
    int got_picture;
    if(av_read_frame(pFormatCtx,packet))
    {
        return -1;  //这里认为视频读取完了返回-1
    }

    if(packet->stream_index == videoStream)
    {
        int ret =avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
        if(ret<0)
        {
            printf("decode error \n");
            return -1;
        }

        if(got_picture)
        {
            sws_scale(img_convert_ctx,(uint8_t const * const *)pFrame->data,
                      pFrame->linesize,0,pCodecCtx->height,pFrameRGB->data,
                      pFrameRGB->linesize);
            //把这个RGB数据 用QImage加载
            QImage tmpImg((uchar *)out_buffer,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
            emit sig_GetOneFrame(image);  //发送信号
        }
    }
    av_free_packet(packet); //释放资源,否则内存会一直上升
    msleep(2); //停一停  不然放的太快了
    return 0;
}

void VideoPlayer::clear()
{
    av_free(out_buffer);
    av_free(pFrameRGB);
    av_free(pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    videoStream =0;
    memset(img_convert_ctx,0,sizeof (img_convert_ctx));

    out_buffer = nullptr;
    pFrame = nullptr;
    pFrameRGB = nullptr;
    pCodecCtx = nullptr;
    pFormatCtx = nullptr;
    img_convert_ctx = nullptr;
    printf("clear data\n");
}
