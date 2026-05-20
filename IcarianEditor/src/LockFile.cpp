// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "LockFile.h"

#ifndef WIN32
#include <sys/file.h>
#include <unistd.h>
#endif

#include "IO.h"
#include "Logger.h"

#ifndef WIN32
static int LockFD;
#endif

bool LockFile::Lock()
{
#ifndef WIN32
    const std::filesystem::path tempPath = IO::GetTempPath();
    if (tempPath.empty())
    {
        Logger::Error("Editor failed to find temp path");

        return false;
    }

    const std::filesystem::path lockfilePath = tempPath / "IcarianEditor.lock";
    const std::string lockfilePathStr = lockfilePath.string();

    LockFD = open(lockfilePathStr.c_str(), O_CREAT, 0655);
    if (LockFD < 0)
    {
        Logger::Error("Editor failed to open lockfile");

        return false;
    }

    if (flock(LockFD, LOCK_EX | LOCK_NB) < 0)
    {
        Logger::Error("Multiple editor processes exist closing");

        return false;
    }

    return true;
#endif

    return false;
}
void LockFile::Close()
{
#ifndef WIN32
    close(LockFD);
#endif
}
void LockFile::Unlock()
{
#ifndef WIN32
    flock(LockFD, LOCK_UN);
#endif

    Close();
}

// MIT License
// 
// Copyright (c) 2026 River Govers
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