#include <cstdio>
#include <cstring>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
#include "PlayM4.h"
#include <list>
#include "pycap.h"
#define USECOLOR 1
#define NUM_FRAME 30
//list容器存储图片帧数，也是python处理每批图片的数量
using namespace std;
using namespace cv;
#include <sstream>

list<IplImage*> list_img;
IplImage *pImg;
IplImage *pImgYCrCb;
IplImage *pImg1 = cvCreateImage(cvSize(100, 100), 8, 3);
int p_count = 0, termi = 0;

list<IplImage*>::iterator it = list_img.begin();
list<IplImage*>::iterator it1 = list_img.begin();

//--------------------------------------------
int iPicNum=0;//Set channel NO.
LONG nPort=-1;
HWND hWnd=NULL;

//string str1 = "G:/cache/", str2 = ".jpg";
string num2str(int i)
{
   std::stringstream ss;
   ss << i;
   return ss.str();
}

void yv12toYUV(char *outYuv, char *inYv12, int width, int height,int widthStep)
{
   int col,row;
   unsigned int Y,U,V;
   int tmp;
   int idx;

  //printf("widthStep=%d.\n",widthStep);

   for (row=0; row<height; row++)
   {
      idx=row * widthStep;
      int rowptr=row*width;

      for (col=0; col<width; col++)
      {
         //int colhalf=col>>1;
         tmp = (row/2)*(width/2)+(col/2);
//         if((row==1)&&( col>=1400 &&col<=1600))
//         {
//          printf("col=%d,row=%d,width=%d,tmp=%d.\n",col,row,width,tmp);
//          printf("row*width+col=%d,width*height+width*height/4+tmp=%d,width*height+tmp=%d.\n",row*width+col,width*height+width*height/4+tmp,width*height+tmp);
//         }
         Y=(unsigned int) inYv12[row*width+col];
         U=(unsigned int) inYv12[width*height+width*height/4+tmp];
         V=(unsigned int) inYv12[width*height+tmp];
//         if ((col==200))
//         {
//         printf("col=%d,row=%d,width=%d,tmp=%d.\n",col,row,width,tmp);
//         printf("width*height+width*height/4+tmp=%d.\n",width*height+width*height/4+tmp);
//         return ;
//         }
         if((idx+col*3+2)> (1200 * widthStep))
         {
          //printf("row * widthStep=%d,idx+col*3+2=%d.\n",1200 * widthStep,idx+col*3+2);
         }
         outYuv[idx+col*3]   = Y;
         outYuv[idx+col*3+1] = U;
         outYuv[idx+col*3+2] = V;
      }
   }
   //printf("col=%d,row=%d.\n",col,row);
}



//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2)
{
    long lFrameType = pFrameInfo->nType;

    if(lFrameType ==T_YV12)
    {
    pImgYCrCb = cvCreateImage(cvSize(pFrameInfo->nWidth,pFrameInfo->nHeight), 8, 3);//得到图像的Y分量
    yv12toYUV(pImgYCrCb->imageData, pBuf, pFrameInfo->nWidth,pFrameInfo->nHeight,pImgYCrCb->widthStep);//得到全部RGB图像
    pImg = cvCreateImage(cvSize(pFrameInfo->nWidth,pFrameInfo->nHeight), 8, 3);
    cvCvtColor(pImgYCrCb,pImg,CV_YCrCb2RGB);
    cvResize(pImg, pImg1);

    if(it == list_img.end() && p_count >= NUM_FRAME)
    {
        ++it;
    }
    else
    {
        *it = pImg;
        ++it;
        if(it == list_img.end())
            p_count++;
    }

    Sleep(500);
    //此时是YV12格式的视频数据，保存在pBuf中，可以fwrite(pBuf,nSize,1,Videofile);
    //fwrite(pBuf,nSize,1,fp);
    }
    /***************
    else if (lFrameType ==T_AUDIO16)
    {
        //此时是音频数据，数据保存在pBuf中，可以fwrite(pBuf,nSize,1,Audiofile);

    }
    else
    {

    }
    *******************/

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

            //设置解码回调函数 解码且显示
            //if (!PlayM4_SetDecCallBackEx(nPort,DecCBFun,NULL,NULL))
            //{
            //  dRet=PlayM4_GetLastError(nPort);
            //  break;
            //}

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


DWORD WINAPI dealFun(LPVOID lpParameter)
{
    int *sig, i;
    IplImage *img[NUM_FRAME];
    //调用python处理图像
    while(1)
    {
        //结束线程信号
        if(termi == 1)
        {
            printf("Thread dealFun exiting...\n");
            break;
        }

        //容器内帧数不足时等待
        if(p_count <= NUM_FRAME)
            continue;

        //从容器中调用图片
        it1 = list_img.begin();
        for(i = 0; i < NUM_FRAME; ++i)
        {
            img[i] = *it1;
            ++it1;
        }

        sig = pycap(img, NUM_FRAME);
        for(i = 0; i < NUM_FRAME; ++i)
            if(sig[i] == 1)
                printf("\a");  //识别目标发出警报
    }
    //释放资源
    for(i = 0; i < NUM_FRAME; ++i)
        cvReleaseImage(&img[i]);
    delete []sig;
    return 0;
}


int main() {

  //---------------------------------------
  // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //---------------------------------------
    // 获取控制台窗口句柄
    //HMODULE hKernel32 = GetModuleHandle((LPCWSTR)"kernel32");
    //GetConsoleWindow = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");

    //---------------------------------------
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

    //---------------------------------------
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);


    //cvNamedWindow("IPCamera");
    //---------------------------------------
    //启动预览并设置回调数据流
    NET_DVR_CLIENTINFO ClientInfo;
    ClientInfo.lChannel = 1;        //Channel number 设备通道号
    ClientInfo.hPlayWnd = NULL;     //窗口为空，设备SDK不解码只取流
    ClientInfo.lLinkMode = 0;       //Main Stream
    ClientInfo.sMultiCastIP = NULL;

    LONG lRealPlayHandle;
    //printf("starting\n");
    lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID,&ClientInfo,fRealDataCallBack,NULL,TRUE);
    //printf("ended\n");
    if (lRealPlayHandle<0)
    {
      printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n",NET_DVR_GetLastError());
      return -1;
    }

    //cvWaitKey(0);

    HANDLE hChildThread;
        hChildThread = CreateThread(
                NULL,    // 使用缺省的安全性
                0,    // 初始提交的栈的大小
                dealFun,    // 线程入口函数
                NULL,    // 传递为线程的参数
                0,    // 附加标记 , 0 表示线程创建后立即运行
                NULL    // 线程 ID
        );

    CloseHandle(hChildThread);

    while(1)
    {
        scanf("%d", &termi);
        if(termi == 1)
        {
            printf("Program exiting in 5 seconds...");
            Sleep(2000);
            break;
        }
    }

    //Sleep(-1);
    //fclose(fp);
    //---------------------------------------
    //关闭预览
    if(!NET_DVR_StopRealPlay(lRealPlayHandle))
    {
      printf("NET_DVR_StopRealPlay error! Error number: %d\n",NET_DVR_GetLastError());
      return -1;
    }
    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    printf("exited");


#if USECOLOR
    cvReleaseImage(&pImgYCrCb);
    cvReleaseImage(&pImg);
#else
    cvReleaseImage(&pImg);
#endif
  cvReleaseImage(&pImg1);
  return 0;
}
