#include "opencv2/opencv.hpp"
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include "HCNetSDK.h"
#include "PlayM4.h"
#include "LinuxPlayM4.h"

using namespace std;
using namespace cv;

static int NUM_FRAME;
static int shape[3];
static Mat dst;
Mat *list_img;
volatile int p_count = 0;
volatile int _count = 0;
volatile int t;
pthread_rwlock_t RW_Lock;

HWND hW = NULL;
LONG nPort = -1;


void __attribute__((__stdcall)) DecCBFun(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, void* nReserved1, LONG nReserved2)
{
    long lFrameType = pFrameInfo->nType;
    if (lFrameType == T_YV12)
    {
        Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight/2, pFrameInfo->nWidth, CV_8UC1, (uchar *)pBuf);
        dst.create(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
        cvtColor(src, dst, CV_YUV2BGR_YV12);

        pthread_rwlock_wrlock(&RW_Lock);  //写锁
        resize(dst, list_img[p_count++], Size(shape[0], shape[1]), 0, 0, CV_INTER_LINEAR);
        if (p_count == NUM_FRAME)
        {
            _count = 1;
            p_count = 0;
        }
        pthread_rwlock_unlock(&RW_Lock);  //写锁解锁
    }
    sleep(t);
}


void __attribute__((__stdcall)) fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize,void* dwUser)
{
    DWORD dRet;
    switch (dwDataType)
    {
        case NET_DVR_SYSHEAD:           //系统头
            if (!PlayM4_GetPort(&nPort))  //获取播放库未使用的通道号
            {
                break;
            }
            if (dwBufSize > 0) {
                if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME)) {
                    dRet = PlayM4_GetLastError(nPort);
                    break;
                }
                if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024)) {
                    dRet = PlayM4_GetLastError(nPort);
                    break;
                }
                //设置解码回调函数 只解码不显示
                //  if (!PlayM4_SetDecCallBack(nPort, DecCBFun)) {
                //     dRet = PlayM4_GetLastError(nPort);
                //     break;
                //  }

                //设置解码回调函数 解码且显示
                if (!PlayM4_SetDecCallBackEx(nPort, DecCBFun, NULL, NULL))
                {
                    dRet = PlayM4_GetLastError(nPort);
                    break;
                }

                //打开视频解码
                if (!PlayM4_Play(nPort, hW))
                {
                    dRet = PlayM4_GetLastError(nPort);
                    break;
                }

                //打开音频解码, 需要码流是复合流
                if (!PlayM4_PlaySound(nPort)) {
                    dRet = PlayM4_GetLastError(nPort);
                    break;
                }
            }
            break;
            //usleep(500);
        case NET_DVR_STREAMDATA:  //码流数据
            if (dwBufSize > 0 && nPort != -1) {
                BOOL inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
                while (!inData) {
                    sleep(0.01);
                    inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
                    std::cerr << "PlayM4_InputData failed \n" << std::endl;
                }
            }
            break;
    }
}


void __attribute__((__stdcall)) g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = {0};
    std::cout << "EXCEPTION_RECONNECT = " << EXCEPTION_RECONNECT << std::endl;
    switch(dwType)
    {
        case EXCEPTION_RECONNECT:    //预览时重连
            printf("pyd----------reconnect--------%d\n", time(NULL));
            break;
        default:
            break;
    }
}


void *getFun(void* ptr)
{
    printf("[M] Child Thread Is Running\n");

    pthread_rwlock_wrlock(&RW_Lock);
    list_img = new Mat[NUM_FRAME];
    pthread_rwlock_unlock(&RW_Lock);

    char **log_info = (char **)ptr;

    LONG lUserID;
    int a1 = 0, a2 = -1;
    int *p1 = &a1, *p2 = &a2;

    //注册设备
    //char IP[]  = "192.168.1.104";  //海康威视网络摄像头的ip
    //char UName[] = "admin";  //海康威视网络摄像头的用户名
    //char PSW[] = "haikang#1";  //海康威视网络摄像头的密码
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(1000, true);
    NET_DVR_SetLogToFile(3, "./sdkLog");
    NET_DVR_DEVICEINFO_V30 struDeviceInfo = {0};
    NET_DVR_SetRecvTimeOut(5000);
    lUserID = NET_DVR_Login_V30(log_info[0], 8000, log_info[1], log_info[2], &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("Login error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return p2;
    }

    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

    long lRealPlayHandle;
    NET_DVR_CLIENTINFO ClientInfo = {0};

    ClientInfo.lChannel = 1;
    ClientInfo.lLinkMode = 0;
    ClientInfo.hPlayWnd = 0;
    ClientInfo.sMultiCastIP = NULL;

    lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack, NULL, 0);
    if (lRealPlayHandle < 0)
    {
        printf("pyd1---NET_DVR_RealPlay_V30 error\n");
        return p2;
    }
    sleep(-1);

    NET_DVR_Cleanup();
    return p1;
}


int cap(int n_frame, int cv_shape[], double pt1, double pt2, char** log)
{
    //设置参数
    pthread_rwlock_wrlock(&RW_Lock);  //写锁
    NUM_FRAME = n_frame;
    t = pt1;

    for(int i = 0; i < 3; ++i)
        shape[i] = cv_shape[i];

    pthread_rwlock_unlock(&RW_Lock);  //解锁

    int mul = 30000, h = shape[0], w = shape[1], n = shape[2];
    printf("[M] Main Thread Is Running\n");
    pthread_t thread;
    int iret = pthread_create(&thread, NULL, getFun, (void*)log);

    if(iret != 0)
    {
        printf("Create pthread error!\n");
    }

    int i;
    Mat *img = new Mat[NUM_FRAME];
    while(1)
    {
        pthread_rwlock_rdlock(&RW_Lock);  //读锁
        //容器内帧数不足时等待
        if(_count == 0)
        {
            pthread_rwlock_unlock(&RW_Lock);  //读锁解锁
            sleep(pt2);
            continue;
        }
        //从数组中调取图片
        for(i = 0; i < NUM_FRAME; ++i)
        {
            //img[i] = imread("../pictures/1.jpg");
            img[i] = list_img[i].clone();  //深拷贝
            imshow("show", img[i]);
            waitKey(1);
        }
        pthread_rwlock_unlock(&RW_Lock);  //读锁解锁

        int *a = new int[mul*NUM_FRAME];
        int j;
        CvScalar s;
        for(int cou = 0; cou < NUM_FRAME; ++cou)
            for(i = 0; i < h; ++i)
                for(j = 0; j < w; ++j)
                {
                    s=img[cou].at<Vec3b>(i,j);
                    a[cou*mul+i*w*n+j*n] = s.val[0];
                    a[cou*mul+i*w*n+j*n+1] = s.val[1];
                    a[cou*mul+i*w*n+j*n+2] = s.val[2];
                }

        delete []a;
    }
    //释放资源
    pthread_rwlock_destroy(&RW_Lock);
    delete []list_img;
    delete []img;
    return 0;
}