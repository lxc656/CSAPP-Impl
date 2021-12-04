Lab官方文档：http://csapp.cs.cmu.edu/3e/cachelab.pdf

docker镜像资源：https://zhuanlan.zhihu.com/p/138881600

这个Lab的Part A是写一个Cache模拟器，要求和官方的标准程序`csim-ref`有同样的输出，有关缓存模拟器的实现思路可以参考笔者的这篇Blog

https://zhuanlan.zhihu.com/p/439700180

由于笔者此前的虚拟机里的Cache模拟已经实现的差不多了，Part A就下次一定了，针对CSAPP本Lab的PartA的具体实现可以移步至

https://zhuanlan.zhihu.com/p/410662053

接下来具体讲解和分析Part B

**Part B: Optimizing Matrix Transpose**

In Part B you will write a transpose function in trans.c that causes as few cache misses as possible. 

我们要做的是在`trans.c`里实现一个名为`transpose_submit`的函数，完成矩阵转置，不仅要逻辑正确，还要尽量减少代码运行过程中的cache miss

具体的要求如下图所示

![](https://tva1.sinaimg.cn/large/008i3skNly1gx1regxntsj313w0dcacj.jpg)

根据它给的`s=5, E=1, b=5`的参数可以得出，Cache有2^5=32个set，每个set中有1个line，每个line的block有2^5=32个字节，因此，物理地址的低5位是block offset，低6～10位是set index，其余的都是tag

我们先分析最直接，逻辑上最简单的矩阵转置程序，即`trans.c`中的`trans`函数

```c
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}
```

我们假设`N`和`M`都是32，即是32x32的矩阵，cache的参数和上面说的一样

矩阵A和B的使用的是二维数组的数据结构来存储，在物理内存中的排列如下图所示（以A为例）

![](https://tva1.sinaimg.cn/large/008i3skNly1gx1tfp473jj311a0boq45.jpg)

每个矩阵元素是int类型，占4个字节，每个cache line的block有32个字节，因此每个cache line能存放物理内存中连续的8个矩阵元素，而且由于block offset占物理地址中最低的5位，block offset左侧的5位是set index，因此物理地址每增大32，就会映射到下一个set，也就是说，在物理内存中连续存放的每8个矩阵元素映射到痛一个set中，而总共有32个set，那也就是连续的8x32个矩阵元素分别映射到不同的set，不会有cache冲突与踢出的情况发生，但如果前后两次访问的矩阵元素的地址相差大于32x32，就有可能发生cache冲突了，比如说，我们先后访问`A[0][0]`，`A[1][0]`, `A[2][0]`, ...  ,`A[7][0]`这同一列的8个矩阵元素， 每两个元素之间的地址差是32x4，也就是每两次访问中间跨过了32个矩阵元素，前面提到过“连续的8x32个矩阵元素分别映射到不同的set里”，因此到目前为止访问到的这8个矩阵元素分别位于cache不同的set里，如果我们此时访问`A[8][0]`，它和`A[0][0]`被映射到了同一个set里，而每个set只有一个cache line，那么就会导致把这个set唯一的cache line里的元素，即`A[0][0]`~`A[0][7]`踢出，换成`A[8][0]`~`A[8][7]`，接下来结合前面的简单矩阵转置的`trans`函数来分析

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx1u1i3eomj30d2068aa5.jpg" style="zoom:50%;" />

按照行优先访问`A[i][j]`对缓存还是友好的，第一次访问`A[0][0]`会cache miss，随后访问`A[0][1]`~`A[0][7]`都是cache hit，而与此同时，第一次访问`B[0][0]`会cache miss，随后访问`B[1][0]~B[7][0]`都会一直cache miss，这是因为每个cache line只有32字节大小的block，访问`B[0][0]`发生cache miss之后，会把`B[0][0]`~`B[0][7]`放入cache line中，因此之后访问`B[1][0]`还会继续cache miss，而且访问到`B[8][0]`时还会发生cache冲突，将`B[0][0]`所在的line的数据踢出，导致后面从`B[0][1]`开始访问下一列的元素时，还是会一直cache miss，因此每次访问B数组的元素都必须要访问内存，效率低下

所以说我们要依据cache的性质对这个矩阵转置的代码进行优化

# 对32x32的矩阵转置

我们可以采用将矩阵分块的策略进行优化，前面分析过，连续读取矩阵同一列的元素时，连续读8个以上就会发生cache冲突，因此我们不妨把32x32的矩阵拆分成16个8x8的矩阵，分别对这16个小矩阵进行转置

这样的话，在对小矩阵进行转置时，在从上到下读取一列元素时，例如先读取从`B[0][0]`到`B[7][0]`，会一直cache miss，之后读取从`B[0][1]`到`B[7][1]`，会一直cache hit，因为一直没有发生cache冲突，所以之前读取`B[0][0]`时也把`B[0][1]`以及后继的一整个cache line的数据读入了cache，同理，其实整个B矩阵中的8x8小矩阵的数据都在最初的8次cache miss后被读入了cache，所以后面访问小矩阵的数据时，只会一直cache hit

由于A数组是以行优先的顺序被访问的，更加的缓存友好，因此也只会发生8次cache miss，其余都是cache hit，没有cache冲突

代码实现如下

```c
for (i = 0; i < N; i += 8) {
    for (j = 0; j < M; j += 8) {
        // 分块
        for (k = i; k < min(i + 8, N); k++) {
            for (s = j; s < min(j + 8, M); s++) {
                B[s][k] = A[k][s];
            }
        }
    }
}
```

这样写代码固然降低了cache miss率，但还有可以优化的地方，分析如下：

在`trancegen.c`中有对矩阵A和B的声明

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx1vbcz0qxj30by0420sx.jpg" style="zoom:50%;" />

二维数组`A`和`B`都是静态全局变量，因此在程序执行时都会位于.bss segment，并且A数组和B数组在.bss segment中连续存放，B数组的起始地址-A数组的起始地址 =256x256x4，即2^18，也就是A数组和B数组的起始地址的低10位都是相同的，因此A数组和B数组的元素可能会被映射到相同的cache line而发生冲突，我们上面的分析其实没有考虑这一点，也就是说，实际上发生cache miss的次数比我们预料的还要多

A或B数组同一列上下的元素的地址之差是2^7，set index是物理地址的低6～10位，因此如果`B[s][k] = A[k][s];`这行代码中的`s`和`k`不相等的话，`B[s][k]`的地址的set index和`A[k][s]`的地址的set index一定不相等，只有在k=s，即转置对角线上的元素时，`B[s][k]`和`A[k][s]`映射到了同一个cache line，相继访问这两个数据就会导致cache冲突，用 `A[n]` 表示A矩阵的第 n 的缓存块，最开始是`A[k]`存储在它对应的cache line中，之后`B[k]`的内容会将这个cache line中的`A[k]`的数据冲掉，之后访问`A[k][s+1]`时，`A[k]`会被放回该cache line，把`B[k]`冲掉，因此就多了两次cache miss

除此以外，n>=2时，在`A[n][n]处`发生完上述的cache冲突的最后，对应cache line中存储的`B[n]` 被 `A[n]` 取代，在后面需要访问`B[n][n+1]`时，又会发生cache miss，如下图红色圆圈处所示

![](https://tva1.sinaimg.cn/large/008i3skNly1gx244qn5suj31bs0pl417.jpg)

对于最开始的简单的转置代码，即下图中的代码

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx1u1i3eomj30d2068aa5.jpg" style="zoom:50%;" />

其执行过程中发生的cache miss可以如下形象的表示（出自https://yangtau.me/computer-system/csapp-cache.html#fn:1）

![](https://tva1.sinaimg.cn/large/008i3skNly1gx24p8odmjj30u00v2gp6.jpg)

如果我们能够做到访问完A矩阵的对角线上的元素后，不立即访问B矩阵的对角线上相同坐标的元素，就会减少一定的cache miss，因此可以如下面的代码所示，引入局部变量`a0`~`a7`，局部变量往往被存储在寄存器中，涉及不到内存访问

```c
for (i = 0; i < 32; i += 8) {
    for (j = 0; j < 32; j += 8) {
        for (k = i; k < i + 8; k++) {
            a0 = A[k][j];
            a1 = A[k][j + 1];
            a2 = A[k][j + 2];
            a3 = A[k][j + 3];
            a4 = A[k][j + 4];
            a5 = A[k][j + 5];
            a6 = A[k][j + 6];
            a7 = A[k][j + 7];
            B[j][k] = a0;
            B[j + 1][k] = a1;
            B[j + 2][k] = a2;
            B[j + 3][k] = a3;
            B[j + 4][k] = a4;
            B[j + 5][k] = a5;
            B[j + 6][k] = a6;
            B[j + 7][k] = a7;
        }
    }
}
```

这种情况下的cache miss仅在于复制 `A[m]`到局部变量时会取代 cache line中的`B[m]`( 第一行除外 )，之后将局部变量的数据写入 `B[m]` 的时候又会重新将`B[m]`载入cache line一次

接下来介绍一种性能更好的方法，可以避免cache冲突

在Lab的要求中有说道，我们不可以修改A矩阵的二维数组，但可以修改B矩阵的，因此就有了如下操作：前面cache冲突的本质是A矩阵在进行行优先访问时，B在进行列优先访问，如果我们使用前面的引入局部变量的方法的同时对B也进行行优先访问，之后再在B内部转置，那么就可以做到没有cache冲突，代码如下

```c
const int len = 8;
for (i = 0; i < N; i += len) {
    for (j = 0; j < N; j += len) {
        // copy
        for (k = i, s = j; k < i + len; k++, s++) {
            a0 = A[k][j];
            a1 = A[k][j + 1];
            a2 = A[k][j + 2];
            a3 = A[k][j + 3];
            a4 = A[k][j + 4];
            a5 = A[k][j + 5];
            a6 = A[k][j + 6];
            a7 = A[k][j + 7];
            B[s][i] = a0;
            B[s][i + 1] = a1;
            B[s][i + 2] = a2;
            B[s][i + 3] = a3;
            B[s][i + 4] = a4;
            B[s][i + 5] = a5;
            B[s][i + 6] = a6;
            B[s][i + 7] = a7;
        }
        // transpose
        for (k = 0; k < len; k++) {
            for (s = k + 1; s < len; s++) {
                a0 = B[k + j][s + i];
                B[k + j][s + i] = B[s + j][k + i];
                B[s + j][k + i] = a0;
            }
        }
    }
}
```

![](https://tva1.sinaimg.cn/large/008i3skNly1gx25stc32vj30xo0iu0w2.jpg)

最终测试一下，可以看到miss的次数小于300，达到目标

# 对64x64的矩阵转置

矩阵变大到了64x64，矩阵的第i行的元素和第i+4行的元素就会映射到相同的cache line，从而发生cache冲突，因此如果继续分块成8x8的矩阵，在小矩阵内部就会发生cache冲突，如果按照4x4来分块，只能使用cache line一半的block，无法充分利用cache

下面介绍一个可以减少cache miss的方案：

我们还是先把64x64的大矩阵分成一个个8x8的小块来处理，之后将每个小块的前四行拆分成两个4x4的矩阵，依次转置之后复制到B矩阵中对应的位置，如下图所示：

这是我们即将复制的A矩阵的4x8的块（彩色部分）

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx27aossdfj30aa09sjrl.jpg" style="zoom:50%;" />

我们把彩色部分拆分成左右两半，每一半都转置之后存入B矩阵中对应的位置（这个操作可以使用前面32x32矩阵的转置的最优化方法完成），如下图，是向B矩阵写入数据之后B矩阵的状态

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx27cr1a9jj30as0aaaab.jpg" style="zoom:50%;" />

此时，B矩阵彩色区域的左半部分是正确的转置结果，彩色区域的右半部分还需要挪到这个8x8的块的左下角

**Step1**:我们接下来将A矩阵的下图蓝色和紫色标注的元素存入本地变量`buf1`和`buf2`

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx27jza1qjj30am0dmjrq.jpg" style="zoom:50%;" />

**Step2**:之后将`buf1`和B矩阵彩色区域的右侧的4x4小块的第一行交换，将`buf2`写入该4x4小块的下方，如下图：

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx27mhwcycj30d607u74i.jpg" style="zoom:50%;" />

**Step3**:之后再将`buf1`中的元素（此时`buf1`被更新过了）写到左侧4x4小块的下方，如下图：

<img src="https://tva1.sinaimg.cn/large/008i3skNly1gx27rwdguaj309w09ot8z.jpg" style="zoom:50%;" />

之后一直重复Step1~Step3，就可以完成这个64x64的矩阵的转置

具体参考代码如下

```c
for (i = 0; i < N; i += block_size) {
    for (j = 0; j < M; j += block_size) {
        for (k = 0; k < block_size / 2; k++) {
            // A top left
            a0 = A[k + i][j];
            a1 = A[k + i][j + 1];
            a2 = A[k + i][j + 2];
            a3 = A[k + i][j + 3];

            // copy
            // A top right
            a4 = A[k + i][j + 4];
            a5 = A[k + i][j + 5];
            a6 = A[k + i][j + 6];
            a7 = A[k + i][j + 7];

            // B top left
            B[j][k + i] = a0;
            B[j + 1][k + i] = a1;
            B[j + 2][k + i] = a2;
            B[j + 3][k + i] = a3;

            // copy
            // B top right
            B[j + 0][k + 4 + i] = a4;
            B[j + 1][k + 4 + i] = a5;
            B[j + 2][k + 4 + i] = a6;
            B[j + 3][k + 4 + i] = a7;
        }
        for (k = 0; k < block_size / 2; k++) {
            // step 1
            a0 = A[i + 4][j + k], a4 = A[i + 4][j + k + 4];
            a1 = A[i + 5][j + k], a5 = A[i + 5][j + k + 4];
            a2 = A[i + 6][j + k], a6 = A[i + 6][j + k + 4];
            a3 = A[i + 7][j + k], a7 = A[i + 7][j + k + 4];
            // step 2
            tmp = B[j + k][i + 4], B[j + k][i + 4] = a0, a0 = tmp;
            tmp = B[j + k][i + 5], B[j + k][i + 5] = a1, a1 = tmp;
            tmp = B[j + k][i + 6], B[j + k][i + 6] = a2, a2 = tmp;
            tmp = B[j + k][i + 7], B[j + k][i + 7] = a3, a3 = tmp;
            // step 3
            B[j + k + 4][i + 0] = a0, B[j + k + 4][i + 4 + 0] = a4;
            B[j + k + 4][i + 1] = a1, B[j + k + 4][i + 4 + 1] = a5;
            B[j + k + 4][i + 2] = a2, B[j + k + 4][i + 4 + 2] = a6;
            B[j + k + 4][i + 3] = a3, B[j + k + 4][i + 4 + 3] = a7;
        }
    }
}
```

这个策略下最终的cache miss数是满足要求的

# 对61x67的矩阵转置

关于最后这个比较玄学，目前没有看到太多的定量的分析，主要思路就是找出一个比较合适的分块策略然后看看cache miss的次数能不能满足要求

查阅资料得知8x23的划分方式可以通过，代码如下

```c
for (i = 0; i < N; i += 8) {
    for (j = 0; j < M; j += 23) {
        if (i + 8 <= N && j + 23 <= M) {
            for (s = j; s < j + 23; s++) {
                a0 = A[i][s];
                a1 = A[i + 1][s];
                a2 = A[i + 2][s];
                a3 = A[i + 3][s];
                a4 = A[i + 4][s];
                a5 = A[i + 5][s];
                a6 = A[i + 6][s];
                a7 = A[i + 7][s];
                B[s][i + 0] = a0;
                B[s][i + 1] = a1;
                B[s][i + 2] = a2;
                B[s][i + 3] = a3;
                B[s][i + 4] = a4;
                B[s][i + 5] = a5;
                B[s][i + 6] = a6;
                B[s][i + 7] = a7;
            }
        } else {
            for (k = i; k < min(i + 8, N); k++) {
                for (s = j; s < min(j + 23, M); s++) {
                    B[s][k] = A[k][s];
                }
            }
        }
    }
}
```

Reference:

https://yangtau.me/computer-system/csapp-cache.html#32-x-32

https://zhuanlan.zhihu.com/p/410662053