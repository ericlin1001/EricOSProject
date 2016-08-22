.8086
_TEXT segment byte public 'CODE'
assume cs:_TEXT
org 100h
start:

idle:
jmp idle

_TEXT ends
end start
