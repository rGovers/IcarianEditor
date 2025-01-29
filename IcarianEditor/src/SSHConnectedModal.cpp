// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/SSHConnectedModal.h"

#include <imgui.h>

#include "SSHPipe.h"

SSHConnectedModal::SSHConnectedModal(SSHPipe* a_pipe) : Modal("Syncing SSH")
{
    m_pipe = a_pipe;
}
SSHConnectedModal::~SSHConnectedModal()
{
    
}

bool SSHConnectedModal::Update()
{
    if (!m_pipe->IsAlive())
    {
        return false;
    }

    ImGui::Text("Syncing SSH Files");

    return true;
}

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