/*v4l2_example.c*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <sys/time.h>



#define TRUE            (1)
#define FALSE           (0)

#define FILE_VIDEO      "/dev/video0"
#define IMAGE           "./img/demo"

// #define IMAGEWIDTH      640
// #define IMAGEHEIGHT     480
#define IMAGEWIDTH      1280
#define IMAGEHEIGHT     720

#define FRAME_NUM       4

int fd;
struct v4l2_buffer buf;

struct buffer
{
    void * start;
    unsigned int length;
    long long int timestamp;
} *buffers;

static int v4l2_init();
static int v4l2_mem_opr();
static int v4l2_frame_process();
static int v4l2_release();

int main(int argc, char const *argv[])
{
    printf("###### begin....\n");
    sleep(0.1);

    int ret = v4l2_init();
    if(ret != TRUE)
    {
        printf("init v4l2 fail !!!\n");
        return FALSE;
    }
    printf("###### init v4l2 success!\n");

    ret = v4l2_mem_opr();
    if(ret != TRUE)
    {
        printf("v4l2_mem_opr fail !!!\n");
        return FALSE;
    }
    printf("###### v4l2_mem_opr success!\n");

    ret = v4l2_frame_process();
    if(ret != TRUE)
    {
        printf("v4l2_frame_process fail !!!\n");
        return FALSE;
    }
    printf("###### v4l2_frame_process success!\n");

    ret = v4l2_release();
    if(ret != TRUE)
    {
        printf("v4l2_release fail !!!\n");
        return FALSE;
    }
    printf("###### v4l2_release success!\n");
    
    return TRUE;
}


int v4l2_init()
{
    //打开摄像头设备,"/dev/video0"
    if ((fd = open(FILE_VIDEO, O_RDWR)) == -1) 
    {
        printf("open v4l2 error! errno:%d\n", errno);
        return FALSE;
    }

    //查询设备属性
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) 
    {
        printf("ioctl VIDIOC_QUERYCAP v4l2 error! errno:%d\n", errno);
        return FALSE;
    }
    else
    {
        printf("driver:\t\t%s\n",cap.driver);
        printf("card:\t\t%s\n",cap.card);
        printf("bus_info:\t%s\n",cap.bus_info);
        printf("version:\t%d\n",cap.version);
        printf("capabilities:\t%x\n",cap.capabilities);
        
        if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) 
        {
            printf("Device %s: supports capture.\n",FILE_VIDEO);
        }

        if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) 
        {
            printf("Device %s: supports streaming.\n",FILE_VIDEO);
        }
    }


    //显示所有支持帧格式
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index=0;
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format:\n");
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
    {
        printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
        fmtdesc.index++;
    }

    //检查是否支持某帧格式
    struct v4l2_format fmt_test;
    fmt_test.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // fmt_test.fmt.pix.pixelformat=V4L2_PIX_FMT_RGB32;
    fmt_test.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
    fmt_test.fmt.pix.height = IMAGEHEIGHT;
    fmt_test.fmt.pix.width = IMAGEWIDTH;
    fmt_test.fmt.pix.field = V4L2_FIELD_NONE;
    if(ioctl(fd,VIDIOC_TRY_FMT,&fmt_test)==-1)
    {
        printf("not support format yuv! errno:%d, pix.pixelformat:%u\n", errno, fmt_test.fmt.pix.pixelformat);      
    }
    else
    {
        printf("support format yuv\n");
    }

    //查看及设置当前格式
    printf("\n\n\n set fmt...\n");
    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32; //jpg格式
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;//yuv格式

    fmt.fmt.pix.height = IMAGEHEIGHT;
    fmt.fmt.pix.width = IMAGEWIDTH;
    // fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    printf("fmt.type:\t\t%u\n",fmt.type);
    printf("pix.pixelformat:\t%c%c%c%c\n",fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,(fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
    printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
    printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);
    if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        printf("Unable to set format  errno:%d, pix.pixelformat:%u\n", errno, fmt.fmt.pix.pixelformat);
        // return FALSE;
    }

    printf("\n\n\n get fmt...\n"); 
    if(ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("Unable to get format errno:%d\n", errno);
        // return FALSE;
    }
    else
    {
        printf("fmt.type:\t\t%d\n",fmt.type);
        printf("pix.pixelformat num:\t\t%d\n",fmt.fmt.pix.pixelformat);
        printf("pix.pixelformat:\t%c%c%c%c\n",fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,(fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
        printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
        printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
        printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);
    }


    // 设置及查看帧速率，这里只能是30帧，就是1秒采集30张图
    struct v4l2_streamparm stream_para;
    memset(&stream_para, 0, sizeof(struct v4l2_streamparm));
    stream_para.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    stream_para.parm.capture.timeperframe.denominator = 30;
    stream_para.parm.capture.timeperframe.numerator = 1;

    if(ioctl(fd, VIDIOC_S_PARM, &stream_para) == -1)
    {
        printf("Unable to set frame rate\n");
        return FALSE;
    }
    if(ioctl(fd, VIDIOC_G_PARM, &stream_para) == -1)
    {
        printf("Unable to get frame rate errno:%d\n", errno);
        return FALSE;       
    }
    
    printf("\n\n\n curent frame: numerator:%d, denominator:%d\n",stream_para.parm.capture.timeperframe.numerator, stream_para.parm.capture.timeperframe.denominator);
    
    return TRUE;
}



int v4l2_mem_opr()
{
    
    //申请帧缓冲
    struct v4l2_requestbuffers req;
    req.count=FRAME_NUM;
    req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory=V4L2_MEMORY_MMAP;
    if(ioctl(fd,VIDIOC_REQBUFS,&req)==-1)
    {
        printf("request for buffers error\n");
        return FALSE;
    }

    // 申请用户空间的地址列
    buffers = malloc(req.count*sizeof (*buffers));
    if (!buffers) 
    {
        printf ("out of memory!\n");
        return FALSE;
    }
    
    // 进行内存映射
    for (int i = 0; i < FRAME_NUM; i++) 
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        //查询
        if (ioctl (fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf("query buffer error\n");
            return FALSE;
        }

        //映射
        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL,buf.length,PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED)
        {
            printf("buffer map error\n");
            return FALSE;
        }
    }
    return TRUE;    
}



int v4l2_frame_process()
{
    unsigned int n_buffers;
    enum v4l2_buf_type type;
    long long int extra_time = 0;
    long long int cur_time = 0;
    long long int last_time = 0;

    time_t time_start;
    time_t time_stop;

    //入队
    for (n_buffers = 0; n_buffers < FRAME_NUM; n_buffers++)
    {
        buf.index = n_buffers;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }
    
    //开启视频采集
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);
    
    time_start = time(NULL);

    //出队，处理，写入yuv文件，入队，循环进行
    int loop = 0;
    while(loop < 30)
    {
        for(n_buffers = 0; n_buffers < FRAME_NUM; n_buffers++)
        {
            //出队
            buf.index = n_buffers;
            ioctl(fd, VIDIOC_DQBUF, &buf);

            //查看采集数据的时间戳之差，单位为微妙
            buffers[n_buffers].timestamp = buf.timestamp.tv_sec*1000000+buf.timestamp.tv_usec;
            cur_time = buffers[n_buffers].timestamp;
            extra_time = cur_time - last_time;
            last_time = cur_time;
            // printf("time_deta:%lld\n\n",extra_time);
            // printf("buf_len:%d\n",buffers[n_buffers].length);
            if (loop % 10 == 0 && n_buffers == 0)
            {
                //处理数据只是简单写入文件，名字以loop的次数和帧缓冲数目有关
                char file_name[100];
                char index_str[10];
                printf("grab image data OK\n");
                memset(file_name,0,sizeof(file_name));
                memset(index_str,0,sizeof(index_str));
                sprintf(index_str,"%d",loop*4+n_buffers);
                strcpy(file_name,IMAGE);
                strcat(file_name,index_str);
                // strcat(file_name,".jpg");
                strcat(file_name,".yuv");
                FILE *fp2 = fopen(file_name, "wb");
                if(!fp2)
                {
                    printf("open %s error\n",file_name);
                    return(FALSE);
                }
                fwrite(buffers[n_buffers].start, 1, buf.bytesused,fp2);
                fclose(fp2);
                printf("save %s OK\n",file_name);
            }


            //入队循环
            ioctl(fd, VIDIOC_QBUF, &buf);       
        }

        loop++;
    }

    time_stop = time(NULL);
    printf("\n #######stop - start: %ld \n", (time_stop - time_start));

    return TRUE;    
}




int v4l2_release()
{
    unsigned int n_buffers;
    enum v4l2_buf_type type;

    //关闭流
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);
    
    //关闭内存映射
    for(n_buffers=0;n_buffers<FRAME_NUM;n_buffers++)
    {
        munmap(buffers[n_buffers].start,buffers[n_buffers].length);
    }
    
    //释放自己申请的内存
    free(buffers);
    
    //关闭设备
    close(fd);
    return TRUE;
}
