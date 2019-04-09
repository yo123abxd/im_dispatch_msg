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

    std::shared_ptr<std::vector<std::pair<ull, std::string>>> IndvMsgMgr::getMessage(ull userId) {
        auto uIt = userDic.find(userId);
        if(uIt == userDic.end()) {
            return nullptr;
        }
        std::vector<Message>& msg = uIt->second.getMessages();
        size_t msgSize = msg.size();
        std::shared_ptr<std::vector<std::pair<ull, std::string>>> retP = 
            make_shared<std::vector<std::pair<ull, std::string>>>(msg.size());
        for(int i = 0; i < msgSize; i++) {
            (*retP)[i].first = msg[i].srcId;
            (*retP)[i].second = *(msg[i].jsonStrP);
        }
        return retP;
    }

    int setMsg(unsigned long long srcId, unsigned long long destId, const std::string& msg) {
        // 是否在dic中，没有则。。。有则。。。
        return 1;
    }




}
//    Group::Group(unsigned long long groupId_, const std::vector<unsigned long long>& memberId) : groupId(groupId_){}
