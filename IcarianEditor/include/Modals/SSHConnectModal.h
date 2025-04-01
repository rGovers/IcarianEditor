// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "Modals/Modal.h"

class AppMain;
class ProcessManager;

class SSHConnectModal : public Modal
{
private:
    // DO NOT GET THE PASSWORD FROM THE USER LET SSH PROMPT FOR IT WE DO NOT WANT TO HANDLE THAT
    // IF THE CLIENT HAS A MECHANISM TO HANDLE IT IT CAN PROBABLY DO IT MORE SECURELY OTHERWISE GET USERS TO USE A DIFFERENT CLIENT
    // I KNOW ALOT DESPITE BEING CONSOLE APPLICATIONS DO HAVE ENOUGH TO POP UP A GUI TO ENTER THE PASSWORD AUTOMATICALLY
    AppMain*        m_app;
    ProcessManager* m_processManager;

    char            m_addr[256];
    char            m_user[256];
    int             m_port;
    int             m_clientPort;
    bool            m_compress;

protected:

public:
    SSHConnectModal(AppMain* a_app, ProcessManager* a_processManager);
    virtual ~SSHConnectModal();

    virtual bool Update();
};

// MIT License
// 
// Copyright (c) 2025 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.