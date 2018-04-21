CC = g++
AR = ar
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib

all: $(LIB_DIR)/libCSed.a

SRC = \
CSed.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CFLAGS = \
-std=c++14 \
-I$(INC_DIR) \
-I. \
-I../../CFile/include \
-I../../CFileUtil/include \
-I../../CStrUtil/include \
-I../../CRegExp/include \
-I../../COS/include \
-I../../CUtil/include \

.SUFFIXES: .cpp

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CFLAGS)

$(LIB_DIR)/libCSed.a: $(OBJS)
	$(AR) crv $(LIB_DIR)/libCSed.a $(OBJS)

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(LIB_DIR)/libCSed.a
