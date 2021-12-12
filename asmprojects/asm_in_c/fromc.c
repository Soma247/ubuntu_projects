//fromc.c
#include <stdio.h>
#include <string.h>
extern int rarea(int, int);
extern int rcircum(int, int);
extern double carea(double);
extern double ccircum(double);
extern void sreverse(char*, int);
extern void adouble(double[], int);
extern double asum(double[], int);
int main(int argc, char** argv){
   char rstring[64];
   int side1, side2, r_area, r_circum;//rect
   double radius, c_area, c_circum, sum;//circle
   double darray[]={70.0, 83.2, 92.1, 54.5, 74.0};
   long int len;
//calling assembler functions:
//int args
   printf("Compute area and circumference of a rectangle\n");
   printf("Enter the length of one side: ");
   scanf ("%d", &side1);
   printf("\nEnter the length of second side: ");
   scanf("%d", &side2);
   printf("\n");
   r_area = rarea(side1, side2);
   r_circum = rcircum(side1, side2);
   printf("The area of rectangle is %d.\n", r_area);
   printf("The circumference of rectangle is %d.\n",r_circum);
//double args
   printf("Compute area and circumference of a circle\n");
   printf("Enter the radius: ");
   scanf ("%lf", &radius);
   printf("\n");
   printf("%lf\n",radius);
   c_area = carea(radius);
   printf("%lf\n",c_area);
   c_circum = ccircum(radius);
   printf("The area of circle is %lf.\n", c_area);
   printf("The circumference of circle is %lf.\n",c_circum);
//cstr args
   printf("Reverse a string\n"); 
   printf("Enter a string(max length is 64 chars): ");
   scanf("%s", rstring);
   sreverse(rstring, strlen(rstring));
   printf("Reversed string is: %s\n", rstring);
//array args
   printf("Some array manipulations\n");
   len = sizeof(darray)/sizeof(double);
   printf("The array has %lu elements\n", len);
   sum = asum(darray, len);
   printf("The sum of elements is %lf\n", sum);
   return 0;
}
