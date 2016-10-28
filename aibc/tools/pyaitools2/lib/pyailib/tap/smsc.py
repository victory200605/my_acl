# -*- coding: UTF-8 -*-
from random import choice
from ConfigParser import SafeConfigParser as ConfigParser
from datetime import datetime
from time import *
from functools import wraps

from twisted.application import service, strports
from twisted.internet import reactor, protocol
from twisted.web import server, resource
from twisted.python import log, usage


from pyailib.protocol.smpp import *
from pyailib.util import *
from pyailib.xml_util import etree
from pyailib.shaper import *

############################################################################
class SMSCSession(SMPPSession):
    SESSION_INIT_TIMER = 5
    
    system_id = None
       
    def connectionMade(self):
        SMPPSession.connectionMade(self)
        
    def connectionLost(self, reason):
        SMPPSession.connectionLost(self, reason)
        if self.system_id:
            self.factory.unregister(self.system_id, self)
    
    ######## handle BIND_*_REQ
    def doBind(self, state, register, command_id, command_status, sequence_number, mandatory, optional):
        system_id = mandatory["system_id"]
        password = mandatory["password"]
        rsp_status = self.factory.login(system_id, password)
        if rsp_status == SMPP_STATUS_ROK:
            self.sendResponse(
                getResponse(command_id),
                rsp_status,
                sequence_number,
                {"system_id":system_id},
            )
            self.setState(state)
            self.system_id = system_id
            register(system_id, self)
        else:
            self.sendResponse(
                getResponse(command_id),
                rsp_status,
                sequence_number,
            )
            self.transport.loseConnection()
        
    @guard("OPEN")
    @decodePacket(SMPP_BIND_TRANSMITTER_REQ)            
    def handle_BIND_TRANSMITTER_REQ(self, *argv):
        self.doBind(
            "BOUND_TX",
            self.factory.registerTX,
            SMPP_BIND_TRANSMITTER_REQ,
            *argv
        )

    @guard("OPEN")    
    @decodePacket(SMPP_BIND_TRANSCEIVER_REQ)        
    def handle_BIND_TRANSCEIVER_REQ(self, *argv):
        self.doBind(
            "BOUND_TRX",
            self.factory.registerTRX,
            SMPP_BIND_TRANSCEIVER_REQ,
            *argv
        )
                   
    @guard("OPEN")
    @decodePacket(SMPP_BIND_RECEIVER_REQ)    
    def handle_BIND_RECEIVER_REQ(self, *argv):
        self.doBind(
            "BOUND_RX",
            self.factory.registerRX,
            SMPP_BIND_RECEIVER_REQ,
            *argv
        )
            
    ######## handle UNBIND_REQ
    @guard("BOUND_TX", "BOUND_TRX", "BOUND_RX")    
    def handle_UNBIND_REQ(self, command_id, command_status, sequence_number, body):
        self.sendCommand(SMPP_UNBIND_RSP, 0, sequence_number)
        self.session_state = "UNBOUND"
        self.transport.loseConnection()
    
    ######## SUBMIT_SM_REQ
    @guard("BOUND_TX", "BOUND_TRX")     
    @decodePacket(SMPP_SUBMIT_SM_REQ)
    def handle_SUBMIT_SM_REQ(self, command_status, sequence_number, mandatory, optional):
        submit_date = datetime.now()
        
        delay = self.factory.response_delay.next()
        if delay >= 0:
            reactor.callLater(
                delay,
                self.sendSubmitResponse,
                sequence_number,
                mandatory,
                submit_date,
            )

    def sendSubmitResponse(self, sequence_number, mandatory, submit_date):
        command_status = self.factory.response_status.next()
        if command_status == SMPP_STATUS_ROK:
            message_id = self.factory.message_id.next()
            self.sendResponse(
                SMPP_SUBMIT_SM_RSP,
                command_status,
                sequence_number,
                {"message_id": str(message_id)},
            )
            
            delay = self.factory.receipt_delay.next() 
            if delay >= 0:              
                reactor.callLater(
                    delay,
                    self.sendSubmitReceipt,
                    mandatory,
                    message_id,
                    submit_date,
                )
        else:
            self.sendResponse(
                SMPP_SUBMIT_SM_RSP,
                command_status,
                sequence_number,
            )
                         
    def sendSubmitReceipt(self, sm, message_id, submit_date):
        proto = self.factory.getRX(self.system_id)
        if proto:     
            stat = self.factory.receipt_stat.next()           
            rcpt = {
                "source_addr": sm["destination_addr"],
                "destination_addr": sm["source_addr"],
                "esm_class": 0x4,
                "short_message": newReceipt(
                    id=message_id,
                    sub=1,
                    dlvrd=1 if stat == "DELIVRD" else 0,
                    submit_date=submit_date.strftime("%y%m%d%H%M"),
                    done_date=datetime.now().strftime("%y%m%d%H%M"),
                    stat=self.factor.receipt_stat.next(),
                    err="",
                    text="",
                ),
            }
            
            self.sendRequest(
                SMPP_DELIVER_SM_REQ,
                body=encode(SMPP_DELIVER_SM_REQ, rcpt),
                size=0,
            )
            
            self.rx_counter += 1

############################################################################
class SMSCFactory(protocol.ServerFactory):
    protocol = SMSCSession
    
    def __init__(self, config):
        ini = ConfigParser()
        ini.read([config])
        
        self.accounts = dict(ini.items("account"))
        
        self.response_delay = RandomWeightedGenerator(type=float)
        self.response_delay.addMulti(ini.items("response-delay"))
        
        self.response_status = RandomWeightedGenerator(type=int)
        self.response_status.addMulti(ini.items("response-status"))
                
        self.receipt_delay = RandomWeightedGenerator(type=float)
        self.receipt_delay.addMulti(ini.items("receipt-delay"))

        self.receipt_stat = RandomWeightedGenerator(type=str)
        self.receipt_stat.addMulti(ini.items("receipt-stat"))
        
        self.proto_mgr = ProtocolManager()
        
        self.message_id = SequenceGenerator(1, 0x7FFFFFFF)
        
        self.deliver_tasks = []
        self.deliver_interval = 1.0 
        
    def login(self, system_id, password):
        real_password = self.accounts.get(system_id)
        if real_password is None:
            return SMPP_STATUS_RBINDFAIL
        if password != real_password:
            return SMPP_STATUS_RBINDFAIL
        return SMPP_STATUS_ROK
        
    def registerTX(self, system_id, session):
        self.proto_mgr.register(system_id, session, "TX")
    
    def registerRX(self, system_id, session):
        self.proto_mgr.register(system_id, session, "RX")

    def registerTRX(self, system_id, session):
        self.proto_mgr.register(system_id, session, "TX", "RX")
            
    def getRX(self, system_id):
        return self.proto_mgr.get(system_id, "RX")
    
    def unregister(self, system_id, session):
        self.proto_mgr.unregister(system_id, session)
    
    def deliverSM(self, system_id, msg):
        proto = self.getRX(system_id)
        print "PROTO", proto
        if proto:
            proto.sendRequest(SMPP_DELIVER_SM_REQ, mandatory=msg, size=0)
        
    def stopFactory(self):
        for task in self.deliver_tasks:
            task.stop()

############################################################################
class DeliverTask(LoopingTask):
    def __init__(self, factory, system_id, rate, count, msg, interval):
        LoopingTask.__init__(self, factory.deliver_tasks)
        self.factory = factory
        self.system_id = system_id
        self.rate = rate
        self.count = count
        self.msg = msg
        self.done_count = 0
        self.active_time = self.start_time = reactor.seconds()
        self.start(interval)
        
    def start(self, interval):
        self.shaper = TokenBucketShaper(
            self.rate, 
            self.rate, 
            interval,
        )
        LoopingTask.start(self, interval)
        
    def run(self):
        self.shaper.update(reactor.seconds())
        while self.done_count < self.count and self.shaper.getAvailableSize() > 0:
            self.factory.deliverSM(
                self.system_id,
                self.msg,
            )
            self.shaper.consume(1)
            self.done_count += 1
        if self.done_count >= self.count:
            self.stop()
            

############################################################################

html = etree.fromstring("""<?xml version="1.0" encoding="UTF-8"?>
<html>
    <head>
        <title/>
        <meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
    </head>
    <body>
        <div id="page_header">
            [ <a href="/session/">会话列表</a> | 
              <a href="/deliver/">发送列表</a> |
              <a href="/send/">发送消息</a> ]
        </div>
        <div id="message_bar"/>
        <div id="page_body"/>
        <div id="page_footer" />
    </body>
</html>
""") 

############################################################################

def generate_page(tmpl):
    def _generate_page(func):
        @wraps(func)
        def _wrapper(*argv, **kw):
            def _set_content(elem, content):
                elem.clear()
                elem.text = ""
                if content is None:
                    return
                elif isinstance(content, (list, tuple)):
                    for c in content:
                        elem.append(c)
                elif isinstance(content, (str, unicode)):
                    elem.text = content
                else:
                   elem.append(content)  
                                   
            title, message, body = func(*argv, **kw)
            tmpl.find("head/title").text = title
            pb = tmpl.find("body")
            
            _set_content(pb[1], message)
            _set_content(pb[2], body)
            return etree.tostring(tmpl, "UTF-8")
        return _wrapper
    return _generate_page


############################################################################

def my_getattr(obj, name):
    if name:
        for i in name.split("."):
            obj = getattr(obj, i)
            if callable(obj):
                obj = obj()
    return obj


def makeButtons(form, buttons):
    global i
    for name, value in buttons:
        etree.SubElement(
            form, 
            "input", 
            type="submit",
            name=name,
            value=value,
        )

############################################################################
class BaseManagerResource(resource.Resource):
    isLeaf = True

    def getChild(self, path):
        return self

    def doRender(self, request, mgr):
        def _makeTH(table, tag):
            tr = etree.SubElement(etree.SubElement(table, tag), "tr")
            etree.SubElement(tr, "th")
            for name, title, func in self.fields:
                etree.SubElement(tr, "th").text = title
                
        form = etree.Element("form", action=self.form_action, method="post")
        makeButtons(form, self.form_buttons)

        table = etree.SubElement(form, "table", border="1")

        _makeTH(table, "thead")
        
        tbody = etree.SubElement(table, "tbody")
        for proto in sorted(mgr):
            pid = str(id(proto))
            tr = etree.SubElement(tbody, "tr", id=pid)
            etree.SubElement(
                etree.SubElement(tr, "td"), 
                "input", 
                type="checkbox", 
                name=pid,
            )
            for name, title, func in self.fields:
                etree.SubElement(tr, "td").text = func(my_getattr(proto, name))

        _makeTH(table, "tfoot")

        makeButtons(form, self.form_buttons)
        
        return form 

############################################################################
class DeliverManagerResource(BaseManagerResource):
    fields = (
        ("", u"ID", lambda o: str(id(o))),
        ("system_id", u"发送账号", str),
        ("start_time", u"开始时间", lambda t: strftime("%Y-%m-%d %H:%M:%S", localtime(t))),        
        ("rate", u"速度限制", str),
        ("count", u"发送总量", str),
        ("done_count", u"完成数量", str),
    )
    
    form_buttons = (
        ("cancel", u"取消发送"),
    )
    
    form_action = "/deliver/"
    
    def __init__(self, dm):
        self.dm = dm
    
    @generate_page(html)  
    def render_GET(self, request):
        return u"短息中心 - 投递任务列表", "", self.doRender(request, self.dm)
        
    @generate_page(html)
    def render_POST(self, request):
        messages = []
        for name, title in self.form_buttons:
            request.args.pop(name, None)
            
        for pid in request.args.keys():
            generator = getByID(self.dm, int(pid), None)
            msg = etree.Element("p")            
            if generator:
                self.dm.remove(generator)
                msg.text = u"任务 `%s` 已取消"%pid
            else:
                msg.text = u"找不到任务 `%s`"%pid
            messages.append(msg)
                
        return u"短息中心 - 投递任务列表", messages, self.doRender(request, self.dm)

############################################################################
class SessionManagerResource(BaseManagerResource):
    fields = (
        ("", u"ID", lambda o: str(id(o))),
        ("session_state", u"会话状态", str),        
        ("system_id", u"登录账号", str),
        ("transport.getPeer", u"客户端地址", lambda a: "%s:%d" % (a.host, a.port)),
        ("transport.getHost", u"服务端地址", lambda a: "%s:%d" % (a.host, a.port)),
        ("last_send_time", u"最后发送时间", lambda t: strftime("%Y-%m-%d %H:%M:%S", localtime(t))),
        ("last_recv_time", u"最后接收时间", lambda t: strftime("%Y-%m-%d %H:%M:%S", localtime(t))),
        ("send_counter", u"发送消息数", str),
        ("recv_counter", u"接收消息数", str),
    )
    
    form_buttons = (
        ("disconnect", u"断开连接"),
    )
    
    form_action = "/session/"

    def __init__(self, pm):
        self.pm = pm
        
    @generate_page(html)  
    def render_GET(self, request):
        return u"短息中心 - 会话列表", "", self.doRender(request, self.pm)
        
    @generate_page(html)
    def render_POST(self, request):
        for name, title in self.form_buttons:
            request.args.pop(name, None)
            
        messages = []
        for pid in request.args.keys():
            proto = self.pm.getByID(int(pid))
            msg = etree.Element("p")            
            if proto:
                proto.transport.loseConnection()
                msg.text = u"连接 `%s` 断线处理中"%pid
            else:
                msg.text = u"找不到连接  `%s`"%pid
            messages.append(msg)
                
        return u"短息中心 - 会话列表", messages, self.doRender(request, self.pm)

############################################################################
class FormResource(resource.Resource):
    isLeaf = True
    
    def getChild(self, path):
        return self
    
    def doRender(self, request):
        form = etree.Element("form", action=self.form_action, method="post")
        makeButtons(form, self.form_buttons)
        table = etree.SubElement(form, "table", border="1")
                
        for name, title, func, type, def_val, kw in self.fields:
            tr = etree.SubElement(table, "tr")
            th = etree.SubElement(tr, "th")
            label = etree.SubElement(th, "label")
            label.text = title
            td = etree.SubElement(tr, "td")
            td2 = etree.SubElement(tr, "td")
            val = request.args.get(name, [None])[0]
            if type == "text":
                etree.SubElement(
                    td, 
                    "input", 
                    kw, 
                    value=val or def_val, 
                    name=name, 
                    type="text",
                )
            elif type == "hidden":
                etree.SubElement(
                    td, 
                    "input", 
                    kw, 
                    value=val or def_val, 
                    name=name, 
                    type="text",
                    hidden="true",
                )                
            elif type == "textarea":
                etree.SubElement(
                    td, 
                    "textarea", 
                    kw,  
                    name=name,
                ).text = val or def_val or " "
            elif type == "select":
                select = etree.SubElement(
                    td,
                    "select",
                    kw,
                    name=name,
                )
                for name, value in def_val:
                    etree.SubElement(
                        select,
                        "option",
                        value=str(value),
                    ).text = "%s/0x%02X"%(name,value)
                select.getchildren()[0].attrib["selected"] = "true"
        
        makeButtons(form, self.form_buttons)
        
        return form

############################################################################
class DeliverFormResource(FormResource):
    TON_VALUES = (
        ("Unknown", 0),
        ("International", 0x1),
        ("National", 0x2),
        ("Network Specific", 0x3),
        ("Subscriber Number", 0x4),
        ("Alphanumeric", 0x5),
        ("Abbreviated", 0x6), 
    )
    
    NPI_VALUES = (
        ("Unknown", 0),
        ("ISDN (E163/E164)", 0x1),
        ("Data (X.121)", 0x3),
        ("Telex (F.69)", 0x4),
        ("Land Mobile (E.212)", 0x6),
        ("National", 0x8),
        ("Private", 0x9),
        ("ERMES", 0xA),
        ("Internet (IP)", 0xE),
        ("WAP Client Id", 0x12),
    )
    
    DATA_CODING_VALUES = (
        ("SMSC Default Alphabet", 0),
        ("IA5 (CCITT T.50)/ASCII (ANSI X3.4)", 1),
        ("Octet unspecified (8-bit binary)", 2),
        ("Latin 1 (ISO-8859-1)", 3),
        ("Octet unspecified (8-bit binary)", 4),
        ("JIS (X 0208-1990)", 5),
        ("Cyrllic (ISO-8859-5)", 6),
        ("Latin/Hebrew (ISO-8859-8)", 7),
        ("UCS2 (ISO/IEC-10646)", 8),
        ("Pictogram Encoding", 9),
        ("ISO-2022-JP (Music Codes)", 0xA),
        ("Extended Kanji JIS(X 0212-1990)", 0xD),
        ("KS C 5601", 0xE),
    )
    
    fields = (
        ("service_type", "Service Type", str, "text", "", {"maxlength":"5"}),
        ("src_addr_ton", "Source Address TON", int, "select", TON_VALUES, {}),
        ("src_addr_npi", "Source Address NPI", int, "select", NPI_VALUES, {}),
        ("source_addr", "Source Address", str, "text", "", {"maxlength":"20"}),
        ("dest_addr_ton", "Destination Address TON", int, "select", TON_VALUES, {}),
        ("dest_addr_npi", "Destination Address NPI", int, "select", NPI_VALUES, {}),
        ("destination_addr", "Destination Address", str, "text", "", {"maxlength":"20"}),
        ("esm_class", "ESM Class", int, "text", "0", {"maxlength":"4"}),
        ("protocol_id", "Protocol ID", int, "text", "0", {"maxlength":"4"}),
        ("priority_flag", "Priority Flag", int, "text", "0", {"maxlength":"4"}),
        ("schedule_delivery_time", "Schedule Delivery Time", str, "text", "", {"maxlength":"17"}),
        ("validity_period", "Validity Period", str, "text", "", {"maxlength":"17"}),
        ("register_delivery", "Registery Delivery", str, "text", "0", {"maxlength":"4"}),
        ("replace_if_present_flag", "Replace If Present Flag", int, "text", "0", {"maxlength":"4"}),
        ("data_coding", "Data Coding", int, "select", DATA_CODING_VALUES, {}),
        ("short_message", "Short Message", str, "textarea", "", {"cols":"40", "rows":"20"}),
        #("_short_message_type", u"输入消息类型", int, "select", [("NONE", 0),("HEX", 1), ("BASE64", 2)], {}),
        ("_system_id", u"发送账号", str, "text", "", {}),
        ("_count", u"发送数量", int, "text", "", {}),
        ("_rate", u"发送速度", float, "text", "100", {}),
    )
    
    form_buttons = (
        ("send", u"发送"),
    )
    
    form_action = "/send/"
    
    def __init__(self, factory, interval):
        self.factory = factory
        self.interval = interval
        
    @generate_page(html)
    def render_GET(self, request):
        return u"短信中心", "", self.doRender(request)
    
    @generate_page(html)
    def render_POST(self, request):
        msg = {}
        for name, title, func, type, def_val, kw in self.fields:
            try:
                msg[name] = func(request.args[name][0])
            except:
                msg[name] = func(def_val)
                    
        system_id = msg.pop("_system_id")
        count = msg.pop("_count")
        rate = msg.pop("_rate")
        
        task = DeliverTask(
            self.factory,
            system_id,
            rate,
            count,
            msg,
            self.interval,
        )
        
        return u"短息中心", u"创建成功", self.doRender(request)
    
############################################################################
class RootResource(resource.Resource):
    @generate_page(html)
    def render_GET(self, request):
        return u"短信中心", "", None

############################################################################
class Options(usage.Options):
    optParameters = (
        ("config-file", "c", "smpp_server.ini", "The config file to load"),
        ("smsc-port", "s", None, "SMSC(SMPP) listen address"),
        ("admin-port", "a", None, "HTTP admin listen address"),
    )
    

############################################################################
def makeService(options):
    my_service = service.MultiService()
    
    if not options.get("smsc-port"):
        print "`smsc-port` option NOT found!"
        raise KeyError
    
    smsc_factory = SMSCFactory(options["config-file"])
    smsc_service = strports.service(
        options["smsc-port"],
        smsc_factory,
    )
    smsc_service.setServiceParent(my_service)

    if options.get("admin-port"):
        root = resource.Resource()
        root.putChild("", RootResource())
        root.putChild(
            "session", 
            SessionManagerResource(
                smsc_factory.proto_mgr
            )
        )
        root.putChild(
            "deliver", 
            DeliverManagerResource(
                smsc_factory.deliver_tasks,
            )
        )
        root.putChild(
            "send", 
            DeliverFormResource(
                smsc_factory,
                smsc_factory.deliver_interval,                                 
            )
        )
        admin_service = strports.service(
            options["admin-port"],
            server.Site(root),
        )
        admin_service.setServiceParent(my_service)

    return my_service
