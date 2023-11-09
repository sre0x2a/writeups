---
title: im_crackme
date: 2023-11-09
---

Below details the analysis to find the correct password for the bytecode VM binary, im_crackme, found on the [crackmes.one](https://crackmes.one/crackme/653784b96451214b1db98670) website. The post is part of the "Blundering to Greatness" series that chronicles the journey of a software engineer and their attempts to become a software reverse engineer. The focus of the blog is on static binary analysis of CTFs, crackmes, and various other binary samples. Where appropriate other techniques will be used.

# Analysis of im_crackme

The binary is a 64 bit Windows PE generated from C/C++ source code and compiled with MS Visual Studio C++ compiler. The binary is listed as a level 3 out of a possible 6 in difficulty. (Ghidra will be used for the static analysis.)

Before the main method executes, a serious of functions are executed during the initialization of the program.

![initterm](https://sre0x2a.github.io/writeups/assets/images/imcrackme/initterm.png "initterm")

![init functions](https://sre0x2a.github.io/writeups/assets/images/imcrackme/init_functions.png "init functions")

The function at 0x140001000 initializes a new std::string instance that will be used to store the user input password.

The function at 0x140001030 will initialize a new std::vector\<int\> instance. This particular instance will be populated with 125 signed integers (32 bit values) after instantiation and while still in the function. The 125 integers taken as single bytes will make up the instruction set for the instruction/bytecode VM.

The function at 0x1400015e0 will also create a new std::vector\<int\> instance and populate the instance with 8 signed integers. These values will be used in comparison operations to validate the correct user input password.

The function at 0x1400016a0 will create the last std::vector\<int\> instance. The vector instance will act as the registers used by the instruction set for the instruction VM.

The main function is located at 0x1400020a0 and prompts the user for a password. The password string instance mentioned above is populated with the user input. A loop is entered that will call the function that is the instruction VM for the challenge until the size of the first vector is less than or equal to iteration variable. The iteration variable is incremented by 1 with each iteration.

Jumping into the instruction VM function at 0x140001db0, we find a switch statement. The current loop iteration variable is effectively the instruction pointer. The 125 element vector will have the element at the current iteration variable copied to a local variable. The local variable value will then be copied to another local variable. That second local variable value will be subtracted by 1. The resulting value will be the opcode for the instruction.

The cases of the switch statement correspond to instructions/opcodes for the instruction set. The table that follows displays the full instruction set with the original value not subtracted by 1 and the number of bytes for the instruction.

|Opcode|Instruction|Bytes|
|---|---|---|
|1|MOV rX,vector[Y]|3|
|2|MOV rX,password[Y]|3|
|3|XOR rX,rY|3|
|4|CMP rX,rY|3|
|5|JZ imm32|2|
|6|JNZ imm32|2|
|7|print "congrat!"|1|
|8|print "try again.."|1|
|9|CMP password.length(), imm32|2|

I have a few notes of the instructions listed in the table above. The `X` and `Y` correspond to the second and third byte of the bytecode for the instruction. The values are used as an index into the respective password string or vector. The vector being referenced is the second vector populated with the 8 integer values. The `imm32` refers to the byte (integer) of the bytecode being used as is and not as an index. The `r` refers to a register.

I wrote a simple disassembler [program](https://sre0x2a.github.io/writeups/assets/files/imcrackme/disasm.c) to show the resulting [code](https://sre0x2a.github.io/writeups/assets/files/imcrackme/listing.txt) to check for a correct password.

The first character of the password is XOR'd by 77 (0x4d) and compared to 4. If the result of the XOR is not 4, the "try again.." message is output. We want to avoid the jumps to address 0x77 in the binary as that will print the above fail message.

```
0x0:	MOV r1,password[0]
0x3:	MOV r2,0x4d
0x6:	XOR r1,r2
0x9:	MOV r2,0x4
0xc:	CMP r1,r2
0xf:	JNZ 0x77
```

The second character is XOR'd by the result of the previous XOR (4). The result of that operation needs to be equal to 73 (0x49).

```
0x11:	MOV r1,password[1]
0x14:	XOR r2,r1
0x17:	MOV r3,0x49
0x1a:	CMP r2,r3
0x1d:	JNZ 0x77
```

The third character needs to be equal to 95 (0x5f).

```
0x1f:	MOV r2,password[2]
0x22:	MOV r1,0x5f
0x25:	CMP r2,r1
0x28:	JNZ 0x77
```

Now comes the fun. The fourth character needs to be equal to 71 (0x47). The result of XOR'ing the fourth character and the fifth and XOR'ing that result by the seventh character needs to be equal to 76 (0x4c).

```
0x2a:	MOV r1,password[3]
0x2d:	MOV r2,password[4]
0x30:	MOV r3,password[5]
0x33:	MOV r4,password[6]
0x36:	MOV r0,0x47
0x39:	CMP r1,r0
0x3c:	JNZ 0x77
0x3e:	MOV r0,0x4c
0x41:	XOR r1,r2
0x44:	XOR r1,r4
0x47:	CMP r0,r1
0x4a:	JNZ 0x77
```

We can try to bruteforce those values but lets see if we can get some more info to fill in the variable values of the fifth and seventh character.

The next check XOR's the seventh character to the sixth character. The result needs to be equal to 11 (0xb).

```
0x4c:	MOV r4,password[6]
0x4f:	XOR r3,r4
0x52:	MOV r4,0xb
0x55:	CMP r3,r4
0x58:	JNZ 0x77
```

The final check that needs to pass is the XOR'ing of seventh character to the fourth. The result being XOR'd to the fifth character with that result being XOR'd to the sixth character. The final result needs to be equal to 3.

```
0x5a:	MOV r1,password[3]
0x5d:	MOV r2,password[4]
0x60:	MOV r3,password[5]
0x63:	MOV r4,password[6]
0x66:	XOR r4,r1
0x69:	XOR r4,r2
0x6c:	XOR r4,r3
0x6f:	MOV r1,0x3
0x72:	CMP r4,r1
0x75:	JZ 0x78
```

Creating equations for the three operation's results being compared above gives:

```
// let p represent the password as an array of char
(1) p[3] ^ p[4] ^ p[6]        = 76
(2) p[5] ^ p[6]               = 11
(3) p[6] ^ p[3] ^ p[4] ^ p[5] = 3
```

Substituting (2) into (3) and using the value of p[3] equaling 71 (0x47) results in the equation

```
(4)  11 ^ 71 ^ 3 = p[4]
```

That results in the fifth character equal to 79 (0x4f). Substituting that value into equation (1) results in the following equation

```
(5) 71 ^ 79 ^ 76 = p[6]
```

That results in the seventh character equal to 73 (0x49). Placing that result into equation (2) results in the sixth character equal to 79 (0x4f).

Putting all the character values together results in the string "IM_GOOD". A final check is performed on the password length. The length of the string needs to be seven characters. The string "IM_GOOD" is seven characters which more proof of a correct password.

```
0x77:	try again..
0x78:	CMP password.length(), 0x7
0x7a:	JNZ 0x77
0x7c:	congrat!
```

When the user inputs the password "IM_GOOD" the "congrat!" string is output indicating a succesful reversing session.
