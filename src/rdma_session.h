/* ***********************************************
MYID	: Chen Fan
LANG	: G++
PROG	: RDMA_SESSION_H
************************************************ */

#ifndef RDMA_SESSION_H
#define RDMA_SESSION_H

#include "rdma_util.h"

class RDMA_Endpoint;
class RDMA_Pre;

class RDMA_Session
{
public:

    friend class RDMA_Endpoint;

    RDMA_Session(char* dev_name = NULL);
    ~RDMA_Session();

    RDMA_Endpoint* add_connection(RDMA_Pre* pre);
    void stop_process();

    const static int CQ_SIZE = 1000;

private:
    int open_ib_device();
    void session_processCQ();

    char* dev_name_;
    // device handle
    ibv_context* context_;
    // ibverbs protection domain
    ibv_pd* pd_;
    // Completion event endpoint, to wait for work completions
    ibv_comp_channel* event_channel_;
    // Completion queue, to poll on work completions
    ibv_cq* cq_;

    std::vector<RDMA_Endpoint*> endpoint_table_;
    //std::thread* thread_;
    std::unique_ptr<std::thread> thread_;
};

#endif // !RDMA_SESSION_H