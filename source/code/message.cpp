#include "../include/message.h"
#include "../thirdparty/json/json.hpp"


namespace imfunction {
    using json = nlohmann::json;
    Message::Message(unsigned long long srcId_, const std::string& json_) : 
        srcId(srcId_), jsonStrP(new std::string(json_)) {}

    IndvMsgCtnr::IndvMsgCtnr(unsigned long long userId_) : 
        userId(userId_), unreadMsgs() {}

    IndvMsgCtnr::IndvMsgCtnr(const std::string& json_) {
        json j = json::parse(json_);
        userId = j["userId"].get<ull>();
        for(auto it = j["unreadMsgs"].begin(), endIt = j["unreadMsgs"].end(); it != endIt; it++) {
            std::string storedStr = (*it)["msg"].get<std::string>();
            unreadMsgs.push_back(Message((*it)["srcId"].get<ull>(), storedStr));
        }
    }

    std::string IndvMsgCtnr::toJson() {
        json j;
        j["userId"] = userId;
        for(auto it = unreadMsgs.begin(); it != unreadMsgs.end(); it++) {
            json msgJson;
            msgJson["msg"] = (*(*it).jsonStrP);
            msgJson["srcId"] = it->srcId;
            j["unreadMsgs"].push_back(msgJson);
        }
        return j.dump();
    }

    std::vector<Message>& IndvMsgCtnr::getMessages() {
        return unreadMsgs;
    }

    void IndvMsgCtnr::addMsg(const Message& toAddMsg) {
        unreadMsgs.push_back(toAddMsg);
    }

    IndvMsgMgr& IndvMsgMgr::getInstance() {
        static IndvMsgMgr mgr;
        return mgr;
    }

    int IndvMsgMgr::addUser(ull userId) {
        return userDic.insert(std::make_pair(userId, IndvMsgCtnr(userId))).second;
    }

    int IndvMsgMgr::removeUser(ull userId) {
        auto it = userDic.find(userId);
        if(it == userDic.end()) {
            return 0;
        }
        userDic.erase(it);
        return 1;
    }

    std::shared_ptr<std::string> IndvMsgMgr::getMessage(ull userId) {
        auto uIt = userDic.find(userId);
        if(uIt == userDic.end()) {
            return std::make_shared<std::string>(nullptr);
        }
        json j = json::array();
        std::vector<Message>& msg = uIt->second.getMessages();
        for(auto it = msg.begin(); it != msg.end(); it++) {
            json tempJs;
            tempJs["srcId"] = it->srcId;
            tempJs["content"] = json::parse(*(it->jsonStrP));
            j.push_back(tempJs);
        }
        return std::make_shared<std::string>(j.dump());
    }

    void setMsg() {
    }




}
//    Group::Group(unsigned long long groupId_, const std::vector<unsigned long long>& memberId) : groupId(groupId_){}
