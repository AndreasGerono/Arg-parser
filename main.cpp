#include <iostream>
#include <functional>
#include <string>
#include <tuple>
#include <array>
#include "parser.h"

template <typename T>
void add_argument(T args, std::function<void(T)> callback)
{
    callback(args);
}

void int_argument(int a) {
    std::cout<<a<<"I' am a from int_argument"<<std::endl;
}

void double_argument(double a) {
    std::cout<<a<<"I' am from double_argument"<<std::endl;
}

void string_argument(std::string a) {
    std::cout<<a<<"I' am from string_argument"<<std::endl;
}

using namespace::std;

int main(void) {
    add_argument<int>(10, &int_argument);
    add_argument<double>(10.69, &double_argument);
    add_argument<string>("test", &string_argument);
    add_argument<array<int, 2>>(array<int,2>{32, 31}, [] (auto args) {
        cout<<"Calling from lambda!: "<<args[0]<<args[1]<<endl;
    });
    cout<<"This is a test!"<<endl;

    auto parser = Parser();
    parser.add_argument<int>(32, "int")
            .help("test of the help")
            .callback([] (auto val) {
                cout<<"Calling from lambda!: "<<val<<endl;
            });
    parser.parse_args();
}