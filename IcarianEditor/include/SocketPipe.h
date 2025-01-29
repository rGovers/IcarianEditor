// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <string_view>

#include "Core/CommunicationPipe.h"
#include "RemotePipeMessage.h"

class SocketPipe : public IcarianCore::CommunicationPipe
{
private:
    constexpr static RemotePipeMessage MalformedMessage =
    {
        .Type = RemoteMessageType_Malformed
    };

    constexpr static RemotePipeMessage PongMessage =
    {
        .Type = RemoteMessageType_Pong
    };

    RemotePipeMessage m_lastPipeMessage;
    uint32_t          m_retries;

#ifndef WIN32
    int               m_socket;
#endif

    SocketPipe();

protected:

public:
    virtual ~SocketPipe();

    bool IsAlive() const;
    
    // void SendRemote(const RemotePipeMessage& a_msg);
    // bool ReceiveRemote(RemotePipeMessage* a_msg, uint32_t a_timeout = 0);

    static SocketPipe* Connect(const std::string_view& a_addr, uint16_t a_port, const std::string_view& a_passcode);
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