CC = g++
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = .
LIB_DIR = ../lib
BIN_DIR = ../bin

all: $(BIN_DIR)/CSed

SRC = \
CSedTest.cpp

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
-I. \
-I$(INC_DIR) \
-I../../CStrUtil/include \
-I../../CRegExp/include \
-I../../CFile/include \
-I../../CUtil/include \

LFLAGS = \
-L$(LIB_DIR) \
-L../../CSed/lib \
-L../../CFile/lib \
-L../../CStrUtil/lib \
-L../../CRegExp/lib \
-L../../COS/lib \

LIBS = \
-lCSed -lCFile -lCStrUtil -lCRegExp -lCOS \
-ltre

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(BIN_DIR)/CSed

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/CSed: $(OBJS) $(LIB_DIR)/libCSed.a
	$(CC) $(LDEBUG) -o $(BIN_DIR)/CSed $(OBJS) $(LFLAGS) $(LIBS)
