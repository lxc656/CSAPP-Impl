p.s. 由于bomb lab和后面的lab需要使用gdb调试器，而m1芯片在硬件层面上对gdb不支持，因此笔者换回了x86 intel芯片的旧Macbook做实验

lab主要是对elf文件反汇编，并根据bomb.c中的提示，来获取每个phase的密钥

参考https://zhuanlan.zhihu.com/p/104130161配置环境

该lab的关于过程调用的前置知识可参考笔者的这篇文章

https://zhuanlan.zhihu.com/p/417283985

开始做lab，首先在Terminal中执行

```shell
objdump -d bomb > bomb.asm
```

对可执行文件反汇编，生成汇编代码bomb.asm

先看主函数

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4ankoic0j60qk0j876s02.jpg" style="zoom:50%;" />

可以看到，如上图，主函数的逻辑就是判断输入的字符(即密钥)是否正确，如果正确，那么就让用户输入下一个密钥，否则引爆炸弹，先看phase_1函数的反汇编结果

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4auyf60dj60rk09a75o02.jpg" style="zoom:50%;" />

x86体系结构的64位汇编中，过程调用的参数传递规则如下：

当参数少于7个时， 参数从左到右放入寄存器: rdi, rsi, rdx, rcx, r8, r9。
当参数为7个以上时， 前 6 个与前面一样， 但后面的依次从 “右向左” 放入栈中

e.g.

H(a, b, c, d, e, f, g, h);
a->%rdi, b->%rsi, c->%rdx, d->%rcx, e->%r8, f->%r9
h->8(%esp)
g->(%esp)
call H

因此，立即数0x402400被作为函数strings_not_equal的第二个参数被传入esi寄存器，也就是rsi寄存器的低32位中，因此有必要分析一下strings_not_equal函数

在strings_not_equal中，有如下代码

![](https://tva1.sinaimg.cn/large/008i3skNly1gv4h7weli7j60se086dh302.jpg)

对%rbx和%rpx分别加1，最后又条件跳转到这段代码的开头，我们有理由推测这段代码是逐个字符对比两个字符串的内容是否一致，因此rbx和rbp寄存器的值应该是指向两个字符串的指针，在代码的最开始，rbp和rbx寄存器的值是rdi和rsi寄存器所赋值的，并且它们是被调用者保存寄存器，因此我们可以推测出，rsi寄存器(也就是%esi)中最开始存储的是字符串指针，而在phase_1最开始，执行了

```assembly
mov    $0x402400,%esi
```

在前面phase_1函数的代码那张图中可以看到，这句代码的地址是0x400ee4，因此我们通过gdb在此打断点，然后查看地址0x402400处的字符串的值

![](https://tva1.sinaimg.cn/large/008i3skNly1gv4hqcvt2tj60uo0alwg002.jpg)

地址0x402400处果然是一个字符串，这样我们就破解出了第一个密钥



接下来分析phase_2

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4lcoca1oj60r606awf702.jpg" style="zoom:50%;" />

phase_2最开始这一段的代码，先分配了0x28字节的栈帧空间，然后将%rsp作为第二个参数存入%rsi，然后调用read_six_numbers，根据这个函数名，我们可以推测出它有至少6个参数

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4lg994mlj60s60d8q5902.jpg" style="zoom:50%;" />

接下来分析read_six_numbers函数：先分配0x18字节的栈帧空间，然后将%rsi作为过程调用的第三个参数，存入%rdx，之后将0x4(%rsi)作为第四个参数存入%rcx(此时%rsi是phase_2中调用read_six_numbers前的栈指针%rsp的值)，然后将0x14(%rsi)地址处的数据存入0x8(%rsp)地址处，再将0x10(%rsi)地址处的数据存入%rsp地址处，之后将0xc(%rsi)作为第六个参数存入%r9，再之后将0x8(%rsi)作为第五个参数存入%r8，然后更新%esi中的值为0x4025c3作为第二个参数，之后调用__isoc99_sscanf@plt函数(简记为scanf函数)，其中0x8(%rsp)地址处存放的应该是第八个参数，%rsp地址处存放的应该是第七个参数，从scanf函数的第三个参数到第八个参数应该就是read_six_numbers中的6个数字，在调用scanf前，它们在栈帧中的布局如下图(参考https://www.zhihu.com/column/c_1325107476128473088)

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5i02llp2j61de0qw77502.jpg" style="zoom:150%;" />

之后继续分析phase_2调用read_six_numbers之后的代码

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4mw1whdvj60ou098gn402.jpg" style="zoom:50%;" />

先将%rsp地址处的值和0x1比较，不相等就引爆炸弹，因此%rsp地址处的值是1，之后跳转到0x400f30地址处

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4n1jvsmcj60oy0340t402.jpg" style="zoom:50%;" />

0x400f30处代码如上图，把这两段代码结合起来分析，0x400f1a地址处的代码执行完之后，%eax中的值是%rsp地址处的值的两倍，之后拿%eax的值和%rbx处的值，即0x4(%rsp)处的值比较，比较的结果必须是相等，否则引爆炸弹，结合上面的栈帧分析图，在read_six_numbers返回后，只剩phase_2的栈帧，图中phase_2的栈帧的六个方框里存放的应该就是读到的6个数字，此时的%rsp指向的应该是图中%rsi指向的位置(图中phase_2的栈帧有一点小问题，没有画出调用read_six_numbers时压入的返回地址，但在此可以忽略这个无关紧要的细节)，刚才分析到，0x4(%rsp)处的值是%rsp地址处的值的两倍，%rsp地址处的值是1，程序执行到了

```assembly
je     400f25 <phase_2+0x29>
```

我们来看phase_2剩下的代码

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4npm8ocmj60re0b80ue02.jpg" style="zoom:50%;" />

可以看到，这是一个循环，因为%rbp的值是0x18(%rsp)，在%rbx没有累加到与其相等之前，都会跳回到地址0x400f17处重复执行将当前%rsp指向的数翻倍后和下一个数比较的操作，因此这6个数是等比数列1 2 4 8 16 32，将这个等比数列作为phase_2的密钥才不会爆炸



接下来分析phase_3

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4ocpzaa8j30ti09aabl.jpg" style="zoom:50%;" />

首先分配栈帧，然后将0xc(%rsp)和0x8(%rsp)这两个地址值作为参数传递给scanf函数，如果scanf成功调用，那么存储在%eax中的返回值是2，之后就会跳转到地址0x400f6a处

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4ophykz9j60o2046t9b02.jpg" style="zoom:50%;" />

首先检查地址0x8(%rsp)处的数值不能超过7否则炸弹爆炸，然后将其存入%eax，之后跳转到m[0x402470+r[%rax]*8]，因为0x8(%rsp)被赋值给了%rdx，根据x86-64参数传递顺序，0x8(%rsp)处存储的应该是scanf读取的第一个数的值，我们尝试phase_3第一个输入0，这样就跳转到了m[0x402470]，然后用gdb查看m[0x402470]的值

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4p4hd3wuj30t90clgnd.jpg" style="zoom:50%;" />

如上图所示，在间接跳转命令处打断点，之后可以看到此时m[0x402470]的值是0x00400f7c，之后在汇编代码中查看0x00400f7c地址处的指令

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4p8nzleuj60p002674f02.jpg" style="zoom:50%;" />

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv4p9e0xolj30oi03474o.jpg" style="zoom:50%;" />

其中将0xc(%rsp)处的值和0xcf(十进制207)进行比较，结合之前对该过程参数传递的分析，0xc(%rsp)是scanf输入的第二个数，因此我们便得出了phase_3的一个密钥 0 207(还有其他密钥，因为第一个输入的数不同，则跳转到的地址也不同，对第二个输入的数的限制也不一样，根据这个逻辑可以得知，C程序源代码中是switch语句)

接下来分析phase_4

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5ifdcgc6j60s8094ta802.jpg" style="zoom:50%;" />

最开始这一段和前面的几个phase差不多，都是调用scanf函数获取输入的数据，0xc(%rsp)地址处存储着输入的第二个数，0x8(%rsp)地址处存储着输入的第一个数，根据后面的

```assembly
cmp    $0x2,%eax
jne    401035 <phase_4+0x29>
```

可以看出scanf返回后,eax寄存器中存储的返回值是2(否则爆炸)，可以进一步确认只输入了两个数，继续阅读phase_4:

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5ikxpkw4j60oy07475e02.jpg" style="zoom:50%;" />

之后将0x8(%rsp)地址处存储的输入的第一个数和立即数0xe做比较，若比0xe大则引起爆炸，那么第一个数必小于等于0x7，之后传递一些参数，调用func4

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5iwivznlj60vc0badi102.jpg" style="zoom:50%;" />

func4最开始的代码如上图，绿色的注释是笔者加上的，以便解释计算过程，执行到

```assembly
cmp    %edi,%ecx
jle    400ff2 <func4+0x24>
```

时，%edi是0x8(%rsp)地址处存储的输入的第一个数的值，%ecx是0x7，如果输入的第一个数大于等于0x7，就执行下面的那行条件跳转指令，不妨我们就第一个数输入7，随即跳转到下图的代码去执行

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5j91lrxqj60o608aab902.jpg" style="zoom:50%;" />

最开始又做了一次比较，因为输入的是7，就又可以执行后面的条件跳转指令，之后就返回了，然后phase_4最后的代码中有

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5jd147djj60ok056wf502.jpg" style="zoom:50%;" />

将0xc(%rsp)地址处存储的输入的第二个数和0比较，不相等则爆炸，因此我们推断出了一个可行的密钥 7 0

func4中，如果不能条件跳转，就修改一些寄存器的值，然后继续调用func4，由此可以推断出func4的逻辑应该是递归调用，逆向工程后的C语言代码可以是下面这样(参考https://zhuanlan.zhihu.com/p/104130161)

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5kyfu2yej60iw0l00u502.jpg" style="zoom:50%;" />

然后在主函数中执行下面的代码，在输入的第一个数的可取值范围内遍历，找出可以让func4成功返回的取值(最后发现是0/1/3/7)

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5l3cractj60fo05iq3302.jpg" style="zoom:50%;" />



接下来分析phase_5

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5mpadqsyj60yc0bcjti02.jpg" style="zoom:50%;" />

绿色的注释大致讲清楚了代码的逻辑(金丝雀值在csapp第三章的缓冲区溢出章节有提及)，随着程序的执行流(跳转到0x4010d2)继续分析，4010d2地址处的指令如下

```assembly
mov    $0x0,%eax
jmp    40108b <phase_5+0x29>
```

0x40108b处是一个循环

![](https://tva1.sinaimg.cn/large/008i3skNly1gv5ng622kij613e0a441302.jpg)

代码的逻辑如注释所说，可以大致被逆向工程为下面的C语言代码

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5nlaq72pj60f004g74b02.jpg" style="zoom:50%;" />

循环结束之后执行下面的代码，让注释里提到的两个字符串比较，结果应该是相等，否则爆炸

![](https://tva1.sinaimg.cn/large/008i3skNly1gv5nte8ogoj61140jggpm02.jpg)

我们先后查看前面提到的0x4024b0和0x40245e地址处的字符串

![](https://tva1.sinaimg.cn/large/008i3skNly1gv5ojgj744j61a603xaan02.jpg)

前面逆向工程的代码里说过

```c
narr[i] = array[input[i]&0xf]
```

此时array应该是0x4024b0，narr是栈中的地址，由前文推断得知，该地址处的字符串"flyers"相等，由此可以推出，'Y_>U6W'可以作为密钥(其他可行的密钥可参考https://zhuanlan.zhihu.com/p/104130161)

目前弄拆除了5个bomb，如下图

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gv5ozhsbe1j60ny0b00ts02.jpg" style="zoom:50%;" />

有关phase_6和传说中的彩蛋phase，且听下回分解
