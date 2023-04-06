#include <syscalls.h>
#include <string.h>
using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;

SyscallHandler::SyscallHandler(InterruptManager *interruptManager, uint8_t InterruptNumber, TaskManager *taskManager)
    : InterruptHandler(interruptManager, InterruptNumber + interruptManager->HardwareInterruptOffset()), taskManager(taskManager)
{
}

SyscallHandler::~SyscallHandler()
{
}

void printf(char *);

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState *cpu = (CPUState *)esp;

    switch (cpu->eax)
    {
    case SYSCALL_FORK: // fork
    {
        // get current task
        Task *parentTask = taskManager->GetCurrentTask();
        // create new task
        Task *childTask = parentTask->Fork();

        // add new task to task manager
        if (taskManager->AddTask(childTask))
        {
            // return child pid to parent, 0 to child
            cpu->eax = childTask->pid;
            cpu->ebx = 0;
        }
        else
        {
            // return -1 to parent, -1 to child
            cpu->eax = -1;
            cpu->ebx = -1;
        }

        break;
    }
    case SYSCALL_EXECVE: // execve
    {
    }
    case SYSCALL_WAITPID: // waitpid
    {

        // get current task
        Task *parentTask = taskManager->GetCurrentTask();
        // get child task
        Task *childTask = taskManager->GetTask(cpu->ebx);
        // wait for child task to finish
        while (childTask->state != Terminated)
        {
            // switch to next task
            parentTask->Yield();
        }
        // return child pid to parent, 0 to child
        cpu->eax = childTask->pid;
        cpu->ebx = 0;
    }
    // ... other syscalls
    default:
        break;
    }

    return esp;
}
