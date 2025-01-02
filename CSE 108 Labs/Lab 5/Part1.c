#include <stdio.h>

typedef enum Operation{

ADDITION,
SUBTRACTION,
MULTILPICATION,
DIVISION

} optype;

void get_parameters (double *a, optype *op, double *b){

    double get_operand1;
    double get_operand2;
    char operator_char;
    
    printf("Please enter first operand \n");
    scanf(" %lf",&get_operand1);

    *a = get_operand1;


    printf("Please enter second operand \n");
    scanf(" %lf",&get_operand2);

    *b = get_operand2;


    printf("Please enter an operator \n");
    scanf(" %c",&operator_char);

    switch(operator_char){

        case '+': 

            *op = ADDITION;
            break;

        case '-': 
        
            *op = SUBTRACTION;
            break;

        case '*': 
        
            *op = MULTILPICATION;
            break;

        case '/': 
        
            *op = DIVISION;
            break;
    }
}

void print_result (double a, optype op, double b){

switch(op){

    case ADDITION: 

            printf("Result is : %lf",a+b);
            break;

        case SUBTRACTION: 
        
            printf("Result is : %lf",a-b);
            break;

        case MULTILPICATION: 
        
            printf("Result is : %lf",a*b);
            break;

        case DIVISION: 

            if(b == 0){
                printf("Error! You can't divide by zero!\n");
                break;
            }

            printf("Result is : %lf",(double)a/b);
            break;
}

}

int main(){

    double operand1;
    double operand2;
    optype op;

    get_parameters(&operand1, &op, &operand2);
    print_result (operand1, op, operand2);

    return 0;
}

