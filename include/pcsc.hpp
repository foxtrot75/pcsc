#pragma once

#include <vector>
#include <string>
#include <sstream>

#include <winscard.h>

class Pcsc
{
public:
    Pcsc();
    ~Pcsc();

    std::vector<std::string> getReaders();

    bool connect(std::string reader);
    bool disconnect();

    std::vector<uint8_t> getAtr();

    std::vector<uint8_t> sendCommand(std::vector<uint8_t> const & command);

    std::string getReader() { return _reader; }

private:
    std::string _getErrorMsg(LONG value) const;

private:
    SCARDCONTEXT        _context;
    SCARDHANDLE         _readerHandle;
    DWORD               _protocolType;
    SCARD_IO_REQUEST    _protocol;

    std::string         _reader;
};
