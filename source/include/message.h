#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>

namespace msgDispatcher {

    class Uncopyable {
    protected:
        Uncopyable() {}
        ~Uncopyable() {}
    private:
        Uncopyable(const Uncopyable&);
        Uncopyable& operator=(const Uncopyable&);
    };

    class Message {
    public:
        std::shared_ptr<const std::string> jsonStrP;
        Message(const std::string& json_);
    };

    class User {
    private:
        typedef unsigned long long ull;
        ull userId;
        std::vector<Message> unreadMsgs;
    public:
        User(unsigned long long userId_);
        User(const std::string& json_);
        std::string toJson();
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
        Group(const std::string& json_);
        std::string toJson();

        int addGroupMember(unsigned long long userId);
        int removeGroupMember(unsigned long long userId);
        void addMessage(const Message& toAddMsg);
        std::vector<Message> getMessage(unsigned long long userId);
    };

    class IndivMsgMgr : public Uncopyable {
    private:
        IndivMsgMgr();
        ~IndivMsgMgr();
        typedef unsigned long long ull;
        std::unordered_map<ull, User> userDic;

    public:
        static IndivMsgMgr& getInstance();

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

    class GroupMsgMgr : public Uncopyable {
    private:
        GroupMsgMgr();
        ~GroupMsgMgr();
        typedef unsigned long long ull;
        std::unordered_map<ull, Group> groupDic;

    };

    std::string getMessage(unsigned long long userId) {

    }

}

#endif
