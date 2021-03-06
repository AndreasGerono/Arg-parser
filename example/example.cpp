#include <iostream>

#include "argumentParser.h"

using namespace std;

int main(int argc, char* argv[]) {
    cout << "This is a example use of argumentParser!" << endl;

    auto parser = ArgumentParser(argc, argv);

    parser.add_argument<string>("type", "2").help("set a type of something...");
    parser.add_argument<string>(Arg{"test", "test1"}, "1").help("test of something");
    parser.add_argument<int>(Arg{"-pw", "--power"});
    parser.add_argument<int>("--tacho", "+2")
        .callback([](vector<int> val) {
            cout << "Calling from lambda!: ";
            for (auto v : val) {
                cout << v << " ";
            }
            cout << endl;
        });

    parser.add_argument<bool>("-o")
        .store_true()
        .callback([](bool o) {
            cout << "Calling from lambda: " << o << endl;
        });

    try {
        parser.parse_args();
    } catch (const exception& e) {
        cout << endl
             << e.what();
    }
    auto test1 = parser.get<string>("type");
    auto test2 = parser.get<int>("-pw");
    auto test3 = parser.get<bool>("-o");
    cout << endl
         << test1;
    cout << endl
         << test2;
    cout << endl
         << test3;
    cout << endl;
    parser.print_help();
}
