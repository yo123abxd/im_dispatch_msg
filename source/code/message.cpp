#include "../include/message.h"
#include "../thirdparty/json/json.hpp"
#include <exception>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
//#include <fstream>


namespace im {
    using json = nlohmann::json;
    Message::Message(unsigned long long srcId_, const std::string& json_) : 
        srcId(srcId_), pJsonStr(new std::string(json_)) {}

    Alter::~Alter() {}
    Alter::Alter(unsigned long long userId_) : userId(userId_) {}

    int Alter::alterType() {
        return Alter::type;
    }



    InsMsgAlter::InsMsgAlter(unsigned long long destId_, const Message& msg_) :
        Alter(destId_), msg(msg_){}

    InsMsgAlter::~InsMsgAlter() {}

    int InsMsgAlter::alterType() {
        return InsMsgAlter::type;
    }




    PopAllMsgAlter::PopAllMsgAlter(unsigned long long userId_) :
        Alter(userId_) {}

    PopAllMsgAlter::~PopAllMsgAlter() {}

    int PopAllMsgAlter::alterType() {
        return PopAllMsgAlter::type;
    }




    InsUserAlter::InsUserAlter(unsigned long long userId_) : 
        Alter(userId_) {}

    InsUserAlter::~InsUserAlter() {}

    int InsUserAlter::alterType() {
        return InsUserAlter::type;
    }




    RmUserAlter::RmUserAlter(unsigned long long userId_) : 
        Alter(userId_) {}

    RmUserAlter::~RmUserAlter() {}

    int RmUserAlter::alterType() {
        return RmUserAlter::type;
    }

    AlterList::AlterList(std::shared_ptr<Alter> pAlter_, 
        std::shared_ptr<AlterList> next_,
        std::shared_ptr<AlterList> sameUidNext_) : 
        pAlter(pAlter_), next(next_), sameUidNext(sameUidNext_) {}

    
    AlterQueue::AlterQueue() : 
        procListHead(std::make_shared<AlterList>()), 
        procListBack(nullptr), indvProcListHeadMap(), size(0), mu() {
            procListBack = procListHead;
        }
    
    void AlterQueue::pushProc(std::shared_ptr<Alter> alter) {
        auto toPush = std::make_shared<AlterList>(alter);

        //插入next
        procListBack->next = toPush;
        procListBack = procListBack->next;

        //插入sameUidNext
        auto it = indvProcListHeadMap.find(alter->userId);
        if(it == indvProcListHeadMap.end()) {
            auto uIdHead = std::make_shared<AlterList>();
            it = indvProcListHeadMap.insert(std::make_pair(
                    alter->userId, std::make_pair(uIdHead, uIdHead)
                    )).first;
        }
        it->second.second->sameUidNext = toPush;
        it->second.second = it->second.second->sameUidNext;
        ++size;
    }
    
    std::shared_ptr<Alter> AlterQueue::topProc() {
        return empty() ? std::shared_ptr<Alter>(nullptr) : procListHead->next->pAlter;
    }

    void AlterQueue::popProc() {
        //还剩下0个
        if(procListHead->next == nullptr) {
            return;
        }
        //还剩下一个
        if(procListHead->next == procListBack) {
            indvProcListHeadMap.erase(procListBack->pAlter->userId);
            procListBack = procListHead;
            procListHead->next = nullptr;
            size = 0;
            return;
        }
        
        //还剩下2个以上
        
        auto uMapIt = indvProcListHeadMap.find(procListHead->next->pAlter->userId);
        
        //同类型仅剩下一个
        if(uMapIt->second.first->sameUidNext == uMapIt->second.second) {
            indvProcListHeadMap.erase(uMapIt);
        }else {
        //同类型剩下一个以上
            uMapIt->second.first->sameUidNext = uMapIt->second.first->sameUidNext->sameUidNext;
        }
        
        procListHead->next = procListHead->next->next;

        --size;
    }

    std::vector<std::shared_ptr<Alter>> AlterQueue::uidProcList(ull userId) {
        std::vector<std::shared_ptr<Alter>> ret;
        auto it = indvProcListHeadMap.find(userId);
        if(it == indvProcListHeadMap.end()) {
            return ret;
        }
        auto nextP = it->second.first->sameUidNext;
        while(nextP != nullptr) {
            ret.push_back(nextP->pAlter);
            nextP = nextP->sameUidNext;
        }
        return ret;
    }

    bool AlterQueue::empty() {
        return procListHead->next == nullptr;
    }

    void AlterQueue::lock() {
        mu.lock(); 
    }

    void AlterQueue::unlock() {
        mu.unlock();
    }


    
    User::User(unsigned long long userId_) :
        userId(userId_), pUnreadMsgs(std::make_shared<std::vector<Message>>()) {}

    std::shared_ptr<std::vector<Message>> User::getAndCleanMsg() {
        auto ret = pUnreadMsgs;
        pUnreadMsgs = std::make_shared<std::vector<Message>>();
        return ret;
    }

    unsigned long long  User::getUId() const {
        return userId;
    }

    void User::addMsg(const Message& toAddMsg) {
        pUnreadMsgs->push_back(toAddMsg);
    }



    /*
     * DBMgr
     */

    DBMgr::DBMgr() : pDbSess(nullptr), alterQu(), 
        pAlterProcTh(nullptr), dbProcMu(), isClosed(true) {}



    void DBMgr::alterProc() {
        int sleepCnt = 0;
        const int sleepMaxCnt = 100;
        const int sleepTime = 100;
        while(true) {
            if(sleepCnt >= sleepMaxCnt) {
                sleepCnt = 0;
                usleep(sleepTime);
            }
            AutoUnlock<std::mutex> auUnlk(&dbProcMu);
            AutoUnlock<AlterQueue> auUnlkQu(&alterQu);
            if(isClosed) {
                if(alterQu.empty()) {
                    pDbSess->close();
                    pDbSess = nullptr;
                    return;
                }
            }

            if(alterQu.empty()) {
                ++sleepCnt;
            }else {
                sleepCnt = 0;
                auto pAl = alterQu.topProc();

                try {
                    switch(pAl->alterType()) {
                        case Alter::type:
                            break;
                        case InsMsgAlter::type:
                            if(auto pMsg = dynamic_cast<InsMsgAlter*>(pAl.get())) {
                                mysqlx::Table msgTable = pDbSess->getSchema("d_im")
                                    .getTable("t_message");

                                msgTable.insert("destId", "srcId", "content")
                                    .values(pMsg->userId, pMsg->msg.srcId, *(pMsg->msg.pJsonStr))
                                    .execute();
                            }
                            break;
                        case PopAllMsgAlter::type:
                            pDbSess->sql("delete from t_message where destId = " + 
                                    std::to_string(pAl->userId) + ";").execute();
                            break;
                        case InsUserAlter::type:
                            pDbSess->sql("insert into t_user values(" + 
                                    std::to_string(pAl->userId) + ");").execute();
                            break;
                        case RmUserAlter::type:
                            pDbSess->sql("delete from t_user where userId = " + 
                                    std::to_string(pAl->userId) + ";").execute();
                            break;
                    }
                }catch (mysqlx::Error& e) {
                    std::cout<<e.what()<<std::endl;
                }
                alterQu.popProc();
            }
        }
    }

    //DONE
    int DBMgr::init(std::string host, int port, std::string user, std::string pwd) {
        if(pDbSess != nullptr || pAlterProcTh != nullptr) {
            return 0;
        }
        //Session sess("localhost", 33060, "root", "HUSTse15!");
        pDbSess = std::make_shared<mysqlx::Session>(host, port, user, pwd);


        std::string createSchemaSql(R"(create database if not exists d_im;)");

        std::string useSchemaSql(R"(use d_im;)");

        std::string createTableUserSql(R"(
            CREATE TABLE IF NOT EXISTS `t_user` (
            `userId` BIGINT UNSIGNED PRIMARY KEY
            );
        )");

        std::string createTableMsgSql(R"(
            CREATE TABLE IF NOT EXISTS `t_message` (
            `destId` BIGINT UNSIGNED NOT NULL,
            `srcId` BIGINT UNSIGNED NOT NULL,
            `content` varchar(1024),
            FOREIGN KEY (destId)
                REFERENCES t_user(userId)
                ON DELETE CASCADE
            );
        )");

        pDbSess->sql(createSchemaSql).execute();
        pDbSess->sql(useSchemaSql).execute();
        pDbSess->sql(createTableUserSql).execute();
        pDbSess->sql(createTableMsgSql).execute();

        isClosed = false;
        pAlterProcTh = std::make_shared<std::thread>(&DBMgr::alterProc, this);

        return 1;
    }

    //DONE
    void DBMgr::close() {
        isClosed = true;

        pAlterProcTh->join();
        pAlterProcTh = nullptr;
    }

    //DONE
    int DBMgr::addToProcList(std::shared_ptr<Alter> pAlter) {
        if(isClosed) {
            return 0;
        }

        AutoUnlock<AlterQueue> lockQu(&alterQu);
        alterQu.pushProc(pAlter);
        return 1;
    }

    //忽略isClosed，默认用户调用close以后不会调用getUser
    std::shared_ptr<User> DBMgr::getUser(unsigned long long userId) {
        AutoUnlock<std::mutex> dbLock(&dbProcMu);

        std::shared_ptr<User> user = nullptr;
        pDbSess->sql("use d_im;").execute();
        auto userDocs = pDbSess->sql("select * from t_user where userId = " + 
                std::to_string(userId) + ";").execute();
        //如果user不存在的话就不进行msg插入了
        if(userDocs.begin() != userDocs.end()) {
            user = std::make_shared<User>(userId);
            auto msgDocs = pDbSess->sql("select * from t_message where destId = " + 
                    std::to_string(userId) + ";").execute();
            for(auto it = msgDocs.begin(); it != msgDocs.end(); ++it) {
                Message toAdd((*it)[1].operator unsigned long(), (*it)[2]);
                user->addMsg(toAdd);
            }
        }
        
        AutoUnlock<AlterQueue> alLock(&alterQu);

        auto toProcList = alterQu.uidProcList(userId);
        //处理alterProcList
        for(size_t i = 0; i < toProcList.size(); ++i) {
            //加个user为nullptr时的保护
            if(!user && toProcList[i]->alterType() != InsUserAlter::type) {
                continue;
            }
            switch(toProcList[i]->alterType()) {
                case Alter::type:
                    break;
                case InsMsgAlter::type:
                    if(auto pMsg = dynamic_cast<InsMsgAlter*>(toProcList[i].get())) {
                        user->addMsg(pMsg->msg);
                    }
                    break;
                case PopAllMsgAlter::type:
                    user->getAndCleanMsg();
                    break;
                case InsUserAlter::type:
                    user = (user == nullptr ? std::make_shared<User>(userId) : user);
                    break;
                case RmUserAlter::type:
                    user = nullptr;
                    break;
            }
        }

        return user;
    }
    
    



    LRUctnr::LRUctnr(size_t maxSize_) : maxSize(maxSize_) {}
    
    void LRUctnr::visit(ull visNum) {
        l.erase(pDic[visNum]);
        pDic[visNum] = l.insert(l.begin(), visNum);
    }

    void LRUctnr::remove(unsigned long long rmNum) {
        l.erase(pDic[rmNum]);
        pDic.erase(rmNum);
    }

    std::pair<bool, unsigned long long> LRUctnr::insert(unsigned long long insNum) {
        pDic[insNum] = l.insert(l.begin(), insNum);
        if(l.size() > maxSize) {
            ull rmNum = l.back();
            pDic.erase(rmNum);
            l.pop_back();
            return std::make_pair(true, rmNum);
        }
        return std::make_pair(false, insNum);
    }
    void LRUctnr::lock() {
        mu.lock();
    }
    void LRUctnr::unlock() {
        mu.unlock();
    }
    

    
    


    //IndvMsgMgr 实现
    IndvMsgMgr::IndvMsgMgr() : userMap(), mapMu(), dbMgr(), lru(msgMaxNum) {
        dbMgr.init("localhost", 33060, "root", "HUSTse15!");
    }
    IndvMsgMgr::~IndvMsgMgr() {
        dbMgr.close();
    }

    IndvMsgMgr& IndvMsgMgr::getInstance() {
        static IndvMsgMgr mgr;
        return mgr;
    }


    int IndvMsgMgr::addUser(ull userId) {
        AutoUnlock<std::mutex> mu(&mapMu);
        //先在内存中寻找user
        if(userMap.find(userId) == userMap.end()) {
            auto userInDB = dbMgr.getUser(userId);
            //在数据库中寻找user，若user不存在则插入
            if(userInDB == nullptr) {
                dbMgr.addToProcList(std::shared_ptr<Alter>(new InsUserAlter(userId)));
                //userMap[userId] = User(userId);
                userMap.insert(std::make_pair(userId, User(userId)));

                AutoUnlock<LRUctnr> lruLock(&lru);
                auto toPop = lru.insert(userId);
                if(toPop.first) {
                    userMap.erase(toPop.second);
                }

                return 1;
            }
        }
        return 0;
    }

    int IndvMsgMgr::removeUser(ull userId) {
        AutoUnlock<std::mutex> mu(&mapMu);
        auto it = userMap.find(userId);
        //在内存中寻找user

        if(it == userMap.end()) {
            auto userInDB = dbMgr.getUser(userId);
            if(userInDB == nullptr) {
                return 0;
            }
        }else {
            userMap.erase(it);
            AutoUnlock<LRUctnr> lruLock(&lru);
            lru.remove(userId);
        }
        dbMgr.addToProcList(std::shared_ptr<Alter>(new RmUserAlter(userId)));
        return 1;
    }

    //返回nullptr即不存在该用户
    std::shared_ptr<std::vector<std::pair<unsigned long long, std::string>>> 
    IndvMsgMgr::getMessage(ull userId) {

        AutoUnlock<std::mutex> mu(&mapMu);
        auto it = userMap.find(userId);
        std::shared_ptr<std::vector<Message>> pUserMsgs(nullptr);
        if(it == userMap.end()) {
            auto ptmpUser = dbMgr.getUser(userId);

            //在内存中没有该user，在数据库中也不存在，则返回nullptr
            if(ptmpUser == nullptr) {
                return nullptr;
            }

            //在数据库中找到该User
            pUserMsgs = ptmpUser->getAndCleanMsg();
            
            //从数据库中调入内存中
            //userMap[userId] = *ptmpUser;
            userMap.insert(std::make_pair(userId, *ptmpUser));
            AutoUnlock<LRUctnr> lruLock(&lru);
            auto toPop = lru.insert(userId);
            if(toPop.first) {
                userMap.erase(toPop.second);
            }

        }else {
            pUserMsgs = it->second.getAndCleanMsg();
            AutoUnlock<LRUctnr> lruLock(&lru);
            lru.visit(userId);
        }
        std::shared_ptr<std::vector<std::pair<ull, std::string>>> ret(new std::vector<std::pair<ull, std::string>>());

        if(pUserMsgs == nullptr) {
        }
        for(auto msgIt = pUserMsgs->begin(); msgIt != pUserMsgs->end(); ++msgIt) {
            if(msgIt->pJsonStr == nullptr) {
            }
            ret->push_back(std::make_pair(msgIt->srcId, *(msgIt->pJsonStr)));
        }

        dbMgr.addToProcList(std::shared_ptr<Alter>(new PopAllMsgAlter(userId)));

        return ret;
    }



    int IndvMsgMgr::setMsg(ull srcId, ull destId, const std::string& msg) {
        
        AutoUnlock<std::mutex> mu(&mapMu);
        auto it = userMap.find(destId);
        Message toAdd(srcId, msg);
        if(it == userMap.end()) {

            auto ptmpUser = dbMgr.getUser(destId);

            //在内存中没有该user，在数据库中也不存在，则返回nullptr
            if(ptmpUser == nullptr) {
                return 0;
            }

            //在数据库中找到该User
            ptmpUser->addMsg(toAdd);

            //从数据库中调入内存中
            userMap.insert(std::make_pair(destId, *ptmpUser));
            AutoUnlock<LRUctnr> lruLock(&lru);
            auto toPop = lru.insert(destId);
            if(toPop.first) {
                userMap.erase(toPop.second);
            }

        }else {
            it->second.addMsg(toAdd);
            AutoUnlock<LRUctnr> lruLock(&lru);
            lru.visit(destId);
        }

        dbMgr.addToProcList(std::shared_ptr<Alter>(new InsMsgAlter(destId, toAdd)));
        
        return 1;
    }



}
