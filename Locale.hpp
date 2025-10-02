#pragma once

#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>

#ifdef _WIN32
#include <io.h>
#include <locale>
#include <windows.h>
#include <fcntl.h>

namespace utils {

    template<typename _ = void>
    inline bool starts_with(const std::string &str, const std::string &prefix) {
        if (prefix.size() > str.size())
            return false;
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    template<typename _ = void>
    inline std::string ansi_to_utf8(const std::string &ansi_str) {
        int wide_size = MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), -1, nullptr, 0);
        if (wide_size <= 0)
            return {};
        std::wstring wide_str(wide_size, L'\0');
        MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), -1, &wide_str[0], wide_size);
        int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8_size <= 0)
            return {};
        std::string utf8_str(utf8_size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &utf8_str[0], utf8_size, nullptr, nullptr);
        utf8_str.resize(utf8_size - 1);

        return utf8_str;
    }

    template<typename _ = void>
    inline std::string utf8_to_ansi(const std::string &utf8_str) {
        int wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
        if (wide_size <= 0)
            return {};
        std::wstring wide_str(wide_size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, wide_str.data(), wide_size);
        int ansi_size = WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (ansi_size <= 0)
            return {};
        std::string ansi_str(ansi_size, '\0');
        WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, ansi_str.data(), ansi_size, nullptr, nullptr);
        ansi_str.resize(ansi_size - 1);

        return ansi_str;
    }

    template<typename _ = void>
    inline std::string utf8_to_unicode_escape(std::string_view utf8_str) {
        const char *src_str = utf8_str.data();
        int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, nullptr, 0);
        const std::size_t wstr_length = static_cast<std::size_t>(len) + 1U;
        auto wstr = new wchar_t[wstr_length];
        memset(wstr, 0, sizeof(wstr[0]) * wstr_length);
        MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wstr, len);

        std::string unicode_escape_str = {};
        constexpr static char hex_code[] = "0123456789abcdef";
        for (const wchar_t *pchr = wstr; *pchr; ++pchr) {
            const wchar_t &chr = *pchr;
            if (chr > 255) {
                unicode_escape_str += "\\u";
                unicode_escape_str.push_back(hex_code[chr >> 12]);
                unicode_escape_str.push_back(hex_code[(chr >> 8) & 15]);
                unicode_escape_str.push_back(hex_code[(chr >> 4) & 15]);
                unicode_escape_str.push_back(hex_code[chr & 15]);
            } else {
                unicode_escape_str.push_back(chr & 255);
            }
        }

        delete[] wstr;
        wstr = nullptr;

        return unicode_escape_str;
    }

    inline std::string load_file_without_bom(const std::filesystem::path &path) {
        std::ifstream ifs(path, std::ios::in);
        if (!ifs.is_open()) {
            return {};
        }
        std::stringstream iss;
        iss << ifs.rdbuf();
        ifs.close();
        std::string str = iss.str();

        if (starts_with(str, "\xEF\xBB\xBF")) {
            str.assign(str.begin() + 3, str.end());
        }

        return str;
    }

    // UTF-8 string 转换为 wstring 的辅助函数
    inline std::wstring utf8_to_wstring(const std::string &utf8_str) {
        if (utf8_str.empty()) {
            return std::wstring();
        }

        // 计算所需的宽字符缓冲区大小
        int size = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
        if (size == 0) {
            // 转换失败
            return std::wstring();
        }

        // 创建宽字符缓冲区并进行转换
        std::wstring wstr(size - 1, L'\0');// size-1 因为不需要包含null终止符
        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wstr[0], size);

        return wstr;
    }

    // wstring 转换为 UTF-8 string 的辅助函数
    inline std::string wstring_to_utf8(const std::wstring &wstr) {
        if (wstr.empty()) {
            return std::string();
        }

        // 计算所需的 UTF-8 缓冲区大小
        int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size == 0) {
            // 转换失败
            return std::string();
        }

        // 创建 UTF-8 缓冲区并进行转换
        std::string utf8_str(size - 1, '\0');// size-1 因为不需要包含null终止符
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8_str[0], size, nullptr, nullptr);

        return utf8_str;
    }

    class utf8_scope {
#ifdef _WIN32
        bool active;
        int thread_mode;
        std::string old_locale;

        static bool is_console_ostream(const std::ostream &ofs) {
            if (&ofs == &std::cout) {
                return _isatty(_fileno(stdout));
            }
            if (&ofs == &std::cerr) {
                return _isatty(_fileno(stderr));
            }
            return false;
        }

    public:
        explicit utf8_scope(std::nullptr_t) {
            active = false;
            thread_mode = 0;
        }

        ~utf8_scope() {
            if (!active) {
                return;
            }
            if (!old_locale.empty()) {
                setlocale(LC_ALL, old_locale.c_str());
            }
            _configthreadlocale(_DISABLE_PER_THREAD_LOCALE);
        }

        explicit utf8_scope(std::ostream &ofs) {
            if (is_console_ostream(ofs)) {
                active = true;
                thread_mode = _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
                if (auto old_locale_ptr = setlocale(LC_ALL, ".UTF-8")) {
                    old_locale = old_locale_ptr;
                }
            } else {
                active = false;
            }
        }
#else
    public:
        utf8_scope(std::ostream &) {}
#endif
    };

    class ansi_ostream {
    public:
        std::ostream &out;

        explicit ansi_ostream(std::ostream &os) : out(os) {}

        ansi_ostream &operator<<(const std::string &utf8str) {
            std::string ansi_str = utf8_to_ansi(utf8str);
            out << ansi_str;
            return *this;
        }

        ansi_ostream &operator<<(const char *utf8str) {
            std::string ansi_str = utf8_to_ansi(std::string(utf8str));
            out << ansi_str;
            return *this;
        }

        template<typename T,
            typename = std::enable_if_t<std::is_fundamental_v<std::decay_t<T>>>>
            ansi_ostream & operator<<(T &&value) {
            out << std::forward<T>(value);
            return *this;
        }

        template<typename T,
            typename = std::enable_if_t<!std::is_fundamental_v<std::decay_t<T>>>,
            typename = void>
        ansi_ostream &operator<<(const T &value) {
            std::stringstream utf8ss;
            utf8ss << value;
            std::string utf8str = utf8ss.str();
            std::string ansi_str = utf8_to_ansi(utf8str);
            out << ansi_str;
            return *this;
        }

        ansi_ostream &operator<<(ansi_ostream &(*manip)(ansi_ostream &)) {
            return manip(*this);
        }
    };

    static ansi_ostream utf2ansi_out(std::cout);

    inline ansi_ostream &endl(ansi_ostream &os) {
        os.out.put('\n');
        os.out.flush();
        return os;
    }

    class ansi_istream {
    public:
        std::istream &in;

        explicit ansi_istream(std::istream &is) : in(is) {}

        std::string get() const {
            std::string line;
            std::getline(in, line);
            return ansi_to_utf8(line);
        }

        friend ansi_istream &operator>>(ansi_istream &ai, std::string &str) {
            std::getline(ai.in, str);
            str = ansi_to_utf8(str);
            return ai;
        }

        template<typename T>
        friend ansi_istream &operator>>(ansi_istream &ai, T &value) {
            ai.in >> value;
            return ai;
        }
    };

    // RAII 类：在构造时设置控制台为 UTF-8，在析构时恢复原始设置
    class ConsoleUTF8Guard {
    public:
        ConsoleUTF8Guard() {
#ifdef _WIN32
            // Backup original code page
            old_output_cp = GetConsoleOutputCP();
            old_input_cp = GetConsoleCP();

            // Set console to UTF-8 code page
            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8);

            // Set standard input/output to UTF-8 mode
            old_stdout_mode = _setmode(_fileno(stdout), _O_U8TEXT);
            old_stdin_mode = _setmode(_fileno(stdin), _O_U8TEXT);
#endif
        }

        ~ConsoleUTF8Guard() {
#ifdef _WIN32
            // Restore original code page
            SetConsoleOutputCP(old_output_cp);
            SetConsoleCP(old_input_cp);

            // Restore original input/output mode
            _setmode(_fileno(stdout), old_stdout_mode);
            _setmode(_fileno(stdin), old_stdin_mode);
#endif
        }

    private:
#ifdef _WIN32
        UINT old_output_cp;
        UINT old_input_cp;
        int old_stdout_mode;
        int old_stdin_mode;
#endif
    };

    static ansi_istream ansi2utf_in(std::cin);

    // OUT_RESET: 作为操纵器对象，避免函数名重载导致的歧义
    struct out_reset_t {};
    inline constexpr out_reset_t OUT_RESET {};
    template <typename Stream>
    inline Stream &operator<<(Stream &os, out_reset_t) {
        os << "\033[0m";
        return os;
    }

    template <typename Stream>
    inline Stream &output(Stream &os) {
        return os;// 仅返回原始流，不改变颜色
    }

    template <typename Stream, typename R, typename G, typename B>
    inline Stream &output(Stream &os, R r, G g, B b) {
        os << "\033[38;2;" << r << ';' << g << ';' << b << 'm';
        return os;
    }

    // 完美转发版本：支持右值（临时）流，如 qDebug()
    // 仅在传入为右值且非 const 时参与重载，避免与左值重载冲突
    template <typename S,
        std::enable_if_t<std::is_rvalue_reference_v<S &&> &&
        !std::is_const_v<std::remove_reference_t<S>>,
        int> = 0>
    inline S &&output(S &&os) {
        return std::forward<S>(os);
    }

    template <typename S, typename R, typename G, typename B,
        std::enable_if_t<std::is_rvalue_reference_v<S &&> &&
        !std::is_const_v<std::remove_reference_t<S>>,
        int> = 0>
    inline S &&output(S &&os, R r, G g, B b) {
        os << "\033[38;2;" << r << ';' << g << ';' << b << 'm';
        return std::forward<S>(os);
    }

    // 三参数版本：返回一个可插入到流中的操纵器对象，并复用四参数实现
    struct rgb_begin_tag_t {
        int r;
        int g;
        int b;
    };

    // 将三参操纵器插入到任意标准输出流时，调用已有的四参实现
    template <typename Stream>
    inline Stream &operator<<(Stream &os, const rgb_begin_tag_t &t) {
        return output(os, t.r, t.g, t.b);
    }

    // 为 utils::ansi_ostream 提供特化，避免与其成员模板产生二义性
    inline ansi_ostream &operator<<(utils::ansi_ostream &os, const rgb_begin_tag_t &t) {
        output(os, t.r, t.g, t.b);
        return os;
    }

    // 三参数 output：构造操纵器对象，颜色在插入到流时生效
    template<typename R, typename G, typename B>
    inline rgb_begin_tag_t output(R r, G g, B b) {
        return rgb_begin_tag_t {static_cast<int>(r), static_cast<int>(g), static_cast<int>(b)};
    }

    // reset_put: 支持两种调用方式的实现
    // 1. 直接作为操纵器：reset_put
    // 2. 带参数的函数调用：reset_put(std::endl) 或 reset_put(utils::endl)

    // 基础 reset_put 结构体（用于无参数情况）
    struct rgb_end_tag_t {};

    template<typename Stream>
    inline Stream &operator<<(Stream &os, rgb_end_tag_t) {
        os << "\033[0m";
        return os;
    }

    // 为 utils::ansi_ostream 提供特化以避免与其成员模板产生二义性
    inline ansi_ostream &operator<<(ansi_ostream &os, rgb_end_tag_t) {
        os << "\033[0m";
        return os;
    }

    // 用于存储任意操纵器的包装类
    template <typename Manipulator>
    struct rgb_end_with_manip_t {
        Manipulator manip;
        explicit rgb_end_with_manip_t(Manipulator m) : manip(std::move(m)) {}
    };

    // 为带操纵器的包装类定义输出操作符
    template <typename Stream, typename Manipulator>
    inline Stream &operator<<(Stream &os, const rgb_end_with_manip_t<Manipulator> &obj) {
        os << "\033[0m";
        return os << obj.manip;
    }

    // 为 utils::ansi_ostream 特化
    template <typename Manipulator>
    inline ansi_ostream &operator<<(ansi_ostream &os, const rgb_end_with_manip_t<Manipulator> &obj) {
        os << "\033[0m";
        return os << obj.manip;
    }

    // reset_put 函数对象：既可以直接使用，也可以作为函数调用
    struct rgb_end_callable_t {
        // 直接转换为 rgb_end_tag_t（用于 reset_put 直接使用的情况）
        // 不要explicit，否则会导致 reset_put 失效
        operator rgb_end_tag_t() const {
            return {};
        }

        // 函数调用操作符，接受任何操纵器
        template <typename Manipulator>
        rgb_end_with_manip_t<Manipulator> operator()(Manipulator manip) const {
            return rgb_end_with_manip_t<Manipulator>{manip};
        }

        // 特殊处理 std::endl 的重载
        rgb_end_with_manip_t<std::ostream &(*)(std::ostream &)> operator()(std::ostream &(*manip)(std::ostream &)) const {
            return rgb_end_with_manip_t<std::ostream & (*)(std::ostream &)>{manip};
        }
    };

    inline constexpr rgb_end_callable_t reset_put {};

}// namespace utils

#endif

#define YELLOW_OUTPUT_START             "\033[93m"
#define RED_OUTPUT_START                "\033[91m"
#define RGB_OUTPUT_START(r, g, b)       "\033[38;2;" #r ";" #g ";" #b "m"
#define OUT_RESET                       "\033[0m"
