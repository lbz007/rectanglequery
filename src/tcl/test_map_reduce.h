#include <tcl.h>

#include "util/map_reduce.h"

namespace open_edi {
namespace tcl {
using MTMRApp = open_edi::util::MTMRApp;
using MTTask = open_edi::util::MTTask;
using MTAppInput = open_edi::util::MTAppInput;

int mapReduceTestingCommand(ClientData cld, Tcl_Interp* itp, int argc,
                            const char* argv[]);

class MRtestInput : public MTAppInput {
  public:
    MRtestInput(int data_num) : data_num_(data_num) {
        printf("data_num:%d \n", data_num);
    }
    int getDataNum() { return data_num_; }

  private:
    int data_num_;
};

class MRtestTask : public MTTask {
  public:
    MRtestTask(int number) : number_(number) {}
    int getNum() { return number_; }

  private:
    int number_;
};

class MRTest : public MTMRApp {
  public:
    MRTest() { test_Input_ = nullptr; }
    MRTest(int data_num) {
        MRtestInput* input = new MRtestInput(data_num);
        test_Input_ = input;
    };

    void setInput(MRtestInput* test_Input) { test_Input_ = test_Input; }
    ~MRTest() { delete test_Input_; };
    virtual void preRun(){};
    virtual void postRun(){};

  private:
    virtual void* runMapper();
    virtual void* runWorker();
    virtual void* runReducer(){return nullptr;};
    MRtestInput* test_Input_;
};

}  // namespace tcl
}  // namespace open_edi
