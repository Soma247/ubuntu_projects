#include <iostream>
#include <iomanip>
#include <thread>
void print(std::string_view sv){
    std::cout<<std::this_thread::get_id()<<std::quoted(sv)<<std::endl;

}

int main(int argc, char* argv[]){
    using namespace std::literals;
    std::thread t(print,"Hello, World!"sv);
    for(--argc;argc>=0;--argc){
      std::cout<<argv[argc]<<std::endl;
    }
    t.join();
    return 0;
}
