/* ***********************************************
MYID	: Chen Fan
LANG	: G++
PROG	: RDMA_MESSAGE_CPP
************************************************ */

#include "rdma_message.h"

std::string get_message(Message_type msgt)
{
    switch(msgt)
    {
        case RDMA_MESSAGE_ACK:
            return "RDMA_MESSAGE_ACK";
            break;
        case RDMA_MESSAGE_BUFFER_UNLOCK:
            return "RDMA_MESSAGE_BUFFER_UNLOCK";
            break;
        case RDMA_MESSAGE_READ_REQUEST:
            return "RDMA_MESSAGE_READ_REQUEST";
            break;
        case RDMA_MESSAGE_CLOSE:
            return "RDMA_MESSAGE_CLOSE";
            break;
        case RDMA_MESSAGE_TERMINATE:
            return "RDMA_MESSAGE_TERMINATE";
            break;
        default:
            return "UNKNOWN MESSAGE";
    }
}

RDMA_Message::RDMA_Message(ibv_pd* pd, ibv_qp* qp)
    : qp_(qp), pd_(pd)
{
    // Create Message Buffer ......
    incoming_ = new RDMA_Buffer(pd_, kMessageTotalBytes);
    outgoing_ = new RDMA_Buffer(pd_, kMessageTotalBytes);

    log_info("RDMA_Message Created");
}

RDMA_Message::~RDMA_Message()
{
    delete incoming_;
    delete outgoing_;

    log_info("RDMA_Message Deleted");
}

void RDMA_Message::write(uint32_t imm_data, size_t size)
{
    struct ibv_sge list;
    list.addr = (uint64_t) outgoing_->buffer_; // Message
    list.length = size; // Message size
    list.lkey = outgoing_->mr_->lkey;

    struct ibv_send_wr wr;
    memset(&wr, 0, sizeof(wr));
    wr.wr_id = (uint64_t) this; // Which RDMA_Message send this message
    wr.sg_list = &list;
    wr.num_sge = 1;
    wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.imm_data = imm_data;
    wr.wr.rdma.remote_addr = (uint64_t) remote_mr_.remote_addr;
    wr.wr.rdma.rkey = remote_mr_.rkey;

    struct ibv_send_wr *bad_wr;
    if (ibv_post_send(qp_, &wr, &bad_wr))
    {
        log_error("Failed to send message: ibv_post_send error");
    } else
    {
        log_info(make_string("Message post: %s", get_message((Message_type)imm_data).data()));
    }
    
}

void RDMA_Message::send(Message_type msgt, Remote_info* msg)
{
    switch(msgt)
    {
        case RDMA_MESSAGE_ACK:
        case RDMA_MESSAGE_BUFFER_UNLOCK:
        case RDMA_MESSAGE_CLOSE:
        case RDMA_MESSAGE_TERMINATE:
        {
            write(msgt, 0);
            break;
        }
        case RDMA_MESSAGE_READ_REQUEST:
        {
            outgoing_->status_ = LOCK;
            char* target = (char*)outgoing_->buffer_;

            char a[] = "helloworld";
            RDMA_Buffer* test_new = new RDMA_Buffer(pd_, sizeof(a));
            memcpy(test_new->buffer_, &a, sizeof(a));
            memcpy(&target[kBufferSizeStartIndex], &(test_new->size_), sizeof(test_new->size_));
            memcpy(&target[kRemoteAddrStartIndex], &(test_new->buffer_), sizeof(test_new->buffer_));
            memcpy(&target[kRkeyStartIndex], &(test_new->mr_->rkey), sizeof(test_new->mr_->rkey));
            
            write(msgt, kMessageTotalBytes);
            
            break;
        }
        default:
        {

        }
    }
}