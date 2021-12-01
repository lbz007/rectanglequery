#include "test_map_reduce.h"

#include "util/monitor.h"
namespace open_edi {
namespace tcl {
void *MRTest::runMapper() {
    MRtestInput *input = static_cast<MRtestInput *>(test_Input_);
    printf("maper ");
    for (int i = 0; i < input->getDataNum(); ++i) {
        MRtestTask *task = new MRtestTask(i);
        task_queue_.push(task);
    }
    return nullptr;
}

void *MRTest::runWorker() {
    while (true) {
        MRtestTask *task = static_cast<MRtestTask *>(task_queue_.pop());
        if (!task) {
            break;
        }
        // open_edi::util::Monitor monitor;
       // printf("worker");

        for (int i = 0; i <= 2; i++) {
            int n = (10 + task->getNum() % 2) * 200;
            for (int k = 10; k < n; k++) {
                // printf("i:%d task: %d n",i,task->getNum());
                int j = 0;
                j = j + 5;
                j = j * 2;
            }
        }
        // monitor.printInternal("task");

        delete task;
    }
    return nullptr;
}

int mapReduceTestingCommand(ClientData cld, Tcl_Interp *itp, int argc,
                            const char *argv[]) {
    if (argc != 3) return 0;
    int thread_num = atoi(argv[1]);
    int data_num = atoi(argv[2]);
    MRtestInput *test_Input = new MRtestInput(data_num);
    MRTest test;
    test.setInput(test_Input);
    open_edi::util::Monitor monitor;
    test.run(3, thread_num, 1);
    monitor.printInternal("map_reduce ");
    delete test_Input;

    return 0;
}

}  // namespace tcl
}  // namespace open_edi