// example.cpp
#include "searfile.hpp"
#include <iostream>

using namespace fsize_literals;

int main() {
    fcommand::create("demo.txt");

    {
        searfile f("demo.txt");

        f.buffed();
        f << "hello world\n";
        f << "abc hello xyz\n";

        std::string temp;
        f >> temp;
        std::cout << temp << '\n';

        f.reflect(); // 여기서 실제 파일에 반영
    }

    {
        searfile f("demo.txt");
        f.buffed(0_bytes, 100_bytes);

        auto st = f.search("hello");
        if (!st.no_match_at_all() && !st.cut) {
            std::cout << "found at s = " << st.s << '\n';
        }
    }

    fcommand::remove("demo.txt");
}
