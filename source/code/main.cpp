#include <iostream>
#include <string>
#include <vector>
#include "../include/message.h"
#include "../thirdparty/cpp-httplib/httplib.h"
#include "../thirdparty/json/json.hpp"
#include <mutex>

//using namespace std;
using namespace im;
using json = nlohmann::json;

httplib::Server& getServer() {
    static httplib::Server svr;
    return svr;
}

void getMsg(json& reqBody, httplib::Response& res) {
    json retJson;
    
    auto pMsgs = IndvMsgMgr::getInstance().getMessage(atoll(reqBody["content"]["userId"].get<std::string>().c_str()));
    retJson["number"] = reqBody["number"];
    if(pMsgs == nullptr) {
        retJson["content"]["text"] = std::string("user is not exist!");
        retJson["errNo"] = std::string("1");
    }else {
        json toRetMsg = json::array();
        for(auto it = pMsgs->begin(); it != pMsgs->end(); ++it) {
            json tmpMsg;
            tmpMsg["srcId"] = std::to_string(it->first);
            tmpMsg["msg"] = it->second;
            toRetMsg.push_back(tmpMsg);
        }
        retJson["content"]["msgs"] = toRetMsg;
        retJson["errNo"] = std::string("0");
    }
    std::string retStr = retJson.dump();
    res.set_content(retStr.c_str(), retStr.size(), "application/json");
}

void setMsg(json& reqBody, httplib::Response& res) {
    json retJson;
    IndvMsgMgr::getInstance().setMsg(
            atoll(reqBody["content"]["srcId"].get<std::string>().c_str()), 
            atoll(reqBody["content"]["destId"].get<std::string>().c_str()), 
            reqBody["content"]["msg"].get<std::string>() 
            );

    retJson["number"] = reqBody["number"];
    retJson["errNo"] = std::string("0");
    retJson["content"]["text"] = std::string("success");
    std::string retStr = retJson.dump();
    res.set_content(retStr.c_str(), retStr.size(), "application/json");
}

void createUser(json& reqBody, httplib::Response& res) {
    json retJson;
    bool isSuccess = IndvMsgMgr::getInstance().addUser(atoll(reqBody["content"]["userId"].get<std::string>().c_str()));
    retJson["number"] = reqBody["number"];
    if(isSuccess) {
        retJson["errNo"] = std::string("0");
        retJson["content"]["text"] = std::string("success");
    }else {
        retJson["errNo"] = std::string("1");
        retJson["content"]["text"] = std::string("user is exist!");
    }
    std::string retStr = retJson.dump();
    res.set_content(retStr.c_str(), retStr.size(), "application/json");
}

void deleteUser(json& reqBody, httplib::Response& res) {
    json retJson;
    bool isSuccess = IndvMsgMgr::getInstance().removeUser(atoll(reqBody["content"]["userId"].get<std::string>().c_str()));
    retJson["number"] = reqBody["number"];
    if(isSuccess) {
        retJson["errNo"] = std::string("0");
        retJson["content"]["text"] = std::string("success");
    }else {
        retJson["errNo"] = std::string("1");
        retJson["content"]["text"] = std::string("user is not exist!");
    }
    std::string retStr = retJson.dump();
    res.set_content(retStr.c_str(), retStr.size(), "application/json");
}

void sigHandler(int sig) {
    if(sig == SIGINT) {
        getServer().stop();
    }
}

int main() {
    httplib::Server& svr = getServer();

    signal(SIGINT, sigHandler);

    svr.set_keep_alive_max_count(1000);
    svr.Post("/im", 
        [] (const httplib::Request& req, httplib::Response& res) {
            json reqBody = json::parse(req.body);
            if(reqBody["method"].get<std::string>() == std::string("get")) {
                getMsg(reqBody, res);
            }else if(reqBody["method"].get<std::string>() == std::string("set")) {
                setMsg(reqBody, res);
            }else if(reqBody["method"].get<std::string>() == std::string("createuser")) {
                createUser(reqBody, res);
            }else if(reqBody["method"].get<std::string>() == std::string("deleteuser")) {
                deleteUser(reqBody, res);
            }else {
                json errRet;
                errRet["errNo"] = std::string("-1");
                errRet["number"] = reqBody["number"];
                errRet["content"]["text"] = std::string("method not found!");
                std::string retStr = errRet.dump();
                res.set_content(retStr.c_str(), retStr.size(), "application/json");
            }

        });
    std::cout<<"start to listen"<<std::endl;
    svr.listen("localhost", 40000);
    
    std::cout<<"end"<<std::endl;

    return 0;
}
