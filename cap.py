import numpy as np
import cv2 as cv
import keras
from keras import backend as K
from keras.models import load_model


def pri(A, B):
    A = np.asarray(A, dtype=np.uint8) #'np.uint8' declaration required
    A = A.reshape(B[0], B[1], B[2])
    print(A)
    print(A.shape)
    cv.imshow("pic", A)
    cv.waitKey(0)


def pred(A, B):
    A = np.asarray(A, dtype=np.uint8)
    A = A.reshape(1, B[0], B[1], B[2])
    model = load_model('D:/Documents/QT/passpic/flame_model_2.h5')
    if K.image_data_format() == 'channels_first':
        A = A.reshape(1, 3, 100, 100)
    pred = model.predict_classes(A)

    if pred[0] == 1:
        return 1;
    else:
        return 0;