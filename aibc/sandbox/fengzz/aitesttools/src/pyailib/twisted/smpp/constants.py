# -*- coding: UTF-8 -*-


MESSAGE_MODE_DEFAULT            = 0x0
MESSAGE_MODE_DATAGRAM           = 0x1
MESSAGE_MODE_TRANSACTION        = 0x2
MESSAGE_MODE_STORE_AND_FORWARD  = 0x3


# 2.3 Session States
SESSION_STATE_OPEN      = 0x01
SESSION_STATE_BOUND_TX  = 0x02
SESSION_STATE_BOUND_RX  = 0x03
SESSION_STATE_BOUND_TRX = 0x04
SESSION_STATE_UNBOUND   = 0x05
SESSION_STATE_CLOSED    = 0x06
SESSION_STATE_OUTBOUND  = 0x07


# 4.7.1 addr_ton, source_addr_ton, dest_addr_ton, esme_addr_ton
TON_UNKNOW              = 0x00
TON_INTERNATIONAL       = 0x01
TON_NATIONAL            = 0x02
TON_NETWORK_SPECIFIC    = 0x03
TON_SUBSCRIBER_NUMBER   = 0x04
TON_ALPHANUMERIC        = 0x05
TON_ABBREVIATED         = 0x06


# 4.7.2 addr_npi, source_addr_npi, dest_addr_npi, esme_addr_npi
NPI_UNKNOW          = 0x00
NPI_ISDN            = 0x01
NPI_DATA            = 0x03
NPI_TELEX           = 0x04
NPI_LAND_MOBILD     = 0x06
NPI_NATIONAL        = 0x08
NPI_PRIVATE         = 0x09
NPI_ERMES           = 0x0A
NPI_INTERNET        = 0x0E
NPI_WAP_CLIENT_ID   = 0x12
    

# 4.7.7 data_coding
DC_MC_SPECIFIC          = 0x00
DC_ASCII                = 0x01
DC_OCTET_UNSPECIFIED_2  = 0x02
DC_LATIN_1              = 0x03
DC_OCTET_UNSPECIFIED_4  = 0x04
DC_JIS                  = 0x05
DC_CYRILLIC             = 0x06
DC_HEBREW               = 0x07
DC_UCS2                 = 0x08
DC_PICTOGRAM            = 0x09
DC_ISO_2022_JP          = 0x0A
DC_KANJI                = 0x0D
DC_KS_C_5601            = 0x0E


# 4.7.15 message_state
STATE_SCHEDULED     = 0x00
STATE_ENROUTE       = 0x01
STATE_DELIVERED     = 0x02
STATE_EXPIRED       = 0x03
STATE_DELETED       = 0x04
STATE_UNDELIVERABLE = 0x05
STATE_ACCEPTED      = 0x06
STATE_UNKNOWN       = 0x07
STATE_REJECTED      = 0x08
STATE_SKIPPED       = 0x09 


PDU_NAMES = {
    PDU_GENERIC_NACK            : "GENERIC_NACK",
    PDU_BIND_RECEIVER           : "BIND_RECEIVER",
    PDU_BIND_RECEIVER_RESP      : "BIND_RECEIVER_RESP",
    PDU_BIND_TRANSMITTER        : "BIND_TRANSMITTER",
    PDU_BIND_TRANSMITTER_RESP   : "BIND_TRANSMITTER_RESP",
    PDU_QUERY_SM                : "QUERY_SM",
    PDU_QUERY_SM_RESP           : "QUERY_SM_RESP",
    PDU_SUBMIT_SM               : "SUBMIT_SM",
    PDU_SUBMIT_SM_RESP          : "SUBMIT_SM_RESP",
    PDU_DELIVER_SM              : "DELIVER_SM",
    PDU_DELIVER_SM_RESP         : "DELIVER_SM_RESP",
    PDU_UNBIND                  : "UNBIND",
    PDU_UNBIND_RESP             : "UNBIND_RESP",
    PDU_REPLACE_SM              : "REPLACE_SM",
    PDU_REPLACE_SM_RESP         : "REPLACE_SM_RESP",
    PDU_CANCEL_SM               : "CANCEL_SM",
    PDU_CANCEL_SM_RESP          : "CANCEL_SM_RESP",
    PDU_BIND_TRANSCEIVER        : "BIND_TRANSCEIVER",
    PDU_BIND_TRANSCEIVER_RESP   : "BIND_TRANSCEIVER_RESP",
    PDU_OUTBIND                 : "OUTBIND",
    PDU_ENQUIRE_LINK            : "ENQUIRE_LINK",
    PDU_ENQUIRE_LINK_RESP       : "ENQUIRE_LINK_RESP",
    PDU_SUBMIT_MULTI            : "SUBMIT_MULTI",
    PDU_SUBMIT_MULTI_RESP       : "SUBMIT_MULTI_RESP",
    PDU_ALERT_NOTIFICATION      : "ALERT_NOTIFICATION",
    PDU_DATA_SM                 : "DATA_SM",
    PDU_DATA_SM_RESP            : "DATA_SM_RESP",    
}           


