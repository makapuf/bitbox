#testkernel makefile
NAME = test_engine

GAME_C_FILES = main.c 
#GAME_C_OPTS = -DVGAMODE_400
#GAME_C_OPTS = -DVGAMODE_640_OVERCLOCK
GAME_BINARY_FILES = ball.spr bg.tset bg.tmap ball_small.spr

USE_ENGINE = 1
NO_USB=1


include $(BITBOX)/lib/bitbox.mk
main.c:bg.h

bg.tset bg.tmap bg.h: bg.tmx bg.png 
	python ../scripts/tmx.py bg.tmx > bg.h

ball.spr : ball/????.png
	#python ../scripts/sprite_encode2.py $@ $? -m p4
	python ../scripts/couples_encode.py $@ $^ 

ball_small.spr : ball_small/????.png
	#python ../scripts/sprite_encode2.py $@ $? -m p4
	python ../scripts/couples_encode.py $@ $^ 


clean::
	rm -f *.spr *.spr.png
	rm -f bg.h