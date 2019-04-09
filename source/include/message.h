#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>

namespace imfunction {

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
        Message(unsigned long long srcId_, const std::string& json_);
        unsigned long long srcId;
        std::shared_ptr<const std::string> jsonStrP;
    };

    class IndvMsgCtnr {
    private:
        typedef unsigned long long ull;
        ull userId;
        std::vector<Message> unreadMsgs;
    public:
        IndvMsgCtnr(unsigned long long userId_);
        IndvMsgCtnr(const std::string& json_);
        std::string toJson();
        std::vector<Message>& getMessages();
        void addMsg(const Message& toAddMsg);
    };

    class IndvMsgMgr : public Uncopyable {
    private:
        IndvMsgMgr();
        ~IndvMsgMgr();
        typedef unsigned long long ull;
        std::unordered_map<ull, IndvMsgCtnr> userDic;

    public:
        static IndvMsgMgr& getInstance();

        int addUser(unsigned long long userId);
        int removeUser(unsigned long long userId);
        std::shared_ptr<std::string> getMessage(unsigned long long userId);
        int setMsg(unsigned long long srcId, unsigned long long destId, const std::string& msg);
    };












    class GroupMsgCtnr {
    private:
        typedef unsigned long long ull;
        ull groupId;
        std::list<std::pair<int, Message>> unreadMsgList;
        std::map<ull, std::list<std::pair<int, Message>>::iterator> bookMark;
    public:
        GroupMsgCtnr(unsigned long long groupId_, const std::vector<unsigned long long>& memberId);
        GroupMsgCtnr(unsigned long long groupId_);
        GroupMsgCtnr(const std::string& json_);
        std::string toJson();

        int addGroupMember(unsigned long long userId);
        int removeGroupMember(unsigned long long userId);
        void addMessage(const Message& toAddMsg);
        std::vector<Message> getMessage(unsigned long long userId);
    };


    class GroupMgr : public Uncopyable {
    private:
        GroupMgr();
        ~GroupMgr();
        typedef unsigned long long ull;
        std::unordered_map<ull, GroupMsgCtnr> groupDic;
    public:
        void setGroupMsg(unsigned long long srcUserId, unsigned long long destGroupId, const std::string& msg);
        int createGroup(unsigned long long groupId);
        int removeGroup(unsigned long long groupId);
        int addUserToGroup(unsigned long long groupId, unsigned long long userId);
        int removeUserFromGroup(unsigned long long groupId, unsigned long long userId);
        std::vector<unsigned long long> getGroupUserId(unsigned long long groupId);

        //修改group属性
        //int modifyGroupName(std::string);

    };

    std::string getMessage(unsigned long long userId) {

    }

}

#endif
