#include <iostream>
#include <string>
#include "../thirdparty/cpp-httplib/httplib.h"

using namespace std;
using namespace httplib;

int main() {
    Server svr;

    svr.Post("/im", [](const Request& req, Response& res) {
        res.set_content("12345678", 5, "text/plain");
    })
    .listen("localhost", 40000);
    return 0;
}
