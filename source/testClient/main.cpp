#include <iostream>
#include <string>
#include <vector>
#include "../thirdparty/cpp-httplib/httplib.h"
#include "../thirdparty/json/json.hpp"
#include <mutex>

using namespace httplib;
using namespace std;



int main(int argc, const char* argv[]) {
    Client cli("localhost", 40000);
    string body(R"(
    {
    "method":"get",
    "number":"1",
    "content":{
        "userId":"777"
    }
}
    )");
    auto ret = cli.Post("/im", body, "text/plain");
    if(ret && ret->status == 200) {
        cout<<ret->body<<endl;
    }

    return 0;
}
