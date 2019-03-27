/*****************************************************************************
    NumeRe: Framework fuer Numerische Rechnungen
    Copyright (C) 2017  Erik Haenel et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "breakpointmanager.hpp"
#include "core/utils/tools.hpp"

BreakpointManager::BreakpointManager()
{
    mBreakpoints.clear();
}

BreakpointManager::BreakpointManager(const BreakpointManager& _messenger) : BreakpointManager()
{
    passBreakpoints(_messenger.mBreakpoints);
}

void BreakpointManager::addBreakpoint(const string& _sFilename, size_t nLine)
{
    if (mBreakpoints.find(replacePathSeparator(_sFilename)) != mBreakpoints.end())
    {
        vector<size_t> vLine = mBreakpoints[replacePathSeparator(_sFilename)];
        for (size_t i = 0; i < vLine.size(); i++)
        {
            if (vLine[i] == nLine)
                return;
        }
        vLine.push_back(nLine);
        mBreakpoints[replacePathSeparator(_sFilename)] = vLine;
    }
    else
    {
        vector<size_t> vLine;
        vLine.push_back(nLine);
        mBreakpoints[replacePathSeparator(_sFilename)] = vLine;
    }
}

void BreakpointManager::removeBreakpoint(const string& _sFilename, size_t nLine)
{
    if (mBreakpoints.find(replacePathSeparator(_sFilename)) == mBreakpoints.end())
    {
        return;
    }
    else
    {
        vector<size_t> vLine = mBreakpoints[replacePathSeparator(_sFilename)];
        for (size_t i = 0; i < vLine.size(); i++)
        {
            if (vLine[i] == nLine)
            {
                vLine.erase(vLine.begin()+i);
            }
        }
        if (!vLine.size())
            mBreakpoints.erase(replacePathSeparator(_sFilename));
        else
            mBreakpoints[replacePathSeparator(_sFilename)] = vLine;
    }
}

void BreakpointManager::clearBreakpoints(const string& _sFilename)
{
    if (mBreakpoints.find(replacePathSeparator(_sFilename)) != mBreakpoints.end())
        mBreakpoints.erase(replacePathSeparator(_sFilename));
}

void BreakpointManager::passBreakpoints(const map<string,vector<size_t> >& _mBreakpoints)
{
    mBreakpoints.clear();
    mBreakpoints = _mBreakpoints;
}


bool BreakpointManager::hasBreakpoints(const string& _sFilename)
{
    if (mBreakpoints.find(replacePathSeparator(_sFilename)) != mBreakpoints.end())
        return true;
    return false;
}

bool BreakpointManager::isBreakpoint(const string& _sFilename, size_t nLine)
{
    if (mBreakpoints.find(replacePathSeparator(_sFilename)) == mBreakpoints.end())
    {
        return false;
    }
    else
    {
        vector<size_t> vLine = mBreakpoints[replacePathSeparator(_sFilename)];
        for (size_t i = 0; i < vLine.size(); i++)
        {
            if (vLine[i] == nLine)
            {
                return true;
            }
        }
    }
    return false;
}

