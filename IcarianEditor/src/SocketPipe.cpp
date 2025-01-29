// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "SocketPipe.h"

#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Core/CRC.h"
#include "Core/Endian.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "RemotePipeMessage.h"

#include "Logger.h"

uint32_t HeaderChecksum(const RemotePipeMessage& a_msg)
{
    return IcarianCore::CRC32((char*)&a_msg + sizeof(a_msg.HeaderChecksum), RemotePipeMessage::HeaderSize - sizeof(a_msg.HeaderChecksum));
}
constexpr uint32_t DataChecksum(const RemotePipeMessage& a_msg)
{
    if (a_msg.Length > 0 && a_msg.Data != nullptr)
    {
        return IcarianCore::CRC32(a_msg.Data, a_msg.Length);
    }

    return 0;
}

SocketPipe::SocketPipe()
{
    m_lastPipeMessage = { };

    m_retries = 0;
}
SocketPipe::~SocketPipe()
{
#ifndef WIN32
    close(m_socket);
#endif
}

bool SocketPipe::IsAlive() const
{
#ifndef WIN32
    if (m_socket < 0)
    {
        return false;
    }

    int err;
    socklen_t len = sizeof(err);
    if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &err, &len) != 0)
    {
        return false;
    }

    return err == 0;
#endif
}

#ifndef WIN32
static void Flush(int a_socket)
{
    char Buffer[128];
    int read;
    do 
    {
        read = recv(a_socket, Buffer, sizeof(Buffer), 0);
    }
    while (read > 0);
}
#endif

// void SocketPipe::SendRemote(const RemotePipeMessage& a_msg)
// {
// #ifndef WIN32
//     if (m_socket == -1)
//     {
//         return;
//     }

//     if (a_msg.Type != RemoteMessageType_Malformed)
//     {
//         m_lastPipeMessage.Type = a_msg.Type;

//         if (a_msg.Data != nullptr && a_msg.Length > 0)
//         {
//             if (m_lastPipeMessage.Data == nullptr || m_lastPipeMessage.Length < a_msg.Length)
//             {
//                 m_lastPipeMessage.Data = realloc(m_lastPipeMessage.Data, a_msg.Length);
//             }

//             memcpy(m_lastPipeMessage.Data, a_msg.Data, a_msg.Length);
//         }

//         m_lastPipeMessage.Length = a_msg.Length;
//     }

//     RemotePipeMessage msg = a_msg;
//     msg.DataChecksum = DataChecksum(msg);
//     msg.HeaderChecksum = HeaderChecksum(msg);

//     const int sent = send(m_socket, &msg, RemotePipeMessage::HeaderSize, 0);
//     if (sent < 0)
//     {
//         return;
//     }

//     if (msg.Data != nullptr && msg.Length > 0)
//     {
//         uint32_t bytesSent = 0;
//         do 
//         {
//             const int sent = send(m_socket, (char*)msg.Data + bytesSent, msg.Length - bytesSent, 0);
//             if (sent < 0)
//             {
//                 return;
//             }

//             bytesSent += (uint32_t)sent;
//         }
//         while (bytesSent < a_msg.Length);
//     }
// #endif
// }
// bool SocketPipe::ReceiveRemote(RemotePipeMessage* a_msg, uint32_t a_timeout)
// {
//     IERRBLOCK;

// #ifndef WIN32
//     if (m_socket == -1)
//     {
//         return false;
//     }

//     IERRDEFER(
//     {
//         close(m_socket);
//         m_socket = -1;
//     });

//     while (true)
//     {
//         pollfd pollfd = 
//         {
//             .fd = m_socket,
//             .events = POLLIN
//         };

//         IERRCHECKRET(poll(&pollfd, 1, -1) >= 0, false);

//         if (pollfd.revents & POLLIN)
//         {
//             break;
//         }

//         IERRCHECKRET(pollfd.revents & POLLHUP, false);
//         IERRCHECKRET(pollfd.revents & POLLERR, false);
//         IERRCHECKRET(pollfd.revents & POLLNVAL, false);

//         return false;
//     }

//     RemotePipeMessage msg = { };
//     const int bytesReceived = recv(m_socket, &msg, RemotePipeMessage::HeaderSize, 0);
//     if (bytesReceived < 0)
//     {
//         return false;
//     }

//     if (bytesReceived != RemotePipeMessage::HeaderSize)
//     {
//         IERRCHECKRET(++m_retries < 3, false);

//         Flush(m_socket);
//         SendRemote(MalformedMessage);

//         return false;
//     }

//     const uint32_t headerChk = HeaderChecksum(msg);
//     if (headerChk != msg.HeaderChecksum)
//     {
//         IERRCHECKRET(++m_retries < 3, false);

//         Flush(m_socket);
//         SendRemote(MalformedMessage);

//         return false;
//     }

//     switch (msg.Type)
//     {
//     case RemoteMessageType_Malformed:
//     {
//         IERRCHECKRET(m_lastPipeMessage.Type != RemoteMessageType_Malformed, false);
//         IERRCHECKRET(++m_retries > 3, false);

//         SendRemote(m_lastPipeMessage);

//         return false;
//     }
//     case RemoteMessageType_Ping:
//     {
//         SendRemote(PongMessage);

//         return ReceiveRemote(a_msg);
//     }
//     case RemoteMessageType_Pong:
//     {
//         m_retries = 0;

//         return ReceiveRemote(a_msg);
//     }
//     default:
//     {
//         break;
//     }
//     }

//     if (msg.Length > 0)
//     {
//         uint32_t bytesReceived = 0;
//         msg.Data = new char[msg.Length];

//         do
//         {
//             const int dataReceived = recv(m_socket, (char*)msg.Data + bytesReceived, msg.Length - bytesReceived, 0);
//             if (dataReceived < 0)
//             {
//                 delete[] (char*)msg.Data;

//                 IERRCHECKRET(++m_retries < 3, false);

//                 Flush(m_socket);
//                 SendRemote(MalformedMessage);

//                 return false;
//             }

//             bytesReceived += (uint32_t)dataReceived;
//         }
//         while (bytesReceived < msg.Length);

//         const uint32_t dataChecksum = DataChecksum(msg);
//         if (dataChecksum != msg.DataChecksum)
//         {
//             delete[] (char*)msg.Data;

//             IERRCHECKRET(++m_retries < 3, false);            

//             Flush(m_socket);
//             SendRemote(MalformedMessage);

//             return false;
//         }
//     }

//     *a_msg = msg;

//     m_retries = 0;

//     return true;
// #endif

//     return false;
// }

// SocketPipe* SocketPipe::Connect(const std::string_view& a_addr, uint16_t a_port, const std::string_view& a_passcode)
// {
//     IERRBLOCK;

// #ifndef WIN32
//     const int sock = socket(AF_INET, SOCK_STREAM, 0);

//     const hostent* serverAddr = gethostbyname(a_addr.data());
//     sockaddr_in socketAddr = 
//     {
//         .sin_family = AF_INET,
//         .sin_port = htons(a_port),
//     };
//     memmove(&socketAddr.sin_addr, serverAddr->h_addr, serverAddr->h_length);

//     if (connect(sock, (struct sockaddr*)&socketAddr, sizeof(socketAddr)) < 0)
//     {
//         return nullptr;
//     }

//     SocketPipe* pipe = new SocketPipe();
//     pipe->m_socket = sock;
// #endif
//     IERRDEFER(delete pipe);

//     RemoteVersion version = 
//     {
//         .Major = IcarianCore::HToN<uint32_t>(0),
//         .Minor = IcarianCore::HToN<uint64_t>(1),
//         .Patch = IcarianCore::HToN<uint64_t>(0),
//     };

//     const RemotePipeMessage versionMsg = 
//     {
//         .Type = RemoteMessageType_Version,
//         .Length = sizeof(version),
//         .Data = &version
//     };

//     pipe->SendRemote(versionMsg);

//     {
//         RemotePipeMessage msg;
//         IERRCHECKRET(pipe->ReceiveRemote(&msg, 500), nullptr);

//         IDEFER(        
//         if (msg.Data != nullptr)
//         {
//             delete[] (char*)msg.Data;
//             msg.Data = nullptr;
//         });

//         IERRCHECKRET(msg.Type == RemoteMessageType_VersionAck, nullptr);
//     }

//     const uint32_t passcodeLength = (uint32_t)a_passcode.length();
//     RemotePipeMessage passcodeMsg =
//     {
//         .Type = RemoteMessageType_Passcode,
//         .Length = passcodeLength
//     };
//     passcodeMsg.Data = new char[passcodeLength];
//     IDEFER(delete[] (char*)passcodeMsg.Data);
//     memcpy(passcodeMsg.Data, a_passcode.data(), passcodeLength);

//     pipe->SendRemote(passcodeMsg);

//     {
//         RemotePipeMessage msg;
//         IERRCHECKRET(pipe->ReceiveRemote(&msg, 500), nullptr);

//         IDEFER(
//         if (msg.Data != nullptr)
//         {
//             delete[] (char*)msg.Data;
//             msg.Data = nullptr;
//         });

//         IERRCHECKRET(msg.Type == RemoteMessageType_PasscodeAck, nullptr);
//     }

//     return pipe;
// }

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