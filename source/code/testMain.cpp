#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <unordered_map>
#include <map>
#include <queue>
#include "../include/message.h"


using namespace std;

void print(vector<shared_ptr<im::Alter>> vec) {
    for(int i = 0; (size_t)i < vec.size(); i++) {
        cout<<vec[i]->userId<<" ";
    }
    cout<<endl;
}

void printTag(int i) {
    cout<<"Tag "<<i<<endl;
}

typedef unsigned long long ull;

void f() {
    cout<<"error"<<endl;
    exit(0);
}

int main() {
    im::AlterQueue aq;

    queue<ull> qu;
    map<ull, queue<ull>> mqu;

    for(int i = 0; i < 10000; i++) {
        int opt = rand() % 2;
        cout<<"-----------------test:"<<i<<"---"<<opt<<"--------------"<<endl;
        if(opt == 1) {
            ull randUid = rand() % 100;
            //ull randUid = 1;
            auto p = make_shared<im::Alter>(randUid);
            aq.pushProc(p);
            qu.push(randUid);
            mqu[randUid].push(randUid);

            if(qu.front() != aq.topProc()->userId) {
                f();
            }
            vector<ull> tmp;
            while(!mqu[randUid].empty()) {
                tmp.push_back(mqu[randUid].front());
                mqu[randUid].pop();
            }
            auto vecTmpP = aq.uidProcList(randUid);
            for(int i = 0; i < vecTmpP.size(); i++) {
                if(vecTmpP[i]->userId != tmp[i]) {
                    f();
                }
            }
            if(vecTmpP.size() != tmp.size()) {
                f();
            }
            for(int i = 0; i < tmp.size(); i++) {
                mqu[randUid].push(tmp[i]);
            }
        }else {
            if(aq.empty()) {
                if(!qu.empty()) {
                    f();
                }
            }else {
                aq.popProc();
                mqu[qu.front()].pop();
                qu.pop();
            }
        }
        
    }

    return 0;
}
