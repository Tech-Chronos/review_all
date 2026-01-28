#include "util.h"

// int main()
// {
//     // std::string str = "abc,,,b,.,,.cvbf,fda,";
//     // std::string sep = ",";
//     // std::vector<std::string> arr;
//     // Util tool;
//     // tool.Split(str, sep , &arr);
//     // //Util::Split(str, sep , &arr);
//     // for (auto& e : arr)
//     // {
//     //     std::cout << e << std::endl;
//     // }

//     // std::string str;
//     // std::string filename = "../Connection.cc";
//     // Util::ReadFile(filename, &str);
//     // std::cout << str << std::endl;

//     // Util::WriteFile("test.txt", &str);
//     std::string str = Util::UrlEncode("password=C  ", false);
//     std::cout << str << std::endl;
//     std::string str2 = Util::UrlDecode(str, false);
//     std::cout << str2 << std::endl;
// 

int main()
{
    std::string desc(Util::GetStatusDesc(500));
    
    std::cout << desc << std::endl;

    std::string suffix(Util::GetMime("a.b.wasm"));
    std::cout << suffix << std::endl;

    std::cout << Util::IsValidPath("/html/hello/../../../Channel.cc") << std::endl;
}