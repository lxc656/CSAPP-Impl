本篇blog继续完成bomb lab，下面开始分析phase_6

phase_6一开始在保存了callee-saved寄存器和分配了栈帧之后执行了如下指令

```assembly
  401100:	49 89 e5             	mov    %rsp,%r13 #r13=rsp
  401103:	48 89 e6             	mov    %rsp,%rsi
  401106:	e8 51 03 00 00       	callq  40145c <read_six_numbers>
```

调用了read_six_numbers读入6个数字，之后执行

```assembly
	40110b:	49 89 e6             	mov    %rsp,%r14 #r14=rsp
  40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  401114:	4c 89 ed             	mov    %r13,%rbp #rbp=r13=rsp
  401117:	41 8b 45 00          	mov    0x0(%r13),%eax #将栈顶元素存入eax
  40111b:	83 e8 01             	sub    $0x1,%eax #eax-=1
  40111e:	83 f8 05             	cmp    $0x5,%eax  
  401121:	76 05                	jbe    401128 <phase_6+0x34>
  401123:	e8 12 03 00 00       	callq  40143a <explode_bomb>
```

根据笔者的注释以及最后三行汇编代码我们可以得出，位于栈顶的元素的数值-1之后必须小于等于5，否则就会explode_bomb，而根据函数的参数入栈的顺序我们可以推断出栈顶的元素是函数read_six_numbers读入的第一个数，因此phase_6输入的第一个数要小于等于6

之后继续分析

```assembly
	401128:	41 83 c4 01          	add    $0x1,%r12d #r12d++
  40112c:	41 83 fc 06          	cmp    $0x6,%r12d #if(r12d==6) break;
  401130:	74 21                	je     401153 <phase_6+0x5f>
```

%r12d最开始是0，然后将其+1，之后和6比较，若等于6则跳转出去，看到这里我们可以判断出这三行代码应该是循环的一部分，而且要循环6次，我们之后继续往下分析

```assembly
  401132:	44 89 e3             	mov    %r12d,%ebx #ebx=r12d
  401135:	48 63 c3             	movslq %ebx,%rax #rax=ebx(32位到64位，采用符号扩展)
  401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax #取栈中下一个4字节的int类型变量放入eax
  40113b:	39 45 00             	cmp    %eax,0x0(%rbp) #将eax和rbp指向的栈中元素做比较
  40113e:	75 05                	jne    401145 <phase_6+0x51> #上一行代码的比较结果必须是不相等，否则就explode了
  401140:	e8 f5 02 00 00       	callq  40143a <explode_bomb>
  401145:	83 c3 01             	add    $0x1,%ebx #ebx+=1
  401148:	83 fb 05             	cmp    $0x5,%ebx
  40114b:	7e e8                	jle    401135 <phase_6+0x41> #这块也是个循环，这个内层循环里做的事情是将栈中元素逐个和rbp指向的元素比较，而rbp被赋值为了r13的值
  40114d:	49 83 c5 04          	add    $0x4,%r13 #r13+=4，也就是说下一轮内层循环中的rbp+=4
  401151:	eb c1                	jmp    401114 <phase_6+0x20>
```

可以看到接下来这段的最后一行代码jmp到了401114地址处，是我们开始的时候分析的代码的地址，因此这意味着外层这一轮循环的结束，根据笔者给的注释，我们可以大致得出目前分析过的所有汇编代码的逻辑用等价的C语言描述应该是下面这样

```c
for(int i=0;i<6;i++)
{
	if(nums[i]>6) explode_bomb();
  for(int j=i+1;j<6;j++)
  {
    if(nums[j]==nums[i]) explode_bomb();
	}
}
```

因此我们可以得出，输入的这6个数都<=6，而且互不相等，显然就是1 2 3 4 5 6的排列组合，我们先输入1 2 3 4 5 6试试

前面的代码的外层循环结束了之后就会跳到0x401153

```assembly
  401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi #rsi=rsp-0x18
```

0x18转换成10进制就是24，也就是跨过了栈中的6个数，在后面我们可以看到，rsi是作为flag用的，我们继续分析：

```assembly
 401158:	4c 89 f0             	mov    %r14,%rax #r14寄存器在程序最开始处被初始化成了栈指针rsp的值，因此rax此时指向栈顶
 40115b:	b9 07 00 00 00       	mov    $0x7,%ecx #ecx=0x7
 401160:	89 ca                	mov    %ecx,%edx #edx=ecx
 401162:	2b 10                	sub    (%rax),%edx #edx-=*rax
 401164:	89 10                	mov    %edx,(%rax) #*rax=edx
```

根据注释可以推断出，上面的代码和如下的C代码等价

```c
num[0] = 7-num[0];
```

之后继续分析

```assembly
  401166:	48 83 c0 04          	add    $0x4,%rax
  40116a:	48 39 f0             	cmp    %rsi,%rax
  40116d:	75 f1                	jne    401160 <phase_6+0x6c>
```

rax指向了栈中下一个元素，并且这也是一个循环，直到rsi==rax循环才结束，而前面的代码中把rsi设置成了rsp-0x18，也就是说只有把栈中的6个整数都按照上面的代码操作了一遍之后才会跳出循环，因此总体上看，等价的C程序如下：

```c
for(int i=0;i<6;i++)
{
  num[i]=7-num[i];
}
```

继续分析

```assembly
  40116f:	be 00 00 00 00       	mov    $0x0,%esi #esi=0
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
```

然后分析0x401197地址处的代码

```assembly
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx
  40119a:	83 f9 01             	cmp    $0x1,%ecx
  40119d:	7e e4                	jle    401183 <phase_6+0x8f>
```

先是将栈中自栈顶偏移量为%rsi*1的元素的值放入ecx寄存器，之后和0x1比较，若<=0x1则跳转，我们假设先不会跳转，继续向下分析

```assembly
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax #eax=1
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx #edx=0x6032d0
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
```

0x6032d0看起来像是个地址，我们取这个地址处的值：

```shell
(gdb) p *0x6032d0
```

值是332，再看看它附近的字节：

![](https://tva1.sinaimg.cn/large/008i3skNly1gw5d47ys69j30ps05yq3z.jpg)

其中第二列的1～6是调试的时候给phase_6需要的这6个数赋的值，更神奇的是，node1这一行里的0x006032e0是node2的地址，node2这一行里的0x006032f0是node3的地址...这是一个链表！刚才我们看到的是链表的next指针

0x6032d0-0x6032e0==0x10，也就是10进制的16，一个node占用了16个字节，而x86-64架构下一个指针8字节，一个int整数4字节，那么每个node里面应该还有个int，应该对应着上图中的第一列

之后继续分析，上面的代码的最后jmp到了0x401176:

```assembly
401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx
40117a:	83 c0 01             	add    $0x1,%eax #eax+=1,eax==2
40117d:	39 c8                	cmp    %ecx,%eax
40117f:	75 f5                	jne    401176 <phase_6+0x82>
```

这块代码是个循环，rdx+0x8地址处是链表节点的next指针，将next指针的值存入rdx寄存器，之后eax++，然后和ecx比较，ecx的值是7-num[0]==6，因此一共在链表上往后走了5个节点，达到了最后一个节点node6，之后执行

```assembly
  401181:	eb 05                	jmp    401188 <phase_6+0x94>
```

跳转到了下面的代码

```assembly
  401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2)
  40118d:	48 83 c6 04          	add    $0x4,%rsi
  401191:	48 83 fe 18          	cmp    $0x18,%rsi
  401195:	74 14                	je     4011ab <phase_6+0xb7>
```

把rdx寄存器的值即&node6存入了(rsp+rsi*2+0x20)地址处，rsi此时是0，之后rsi+=4，然后和0x18即24(demical)比较，不相等，之后不跳转，继续向下执行

```assembly
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx
  40119a:	83 f9 01             	cmp    $0x1,%ecx
  40119d:	7e e4                	jle    401183 <phase_6+0x8f>
```

之后(rsp+1*rsi)地址处的数值即(7-num[1]==5)被存入了ecx寄存器，然后ecx和1比较，不跳转继续执行

```assembly
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
```

之后就又跳转回了前面，这又是一个循环，在下一轮的循环中，ecx的值由上一轮的7-num[0]==6变成了7-num[1]==5，因此最终会将&node5存到了(rsp+(rsi=4)*2+0x20)处，然后再进行一轮循环，会把&node4存入(rsp+8 * 2+0x20)地址处...直至把&node1存入(rsp+20*2+0x20)地址处，因此这一段代码做的事情就是将第7-num[i]个链表节点的地址存入(rsp+(4 * i * 2+0x20)地址处，这段循环完成了之后跳转到0x4011ab

```assembly
 	4011ab:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx
  4011b0:	48 8d 44 24 28       	lea    0x28(%rsp),%rax
  4011b5:	48 8d 74 24 50       	lea    0x50(%rsp),%rsi #0x50(%rsp)应该是个end-flag
  4011ba:	48 89 d9             	mov    %rbx,%rcx #rbx,rcx的值都是0x20(%rsp)地址处存储的链表节点地址
  4011bd:	48 8b 10             	mov    (%rax),%rdx #将0x28(%rsp)地址处的链表节点地址存入rdx寄存器
  4011c0:	48 89 51 08          	mov    %rdx,0x8(%rcx) #将rdx寄存器的内容，即0x28(%rsp)地址处的链表节点地址存入0x8(%rcx)地址处，0x8(%rcx)这个地址存有rcx此时指向的链表节点的next指针
  4011c4:	48 83 c0 08          	add    $0x8,%rax
  4011c8:	48 39 f0             	cmp    %rsi,%rax
  4011cb:	74 05                	je     4011d2 <phase_6+0xde>
  4011cd:	48 89 d1             	mov    %rdx,%rcx #更新迭代
  4011d0:	eb eb                	jmp    4011bd <phase_6+0xc9> #循环往复
```

之所以说0x8(%rcx)这个地址存有rcx此时指向的链表节点的next指针，是因为每个node的结构大致是下面这样

```c
typedef struct node
{
  int value1;
  int value2;
  node* next;
}node_t
```

因此上面的代码实现了这样的操作

```c
node_(7-num[i]).next = node_(7-num[i+1])
```

用个图来表示就是下面这样

![](https://tva1.sinaimg.cn/large/008i3skNly1gw5grt8n6oj30o30bdjs5.jpg)

因此截至目前为止执行的操作等价的C代码为：(就我们的input是1 2 3 4 5 6而言)

```c
for(int i=0;i<6;i++)
{
	if(nums[i]>6) explode_bomb();
  for(int j=i+1;j<6;j++)
  {
    if(nums[j]==nums[i]) explode_bomb();
	}
}

for(int i=0;i<6;i++)
{
  num[i]=7-num[i];
}
//上面的都是之前分析过的，下面是新得到的结果
for(int i=6;i>=1;i--) node[i].next = node[i-1] 
```

通过最后一行代码我们可以看到，我们完成了一个链表的反转

之后继续分析剩下的全部代码：

```assembly
	4011d2:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx) #让链表的最后一个节点的next指针是null
  4011d9:	00 
  4011da:	bd 05 00 00 00       	mov    $0x5,%ebp #rbp=0x5
  4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax #rbx存储的是第一个链表节点的地址,rbx+8处存储着下一个链表节点的地址
  4011e3:	8b 00                	mov    (%rax),%eax #eax存储链表下一个节点的数据值
  4011e5:	39 03                	cmp    %eax,(%rbx) #比较两个相邻链表节点的数据值的大小
  4011e7:	7d 05                	jge    4011ee <phase_6+0xfa> #rbx指向的值更大或相等则jmp，否则爆炸
  4011e9:	e8 4c 02 00 00       	callq  40143a <explode_bomb>
  4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx #之后循环往复比较直到遍历完链表
  4011f2:	83 ed 01             	sub    $0x1,%ebp
  4011f5:	75 e8                	jne    4011df <phase_6+0xeb>
  4011f7:	48 83 c4 50          	add    $0x50,%rsp
  4011fb:	5b                   	pop    %rbx
  4011fc:	5d                   	pop    %rbp
  4011fd:	41 5c                	pop    %r12
  4011ff:	41 5d                	pop    %r13
  401201:	41 5e                	pop    %r14
  401203:	c3                   	retq   
```

笔者给出的注释里说的每个链表节点的数据值是下图中的第一列，即0x14c,0xa8,0x39c,0x2b3这些

![](https://tva1.sinaimg.cn/large/008i3skNly1gw5d47ys69j30ps05yq3z.jpg)

因此我们可以得出，链表反转之后，后一个节点的数据值应该比前面的节点的数据值要大，所以我们的目的是通过输入的6个数把最终反转后的链表排列成一个单调递减的序列

输入的6个数：1～6 into array num[]

链表的从第一个节点到第六个的数据值的10进制表示：

332 168 924 691 477 443

又有：

第7-num[i]个链表节点的地址存入(rsp+(4 * i * 2+0x20)地址处

并且，链表反转时：

```c
 node_(7-num[i]).next = node_(7-num[i+1])
```

924是链表所有node的数值里最大的，应该作为链表反转后的头节点，因此num[0]=4，以此类推，num[1]=3，num[2]=2，num[3]=1，num[4]=6，num[5]=5，应该输入4 3 2 1 6 5，至此6个phase全部完成！

由于笔者对逆向工程目前兴趣不大，感觉水太深把控不住(doge)，彩蛋phase就下次一定了233333

Reference:

https://www.bilibili.com/video/BV1vu411o7QP?spm_id_from=333.999.0.0