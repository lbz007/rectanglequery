/** 
 * @file  version.cpp
 * @date  09/27/2020 08:13:44 PM CST
 * @brief ""
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "util/version.h"
#include <sstream>

namespace open_edi {
namespace util {

using namespace std;

Version::Version() { 
    reset(); 
}

Version::~Version() {}

void Version::reset() {
    major_ = -1;
    minor_ = -1;
    revision_ = -1;
}

void Version::init() {
    major_ = 1;
    minor_ = 0;
    revision_ = 0;
}

void Version::set(Version & v) {
    major_ = v.major_;
    minor_ = v.minor_;
    revision_ = v.revision_;
}

const std::string & Version::getVersionString() {
    stringstream sstream;
    sstream << kHeaderChar << major_ << kVersionDelimiter << minor_ << kVersionDelimiter << revision_;
    version_string_ = sstream.str();
    return version_string_;
}

void Version::writeToFile(IOManager & io_manager, bool debug)
{
    const char *version = getVersionString().c_str();
    Bits8 size = strlen(version);
    if (debug) {
        cout << "RWDBGINFO: version size " << (int)size << endl;
    }
    if (size == 0) return;

    io_manager.write(sizeof(Bits8), (void *) &(size));
    if (debug) { 
        cout << "RWDBGINFO: version string: " << version << endl;
    }
    io_manager.write(size, (void *)version);
}

void Version::readFromFile(IOManager & io_manager, bool debug)
{
    Bits8 size = 0;
    io_manager.read((void *) &(size), sizeof(Bits8));
    if (debug) { 
        cout << "RWDBGINFO: version size " << (int)size << endl;
    }
    if (size == 0) return; //shouldn't happen

    char *version = new char [size + 1];
    io_manager.read(version, size);
    version[size] = '\0';
    stringstream sstream(version);
    char header;
    char delimiter;
    sstream >> header >> major_ >> delimiter >> minor_ >> delimiter >> revision_;
    if (debug) { 
        cout << "RWDBGINFO: version string: " << version << endl;
        cout << "RWDBGINFO: version: " << header << major_ << delimiter << minor_ << delimiter << revision_ << endl;
    }
    delete [] version;
}

}  // namespace util
}  // namespace open_edi


