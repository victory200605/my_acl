from py_cmpp import *

VERSION = 0x20



class Connect(CMPP_Object):
    COMMAND_ID = CONNECT
    DEFINE_LIST = (
        ("source_addr",             "6s",   str,    None),
        ("authenticator_source",    "16s",  str,    None),
        ("version",                 "B",    int,    None),
        ("timestamp",               "!L",   int,    None),
    )



class Connect_Resp(CMPP_Object):
    COMMAND_ID = CONNECT_RESP
    DEFINE_LIST = (
        ("status",              "B",    int,    None),
        ("authenticator_ismg",  "16s",  str,    None),
        ("version",             "B",    int,    None),
    )



class Terminate(CMPP_Object):
    COMMAND_ID = TERMINATE
    DEFINE_LIST = ()



class Terminate_Resp(CMPP_Object):
    COMMAND_ID = TERMINATE_RESP
    DEFINE_LIST = ()



class Active_Test(CMPP_Object):
    COMMAND_ID = ACTIVE_TEST
    DEFINE_LIST = ()



class Active_Test_Resp(CMPP_Object):
    COMMAND_ID = ACTIVE_TEST_RESP
    DEFINE_LIST = (
        ("reserved", "s", str,  None),
    )



class Submit(CMPP_Object):
    COMMAND_ID = SUBMIT
    DEFINE_LIST = (
        ("msg_id",              "!Q",   long,   None),
        ("pk_total",            "B",    int,    None),
        ("pk_number",           "B",    int,    None),
        ("registered_delivery", "B",    int,    None),
        ("msg_level",           "B",    int,    None),
        ("service_id",          "10s",  str,    None),
        ("fee_user_type",       "B",    int,    None),
        ("fee_terminal_id",     "21s",  str,    None),
        ("tp_pid",              "B",    int,    None),
        ("tp_udhi",             "B",    int,    None),
        ("msg_fmt",             "B",    int,    None),
        ("msg_src",             "6s",   str,    None),
        ("fee_type",            "2s",   str,    None),
        ("fee_code",            "6s",   str,    None),
        ("valid_time",          "17s",  str,    None),
        ("at_time",             "17s",  str,    None),
        ("src_id",              "21s",  str,    None),
        ("dest_usr_tl",         "B",    str,    None),
        ("dest_terminal_ids",   "21s",  list,   "dest_usr_tl"),
        ("msg_length",          "B",    int,    None),
        ("msg_content",         "1s",   str,    "msg_length"),
        ("reserved",            "8s",   str,    None),
    )



class Submit_Resp(CMPP_Object):
    COMMAND_ID = SUBMIT_RESP
    DEFINE_LIST = (
        ("msg_id", "!Q",    long,   None),
        ("result", "B",     int,    None),
    )



class Deliver(CMPP_Object):
    COMMAND_ID = DELIVER
    DEFINE_LIST = (
        ("msg_id",              "!Q",   long,   None),
        ("dest_id",             "21s",  str,    None),
        ("service_id",          "10s",  str,    None),
        ("tp_pid",              "B",    int,    None),
        ("tp_udhi",             "B",    int,    None),
        ("msg_fmt",             "B",    int,    None),
        ("src_terminal_id",     "21s",  str,    None),
        ("registered_delivery", "B",    int,    None),
        ("msg_length",          "B",    int,    None),
        ("msg_content",         "s",    str,    "msg_length"),
        ("reserved",            "8s",   str,    None),
    )



class Deliver_Resp(CMPP_Object):
    COMMAND_ID = DELIVER_RESP
    DEFINE_LIST = (
        ("msg_id", "!Q",    long,   None),
        ("result", "B",     int,    None),
    )



class Fwd(CMPP_Object):
    COMMAND_ID = FWD
    DEFINE_LIST = (
        ("source_id",               "6s",   str,    None),
        ("destination_id",          "6s",   str,    None),
        ("nodes_count",             "B",    int,    None),
        ("msg_fwd_type",            "B",    int,    None),
        ("msg_id",                  "!Q",   long,   None),
        ("tp_pid",                  "B",    int,    None),
        ("tp_udhi",                 "B",    int,    None),
        ("registered_delivery",     "B",    int,    None),
        ("msg_level",               "B",    int,    None),
        ("service_id",              "10s",  str,    None),
        ("fee_user_type",           "B",    int,    None),
        ("fee_terminal_id",         "21s",  str,    None),
        ("tp_pid",                  "B",    int,    None),
        ("tp_udih",                 "B",    int,    None),
        ("msg_fmt",                 "B",    int,    None),
        ("msg_src",                 "6s",   str,    None),
        ("fee_type",                "2s",   str,    None),
        ("fee_code",                "6s",   str,    None),
        ("valid_time",              "17s",  str,    None),
        ("at_time",                 "17s",  str,    None),
        ("src_id",                  "21s",  str,    None),
        ("dest_usr_tl",             "B",    int,    None),
        ("dest_ids",                "21s",  list,   "dest_usr_tl"),
        ("msg_length",              "B",    int,    None),
        ("msg_content",             "s",    str,    "msg_length"),
        ("reserved",                "8s",   str,    None),
    )



class Fwd_Resp(CMPP_Object):
    COMMAND_ID = FWD_RESP
    DEFINE_LIST = (
        ("msg_id",      "!Q",   long,   None),
        ("pk_total",    "B",    int,    None),
        ("pk_number",   "B",    int,    None),
        ("result",      "B",    int,    None),
    )


class MT_StatReport(CMPP_Object):
    DEFINE_LIST = (
        ("msg_id",              "!Q",   long,   None),
        ("stat",                "7s",   int,    None),
        ("submit_time",         "10s",  str,    None),
        ("done_time",           "10s",  str,    None),
        ("dest_terminal_id",    "21s",  str,    None),
        ("smsc_sequence",       "!L",   int,    None),
    )


class MO_StatReport(CMPP_Object):
    DEFINE_LIST = (
        ("msg_id",                  "!Q",   long,   None),
        ("stat",                    "7s",   str,    None),
        ("cmpp_deliver_time",       "10s",  str,    None),
        ("cmpp_deliver_resp_time",  "10s",  str,    None),
        ("dest_id",                 "21s",  str,    None),
        ("reserved",                "!L",   int,    None),
    )
    

