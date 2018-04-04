import warnings
warnings.filterwarnings('ignore')
import numpy as np
import cv2 as cv
import keras
from keras import backend as K
from keras.models import load_model
print('model loading...')
model = load_model('D:/Documents/QT/passpic/flame_model_2.h5')
print('model loaded.')


def pri(A, B, n):
    A = np.asarray(A, dtype=np.uint8) #'np.uint8' declaration required
    A = A.reshape(n, B[0], B[1], B[2])
    #print(A)
    print(A.shape)
    for i in range(0, n):
        A1 = A[i]
        print(A1.shape)
        cv.imshow("pic", A1)
        cv.waitKey(1000)


def pred(A, B, n):
    A = np.asarray(A, dtype=np.uint8)
    A = A.reshape(n, B[0], B[1], B[2])
    ret = []
    for i in range(0, n):
        A1 = A[i]
        if K.image_data_format() == 'channels_first':
            A1 = A1.reshape(1, B[2], B[0], B[1])
        else:
            A1 = A1.reshape(1, B[0], B[1], B[2])
        pred = model.predict_classes(A1)
        #print('hhh')
        if pred[0] == 1:
            #ret[i] = 1 false statement (list assignment index out of range)
            ret.append(1)
        else:
            ret.append(0)
    return ret