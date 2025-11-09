#include <core/eos.h>
#define STACK_SIZE 8096

int32u_t stack0[STACK_SIZE];
int32u_t stack1[STACK_SIZE];
int32u_t stack2[STACK_SIZE];
eos_tcb_t tcb0;
eos_tcb_t tcb1;
eos_tcb_t tcb2;

void task0()
{
    while (1)
    {
        PRINT("C\n");
        eos_sleep(0);
    } // ‘A 출력 후 다음 주기까지 기다림
}

void task1()
{
    while (1)
    {
        PRINT("D\n");
        eos_sleep(0);
    } // ‘B’ 출력 후 다음 주기까지 기다림
}

void task2()
{
    while (1)
    {
        PRINT("E\n");
        eos_sleep(0);
    } // ‘C’ 출력 후 다음 주기까지 기다림
}

void eos_user_main3()
{
    eos_create_task(&tcb0, stack0, STACK_SIZE, task0, NULL, 1);
    eos_set_period(&tcb0, 2);
    // // 태스크 0 생성
    eos_create_task(&tcb1, stack1, STACK_SIZE, task1, NULL, 10);
    eos_set_period(&tcb1, 4); // 태스크 1 생성

    eos_create_task(&tcb2, stack2, STACK_SIZE, task2, NULL, 50);
    eos_set_period(&tcb2, 8);
}