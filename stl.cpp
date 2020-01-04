#include <iostream>
#include <iterator>
#include <string>
int main(int argc, char* argv[]){
   std::string str{};
   for(int i=0;i<argc;++i){
      str+=argv[i];
   }
   str+="Hello, world!";
   std::cout<<str<<std::endl;
   return 0;
}
