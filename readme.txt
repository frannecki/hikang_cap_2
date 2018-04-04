4/2
For unknown reasons the child threads are unable to call the python module (error occurs).
For unknown reasons the main fuction are unable to call pycap() multiple times (error occurs).

4/4
1. rwlockhead.h中的CRWLock类定义读写锁。在抓图的子线程中，每次向IplImage* 数组中写入数据时采用写锁，
   此时主线程中被锁的读图代码块挂起；写入完毕后解锁，读图代码执行，此时写入数据的代码块挂起。读写锁针对
   两线程共享的对象，避免同时进行访问导致内存错误。
2. 子线程每次抓图后将图片resize成为100x100的BGR三通道图片，深度为8，触发写锁，写入数组（数组容量为30）。
   解锁后挂起0.2s，继续试图写入。
3. 主线程预先加载python模型及函数，加载完毕后创建子线程，待存放图片的数组存入30帧图片时触发读锁，开始
   读取，完毕后解锁，将图片传入python函数用训练好的模型进行识别。挂起1000ms后继续试图读取。