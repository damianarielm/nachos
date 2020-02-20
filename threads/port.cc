#include "synch.hh"
#include "system.hh"

Port::Port(const char* debugName) {
    name = debugName;
    messages = new List<int>;
    lock = new Lock(name);
    senders = new Condition(name, lock);
    receivers = new Condition(name, lock);
}

Port::~Port() {
    delete lock;
    delete senders;
    delete receivers;
}

const char*
Port::GetName() const {
    return name;
}

void
Port::Send(int message) {
    DEBUG('p', "Thread %s " BOLD GREEN "sending", currentThread->GetName());
    DEBUG_CONT('p', " %d through port %s.\n", message, name);

    lock->Acquire();
        messages->Append(message);
        receivers->Signal();
        int m = messages->Head();
        while (!messages->IsEmpty() && m == messages->Head()) senders->Wait();
    lock->Release();
}

void
Port::Receive(int* message) {
    lock->Acquire();
        while (messages->IsEmpty()) receivers->Wait();
        *message = messages->Pop();
        senders->Signal();
    lock->Release();

    DEBUG('p', "Thread %s " BOLD RED "received", currentThread->GetName());
    DEBUG_CONT('p', " %d through port %s.\n", *message, name);
}
