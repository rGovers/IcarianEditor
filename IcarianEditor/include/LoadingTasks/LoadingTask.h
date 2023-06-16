#pragma once

class LoadingTask
{
private:

protected:

public:
    LoadingTask() { }
    virtual ~LoadingTask() { }

    virtual void Run() = 0;    
};
