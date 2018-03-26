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

int h = 100, w = 100, n = 3;
int shape[3] = {h, w, n};
int mul = h*w*n;
int *a = new int[mul];

int pycap(IplImage* img)
{
    Py_Initialize();    // 初始化

    // 将Python工作路径切换到待调用模块所在目录，一定要保证路径名的正确性

    string path = "D:\\Documents\\QT\\haikang_cap";
    string chdir_cmd = string("sys.path.insert(0,\"") + path + "\")";
    //一定要将模块目录添加到sys.path的首位，否则可能添加失败

    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cstr_cmd);
    //PyRun_SimpleString("print(sys.path)");

    // 加载模块
    PyObject* pModule = PyImport_ImportModule("cap");
    if (!pModule) // 加载模块失败
    {
        //std::cout << "[ERROR] Python get module failed." << std::endl;
        return -1;
    }


    // 加载函数
    PyObject* pv = PyObject_GetAttrString(pModule, "pred");
    if (!pv || !PyCallable_Check(pv))  // 验证是否加载成功
    {
        //std::cout << "[ERROR] Can't find function (pri)" << std::endl;
        return -1;
    }

//****************************************************************************
    int i, j, k;

    for(i = 0; i < h; i++)
        for(j = 0; j < w; j++)
        {
            CvScalar s=cvGet2D(img,i,j);
            a[i*w*n+j*n] = s.val[0];
            a[i*w*n+j*n+1] = s.val[1];
            a[i*w*n+j*n+2] = s.val[2];
        }
    PyObject* args = PyTuple_New(2);   // 2个参数
    PyObject* arg1 = PyList_New(mul);
    PyObject* arg2 = PyList_New(3);

    for(k = 0; k < 3; k++)
            PyList_SetItem(arg2, k, PyInt_FromLong(shape[k]));

    for(k = 0; k < mul; k++)
        PyList_SetItem(arg1, k, PyInt_FromLong(a[k]));

    PyTuple_SetItem(args, 0, arg1);
    PyTuple_SetItem(args, 1, arg2);

    // 调用函数
    PyObject* pRet = PyObject_CallObject(pv, args);
    int ret = PyInt_AsLong(pRet);
    delete []a;
    return ret;
}
