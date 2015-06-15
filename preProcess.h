#include "global.h"
#include "logUtil.h"
#include "dbUtil.h"
#include "strUtil.h"
#include "parseUtil.h"
using namespace std;

class PreProcess
{
private:
    static PreProcess *_pInstance;
    int totalStatus;
public:
    PreProcess();
    static PreProcess *GetInstance();
    bool Init();
    bool ProcessStatus();
    bool CleanCont(vector<Status> &statusList);
    bool Test();
    ~PreProcess();
};
