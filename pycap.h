#ifndef PYCAP_H
#define PYCAP_H

#endif // PYCAP_H
#include <cmath>
#include "Python.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <opencv2\opencv.hpp>
#include "cv.h"
#include "highgui.h"
using namespace cv;
#include <sstream>


int* pycap(IplImage* img[], int NUM)
{
    int h = 100, w = 100, n = 3;
    int shape[3] = {h, w, n};
    int mul = h*w*n;
    Py_Initialize();    // 初始化

    //异常退出返回值
    int* b = new int[NUM];
    for(int j = 0; j < NUM; ++j)
        b[j] = -1;

    // 将Python工作路径切换到待调用模块所在目录，一定要保证路径名的正确性

    string path = "D:\\Documents\\QT\\hikang_cap_2";
    string chdir_cmd = string("sys.path.insert(0,\"") + path + "\")";
    //一定要将模块目录添加到sys.path的首位，否则可能添加失败

    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cstr_cmd);
    //PyRun_SimpleString("import warnings");
    //PyRun_SimpleString("warnings.filterwarnings('ignore')");
    //PyRun_SimpleString("print(sys.path)");

    // 加载模块
    PyObject* pModule = PyImport_ImportModule("cap");
    if (!pModule) // 加载模块失败
    {
        std::cout << "[ERROR] Python get module failed." << std::endl;
        return b;
    }


    // 加载函数
    PyObject* pv = PyObject_GetAttrString(pModule, "pred");
    if (!pv || !PyCallable_Check(pv))  // 验证是否加载成功
    {
        std::cout << "[ERROR] Can't find function (pri)" << std::endl;
        return b;
    }

//****************************************************************************

    int *a = new int[mul*NUM];
    int i, j, k;
        CvScalar s;
        for(int cou = 0; cou < NUM; ++cou)
            for(i = 0; i < h; ++i)
                for(j = 0; j < w; ++j)
                {
                    s=cvGet2D(img[cou],i,j);
                    a[cou*mul+i*w*n+j*n] = s.val[0];
                    a[cou*mul+i*w*n+j*n+1] = s.val[1];
                    a[cou*mul+i*w*n+j*n+2] = s.val[2];
                }

    printf("No Problems Yet.\n");


    //设置参数
    PyObject* args = PyTuple_New(3);
    PyObject* arg1 = PyList_New(mul*NUM);
    PyObject* arg2 = PyList_New(3);
    PyObject* arg3 = PyInt_FromLong(NUM);

    for(k = 0; k < 3; k++)
        PyList_SetItem(arg2, k, PyInt_FromLong(shape[k]));

    for(k = 0; k < mul * NUM; k++)
        PyList_SetItem(arg1, k, PyInt_FromLong(a[k]));

    PyTuple_SetItem(args, 0, arg1);
    PyTuple_SetItem(args, 1, arg2);
    PyTuple_SetItem(args, 2, arg3);

    // 调用函数
    PyObject* pRet = PyObject_CallObject(pv, args);

    int *ret = new int[NUM];
    for(k = 0; k < NUM; ++k)
        ret[k] = PyInt_AsLong(PyList_GetItem(pRet, k));

    //释放资源
    Py_Finalize();
    delete []a;
    delete []b;
    return ret;
}
