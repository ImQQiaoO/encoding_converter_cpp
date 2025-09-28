#include <iostream>
#include "Locale.hpp"

class test {
public:
    std::string te;
    explicit test(const std::string &str): te(str) {}

    friend std::ostream &operator<<(std::ostream &os, const test &te) {
        os << te.te;
        return os;
    }
};

int main() {
    
    std::string str = "你好！";
    utils::utf2ansi_out << str << '\n';
    utils::utf2ansi_out << "世界" << '\n';
    utils::utf2ansi_out << "Hello, World!" << '\n';
    utils::utf2ansi_out << 123 << utils::endl;
    std::cout << str << std::endl;
    
    utils::utf2ansi_out << test("测试重载") << utils::endl;

    std::string str_in;
    utils::ansi2utf_in >> str_in;
    utils::utf2ansi_out << str_in << utils::endl;

    int i_in;
    utils::ansi2utf_in >> i_in;
    std::cout << i_in << std::endl;
    utils::utf2ansi_out << i_in << utils::endl;

    std::string u8str = "测试";
    std::cout << utils::utf8_to_ansi(u8str) << '\n';

    utils::output(std::cout) << "Hello" << utils::reset_put(std::endl);
    utils::output(utils::utf2ansi_out) << "你好" << utils::reset_put(utils::endl);

    utils::output(std::cout, 86,146,118) << "Hello" << utils::reset_put(std::endl);
    utils::output(utils::utf2ansi_out, 182,185,98) << "你好" << utils::reset_put(utils::endl);
    utils::output(std::cout, 86,146,118) << "Hello" << utils::reset_put << " World" << utils::reset_put(std::endl);

    // 编译错误
    // utils::output(std::cout, 182,185,98) << "Hello" << utils::output(std::cout, 86,146,118) << " World" << utils::reset_put(std::endl);
}
