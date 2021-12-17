//asm_in_c.c USE EXTENDED CODE
#include <stdio.h>
#include <string.h>

int x=11, y=12, sum, prod;

int subtract(int lhs, int rhs){//use arguments in registers
   __asm__(
         ".intel_syntax noprefix;"
         "mov rax, rdi;"
         "sub rax, rsi");
}

void multiply(){//use global variables
   __asm__(
         ".intel_syntax noprefix;"
         "mov rax, x;"
         "imul rax, y;"
         "mov prod, rax;"
         :"=a"(prod)//output operands, a is rax
         :"a"(x),"c"(y)//input operands
         :"rbx"//clobbers(changed)
         //goto labels(clear)
);

}


int main(int argc, char* argv[]){
   printf("The numbers are %d and %d\n", x, y);
   __asm__(".intel_syntax noprefix;"
         "mov rax, x;"
         "add rax, y;"
         "mov sum, rax;");
   printf("The sum is %d.\n", sum);
   printf("The difference is %d.\n", subtract(x, y));
   multiply();
   printf("The product is %d.\n", prod);
   return 0;
}
