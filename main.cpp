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

int main(int argc, char *argv[]) {
    cout<<"This is a test!"<<endl;

    auto parser = Parser(argc, argv);

    parser.add_argument<int>("-pw", "--power");
    parser.add_argument<double>("t", "type");
    parser.add_argument<array<int, 4>>("--tacho")
            .callback([] (auto val) {
                cout<<"Calling from lambda!: "<<val[0]<<endl;
            });
    parser.add_argument<bool>("+o");

     
    cout<<parser.parse_args();
}