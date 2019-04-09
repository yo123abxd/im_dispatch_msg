#include "../include/message.h"
#include "../thirdparty/json/json.hpp"

using json = nlohmann::json;

namespace imfunction {
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

    //TODO
    Group::Group(unsigned long long groupId_, const std::vector<unsigned long long>& memberId) : groupId(groupId_){
        

    }
}
