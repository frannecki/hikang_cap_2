#hikang_cap_2
运行环境：
Ubuntu 64bit
CMake 3.5.1
gcc 5.4.0
opencv-3.1.0
Haikang SDK for Linux

demo说明：
cap.h包含抓图模块和处理模块（前者为子线程，后者为主线程）。
main.cpp提供最简单的函数调用实例。其中main函数的参数分别为：
批量处理图片数量，图像参数（高度，宽度，以及通道数）（图片深度为8bit），每抓取一帧图片后等待时间，
抓图数组未存满时主线程德等待时间以及用户登录信息（包括IP，用户名及密码）.

refs:
https://blog.csdn.net/ding977921830/article/details/75272384
