# -*- coding: UTF-8 -*-
from pyailib.twisted.smpp import *
from pyailib.utils import cdr

class PDUError(Exception): pass


def TLV():
    yield UINT16("tag")
    length = yield UINT16("length")
    yield BYTES("value", length=length)


def TLVS():
    i = 0
    while True:
        yield DICT(i, TLV)
        i += 1


def BIND():
    yield CSTR("system_id", maxLength=16)
    yield CSTR("password", maxLength=9)
    yield CSTR("system_type", maxLength=13)
    yield UINT8("interface_version")
    yield UINT8("addr_ton")
    yield UINT8("addr_npi")
    yield CSTR("address_range", maxLength=41)


def BIND_RESPONSE():
    yield CSTR("system_id", maxLength=1)
    

def OUTBIND():
    yield CSTR("system_id", maxLength=16)
    yield CSTR("password", maxLength=9)
    

def ALERT_NOTIFICATION():
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=65)
    yield UINT8("esme_addr_ton")
    yield UINT8("esme_addr_npi")
    yield CSTR("esme_addr", maxLength=65)


def SM():
    yield CSTR("service_type", maxLength=6)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
    yield UINT8("dest_addr_ton")
    yield UINT8("dest_addr_npi")
    yield CSTR("destination_addr", maxLength=21)
    yield UINT8("esm_class")
    yield UINT8("protocol_id")
    yield UINT8("priority_flag")
    yield CSTR("schedule_delivery_time", 1, 17)
    yield CSTR("validity_period", 1, 17)
    yield UINT8("registered_delivery")
    yield UINT8("replace_if_present_flag")
    yield UINT8("data_coding")
    yield UINT8("sm_default_msg_id")
    smLength = yield UINT8("sm_length")
    yield BYTES("short_message", length=sm_length)

    
def SM_RESP():
    yield CSTR("message_id", maxLength=65)

    
def DATA_SM():
    yield CSTR("service_type", maxLength=6)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=65)
    yield UINT8("dest_addr_ton")
    yield UINT8("dest_addr_npi")
    yield CSTR("destination_addr", maxLength=65)
    yield UINT8("esm_class")
    yield UINT8("registered_delivery")
    yield UINT8("data_coding")


def SUBMIT_MULTI(buf, fields):
    def DEST_ADDRESS():
        destFlag = yield UINT8("dest_flag")
        if destFlag == 0x01:
            yield UINT8("dest_addr_ton")
            yield UINT8("dest_addr_npi")
            yield CSTR("destination_addr", maxLength=21)
        elif destFlag == 0x02:
            yield CSTR("dl_name", maxLength=21)
        else:
            raise PDUError, "invalid dest_flag"
                
    yield CSTR("service_type", maxLength=6)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
    numberOfDests = yield UINT8("number_of_dests")
    yield LIST_OF("dest_address", numberOfDests, DEST_ADDRESS)
    yield UINT8("esm_class")
    yield UINT8("protocol_id")
    yield UINT8("priority_flag")
    yield CSTR("schedule_delivery_time", 1, 17)
    yield CSTR("validity_period", 1, 17)
    yield UINT8("registered_delivery")
    yield UINT8("replace_if_present_flag")
    yield UINT8("data_coding")
    yield UINT8("sm_default_msg_id")
    smLength = yield UINT8("sm_length")
    yield BYTES("short_message", length=smLength)
    

def SUBMIT_MULTI_RESP():
    def UNSUCCESS_SME():
        yield UINT8("dest_addr_ton")
        yield UINT8("dest_addr_npi")
        yield CSTR("destination_addr", maxLength=21)
        yield UINT32("error_status_code")
        
    yield CSTR("message_id", maxLength=65)
    noUnsuccess = yield UINT8("no_unsuccess")
    yield LIST_OF("unsuccess_sme", noUnsuccess, UNSUCCESS_SME)


def CANCEL_SM():
    yield CSTR("service_type", maxLength=6)
    yield CSTR("message_id", maxLength=65)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
    yield UINT8("dest_addr_ton")
    yield UINT8("dest_addr_npi")
    yield CSTR("destination_addr", maxLength=21)
    
def QUERY_SM():
    yield CSTR("message_id", maxLength=65)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
    
def QUERY_SM_RESP():
    yield CSTR("message_id", maxLength=65)
    yield CSTR("final_date", 1, 17)
    yield UINT8("message_state")
    yield UINT8("error_code")

def REPLACE_SM():
    yield CSTR("message_id", maxLength=65)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
    yield CSTR("schedule_delivery_time", 1, 17)
    yield CSTR("validity_period", 1, 17)
    yield UINT8("registered_delivery")
    yield UINT8("replace_if_present_flag")
    yield UINT8("data_coding")
    yield UINT8("sm_default_msg_id")
    smLength = yield UINT8("sm_length")
    yield BYTES("short_message", length=sm_length)


def BROADCAST_SM():
    yield CSTR("service_type", maxLength=6)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
    yield CSTR("message_id", maxLength=65)
    yield UINT8("priority_flag")
    yield CSTR("schedule_delivery_time", 1, 17)
    yield CSTR("validity_period", 1, 17)
    yield UINT8("replace_if_present_flag")
    yield UINT8("data_coding")
    yield UINT8("sm_default_msg_id")
    smLength = yield UINT8("sm_length")
    yield STR("short_message", length=smLength)    
    yield DICT("broadcast_area_identifer", TLV)
    yield DICT("broadcast_content_type", TLV)
    yield DICT("broadcast_rep_num", TLV)
    yield DICT("broadcast_frequency_interval", TLV)


def BROADCAST_SM_RESP():
    yield CSTR("message_id", maxLength=65)


def QUERY_BROADCAST_SM():
    yield CSTR("message_id", maxLength=65)
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
 
    
def QUERY_BROADCAST_SM_RESP():
    yield CSTR("message_id", maxLength=65)
    yield DICT("message_state", TLV)
    yield DICT("broadcast_area_identifier", TLV)
    yield DICT("broadcast_area_success", TLV)


def CANCEL_BROADCAST_SM():
    yield CSTR("service_type", maxLength=6)
    yield CSTR("message_id", maxLength=65)       
    yield UINT8("source_addr_ton")
    yield UINT8("source_addr_npi")
    yield CSTR("source_addr", maxLength=21)
 
PDU_MAP = {
    PDU_GENERIC_ACK:                NULL,
    PDU_ENQUIRE_LINK:               NULL,
    PDU_ENQUIRE_LINK_RESP:          NULL,
    PDU_BIND_TRANSMITTER:           BIND,
    PDU_BIND_TRANSMITTER_RESP:      BIND_RESP,
    PDU_BIND_RECEIVER:              BIND,
    PDU_BIND_RECEIVER_RESP:         BIND_RESP,
    PDU_BIND_TRANSCEIVER:           BIND,
    PDU_BIND_TRANSCEIVER_RESP:      BIND_RESP,
    PDU_UNBIND:                     NULL,
    PDU_UNBIND_RESP:                NULL,
    PDU_OUTBIND:                    OUTBIND,
    PDU_ALERT_NOTIFICATION:         ALERT_NOTIFICATION,
    PDU_SUBMIT_SM:                  SM,
    PDU_SUBMIT_SM_RESP:             SM_RESP,
    PDU_DATA_SM:                    DATA_SM,
    PDU_DATA_SM_RESP:               DATA_SM_RESP,
    PDU_SUBMIT_MULTI:               SUBMIT_MULTI,
    PDU_SUBMIT_MULTI_RESP:          SUBMIT_MULTI_RESP,
    PDU_CANCEL_SM:                  CANCEL_SM,
    PDU_CANCEL_SM_RESP:             NULL,
    PDU_QUERY_SM:                   QUERY_SM,
    PDU_QUERY_SM_RESP:              QUERY_SM_RESP,
    PDU_REPLACE_SM:                 REPLACE_SM,
    PDU_REPLACE_SM_RESP:            REPLACE_SM_RESP,
    PDU_BROADCAST_SM:               BROADCAST_SM,
    PDU_BROADCAST_SM_RESP:          BROADCAST_SM_RESP,
    PDU_QUERY_BROADCAST_SM:         QUERY_BROADCAST_SM,
    PDU_QUERY_BROADCAST_SM_RESP:    QUERY_BROADCAST_SM_RESP,
    PDU_CANCEL_BROADCAST_SM:        CANCEL_BROADCAST_SM,
    PDU_CANCEL_BROADCAST_SM_RESP:   NULL,
}

def SMPP(pduMap=PDU_MAP):
    commandID = yield UINT32("command_id")
    yield UINT32("command_status")
    yield UINT32("command_sequence")
    
    yield pduMap[commandID]


def unpackPDU(syntax, argName=-1):
    def _wrapping(func):
        def _wrapper(*argv, **kwargs):
            if isinstance(argName, (int, long)):
                pdu = cdr.fromString(argv[argName])
                argv = list(argv)
                argv[argName] = pdu
            elif isinstance(argName, (str, unicode)):
                pdu = cdr.fromString(kwargs[argName])
                kwargs = kwargs.copy()
                kwargs[argName] = pdu
            return func(*argv, **kwargs)
        _wrapper.__name__ = func.__name__
        _wrapper.__module__ = func.__module__
        _wrapper.__doc__ = func.__doc__
    return _wrapping

