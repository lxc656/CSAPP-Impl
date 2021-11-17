int narr[];
int array[];
int input[];
int main()
{
    for(int i=0;i<6;i++)
    {
        narr[i] = array[input[i]&0xf];
    }
}