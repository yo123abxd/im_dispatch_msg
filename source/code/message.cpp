#include "../include/message.h"
#include "../thirdparty/json/json.hpp"

using json = nlohmann::json;

Message::Message(const std::string& json_) : jsonStrP(new std::string(json_)) {

}

User::User(unsigned long long userId_) : userId(userId_), unreadMsgs() {
    
}

User::User(const std::string& json_) {
    json j = json::parse(json_);
    userId = j["userId"].get<ull>();
    for(auto it = j["unreadMsgs"].begin(), endIt = j["unreadMsgs"].end(); it != endIt; it++) {
        std::string storedStr = (*it)["msg"].get<std::string>();
        unreadMsgs.push_back(Message(storedStr));
    }
}

std::string User::toJson() {
    json j;
    j["userId"] = userId;
    for(auto it = unreadMsgs.begin(); it != unreadMsgs.end(); it++) {
        json msgJson;
        msgJson["msg"] = (*(*it).jsonStrP);
        j["unreadMsgs"].push_back(msgJson);
    }
    return j.dump();
}

std::vector<Message>& User::getMessages() {
    return unreadMsgs;
}

void User::addMsg(const Message& toAddMsg) {
    unreadMsgs.push_back(toAddMsg);
}

Group::Group(unsigned long long groupId_, const std::vector<unsigned long long>& memberId) {

}

