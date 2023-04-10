
#include <multitasking.h>

using namespace myos;
using namespace myos::common;

void myos::Task::Yield()
{
    asm volatile("int $0x20");
}
int myos::fork()
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(1));
    return result;
}

Task::Task(GlobalDescriptorTable *gdt, void entrypoint())
{
    cpustate = (CPUState *)(stack + 4096 - sizeof(CPUState));

    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;

    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;

    /*
    cpustate -> gs = 0;
    cpustate -> fs = 0;
    cpustate -> es = 0;
    cpustate -> ds = 0;
    */

    // cpustate -> error = 0;

    // cpustate -> esp = ;
    cpustate->eip = (uint32_t)entrypoint;
    cpustate->cs = gdt->CodeSegmentSelector();
    // cpustate -> ss = ;
    cpustate->eflags = 0x202;
}

Task::~Task()
{
}

Task *myos::Task::Fork()
{
    Task *newTask = (Task *)this;
    newTask->pid = 0;
    return newTask;
}

TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = -1;
}

TaskManager::~TaskManager()
{
}

bool TaskManager::AddTask(Task *task)
{
    if (numTasks >= 256)
        return false;
    task->pid = numTasks;
    tasks[numTasks++] = task;
    return true;
}
Task *TaskManager::GetCurrentTask()
{
    if (currentTask >= 0)
    {
        return tasks[currentTask];
    }
    else
    {
        return nullptr;
    }
}

Task *myos::TaskManager::GetTask(pid_t pid)
{
    for (int i = 0; i < numTasks; i++)
    {
        if (tasks[i]->pid == pid)
        {
            return tasks[i];
        }
    }
    return nullptr;
}

CPUState *TaskManager::Schedule(CPUState *cpustate)
{
    if (numTasks <= 0)
        return cpustate;

    if (currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;

    if (++currentTask >= numTasks)
        currentTask %= numTasks;
    return tasks[currentTask]->cpustate;
}
