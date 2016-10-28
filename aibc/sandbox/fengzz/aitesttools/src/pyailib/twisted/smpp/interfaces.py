# -*- coding: UTF-8 -*-
from zope.interfaces import Interface


class IMCSessionManagement(Interface):
    def bindTransmitter(proto, seq, status, data):
        """
        4.1.1.1 bind_transmitter Syntax
        4.1.1.2 bind_transmitter_resp Syntax
        """ 

    def bindReceiver(proto, seq, status, data):
        """ 
        4.1.1.3 bind_receiver Syntax
        4.1.1.4 bind_receiver_resp Syntax
        """

    def bindTransceiver(proto, seq, status, data):
        """
        4.1.1.5 bind_transceiver Syntax
        4.1.1.6 bind_transceiver_resp Syntax
        """

    def unbind(proto, seq, status, data):
        """
        4.1.1.8 unbind Syntax
        4.1.1.9 unbind_resp Syntax
        """

    def enquireLink(proto, seq, status, data):
        """
        4.1.2.1 enquire_link Syntax
        4.1.2.2 enquire_link_resp Syntax
        """


class IESMESessionManagement(Interface):
    def outbind(seq, status, data):
        """
        4.1.1.7 outbind Syntax
        """

    def alertNotification(seq, status, data):
        """
        4.1.3.1 alert_notification Syntax
        """

    def unbind(seq, status, data):
        """
        4.1.1.8 unbind Syntax
        4.1.1.9 unbind_resp Syntax
        """

    def enquireLink(seq, status, data):
        """
        4.1.2 Enquire Link Operation
        """


class IMessageSubmition(Interface):
    def submitSM(seq, status, data):
        """
        4.2.1 submit_sm Operation
        """

    def dataSM(seq, status, data):
        """
        4.2.2 data_sm Operation
        """

    def submitMulti(seq, status, data):
        """
        4.2.3 submit_multi Operation
        """


class IMessageDelivery(Interface):
    def deliverSM(seq, status, data):
        """
        4.3.1 deliver_sm Operation
        """

    def dataSM(seq, status, data):
        """
        4.3.2 data_sm Operation
        """


class IMessageBroadcast(Interface):
    def broadcastSM(seq, status, data):
        """
        4.4.1 broadcast_sm Operation
        """


class IAncillarySubmission(Interface):
    def cancelSM(seq, status, data):
        """
        4.5.1 cancel_sm Operation
        """

    def querySM(seq, status, data):
        """
        4.5.2 query_sm Operation
        """

    def replaceSM(seq, status, data):
        """
        4.5.3 replace_sm Operation
        """


class IAncillaryBroadcast(Interface):
    def queryBroadcastSM(seq, status, data):
        """
        4.6.1 query_broadcast_sm Operation
        """

    def cancelBroadcastSM(seq, status, data):
        """
        4.6.2 cancel_broadcast_sm Operation
        """


