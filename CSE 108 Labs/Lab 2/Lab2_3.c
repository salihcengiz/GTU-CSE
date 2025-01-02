#include <stdio.h>
#include <math.h>

void quadrant(int a, int b){

if(a>0 && b>0)
        printf(" Point in 1. quadrant \n");  
    
    if(a>0 && b<0)
        printf(" Point in 4. quadrant \n");  
    
    if(a<0 && b>0)
        printf(" Point in 2. quadrant \n");

    if(a<0 && b<0)
        printf(" Point in 3. quadrant \n");
}

int distance(int a, int b, int c, int d){

    int m, n;

    m = a - b;
    
    n = c - d;

    return sqrt(m*m + n*n);
}

int main() {

    int x1, y1, x2, y2, a, b, Distance;

    printf("Please enter values of point A: \n");
    scanf("%d%d",&x1,&y1);

    printf("Please enter values of point B: \n");
    scanf("%d%d",&x2,&y2);

    quadrant(x1,y1);
     
    quadrant(x2,y2);

    Distance = distance(x1,x2,y1,y2);

    printf("The distance is %d", Distance);

    return 0;
}