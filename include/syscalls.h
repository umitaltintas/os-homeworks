#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;

// Add these definitions to syscalls.h
#define SYSCALL_FORK 1
#define SYSCALL_EXECVE 2
#define SYSCALL_WAITPID 3

namespace myos
{

    class SyscallHandler : public InterruptHandler
    {
    public:
        SyscallHandler(InterruptManager *interruptManager, uint8_t InterruptNumber, TaskManager *taskManager);
        ~SyscallHandler();
        virtual uint32_t HandleInterrupt(uint32_t esp) override;

    private:
        TaskManager *taskManager;
    };

}

#endif