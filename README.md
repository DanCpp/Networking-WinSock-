# Networking-WinSock-
Try to make usual messenger using C++ WinSock2 library


1. Before you will try to "turn on" {(UI)Client}.exe, you have to turn on {Server.exe} otherwise the {(UI)Client.exe} will throw exception and shutdown.
2. On the CMD Client before sending a message or file you have to choose which type of message will it be by clicking on right key.
"""
KEYS:
'f' - File
'F' - File
'm' - Message
'M' - Message
"""
if you selected File Type, you would write a path to File (IF YOU USE CMD CLIENT).
3.This Server can send all of File types, videos, audios, txt. All in.
4.Max amount of clients on server could be 100, if you didn't correct it on Server/main.cpp
5. When client dissconnected, Server can not add new client on this place, it will be fixed in future.
