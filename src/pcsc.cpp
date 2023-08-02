#include <iomanip>

#include <loguru.hpp>

#include "pcsc.hpp"

std::string bytesToHex(std::vector<uint8_t> const& v)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(auto it = v.begin(); it != v.end(); ++it)
        ss << std::setw(2) << (uint16_t)(*it);

    return ss.str();
}

Pcsc::Pcsc()
{
    LONG rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM,
                                    nullptr,
                                    nullptr,
                                    &_context);
    if(rv != SCARD_S_SUCCESS) {
        LOG(error) << "Error SCardEstablishContext: " << _getErrorMsg(rv);
        return;
    }

    LOG(info) << "Establish context";
}

Pcsc::~Pcsc()
{
    if(_readerHandle != 0)
        SCardDisconnect(_readerHandle, SCARD_LEAVE_CARD);

    if(_context != 0)
        SCardReleaseContext(_context);

    LOG(info) << "Release context";
}

std::vector<std::string> Pcsc::getReaders()
{
    LPSTR names = nullptr;
    DWORD count = SCARD_AUTOALLOCATE;

    LONG rv = SCardListReaders(_context,
                               nullptr,
                               (LPTSTR)&names,
                               &count);
    if(rv != SCARD_S_SUCCESS) {
        LOG(error) << "Error SCardListReadersA: " << _getErrorMsg(rv);
        return std::vector<std::string>();
    }

    std::vector<std::string> readers;
    if(names != nullptr) {
        LPSTR reader = names;
        for( ; *reader != '\0'; ) {
            std::string name(reader);
            LOG(info) << "Reader: " << name;
            readers.push_back(name);
            reader += name.size()+1;
        }
    }

    if(_context != 0)
        SCardFreeMemory(_context, names);

    return readers;
}

bool Pcsc::connect(std::string reader)
{
    _reader = reader;

    LONG rv = SCardConnect(_context,
                           _reader.data(),
                           SCARD_SHARE_SHARED,
                           SCARD_PROTOCOL_ANY,
                           &_readerHandle,
                           &_protocolType);
    if(rv != SCARD_S_SUCCESS) {
        LOG(error) << "Error SCardConnectA: " << _getErrorMsg(rv);
        return false;
    }

    LOG(info) << "Connect reader: " << _reader;

    switch(_protocolType) {
        case SCARD_PROTOCOL_T0:
            _protocol = *SCARD_PCI_T0;
            LOG(info) << "Protocol: T0";
        break;

        case SCARD_PROTOCOL_T1:
            _protocol = *SCARD_PCI_T1;
            LOG(info) << "Protocol: T1";
        break;
    }

    return true;
}

bool Pcsc::disconnect()
{
    if(_readerHandle != 0)
        SCardDisconnect(_readerHandle, SCARD_LEAVE_CARD);

    return true;
}

std::vector<uint8_t> Pcsc::getAtr()
{
    DWORD state;
    DWORD protocol;
    DWORD size = 255;
    LPSTR names = nullptr;
    DWORD count = SCARD_AUTOALLOCATE;
    std::vector<uint8_t> atr(255, 0);

    LONG rv = SCardStatus(_readerHandle,
                          (LPTSTR)&names,
                          &count,
                          &state,
                          &protocol,
                          &atr[0],
                          &size);
    if(rv != SCARD_S_SUCCESS) {
        LOG(error) << "Error SCardStatus: " << _getErrorMsg(rv);
        return std::vector<uint8_t>();
    }

    atr.resize(size);

    LOG(info) << "Atr: " << bytesToHex(atr);

    if(_context != 0)
        SCardFreeMemory(_context, names);

    return atr;
}

std::vector<uint8_t> Pcsc::sendCommand(std::vector<uint8_t> const & command)
{
    LONG rv;
    DWORD respSize = 255;
    std::vector<uint8_t> resp(255, 0);

    LOG(info) << "Command: " << bytesToHex(command);

    rv = SCardTransmit(_readerHandle,
                       &_protocol,
                       command.data(),
                       command.size(),
                       nullptr,
                       &resp[0],
                       &respSize);
    if(rv != SCARD_S_SUCCESS) {
        LOG(error) << "Error SCardTransmit: " << _getErrorMsg(rv);
        return std::vector<uint8_t>();
    }

    resp.resize(respSize);

    LOG(info) << "Response: " << bytesToHex(resp);

    if(*(resp.rbegin()+1) == 0x61) {
        std::vector<uint8_t> getRespCommand = {
            0x00, 0xc0, 0x00, 0x00, *resp.rbegin()
        };

        respSize = 255;
        resp.resize(255, 0);

        rv = SCardTransmit(_readerHandle,
                           &_protocol,
                           getRespCommand.data(),
                           getRespCommand.size(),
                           nullptr,
                           &resp[0],
                           &respSize);
        if(rv != SCARD_S_SUCCESS) {
            LOG(error) << "Error SCardTransmit: " << _getErrorMsg(rv);
            return std::vector<uint8_t>();
        }

        resp.resize(respSize);

        LOG(info) << "Response: " << bytesToHex(resp);
    }

    return resp;
}

std::string Pcsc::_getErrorMsg(LONG value) const
{
    #define makeErrorMsg(error) case error: return #error
    switch(value) {
        makeErrorMsg(SCARD_F_INTERNAL_ERROR          );
        makeErrorMsg(SCARD_E_CANCELLED               );
        makeErrorMsg(SCARD_E_INVALID_HANDLE          );
        makeErrorMsg(SCARD_E_INVALID_PARAMETER       );
        makeErrorMsg(SCARD_E_INVALID_TARGET          );
        makeErrorMsg(SCARD_E_NO_MEMORY               );
        makeErrorMsg(SCARD_F_WAITED_TOO_LONG         );
        makeErrorMsg(SCARD_E_INSUFFICIENT_BUFFER     );
        makeErrorMsg(SCARD_E_UNKNOWN_READER          );
        makeErrorMsg(SCARD_E_TIMEOUT                 );
        makeErrorMsg(SCARD_E_SHARING_VIOLATION       );
        makeErrorMsg(SCARD_E_NO_SMARTCARD            );
        makeErrorMsg(SCARD_E_UNKNOWN_CARD            );
        makeErrorMsg(SCARD_E_CANT_DISPOSE            );
        makeErrorMsg(SCARD_E_PROTO_MISMATCH          );
        makeErrorMsg(SCARD_E_NOT_READY               );
        makeErrorMsg(SCARD_E_INVALID_VALUE           );
        makeErrorMsg(SCARD_E_SYSTEM_CANCELLED        );
        makeErrorMsg(SCARD_F_COMM_ERROR              );
        makeErrorMsg(SCARD_F_UNKNOWN_ERROR           );
        makeErrorMsg(SCARD_E_INVALID_ATR             );
        makeErrorMsg(SCARD_E_NOT_TRANSACTED          );
        makeErrorMsg(SCARD_E_READER_UNAVAILABLE      );
        makeErrorMsg(SCARD_P_SHUTDOWN                );
        makeErrorMsg(SCARD_E_PCI_TOO_SMALL           );
        makeErrorMsg(SCARD_E_READER_UNSUPPORTED      );
        makeErrorMsg(SCARD_E_DUPLICATE_READER        );
        makeErrorMsg(SCARD_E_CARD_UNSUPPORTED        );
        makeErrorMsg(SCARD_E_NO_SERVICE              );
        makeErrorMsg(SCARD_E_SERVICE_STOPPED         );
        makeErrorMsg(SCARD_E_UNEXPECTED              );
        makeErrorMsg(SCARD_E_ICC_INSTALLATION        );
        makeErrorMsg(SCARD_E_ICC_CREATEORDER         );
        // makeErrorMsg(SCARD_E_UNSUPPORTED_FEATURE     );
        makeErrorMsg(SCARD_E_DIR_NOT_FOUND           );
        makeErrorMsg(SCARD_E_FILE_NOT_FOUND          );
        makeErrorMsg(SCARD_E_NO_DIR                  );
        makeErrorMsg(SCARD_E_NO_FILE                 );
        makeErrorMsg(SCARD_E_NO_ACCESS               );
        makeErrorMsg(SCARD_E_WRITE_TOO_MANY          );
        makeErrorMsg(SCARD_E_BAD_SEEK                );
        makeErrorMsg(SCARD_E_INVALID_CHV             );
        makeErrorMsg(SCARD_E_UNKNOWN_RES_MNG         );
        makeErrorMsg(SCARD_E_NO_SUCH_CERTIFICATE     );
        makeErrorMsg(SCARD_E_CERTIFICATE_UNAVAILABLE );
        makeErrorMsg(SCARD_E_NO_READERS_AVAILABLE    );
        makeErrorMsg(SCARD_E_COMM_DATA_LOST          );
        makeErrorMsg(SCARD_E_NO_KEY_CONTAINER        );
        makeErrorMsg(SCARD_E_SERVER_TOO_BUSY         );
        makeErrorMsg(SCARD_W_UNSUPPORTED_CARD        );
        makeErrorMsg(SCARD_W_UNRESPONSIVE_CARD       );
        makeErrorMsg(SCARD_W_UNPOWERED_CARD          );
        makeErrorMsg(SCARD_W_RESET_CARD              );
        makeErrorMsg(SCARD_W_REMOVED_CARD            );
        makeErrorMsg(SCARD_W_SECURITY_VIOLATION      );
        makeErrorMsg(SCARD_W_WRONG_CHV               );
        makeErrorMsg(SCARD_W_CHV_BLOCKED             );
        makeErrorMsg(SCARD_W_EOF                     );
        makeErrorMsg(SCARD_W_CANCELLED_BY_USER       );
        makeErrorMsg(SCARD_W_CARD_NOT_AUTHENTICATED  );
        // makeErrorMsg(SCARD_W_CACHE_ITEM_NOT_FOUND    );
        // makeErrorMsg(SCARD_W_CACHE_ITEM_STALE        );
        default: return "Unknown";
    }
    #undef makeErrorMsg
}
