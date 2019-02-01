CC= g++
OUT= app
INC= -I/usr/local/include/opencv -I/usr/local/include 
CFLAGS= -g -std=c++11 -c
LIBS= -L/usr/local/lib \
	-lopencv_cudastereo \
	-lopencv_videostab \
	-lopencv_photo \
	-lopencv_stitching \
	-lopencv_shape \
	-lopencv_cudaobjdetect \
	-lopencv_dnn \
	-lopencv_superres \
	-lopencv_cudaoptflow \
	-lopencv_cudafeatures2d \
	-lopencv_cudacodec \
	-lopencv_cudabgsegm \
	-lopencv_cudalegacy \
	-lopencv_calib3d \
	-lopencv_features2d \
	-lopencv_highgui \
	-lopencv_videoio \
	-lopencv_imgcodecs \
	-lopencv_cudaimgproc \
	-lopencv_cudafilters \
	-lopencv_video \
	-lopencv_objdetect \
	-lopencv_flann \
	-lopencv_cudaarithm \
	-lopencv_ml \
	-lopencv_cudawarping \
	-lopencv_imgproc \
	-lopencv_core \
	-lopencv_cudev \
	-lopencv_ximgproc \
	/usr/local/lib/libtinyxml2.a


OBJDIR=obj
OBJS= $(addprefix $(OBJDIR)/, \
	main.o \
	DETECTION.o \
	FLC.o \
	clientsocket.o)


all: $(OUT)


$(OUT): $(OBJS)
	@$(CC) -g -o $(OUT) $^ $(LIBS)


$(OBJDIR)/%.o: %.cpp
	@$(CC) $(CFLAGS) $(INC) -o $@ $<


.PHONY: clean
clean:
	rm $(OBJDIR)/*.o $(OUT)
