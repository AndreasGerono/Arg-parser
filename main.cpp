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

    auto parser = ArgumentParser(argc, argv);

    parser.add_argument<string>("type", "2");
    parser.add_argument<string>(Arg{"test", "test1"}, "1");
    parser.add_argument<int>(Arg{"-pw", "--power"});
    // parser.add_argument<double>("t", "type");
    parser.add_argument<int>("--tacho", "+2")
            .callback([] (vector<int> val) {
                cout<<"Calling from lambda!: ";
                for (auto v: val) {
                    cout<<v<<" ";
                }
                cout<<endl;
            });

    parser.add_argument<bool>("-o")
                    .store_true()
                    .callback([] (bool o) {
                        //cout<<"Calling from lambda: "<<o<<endl;
                    });

    try {
        parser.parse_args();
    }
    catch(const exception& e) {
        cout<<endl<<e.what();
    }
}
