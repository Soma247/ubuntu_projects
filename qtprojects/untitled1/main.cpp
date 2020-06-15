#include <iostream>
#include <iomanip>
#include <thread>
void print(std::string_view sv){
    std::cout<<std::this_thread::get_id()<<std::quoted(sv)<<std::endl;

}
int main(){
    using namespace std::literals;
    std::thread t(print,"Hello, World!"sv);
    t.join();
    return 0;
}
