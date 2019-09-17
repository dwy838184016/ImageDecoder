# ImageDecoder

This is a ADT project. Use Mediacodec to decode .H264 data, and use arm neon to to realize NV12(Or YUV420) to RGBA8888.
 
The core code is implemented in JNI.

The data file of test is in the root directory, named vedio_25871.h264. 
The data structure is customized and the structure is as follows, [one frame data size(4byte)][width(4byte)][height(4byte)][one frame data]......