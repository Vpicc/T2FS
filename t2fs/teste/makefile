#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: delete dir file soft sizing write path api read

delete:
	$(CC) -o deleteTest deleteTest.c -L../lib -lt2fs -Wall
dir:
	$(CC) -o dirTest dirTest.c -L../lib -lt2fs -Wall
file:
	$(CC) -o fileTests fileTests.c -L../lib -lt2fs -Wall
soft:
	$(CC) -o testeSoft softlinkTest.c -L../lib -lt2fs -Wall
sizing:
	$(CC) -o testeSizing testSizing.c -L../lib -lt2fs -Wall
write:
	$(CC) -o testeWrite writeTest.c -L../lib -lt2fs -Wall
path:
	$(CC) -o testePath testPath.c -L../lib -lt2fs -Wall
api:
	$(CC) -o testeApi testApiDisk.c -L../lib -lt2fs -Wall
read:
	$(CC) -o testeRead testRead.c -L../lib -lt2fs -Wall
clean:
	rm -rf deleteTest dirTest fileTests testeSoft testeSizing testeWrite testePath testeApi testeRead


