#include <core/eos.h>
#if 0
#define STACK_SIZE 8096

int32u_t stack0[STACK_SIZE];
int32u_t stack1[STACK_SIZE];
int32u_t stack2[STACK_SIZE];
eos_tcb_t tcb0;
eos_tcb_t tcb1;
eos_tcb_t tcb2;

// // tcb for task1
static void print_numbers(void *arg)
{
    // Function for task0: Print numbers from 1 to 20 repeatedly
    int i = 1;
    while (1)
    {
        PRINT("%d\n", i);
        eos_schedule();
        // Yield CPU to task1
        if (i++ == 20)
        {
            i = 1;
        }
    }
}
static void print_alphabet(void *arg)
{
    // Function for task1: Print alphabet from ‘a’ to ‘z’ repeatedly
    int i = 97;
    while (1)
    {
        PRINT("%c\n", i);
        eos_schedule();
        // Yield CPU to task0
        if (i++ == 122)
        {
            i = 97;
        }
    }
}

void eos_user_main2()
{
    eos_create_task(&tcb0, stack0, STACK_SIZE, print_numbers, NULL, 0);
    eos_create_task(&tcb1, stack1, STACK_SIZE, print_alphabet, NULL, 0);
}
#endif