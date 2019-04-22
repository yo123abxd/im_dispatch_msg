#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>
#include <mysql-cppconn/mysqlx/xdevapi.h>
#include <mutex>
#include <thread>

namespace im {

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
        std::shared_ptr<const std::string> pJsonStr;
    };




    class Alter {
    public:
        virtual ~Alter();
        Alter(unsigned long long userId_ = 0);
        virtual int alterType();
        unsigned long long userId;
        static const int type = -1;
    };

    class InsMsgAlter : public Alter {
    public:
        InsMsgAlter(unsigned long long userId_, const Message& msg_);
        virtual ~InsMsgAlter();
        virtual int alterType();

        static const int type = 0;
        Message msg;
    };

    class PopAllMsgAlter : public Alter {
    public:
        virtual ~PopAllMsgAlter();
        PopAllMsgAlter(unsigned long long userId_);
        virtual int alterType();

        static const int type = 1;
    };

    class InsUserAlter : public Alter {
    public:
        virtual ~InsUserAlter();
        InsUserAlter(unsigned long long userId_);
        virtual int alterType();

        static const int type = 2;
    };

    class RmUserAlter : public Alter {
    public:
        virtual ~RmUserAlter();
        RmUserAlter(unsigned long long userId_);
        virtual int alterType();

        static const int type = 3;
    };

    class AlterList {
    public:
        AlterList(std::shared_ptr<Alter> pAlter_ = nullptr, 
                std::shared_ptr<AlterList> next_ = nullptr, 
                std::shared_ptr<AlterList> sameUidNext = nullptr);
        std::shared_ptr<Alter> pAlter;
        std::shared_ptr<AlterList> next;
        std::shared_ptr<AlterList> sameUidNext;
    };


    class AlterQueue {
    private:
        typedef unsigned long long ull;
        std::shared_ptr<AlterList> procListHead;
        std::shared_ptr<AlterList> procListBack;
        //map[i].first : head, second : back
        std::unordered_map<ull, 
            std::pair<std::shared_ptr<AlterList>, std::shared_ptr<AlterList>>
            > indvProcListHeadMap;
        ull size;
        std::mutex mu;
        AlterQueue(const AlterQueue& mgr_);
        AlterQueue& operator=(const AlterQueue& r__);
    public:
        AlterQueue();
        void pushProc(std::shared_ptr<Alter> alter);
        std::shared_ptr<Alter> topProc();
        void popProc();
        std::vector<std::shared_ptr<Alter>> uidProcList(ull userId); 
        bool empty();
        void lock();
        void unlock();
    };

    class User {
    private:
        typedef unsigned long long ull;
        ull userId;
        std::shared_ptr<std::vector<Message>> pUnreadMsgs;
    public:
        User(unsigned long long userId_);
        //返回副本，User内含的unreadMsgs自动清空
        std::shared_ptr<std::vector<Message>> getAndCleanMsg();
        unsigned long long getUId() const;
        void addMsg(const Message& toAddMsg);
    };


    template<class T_>
    class AutoUnlock {
    private:
        T_* pt;
    public:
        AutoUnlock(T_* pt_) : pt(pt_) {
            pt->lock();
        }
        ~AutoUnlock() {
            pt->unlock();
        }
    };

    class DBMgr {
    private:
        typedef unsigned long long ull;
        std::shared_ptr<mysqlx::Session> pDbSess;
        AlterQueue alterQu;
        std::shared_ptr<std::thread> pAlterProcTh;
        std::mutex dbProcMu;
        bool isClosed;
        void alterProc();

    public:
        DBMgr(const DBMgr& mgr_) = delete;
        DBMgr& operator=(const DBMgr& r__) = delete;
        DBMgr();
        int init(std::string host, int port, std::string user, std::string pwd);
        void close();

        int addToProcList(std::shared_ptr<Alter> pAlter);
        std::shared_ptr<User> getUser(unsigned long long userId);
    };



    class LRUctnr {
    private:
        typedef unsigned long long ull;
        std::list<ull> l;
        std::unordered_map<ull, std::list<ull>::iterator> pDic;
        const size_t maxSize;
        std::mutex mu;
    public:
        LRUctnr(size_t maxSize_);
        void visit(ull visNum);
        void remove(ull rmNum);
        std::pair<bool, ull> insert(ull insNum);
        void lock();
        void unlock();
    };




    class IndvMsgMgr : public Uncopyable {
    private:
        IndvMsgMgr();
        ~IndvMsgMgr();
        typedef unsigned long long ull;
        std::unordered_map<ull, User> userMap;
        std::mutex mapMu;
        DBMgr dbMgr;
        LRUctnr lru;
        static const int msgMaxNum = 10000;
    public:
        static IndvMsgMgr& getInstance();

        int addUser(unsigned long long userId);
        int removeUser(unsigned long long userId);
        std::shared_ptr<std::vector<std::pair<unsigned long long, std::string>>> getMessage(unsigned long long userId);
        int setMsg(unsigned long long srcId, unsigned long long destId, const std::string& msg);
    };










/*
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
        std::unordered_map<ull, GroupMsgCtnr> groupMap;
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
*/

}

#endif
