#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>
#include "../thirdparty/json/json.hpp"


class Message {
public:
    std::shared_ptr<std::string> jsonStrP;
};

class User {
private:
    typedef unsigned long long ull;
    ull userId;
    std::vector<Message> unreadMsgs;
public:
    User(unsigned long long userId_);
    std::vector<Message>& getMessages();
    void addMsg(const Message& toAddMsg);
};

class Group {
private:
    typedef unsigned long long ull;
    ull groupId;
    std::list<std::pair<int, Message>> unreadMsgList;
    std::map<ull, std::list<std::pair<int, Message>>::iterator> bookMark;
public:
    Group(unsigned long long groupId_, const std::vector<unsigned long long>& memberId);
    Group(unsigned long long groupId_);

    int addGroupMember(unsigned long long userId);
    int removeGroupMember(unsigned long long userId);
    void addMessage(const Message& toAddMsg);
    std::vector<Message> getMessage(unsigned long long userId);
};

class MsgManager {
private:
    typedef unsigned long long ull;
    std::unordered_map<ull, User> userDic;
    std::unordered_map<ull, Group> groupDic;

public:
    std::string getMessage(unsigned long long userId);
    void setOneMsg(unsigned long long srcId, unsigned long long destId, const std::string& msg);
    void setGroupMsg(unsigned long long srcUserId, unsigned long long destGroupId, const std::string& msg);
    int addUser(unsigned long long userId);
    int removeUser(unsigned long long userId);
    int createGroup(unsigned long long groupId);
    int removeGroup(unsigned long long groupId);
    int addUserToGroup(unsigned long long groupId, unsigned long long userId);
    int removeUserFromGroup(unsigned long long groupId, unsigned long long userId);
    std::vector<unsigned long long> getGroupUserId(unsigned long long groupId);
};



#endif
