
# COMPILER FLAGS
CC = gcc
CCPLUS = g++  
# g++ gcc -lstdc++
CFLAGS = -Wall -pedantic -lpthread -fpermissive
LIBS =  -ldl -L./libs/ffmpeg/ -lx264 -lavcodec -lswscale -lavutil -lswresample
DEBUG = 0

ifeq ($(DEBUG),1)
	OPTIMIZEFLAG = -g
else
	OPTIMIZEFLAG = -O3  
endif
 

LIBS_PATH = ./libs/

INC_PATH = -I./libs/json/include/ -I./libs/ffmpeg/ 
 
# OBJECTS
COMMON_OBJECTS = processcom.c 
COMMON_OUTPUT = libprocesscom.so
COMMON_LIB = -L$(LIBS_PATH) -lprocesscom

CLIENT_OBJECTS = client.c 
CLIENT_OUTPUT = Client

SERVER_OBJECTS = mainServer.c   
SERVER_OUTPUT = Server
 

#Decoder
S_OBJECTS = ffdecoder.c server.c  
S_OUTPUT = libvaserver.so
S_LIB = -L$(LIBS_PATH) -lvaserver


JSONSRC =	$(LIBS_PATH)/json/src/lib_json/json_reader.cpp \
			$(LIBS_PATH)/json/src/lib_json/json_value.cpp \
			$(LIBS_PATH)/json/src/lib_json/json_writer.cpp
JSONOBJS 	= $(JSONSRC:.cpp=.o) 
JSON_OUTPUT = libjson.so
JSON_LIB = -L$(LIBS_PATH) -ljson

 
# DEPENDENCIES
all:	    commonlib jsonlib serverlib client server
 
commonlib: $(COMMON_OBJECTS)
	$(CC) $(COMMON_OBJECTS) -fPIC -shared -o $(COMMON_OUTPUT)
	cp ./$(COMMON_OUTPUT) /usr/lib/
	mv ./$(COMMON_OUTPUT) $(LIBS_PATH)
	ldconfig

jsonlib: $(JSONSRC)
	$(CC) -I./libs/json/include/ $(JSONSRC) -fPIC -shared -o $(JSON_OUTPUT)
	cp ./$(JSON_OUTPUT) /usr/lib/
	mv ./$(JSON_OUTPUT) $(LIBS_PATH)
	ldconfig	

serverlib: $(S_OBJECTS)
	$(CC) $(LIBS) $(S_OBJECTS) -fPIC -shared -o $(S_OUTPUT) $(COMMON_LIB)  $(JSON_LIB)   
	cp ./$(S_OUTPUT) /usr/lib/
	mv ./$(S_OUTPUT) $(LIBS_PATH)
	ldconfig


client: $(CLIENT_OBJECTS)  
	$(CC) $(CFLAGS) $(OPTIMIZEFLAG) -o $(CLIENT_OUTPUT) $(CLIENT_OBJECTS) $(COMMON_LIB)
 
server: $(SERVER_OBJECTS)    
	$(CCPLUS)  $(CFLAGS) $(OPTIMIZEFLAG) -o $(SERVER_OUTPUT) $(SERVER_OBJECTS) $(S_LIB)   

clean:
	rm -rf $(SERVER_OUTPUT) $(CLIENT_OUTPUT) $(COMMON_OUTPUT)  
 