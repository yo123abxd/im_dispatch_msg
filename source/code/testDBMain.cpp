#include <iostream>
#include "../include/message.h"
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;
using namespace im;


//TODO 测试db类
typedef unsigned long long ull;

int main() {
    DBMgr dbmgr;
    dbmgr.init("localhost", 33060, "root", "HUSTse15!");


    unordered_map<unsigned long long, shared_ptr<vector<Message>>> dic;

    for(int i = 0; i < 1000; i++) {
        int optType = abs(rand()) % 100;
        ull destId = abs(rand()) % 100;
        ull srcId = abs(rand()) % 100;

        if(optType >= 90) {
            //getUser
            //cout<<"getUser"<<endl;
            auto pUser = dbmgr.getUser(destId);
            if((pUser == nullptr) ^ (dic[destId] == nullptr)) {
                cout<<"diff delete"<<endl;
                dbmgr.close();
                return 0;
            }
            if(pUser != nullptr && dic[destId] != nullptr) {
                auto msgVec = pUser->getAndCleanMsg();
                if(msgVec->size() != dic[destId]->size()) {
                    cout<<"diff size"<<endl;
                    cout<<msgVec->size()<<"---"<<dic[destId]->size()<<endl;
                    dbmgr.close();
                    return 0;
                }
            }
            
        }else if(optType >= 80) {
            //insertUser
            //cout<<"insertUser"<<endl;
            dbmgr.addToProcList(std::shared_ptr<Alter>(new InsUserAlter(destId)));

            if(dic[destId] == nullptr) {
                dic[destId] = make_shared<vector<Message>>();
            }
        }else if(optType >= 70) {
            //deleteUser
            //cout<<"deleteUser"<<endl;
            dbmgr.addToProcList(std::shared_ptr<Alter>(new RmUserAlter(destId)));


            dic[destId] = nullptr;

        }else if(optType >= 30) {
            //insertMsg
            //cout<<"insertMsg"<<endl;
            string sTmp(to_string(i));
            Message toAdd(srcId, sTmp);
            dbmgr.addToProcList(std::shared_ptr<Alter>(new InsMsgAlter(destId, toAdd)));

            if(dic[destId] != nullptr) {
                dic[destId]->push_back(toAdd);
            }

        }else {
            //popAll
            //cout<<"popAll"<<endl;
            dbmgr.addToProcList(std::shared_ptr<Alter>(new PopAllMsgAlter(destId)));

            if(dic[destId] != nullptr) {
                dic[destId] = make_shared<vector<Message>>();
            }
        }
        if(i % 100 == 0) {
            cout<<i<<endl;
        }
    }
    sleep(2);
    for(auto it = dic.begin(); it != dic.end(); ++it) {
        auto pUser = dbmgr.getUser(it->first);
        if(it->second == nullptr) {
            if(pUser == nullptr) {
                continue;
            }else {
                cout<<"ERROR123"<<endl;
                continue;
            }
        }
        if(it->second->size() != pUser->getAndCleanMsg()->size()) {
            cout<<"ERROR123"<<endl;
        }
    }



    dbmgr.close();
    return 0;
}
