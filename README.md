---
title: 关于内存中的数组和数组越界
date: 2019-07-13 17:18:37
tags: pwn
---

读了

[逆向知识十三讲,汇编中数组的表现形式,以及还原数组](https://www.cnblogs.com/ye-ming/articles/7990986.html)

[[原创]数组越界之入门向](https://bbs.pediy.com/thread-228652.htm)

有感就瞎写这个笔记

实践就用这个代码：

```c
//arr.c
#include<stdio.h>
int
main ()
{
  int minus_one = -1;
  char arr[8] = "testAr";
  int i;
  unsigned int uint; //数组下标就是无符号的
  while (1)
    {
      scanf ("%d", &i);
      printf("var i : %d\n",i);
      uint = i;
      printf("index array : %u\n",uint);
      printf ("%c\n", arr[i]);
    }
  return 0;
}
```

编译运行：

```
mk at ubuntu in ~/disk/CTF/pwn/temp
$ gcc -fno-stack-protector -g arr.c -o arr

mk at ubuntu in ~/disk/CTF/pwn/temp
$ ./arr
-1
var i : -1
array index: 4294967295
�
-2
var i : -2
array index : 4294967294
�
1
var i : 1
array index : 1
e
```

至于为什么会这样请看：

[CTF-all-in-one 的整数溢出](https://firmianay.gitbooks.io/ctf-all-in-one/content/doc/3.1.2_integer_overflow.html)



gdb 载入

反汇编 main 函数：

```assembly
gdb-peda$ disassemble main
Dump of assembler code for function main:
   0x0000000000400596 <+0>:	push   rbp
   0x0000000000400597 <+1>:	mov    rbp,rsp
   0x000000000040059a <+4>:	sub    rsp,0x30
   0x000000000040059e <+8>:	mov    QWORD PTR [rbp-0x8],0xffffffffffffffff
   0x00000000004005a6 <+16>:	movabs rax,0x724174736574
   0x00000000004005b0 <+26>:	mov    QWORD PTR [rbp-0x20],rax
=> 0x00000000004005b4 <+30>:	lea    rax,[rbp-0x24]
   0x00000000004005b8 <+34>:	mov    rsi,rax
   0x00000000004005bb <+37>:	mov    edi,0x4006a4
   0x00000000004005c0 <+42>:	mov    eax,0x0
   0x00000000004005c5 <+47>:	call   0x400480 <__isoc99_scanf@plt>
   0x00000000004005ca <+52>:	mov    eax,DWORD PTR [rbp-0x24]
   0x00000000004005cd <+55>:	mov    esi,eax
   0x00000000004005cf <+57>:	mov    edi,0x4006a7
   0x00000000004005d4 <+62>:	mov    eax,0x0
   0x00000000004005d9 <+67>:	call   0x400460 <printf@plt>
   0x00000000004005de <+72>:	mov    eax,DWORD PTR [rbp-0x24]
   0x00000000004005e1 <+75>:	mov    DWORD PTR [rbp-0xc],eax
   0x00000000004005e4 <+78>:	mov    eax,DWORD PTR [rbp-0xc]
   0x00000000004005e7 <+81>:	mov    esi,eax
   0x00000000004005e9 <+83>:	mov    edi,0x4006b3
   0x00000000004005ee <+88>:	mov    eax,0x0
   0x00000000004005f3 <+93>:	call   0x400460 <printf@plt>
   0x00000000004005f8 <+98>:	mov    eax,DWORD PTR [rbp-0x24]
   0x00000000004005fb <+101>:	cdqe   
   0x00000000004005fd <+103>:	movzx  eax,BYTE PTR [rbp+rax*1-0x20]
   0x0000000000400602 <+108>:	movsx  eax,al
   0x0000000000400605 <+111>:	mov    esi,eax
   0x0000000000400607 <+113>:	mov    edi,0x4006c5
   0x000000000040060c <+118>:	mov    eax,0x0
   0x0000000000400611 <+123>:	call   0x400460 <printf@plt>
   0x0000000000400616 <+128>:	jmp    0x4005b4 <main+30>
End of assembler dump.
```

看到了吗 

```
0x000000000040059e <+8>:	mov    QWORD PTR [rbp-0x8],0xffffffffffffffff
```

c code 就是 `int minus_one = -1;`



数组元素在栈中的布局：

```
           低地址
+--------------------------+<--- [rbp - 0x20 - 0x8]
|         数组下标          |          
+--------------------------+<--- [rbp - 0x20]
|         arr[0]           |
+--------------------------+
|         arr[1]           |
+--------------------------+
|         arr[2]           |
+--------------------------+
|         arr[..]          |
+--------------------------+
|           ...            |
+--------------------------+
|           rbp            |
+--------------------------+
|           ret            |
+--------------------------+
           高地址
```

现在知道

数组下标在：`[rbp - 0x20 - 0x8]（0x7fffffffda68）`

我们输入 `1`

看到 stack 上的内容

```
00:0000│ rsp  0x7fffffffda60 —▸ 0x7fffffffda8e ◂— 0x400620ffff
01:0008│      0x7fffffffda68 ◂— 0x100000000 /* 看到了吗，数组下标为 0x1 (高 8 位无效，下面解释) */
02:0010│      0x7fffffffda70 ◂— 0x724174736574 /* 'testAr' */
03:0018│      0x7fffffffda78 —▸ 0x4004a0 (_start) ◂— xor    ebp, ebp
04:0020│      0x7fffffffda80 ◂— 0x1ffffdb70
05:0028│      0x7fffffffda88 ◂— 0xffffffffffffffff
06:0030│ rbp  0x7fffffffda90 —▸ 0x400620 (__libc_csu_init) ◂— push   r15
```



数组的内容存在：`[rbp - 0x20]（0x7fffffffda70）`

```
gdb-peda$ p $rbp-0x20
$17 = (void *) 0x7fffffffda70
gdb-peda$ x $rbp-0x20
0x7fffffffda70:	0x0000724174736574
gdb-peda$ x/s $rbp-0x20
0x7fffffffda70:	"testAr"
```





寻找数组的元素，`rbp - 0x24`  是我们的输入  ` i`  

```
   0x4005f8 <main+98>     mov    eax, dword ptr [rbp - 0x24]
   0x4005fb <main+101>    cdqe   
   0x4005fd <main+103>    movzx  eax, byte ptr [rbp + rax - 0x20]
```

可以看出 rbp-0x20就是我们数组的首元素的位置，我们输入 `0` 相当于  `arr[0]`  取  `rbp + 0 - 0x20`   

```
gdb-peda$ set $eax = 0
gdb-peda$ x/c $rbp + $rax - 0x20
0x7fffffffda70:	0x74 //字符 t 
gdb-peda$ set $eax = 1
gdb-peda$ x/c $rbp + $rax - 0x20
0x7fffffffda71:	0x65 ////字符 e
```



当我们输入 -1 时

stack 上的内容：

```
00:0000│ rsp  0x7fffffffda60 —▸ 0x7fffffffda8e ◂— 0x400620ffff
01:0008│      0x7fffffffda68 ◂— 0xffffffff00000000 /* 看到了吗，数组下标为 0xffffffff */
02:0010│      0x7fffffffda70 ◂— 0x724174736574 /* 'testAr' */
03:0018│      0x7fffffffda78 —▸ 0x4004a0 (_start) ◂— xor    ebp, ebp
04:0020│      0x7fffffffda80 ◂— 0xffffffffffffdb70
05:0028│      0x7fffffffda88 ◂— 0xffffffffffffffff
06:0030│ rbp  0x7fffffffda90 —▸ 0x400620 (__libc_csu_init) ◂— push   r15
```



```
gdb-peda$ p $eax
$20 = 0xffffffff
gdb-peda$ p $rax
$21 = 0xffffffff
```

`[rbp + rax - 0x20]`  就变成了 `[rbp + 0xffffffff - 0x20]` 

读到的是 `[rbp + 0xffffffff - 0x20]`

rax 的值（低 16 位，也就是 eax，因为是  `mov    eax, dword ptr [rbp - 0x24] `  ）是可控的

如果精心构造 payload 就能任意读高地址的东西

如果现在是一个 scanf 就能任意写高地址上的东西



