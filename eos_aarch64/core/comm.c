/********************************************************
 * Filename: core/comm.c
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/30/24
 *
 * Description: Message queue management
 ********************************************************/

#include <core/eos.h>


void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type)
{
    if (mq == NULL || queue_start == NULL || queue_size == 0 || msg_size == 0) {
        PRINT("eos_init_mqueue: invalid args mq=%p buf=%p size=%u msg=%u\n", 
            (void*)mq, queue_start, (int32u_t)queue_size, (int32u_t)msg_size);
        return;
    }
    // To be filled by students: Project 4
    mq->queue_start = queue_start;
    mq->queue_size = queue_size;
    mq->msg_size = msg_size;
    mq->queue_type = queue_type;
    mq->front = 0;
    mq->rear = 0;
    eos_init_semaphore(&mq->putsem, queue_size, queue_type);
    eos_init_semaphore(&mq->getsem, 0, queue_type);
    PRINT("eos_init_mqueue: mq=%p buf=%p size=%u msg=%u type=%u\n", 
        (void*)mq, queue_start, (int32u_t)queue_size, (int32u_t)msg_size, (int32u_t)queue_type);

}


int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) 
{
    PRINT("eos_send_message: mq=%p msg=%p timeout=%d\n", (void*)mq, message, (int32u_t)timeout);
    // To be filled by students: Project 4
    int8u_t lock_flag;

    if (!mq || !message) { /* message queue or message buffer does not exist */
        PRINT("invalid args mq=%p msg=%p\n", (void*)mq, message);
        return 0;
    }
    PRINT("HELLO1\n");

    /* get semaphore for writing to the message queue */
    if (eos_acquire_semaphore(&mq->putsem, timeout) == 0) {
        return 0;
    }

    PRINT("HELLO2\n");

    lock_flag = eos_lock_scheduler();

    /* set pointers */
    int8u_t *dest = (int8u_t *) mq->queue_start + mq->msg_size*mq->rear;
    int8u_t *src = (int8u_t *) message;

    /* wrap rear pointer */
    mq->rear++;
    if (mq->rear == mq->queue_size) {
        mq->rear = 0;
    }

    /* copy message to the message queue */
    for (int8u_t i = 0; i < mq->msg_size; i++) {
        dest[i] = src[i];
    }

    PRINT("HELLO3\n");

    eos_restore_scheduler(lock_flag);

    eos_release_semaphore(&mq->getsem);

    PRINT("Message sent to mq=%p\n", (void*)mq);
    PRINT("front=%u, rear=%u\n", (int32u_t)mq->front, (int32u_t)mq->rear);

    return mq->msg_size;

}


int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout)
{
    // To be filled by students: Project 4
    int8u_t lock_flag;

    if (!mq || !message) { /* message queue or output buffer does not exist */
        PRINT("invalid args mq=%p msg=%p\n", (void*)mq, message);
        return 0;
    }

    /* get semaphore for receiving message from the queue */
    if (eos_acquire_semaphore(&mq->getsem, timeout) == 0) {
        return 0;
    }

    lock_flag = eos_lock_scheduler();

    /* set pointers */
    int8u_t *src = (int8u_t *)mq->queue_start + mq->msg_size*mq->front;
    int8u_t *dest = (int8u_t *)message;

    /* wrap front pointer */
    mq->front++;
    if (mq->front == mq->queue_size) {
        mq->front = 0;
    }

    /* copy message from the message queue */
    for (int8u_t i = 0; i < mq->msg_size; i++) {
        dest[i] = src[i];
    }

    eos_restore_scheduler(lock_flag);

    eos_release_semaphore(&mq->putsem);

    return mq->msg_size;
}
