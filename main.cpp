#include <cstdio>
#include <cstring>
#include <iostream>
#include <cmath>
#include "Python.h"
#include <opencv2\opencv.hpp>
#include "cv.h"
#include "highgui.h"
#include "Windows.h"
#include "HCNetSDK.h"
#include "PlayM4.h"
#include "rwlockhead.h"
#define USECOLOR 1
#define NUM_FRAME 30
//list容器存储图片帧数，也是python处理每批图片的数量
using namespace std;
using namespace cv;

IplImage *list_img[NUM_FRAME];
volatile int termi = 0;
volatile int p_count = 0;
volatile int _count = 0;
CRWLock RW_Lock;

int iPicNum=0;//Set channel NO.
LONG nPort=-1;
HWND hWnd=NULL;


void yv12toYUV(char *outYuv, char *inYv12, int width, int height,int widthStep)
{
   int col,row;
   unsigned int Y,U,V;
   int tmp;
   int idx;

   for (row=0; row<height; row++)
   {
      idx=row * widthStep;
      int rowptr=row*width;

      for (col=0; col<width; col++)
      {
         tmp = (row/2)*(width/2)+(col/2);
         Y=(unsigned int) inYv12[row*width+col];
         U=(unsigned int) inYv12[width*height+width*height/4+tmp];
         V=(unsigned int) inYv12[width*height+tmp];
         if((idx+col*3+2)> (1200 * widthStep))
         {
          //printf("row * widthStep=%d,idx+col*3+2=%d.\n",1200 * widthStep,idx+col*3+2);
         }
         outYuv[idx+col*3]   = Y;
         outYuv[idx+col*3+1] = U;
         outYuv[idx+col*3+2] = V;
      }
   }
}



//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2)
{
    long lFrameType = pFrameInfo->nType;
    IplImage *pImg = cvCreateImage(cvSize(pFrameInfo->nWidth,pFrameInfo->nHeight), 8, 3);
    IplImage *pImgYCrCb = cvCreateImage(cvSize(pFrameInfo->nWidth,pFrameInfo->nHeight), 8, 3);//得到图像的Y分量
    IplImage *pImg1 = cvCreateImage(cvSize(100, 100), 8, 3);
    if(lFrameType ==T_YV12)
    {
        yv12toYUV(pImgYCrCb->imageData, pBuf, pFrameInfo->nWidth,pFrameInfo->nHeight,pImgYCrCb->widthStep);//得到全部RGB图像     
        cvCvtColor(pImgYCrCb,pImg,CV_YCrCb2RGB);
        cvResize(pImg, pImg1);

        RW_Lock.WriteLock();  //写锁
        if(pImg1 && pImg1->height == 100 && pImg1->width == 100
                && pImg1->depth == 8 && pImg1->nChannels == 3)
        {
            cvCopyImage(pImg1, list_img[p_count++]);  //深拷贝
            if(p_count == NUM_FRAME)
            {
                _count = 1;
                p_count = 0;
            }
        }
        RW_Lock.WriteUnlock();  //写锁解锁
        cvReleaseImage(&pImg1);       
#if USECOLOR
    cvReleaseImage(&pImgYCrCb);
    cvReleaseImage(&pImg);
#else
    cvReleaseImage(&pImg);
#endif
    Sleep(200);  //挂起0.2s
    }
}


///实时流回调
void CALLBACK fRealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize,void *pUser)
{
    DWORD dRet;
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD:    //系统头
        if (!PlayM4_GetPort(&nPort)) //获取播放库未使用的通道号
        {
            break;
        }
        if(dwBufSize > 0)
        {
            if (!PlayM4_OpenStream(nPort,pBuffer,dwBufSize,1024*1024))
            {
                dRet=PlayM4_GetLastError(nPort);
                break;
            }
            //设置解码回调函数 只解码不显示
            if (!PlayM4_SetDecCallBack(nPort,DecCBFun))  //calling DecCBFun
            {
                dRet=PlayM4_GetLastError(nPort);
                break;
            }


            //打开视频解码
            if (!PlayM4_Play(nPort,hWnd))
            {
                dRet=PlayM4_GetLastError(nPort);
                break;
            }

            //打开音频解码, 需要码流是复合流
            if (!PlayM4_PlaySound(nPort))
            {
                dRet=PlayM4_GetLastError(nPort);
                break;
            }
        }
        break;

    case NET_DVR_STREAMDATA:   //码流数据
        if (dwBufSize > 0 && nPort != -1)
        {
            BOOL inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
            while (!inData)
            {
                Sleep(10);
                inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
                OutputDebugString(L"PlayM4_InputData failed \n");
            }
        }
        break;
    }
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = {0};
    switch(dwType)
    {
    case EXCEPTION_RECONNECT:    //预览时重连
    printf("----------reconnect--------%d\n", time(NULL));
    break;
    default:
    break;
    }
}

DWORD WINAPI getFun(LPVOID lpParameter)
{
    RW_Lock.WriteLock();
    for(int cou = 0; cou < NUM_FRAME; ++cou)
    {
        list_img[cou] = cvCreateImage(cvSize(100,100), 8, 3);  //深拷贝
    }
    RW_Lock.WriteUnlock();
    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    // 注册设备
    LONG lUserID;
    NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    lUserID = NET_DVR_Login_V30("192.168.1.104", 8000, "admin", "haikang#1", &struDeviceInfo);
    if (lUserID < 0)
    {
         printf("Login error, %d\n", NET_DVR_GetLastError());
         NET_DVR_Cleanup();
         return -1;
    }

    printf("child thread running\n");
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);

    //启动预览并设置回调数据流
    NET_DVR_CLIENTINFO ClientInfo;
    ClientInfo.lChannel = 1;        //Channel number 设备通道号
    ClientInfo.hPlayWnd = NULL;     //窗口为空，设备SDK不解码只取流
    ClientInfo.lLinkMode = 0;       //Main Stream
    ClientInfo.sMultiCastIP = NULL;

    LONG lRealPlayHandle;
    lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID,&ClientInfo,fRealDataCallBack,NULL,TRUE);
    if (lRealPlayHandle<0)
    {
      printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n",NET_DVR_GetLastError());
      return -1;
    }

    Sleep(-1);
    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    return 0;
}


int main() {

    //=============================================================
    //加载python函数
    int h = 100, w = 100, n = 3;
    int shape[3] = {h, w, n};
    int mul = h*w*n;
    Py_Initialize();    // 初始化


    // 将Python工作路径切换到待调用模块所在目录，一定要保证路径名的正确性

    string path = "D:\\Documents\\QT\\hikang_cap_2";
    string chdir_cmd = string("sys.path.insert(0,\"") + path + "\")";
    //一定要将模块目录添加到sys.path的首位，否则可能添加失败

    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cstr_cmd);

    PyObject* pModule = PyImport_ImportModule("cap");
    PyObject* pv = PyObject_GetAttrString(pModule, "pred");
    //PyObject* pv1 = PyObject_GetAttrString(pModule, "pri");
    //=============================================================

    HANDLE hChildThread;
        hChildThread = CreateThread(
                NULL,    // 使用缺省的安全性
                0,    // 初始提交的栈的大小
                getFun,    // 线程入口函数
                NULL,    // 传递为线程的参数
                0,    // 附加标记 , 0 表示线程创建后立即运行
                NULL    // 线程 ID
        );
    CloseHandle(hChildThread);
    int *ret = new int[NUM_FRAME];
    int i;
    IplImage *img[NUM_FRAME];
    for(i = 0; i < NUM_FRAME; ++i)
    {
        img[i] = cvCreateImage(cvSize(100,100), 8, 3);
    }
    while(1)
    {
        //读锁
        RW_Lock.ReadLock();
        //容器内帧数不足时等待
        if(_count == 0)
        {
            RW_Lock.ReadUnlock();  //读锁解锁
            Sleep(100);
            continue;
        }
        //从数组中调取图片
        for(i = 0; i < NUM_FRAME; ++i)
        {
            cvCopyImage(list_img[i], img[i]);  //深拷贝
        }
        RW_Lock.ReadUnlock();  //读锁解锁

        //不可多次调用pycap函数，原因不明。
        //============================================================
        //设置参数
        int *a = new int[mul*NUM_FRAME];
        int j, k;
            CvScalar s;
            for(int cou = 0; cou < NUM_FRAME; ++cou)
                for(i = 0; i < h; ++i)
                    for(j = 0; j < w; ++j)
                    {
                        s=cvGet2D(img[cou],i,j);
                        a[cou*mul+i*w*n+j*n] = s.val[0];
                        a[cou*mul+i*w*n+j*n+1] = s.val[1];
                        a[cou*mul+i*w*n+j*n+2] = s.val[2];
                    }

        PyObject* args = PyTuple_New(3);
        PyObject* arg1 = PyList_New(mul*NUM_FRAME);
        PyObject* arg2 = PyList_New(3);
        PyObject* arg3 = PyInt_FromLong(NUM_FRAME);

        for(k = 0; k < 3; k++)
            PyList_SetItem(arg2, k, PyInt_FromLong(shape[k]));

        for(k = 0; k < mul * NUM_FRAME; k++)
            PyList_SetItem(arg1, k, PyInt_FromLong(a[k]));

        PyTuple_SetItem(args, 0, arg1);
        PyTuple_SetItem(args, 1, arg2);
        PyTuple_SetItem(args, 2, arg3);

        //PyObject_CallObject(pv1, args);
        PyObject* pRet = PyObject_CallObject(pv, args);
        for(k = 0; k < NUM_FRAME; ++k)
        {
            ret[k] = PyInt_AsLong(PyList_GetItem(pRet, k));
            printf("%d ", ret[k]);
            if(ret[k] == 1)
                printf("\a");  //识别目标发出警报
        }
        printf("\n");

        delete []a;
        Sleep(1000);
        //============================================================
    }
  //释放资源
  for(i = 0; i < NUM_FRAME; ++i)
  {
      cvReleaseImage(&img[i]);
      cvReleaseImage(&list_img[i]);
  }
  Py_Finalize();
  delete []ret;
  return 0;
}
