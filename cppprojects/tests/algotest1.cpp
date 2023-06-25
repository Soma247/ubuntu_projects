#include <iostream>
#include <string>
int main(int argc, char** argv){
   while(argc--)
      std::cout<<argv[argc]<<std::endl;
   std::cout<<"hello world!"<<std::endl;
   return 0;

}
