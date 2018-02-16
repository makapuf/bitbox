# Makefile for the framebuffer demo.
# DO NOT FORGET to define BITBOX environment variable

NO_USB=1

DEFINES += VGA_MODE=320 FRAMEBUFFER_BPP=4 VGA_BPP=8

NAME = framebuf
GAME_C_FILES = demo.c $(realpath ../../sdk/lib/framebuffer/framebuffer.c)

# see this file for options
include $(realpath ../../sdk/kernel/bitbox.mk)