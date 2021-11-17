// a:%rdi b:%rsi  c:%rdx
int func4(int a, int b, int c){
  int return_v = c - b; // %rax
  int t = ((unsigned)return_v) >> 31; // %rcx
  return_v = (t + return_v) >> 1;
  t = return_v + b;
  if (t - a <= 0){
    return_v = 0;
    if (t - a >= 0){
      return return_v;
    }else{
      b = t + 1;
      int r = func4(a,b,c);
      return 2 * r + 1;
    }
  } else {
    c = t - 1;
    int r = func4(a, b, c);
    return 2*r;
  }
}

int main(){
    for(int i = 0; i <= 0xc; i++)
        if(!func4(i, 0, 14))
            printf("%d\n", i);
}