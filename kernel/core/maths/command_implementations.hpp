/*****************************************************************************
    NumeRe: Framework fuer Numerische Rechnungen
    Copyright (C) 2019  Erik Haenel et al.

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

#ifndef COMMAND_IMPLEMENTATIONS_HPP
#define COMMAND_IMPLEMENTATIONS_HPP

#include "../datamanagement/datafile.hpp"
#include "../ParserLib/muParser.h"
#include "../settings.hpp"
#include "define.hpp"

#include <string>
#include <vector>

using namespace std;
using namespace mu;

vector<double> integrate(const string&, Datafile&, Parser&, const Settings&, Define&);
vector<double> integrate2d(const string&, Datafile&, Parser&, const Settings&, Define&);
vector<double> differentiate(const string& sCmd, Parser& _parser, Datafile& _data, const Settings& _option, Define& _functions);
bool findExtrema(string& sCmd, Datafile& _data, Parser& _parser, const Settings& _option, Define& _functions);
bool findZeroes(string& sCmd, Datafile& _data, Parser& _parser, const Settings& _option, Define& _functions);
void taylor(string& sCmd, Parser& _parser, const Settings& _option, Define& _functions);
bool fitDataSet(string& sCmd, Parser& _parser, Datafile& _data, Define& _functions, const Settings& _option);
bool fastFourierTransform(string& sCmd, Parser& _parser, Datafile& _data, const Settings& _option);
bool fastWaveletTransform(string& sCmd, Parser& _parser, Datafile& _data, const Settings& _option);
bool evalPoints(string& sCmd, Datafile& _data, Parser& _parser, const Settings& _option, Define& _functions);
bool createDatagrid(string& sCmd, string& sTargetCache, Parser& _parser, Datafile& _data, Define& _functions, const Settings& _option);
bool writeAudioFile(string& sCmd, Parser& _parser, Datafile& _data, Define& _functions, const Settings& _option);
bool regularizeDataSet(string& sCmd, Parser& _parser, Datafile& _data, Define& _functions, const Settings& _option);
bool analyzePulse(string& sCmd, Parser& _parser, Datafile& _data, Define& _functions, const Settings& _option);
bool shortTimeFourierAnalysis(string& sCmd, string& sTargetCache, Parser& _parser, Datafile& _data, Define& _functions, const Settings& _option);
bool calculateSplines(string& sCmd, Parser& _parser, Datafile& _data, Define& _functions, const Settings& _option);

#endif // COMMAND_IMPLEMENTATIONS_HPP

