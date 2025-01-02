#include <stdio.h>
#define MAXIT 100

int hs (int nums[], int *cn, int maxit){

    if(*cn < maxit && nums[*cn-1] == 1)
        return 0;
    else{

        if(nums[*cn-1] % 2 == 0)
            nums[*cn] = nums[*cn-1] / 2;
        else    
            nums[*cn] = 3*nums[*cn-1]+1;

        *cn = *cn +1;

        printf("%d ",nums[*cn-1]);

        return hs(nums, cn, maxit);
    }
}

int main(){
    

    int i, n = 1;
    int a[MAXIT] = {3};

    printf("%d ",a[0]);

    n = hs(a, &n, MAXIT);

    return 0;
}
