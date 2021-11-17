/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitXor - x^y 只通过 ~ 和 &来实现 
 *   例子: bitXor(4, 5) = 1
 */
//只有对应的位上的数值不同时，即不同时为0或不同时为1时，异或的结果才为1
//x&y用于判断对应的位是否同时为1，(~x&~y)用于判断对应的位是否同时为0，因此代码如下:
int bitXor(int x, int y) {
  return ~(~x&~y)&~(x&y);
}
/* 
 * tmin - 使用位运算计算出用二进制补码表示的整数里值最小的整数
 *   可使用的操作符: ! ~ & ^ | + << >>
 *   最多使用4个操作符
 */
/*
一个数的补码A1A2A3...An对应的整数值是A1*(-1)*2^(n-1)+A2*2^(n-2)+A3*2^(n-3)+...+An,因此用二进制补码表示的数里值最小的整数的补码表示是100...0(共31个0)
*/
int tmin(void) {

  return 0x1<<31;

}
/*
 * isTmax -如果输入的数是用二进制补码表示的整数里值最大的整数则返回1，否则返回0
 *   可使用的操作符: ! ~ & ^ | +
 *   最多可使用10个操作符
 */
/*
显然，根据上一个函数里面说到的公式，用二进制补码表示的数里值最大的整数，是01...1(共31个1)，而最后函数的返回值是0或1，所以如果我们设计出将01...1(共31个1)转换成1的程序，那么若程序的结果为1，则输入的整数也是01...1(共31个1),按照这个思路来设计的话，就应该像下面这样：
int isTmax(int x) {
  int i=x+1;
  x=x+i;
  x=~x; //此时，若输入的i是期望的值的话，此时x已经是0了
  return !x;
}
但这么设计的函数是无法通过测试的，因为有一个特例，如果输入的参数是-1，其补码表示为0xff...f(8个f)，每一位都是1，带入函数计算所得返回值也是1，也就是函数的设计出现了二义性，所以要想办法将输入值为-1的情况排除
*/
int isTmax(int x) {
  int i=x+1;
  x=x+i;
  x=~x; //此时x是0
  i=!i; //若输入的是0xff...f，则此时i为1，若输入的是0x01...1,则此时i是0
  x=x+i;//若输入的是0xff...f，则此时x为1，若输入的是0x01...1,则此时x是0
  return !x; //终于排除了参数为-1的情况
}
/* 
 * allOddBits - 如果所有函数的参数所有奇数位都是1，那么返回1
 *   整数类型共有0-31位，统共32位
 *   例如 allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   可使用的操作符: ! ~ & ^ | + << >>
 *   使用的操作符数量上限: 12
 */
/*
解决方法，构造一个只有奇数位为1的掩码然后和输入的参数进行与运算再让掩码和之前与运算的结果进行异或运算
*/
int allOddBits(int x) {
    int mask_number = 0xAA+(0xAA<<8); //这里不能直接赋值为0xAAAAAAAA,因为前面的实验规定里面说过，不可以使用超过8位的整数常量,因此这行和下行代码便一点一点构造出0xAAAAAAAA
        mask_number=mask_number+(mask_number<<16);
    return !((mask_number&x)^mask_number);
}
/* 
 * negate - 返回函数参数的相反数
 *   例如: negate(1) = -1
 *   可用运算符: ! ~ & ^ | + << >>
 *   运算符数量上限: 5
 */
int negate(int x) {
  return ~x+1; /*一个正数的相反数的补码就是这么算出来的，对于负数同样成立，因为补码是阿贝尔群(具体证明和抽象代数有关，笔者目前也不会QAQ)*/
}
/* 
 * isAsciiDigit - 如果0x30 <= x <= 0x39 (是字符'0'到'9'的ASCII码)，就返回1
 *   例如: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   可用运算符: ! ~ & ^ | + << >>
 *   运算符数量上限: 15
 */
int isAsciiDigit(int x) {
  int lowerBound=~0x30+1; //按照negate函数的方式求得0x30的相反数，小于0x30的数和它相加，最终的值就是负数，符号位是1，其他情况下符号位是0
  int upperBound=~0x39+1; //~0x39+1是0x39的相反数，因此大于0x39的数和upperBound相加，符号位是0，其他情况下是1
  int flag1=((lowerBound+x)>>31)&(0x1<<31); //这些关于flag的操作都是为了用于得到符号位
  flag1 = flag1>>31;
  int flag2=((lowerBound+x)>>31)&(0x1<<31);
  flag2 = flag2>>31;
  return ((!flag1)&flag2);
}
/* 
 * conditional - 这个函数的功能和 x?y:z 一样
 *   例如: conditional(2,4,5) = 4
 *   可用运算符: ! ~ & ^ | + << >>
 *   运算符数上限: 16
 */
int conditional(int x, int y, int z) {
  x=!(!x); //这样的话，x如果是非0，就可以被转换成1,x是0的话这句代码执行完还是0不变
  x=~x+1; //上行代码中，x如果被转换成了1，那么这里x就被转换成了1的相反数-1，-1补码每一位都是1，x如果在上一行代码执行结束后还是0，则x的相反数也是0，这行代码对x的值没有影响
  return (x&y)|(~x&z); //根据上面的注释再加上x?y:z的语义，这行代码比较易懂
}
/* 
 * isLessOrEqual - 如果x <= y，返回1，否则返回0 
 *   例如: isLessOrEqual(4,5) = 1.
 *   可用运算符: ! ~ & ^ | + << >>
 *   运算符数上限: 24
 */
int isLessOrEqual(int x, int y) {
  /*我们不能简简单单的用y-x的符号来判断，因为如果y是很大的正整数，x是很小的负整数，很大概率会溢出，因此我们要先判断y和x是不是同号的，同号的话就放心去做减法，不会溢出，异号的话如果x是负数，那么一定y更大*/
  int x_sign = (x & (1<<31))<<31; //得出x的符号
  int y_sign = (y & (1<<31))<<31; //得出y的符号
  int xor_sign = x_sign & y_sign; //用于判断x和y是否异号
  int neg_x = ~x+1; //求x的相反数
  int y_sub_x = neg_x + y; //求y - x
  int y_sub_x_sign = y_sub_x & (1<<31); //得出y-x的符号
  return ((!xor_sign) & (!y_sub_x_sign)) | (xor_sign & x_sign); //按照上面说的逻辑进行判断
}
/* 
 * logicalNeg - 使用前面说过的可用的运算符中除了!以外的运算符来实现!运算符的功能
 *   例如: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   可用运算符: ~ & ^ | + << >>
 *   运算符数上限: 12
 */
/*
!运算符的特点是若输入0，则输出1，输入其他的值，则输出0，而除了0和最小整数(其补码为10...0,31个0)，其他的整数取反加一之后得到的都是其相反数，利用这一特点可以把0和其他整数区分开来
*/
int logicalNeg(int x) {
  int flag = x | (~x+1); //只有0会在这一步结果是0，其他的整数在这步运算之后得到的整数的左侧最高位必定是1
  return (flag>>31 + 1); //所以只有参数0时最终的返回值才是0
}
/* howManyBits - 该函数计算出一个数的补码至少需要几位来表示
 *  例如: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  可用运算符: ! ~ & ^ | + << >>
 *  运算符数上限: 90
 */
/*
对于正数来说，找到它最高的是1的位，假设这个位是第n位，再算上符号位，结果为n+2
对于负数，需要知道它最高的是0的位，再在这个位左侧加上一个为1的最高位，即符号位，就可以正确表示这个负数，在左侧加两个及以上的符号位也是可以的，就是会占用更多的位，不符合本题的要求，比如说，1002和11002按照补码表示法可以表示同一个负数
*/
int howManyBits(int x) {
  int sign = x >> 31; //判断x的符号
  x = (sign & ~x) | (~sign & x); /*x为正则不变，否则按位取反，这样的话无论x之前是正是负，我们接下来都只需寻找新的x的最高的为1的位*/
  int High_16 = (!(!(x>>16))) << 4; //高十六位有1则此值为16
  x = x >> High_16; //如果高十六位有1，将参数右移16位
  int High_8 =  (!(!(x>>8))) << 3;
  x = x >> High_8; //接下来剩余的位里，高8位有1则继续右移8位
  int High_4 =  (!(!(x>>4))) << 2;
  x = x >> High_4; //接下来剩余的位里，高4位有1则继续右移4位
  int High_2 =  (!(!(x>>2))) << 1;
  x = x >> High_2; //接下来剩余的位里，高2位有1则继续右移2位
  int High_1 =  (!(!(x>>1)));
  x = x >> High_1; //接下来剩余的位里，高1位是1则继续右移1位
  int High_0 = x; //这时已经移到了尽头，x要么是0要么是1，因此High_0要么是0要么是1
  return High_16 + High_8 + High_4 + High_2 + High_1 + High_0 + 1; /*最后+1是因为要算上符号位，从高16位到高8位到高4位再到高2位再到高1位的检查过程有一点像二分查找*/
}
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interprete(理解) as the bit-level representation of
 *   single-precision(单精度) floating point values.这块有些只可意会不可言传(实际上笔者英文翻译水平不够，不过感觉各位也能看懂应该，看不懂的话看一下下面的函数的参数与返回值的类型的信息也就懂了)
 *   如果参数不是一个数(NAN)，将其原封不动返回
 *   可用运算符: 任何用于整数型/无符号整数型的运算符 包括 ||, && 还有 if, while
 *   运算符数上限: 30
 */
/*
单精度浮点数的第0位是符号位，第1-8位是阶码，第9-31位是尾数
*/
unsigned floatScale2(unsigned uf) {
  int exp = (uf & 0x7f800000) >> 23; //此处提取阶码,0x7f800000正好是阶码所在的第1-8位全是1，其他位都是0
  int sign = uf & (1<<31); //提取符号位
  if(exp == 0) return uf << 1 | sign; //对于非规范数和0，整体左移一位再还原符号位
  if(exp == 255) return uf; //让NAN和无穷大的数(包括正无穷和负无穷)原封不动返回
  exp++; //因为是乘2，所以阶码自增
  if(exp = 255) return 0x7f800000 | sign; //如果完成运算之后成为了无穷大数，就保留之前的符号，其他的位按照无穷大数的规则来安排
  return (exp << 23) | (uf & 0x807fffff); //将更新后的阶码放到应该放的位置，其他不变(对0x807fffff的理解方式同上面的0x7f800000，结合单精度浮点数的二进制结构来分析)
}

/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 */
int floatFloat2Int(unsigned uf) {
  int sign = uf >> 31; //提取符号位
  int exp = ((uf & 0x7f800000) >> 23) - 127; //根据浮点数表示的规则计算最后乘的是2的多少次幂
  int frac = (uf & 0x807fffff) | 0x00800000; //提取尾数，并在尾数前加上一位，该位的值是1
  if(!(uf&0x7fffffff)) return 0; //如果浮点数是0，则返回0

  if(exp > 31) return 0x80000000;  //最后进行计算的时候，指数比31大，就会超出int值可以表示的范围，属于溢出，NAN和infinit的情况也在此处理范围内
  if(exp < 0) return 0; //最后进行计算的时候，指数比0小，最后的结果就比1小，因此按0算

  if(exp > 23) frac <<= (exp-23); //由于frac小数部分有23位，故作此讨论
  else frac >>= (23-exp);

  if(frac >> 31) return 0x80000000; //发生溢出
  else if(!((frac >> 31)^sign)) return frac; //正的浮点数，且没有溢出，正常返回
  else return ~frac + 1; //uf对应的浮点数是负数
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 */
//求2的x次幂
unsigned floatPower2(int x) {
        if(x<=-150) return 0;
        else if(x<=-127) return (1<<23)>>(-126-x);
        else if(x<=127) return (0x7f+x)<<23;
        else return 0x7f800000u;
}