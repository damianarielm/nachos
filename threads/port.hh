#include "synch.hh"

class Port {
public:

    Port(const char* debugName);

    ~Port();

    const char* GetName() const;

    void Send(int message);

    void Receive(int* message);

private:

    const char* name;

    List<int> *messages;

    Lock* lock;

    Condition *senders;

    Condition *receivers;
};
