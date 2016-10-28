#include "anf/protocol/mm4/MM4HTTP.h"
#include "aaf/LogService.h"

ANF_NAMESPACE_START

namespace mm4_http{

static stTable const MessagePriorityTable[] = 
{
    {MESSAGE_PRIORITY_LOW,      "Low"   },
    {MESSAGE_PRIORITY_NORMAL,   "Normal"},
    {MESSAGE_PRIORITY_HIGH,     "High"  },
    {-1,                        "End"   }
};

static stTable const TokenTable[] = 
{
    {TYPE_YES,                  "Yes"   },
    {TYPE_NO,                   "No"    },
    {-1,                        "End"   }
};


static stTable const MessageClassTable[] =
{
    {MESSAGE_CLASS_PERSONAL,        "Personal"      },
    {MESSAGE_CLASS_ADVERTISEMENT,   "Advertisement" },
    {MESSAGE_CLASS_INFOMATIONAL,    "Informational" },
    {MESSAGE_CLASS_ATUO,            "Auto"          },
    {MESSAGE_CLASS_TEST,            "Test"          },
    {-1,                            "End"           }   
};

static stTable const VisibilityTable[] =
{
    {SENDER_VISIBILIT_HIDE, "Hide"  },
    {SENDER_VISIBILIT_SHOW, "Show"  },
    {-1,                    "End"   }   
};

static stTable const ReqStatusCodeTable[] =
{
    {REQ_STATUS_CODE_OK,                            "Ok"                                },
    {REQ_STATUS_CODE_UNSPECIFIED,                   "Error-unspecified"                 },
    {REQ_STATUS_CODE_SERVICE_DENIED,                "Error-service-denied"              },
    {REQ_STATUS_CODE_MESSAGE_FORMAT_CORRUPT,        "Error-message-format-corrupt"      },
    {REQ_STATUS_CODE_SENDING_ADDRESS_UNRESOLVED,    "Error-sending-address-unresolved"  },
    {REQ_STATUS_CODE_MESSAGE_NOT_FOUND,             "Error-message-not-found"           },
    {REQ_STATUS_CODE_NETWORK_PROBLEM,               "Error-network-problem"             },
    {REQ_STATUS_CODE_CONTENT_NOT_ACCEPTED,          "Error-content-not-accepted"        },
    {REQ_STATUS_CODE_UNSUPPORTED_MESSAGE,           "Error-unsupported-message"         },
    {REQ_STATUS_CODE_UNSUPPORTED_MESSAGE,           "Error-test-failed"                 },
    {-1,                                            "End"                               }   
};

static stTable const MmStatusCodeTable[] =
{
    {MM_STATUS_CODE_EXPIRED,            "Expired"           },  
    {MM_STATUS_CODE_RETRIEVED,          "Retrieved"         },  
    {MM_STATUS_CODE_REJECTED,           "Rejected"          },  
    {MM_STATUS_CODE_DEFERRED,           "Deferred"          },  
    {MM_STATUS_CODE_UNRECOGNIZED,       "Unrecognized"      },  
    {MM_STATUS_CODE_INDETERMINATE,      "Indeterminate"     },  
    {MM_STATUS_CODE_FORWARDED,          "Forwarded"         },  
    {MM_STATUS_CODE_FORWARDED_TO_MMBOX, "ForwardedToMMBox"  },
    {-1,                                "End"               }   
};

static stTable const MmStatusExtensionTable[] =
{
    {MM_STATUS_EXTENSION_REJECTED_BY_RECIPIENT,         "Rejected-by-recipient"         },  
    {MM_STATUS_EXTENSION_REJECTED_BY_OTHER_MMSC,        "Rejected-by-other-MMSC"        },
    {MM_STATUS_EXTENSION_REJECTED_BY_VAS_AREA_LIMIT,    "Rejected-by-VAS-area-limit"    },
    {-1,                                                "End"                           }   
};


static stTable const ReadStatusTable[] =
{
    {READ_STATUS_READ,                          "Read"                      },
    {READ_STATUS_DELETED_WITHOUT_BEING_READ,    "Deletedwithoutbeingread"   },
    {-1,                                        "End"                       }   
};

static stTable const ContentTypeTable[] =
{
    {0x00, "*/*"                                        }, 
    {0x01, "text/*"                                     },
    {0x02, "text/html"                                  },
    {0x03, "text/plain"                                 },
    {0x04, "text/x-hdml"                                },
    {0x05, "text/x-ttml"                                },
    {0x06, "text/x-vCalendar"                           },
    {0x07, "text/x-vCard"                               },
    {0x08, "text/vnd.wap.wml"                           },
    {0x09, "text/vnd.wap.wmlscript"                     },
    {0x0A, "text/vnd.wap.wta-event"                     },
    {0x0B, "multipart/*"                                },
    {0x0C, "multipart/mixed"                            },
    {0x0D, "multipart/form-data"                        },
    {0x0E, "multipart/byterantes"                       },
    {0x0F, "multipart/alternative"                      },
    {0x10, "application/*"                              },
    {0x11, "application/java-vm"                        },
    {0x12, "application/x-www-form-urlencoded"          },
    {0x13, "application/x-hdmlc"                        },
    {0x14, "application/vnd.wap.wmlc"                   },
    {0x15, "application/vnd.wap.wmlscriptc"             },
    {0x16, "application/vnd.wap.wta-eventc"             },
    {0x17, "application/vnd.wap.uaprof"                 },
    {0x18, "application/vnd.wap.wtls-ca-certificate"    },
    {0x19, "application/vnd.wap.wtls-user-certificate"  },
    {0x1A, "application/x-x509-ca-cert"                 },
    {0x1B, "application/x-x509-user-cert"               },
    {0x1C, "image/*"                                    },
    {0x1D, "image/gif"                                  },
    {0x1E, "image/jpeg"                                 },
    {0x1F, "image/tiff"                                 },
    {0x20, "image/png"                                  },
    {0x21, "image/vnd.wap.wbmp"                         },
    {0x22, "application/vnd.wap.multipart.*"            },
    {0x23, "application/vnd.wap.multipart.mixed"        },
    {0x24, "application/vnd.wap.multipart.form-data"    },
    {0x25, "application/vnd.wap.multipart.byteranges"   },
    {0x26, "application/vnd.wap.multipart.alternative"  },
    {0x27, "application/xml"                            },
    {0x28, "text/xml"                                   },
    {0x29, "application/vnd.wap.wbxml"                  },
    {0x2A, "application/x-x968-cross-cert"              },
    {0x2B, "application/x-x968-ca-cert"                 },
    {0x2C, "application/x-x968-user-cert"               },
    {0x2D, "text/vnd.wap.si"                            },
    {0x2E, "application/vnd.wap.sic"                    },
    {0x2F, "text/vnd.wap.sl"                            },
    {0x30, "application/vnd.wap.slc"                    },
    {0x31, "text/vnd.wap.co"                            },
    {0x32, "application/vnd.wap.coc"                    },
    {0x33, "application/vnd.wap.multipart.related"      },
    {0x34, "application/vnd.wap.sia"                    },
    {0x35, "text/vnd.wap.connectivity-xml"              },
    {0x36, "application/vnd.wap.connectivity-wbxml"     },
    {0x37, "application/pkcs7-mime"                     },
    {0x38, "application/vnd.wap.hashed-certificate"     },
    {0x39, "application/vnd.wap.signed-certificate"     },
    {0x3A, "application/vnd.wap.cert-response"          },
    {0x3B, "application/xhtml+xml"                      },
    {0x3C, "application/wml+xml"                        },
    {0x3D, "text/css"                                   },
    {0x3E, "application/vnd.wap.mms-message"            },
    {0x3F, "application/vnd.wap.rollover-certificate"   },
    {0x40, "application/vnd.wap.locc+wbxml"             },
    {0x41, "application/vnd.wap.loc+xml"                },
    {0x42, "application/vnd.syncml.dm+wbxml"            },
    {0x43, "application/vnd.syncml.dm+xml"              },
    {0x44, "application/vnd.syncml.notification"        },
    {0x45, "application/vnd.wap.xhtml+xml"              },
    {0x46, "application/vnd.wv.csp.cir"                 },
    {0x47, "application/vnd.oma.dd+xml"                 },
    {0x48, "application/vnd.oma.drm.message"            },
    {0x49, "application/vnd.oma.drm.content"            },
    {0x4A, "application/vnd.oma.drm.rights+xml"         },
    {0x4B, "application/vnd.oma.drm.rights+wbxml"       },
    {-1,   "End"                                        }   
};

static stTable const ContentTypeParamTable[] =
{
    {0x01,  "charset"       },
    {0x09,  "type"          },
    {0x03,  "type"          },
    {0x19,  "start"         }, //v1.4  
    {0x0A,  "start"         }, //v1.2
    {0x17,  "name"          }, //v1.4
    {0x05,  "name"          }, //v1.1
    {0x18,  "filename"      }, //v1.4
    {0x06,  "filename"      }, //v1.1
    {0x1A,  "start-info"    }, //v1.4
    {0x0B,  "start-info"    }, //v1.2
    {-1,    "End"           }   
};

static stTable const CharSetTable[] =
{
    {3,    "us-ascii"           },
    {4,    "iso-8859-1"         },
    {5,    "iso-8859-2"         },
    {6,    "iso-8859-3"         },
    {7,    "iso-8859-4"         },
    {8,    "iso-8859-5"         },
    {9,    "iso-8859-6"         },
    {10,   "iso-8859-7"         },
    {11,   "iso-8859-8"         },
    {12,   "iso-8859-9"         },
    {17,   "Shift_JIS"          },
    {106,  "utf-8"              },
    {113,  "GBK"                },
    {114,  "GB18030"            },
    {1000, "iso-10646-ucs-2"    },
    {2025, "GB2312"             }, 
    {2026, "big5"               },
    {-1,   "End"                }   
};

static stTable const MultipartHeaderTable[] =
{
    {0x0E,    "Content-Location"            },
    {0x27,    "Content-Transfer-Encoding"   },
    {0x40,    "Content-ID"                  },
    {-1,      "End"                         }   
};

/*
MM4Message
*/
CMM4Message::CMM4Message()
{}

CMM4Message::~CMM4Message()
{}

/*
MM4_capability_negotiation.REQ
*/
CMM4CapabilityNegotiationREQ::CMM4CapabilityNegotiationREQ()
{
    moMessageType = "MM4_capability_negotiation.REQ";
    miMessageType = MESSAGE_TYPE_CAPABILIT_REQ;
}

CMM4CapabilityNegotiationREQ::~CMM4CapabilityNegotiationREQ()
{}

apl_ssize_t CMM4CapabilityNegotiationREQ::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    RETURN_ERR_IF((this->moTransactionID.empty()), MM4_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((Get(TokenTable, this->moReplyChargingSupporting.c_str()) < 0), MM4_ERROR_REPLY_CHARGING_SUPPORTING);
    RETURN_ERR_IF((Get(TokenTable, this->moReadReportSupporting.c_str()) < 0), MM4_ERROR_READ_CHARGING_SUPPORTING);
    
    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_CAPABILIT_REQ);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //ReplyChargingSupporting: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_RERLY_CHARGING_SUPPORT, Get(TokenTable, this->moReplyChargingSupporting.c_str()));
    
    //ReadReportSupporting: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_PDU_READ_REPORT_SUPPORT, Get(TokenTable, this->moReadReportSupporting.c_str()));

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4CapabilityNegotiationREQ::Decode(std::list<stField> aoFieldList)
{   
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
                
            case MM4_X_MMS_RERLY_CHARGING_SUPPORT:
                this->moReplyChargingSupporting = loIter->moValue;
                break;
                
            case MM4_X_MMS_PDU_READ_REPORT_SUPPORT:
                this->moReadReportSupporting = loIter->moValue;
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}

void CMM4CapabilityNegotiationREQ::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }

    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4CapabilityNegotiationREQ/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",               this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",           this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",             this->moTransactionID.c_str(),
        "X-Mms-Reply-Charging-Support",     this->moReplyChargingSupporting.c_str(),
        "X-Mms-PDU-Read-Report-Support",    this->moReadReportSupporting.c_str()
        );
    
    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}

/*
MM4_capability_negotiation.RES
*/
CMM4CapabilityNegotiationRES::CMM4CapabilityNegotiationRES()
{
    moMessageType = "MM4_capability_negotiation.RES";
    miMessageType = MESSAGE_TYPE_CAPABILIT_RES;
}

CMM4CapabilityNegotiationRES::~CMM4CapabilityNegotiationRES()
{}

apl_ssize_t CMM4CapabilityNegotiationRES::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    RETURN_ERR_IF((this->moTransactionID.empty()), MM4_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((Get(TokenTable, this->moReplyChargingSupporting.c_str()) < 0), MM4_ERROR_REPLY_CHARGING_SUPPORTING);
    RETURN_ERR_IF((Get(TokenTable, this->moReadReportSupporting.c_str()) < 0), MM4_ERROR_READ_CHARGING_SUPPORTING);

    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_CAPABILIT_RES);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //ReplyChargingSupporting: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_RERLY_CHARGING_SUPPORT, Get(TokenTable, this->moReplyChargingSupporting.c_str()));
    
    //ReadReportSupporting: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_PDU_READ_REPORT_SUPPORT, Get(TokenTable, this->moReadReportSupporting.c_str()));

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4CapabilityNegotiationRES::Decode(std::list<stField> aoFieldList)
{   
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
                
            case MM4_X_MMS_RERLY_CHARGING_SUPPORT:
                this->moReplyChargingSupporting = loIter->moValue;
                break;
                
            case MM4_X_MMS_PDU_READ_REPORT_SUPPORT:
                this->moReadReportSupporting = loIter->moValue;
                break;
                
            default:
                break;
        }
    }

    return 0;
}

void CMM4CapabilityNegotiationRES::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }

    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4CapabilityNegotiationRES/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",               this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",           this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",             this->moTransactionID.c_str(),
        "X-Mms-Reply-Charging-Support",     this->moReplyChargingSupporting.c_str(),
        "X-Mms-PDU-Read-Report-Support",    this->moReadReportSupporting.c_str()
        );
    
    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}

/*
MM4_Forward.REQ
*/
CMM4ForwardREQ::CMM4ForwardREQ()
{
    moMessageType = "MM4_forward.REQ";
    miMessageType = MESSAGE_TYPE_FORWARD_REQ;
}

CMM4ForwardREQ::~CMM4ForwardREQ()
{}

apl_ssize_t CMM4ForwardREQ::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),              MM4_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                  MM4_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRecipientAddress.empty()),           MM4_ERROR_TO);
    RETURN_ERR_IF((this->moSenderAddress.empty()),              MM4_ERROR_FROM);
    RETURN_ERR_IF((this->moContentType.empty()),                MM4_ERROR_CONTENT_TYPE);
    RETURN_ERR_IF((this->moDate.empty()),                       MM4_ERROR_DATE);
    RETURN_ERR_IF((this->moOriginatorSystem.empty()),           MM4_ERROR_ORIGINATOR_SYSTEM);
    RETURN_ERR_IF((this->moRcptTo.empty()),                     MM4_ERROR_RCPT_To);

    aoMemoryBlock.Resize(2*1024+this->moContent.size());

    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_FORWARD_REQ);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //MessageID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_MESSAGE_ID, this->moMessageID.c_str());

    //RecipientAddress: Encoded-string-value
    if(apl_strchr(this->moRecipientAddress.c_str(), ',') != NULL)
    {
        acl::CSpliter loSpliter(",");
        apl_uint32_t lu32Num = loSpliter.Parse(this->moRecipientAddress);
        for(apl_uint32_t lu32Index = 0; lu32Index < lu32Num; lu32Index++)
        {
            EncodeEncodeString(aoMemoryBlock, MM4_TO, -1, loSpliter.GetField(lu32Index));
        }
    }
    else if(apl_strchr(this->moRecipientAddress.c_str(), ';') != NULL)
    {
        acl::CSpliter loSpliter(";");
        apl_uint32_t lu32Num = loSpliter.Parse(this->moRecipientAddress);
        for(apl_uint32_t lu32Index = 0; lu32Index < lu32Num; lu32Index++)
        {
            EncodeEncodeString(aoMemoryBlock, MM4_TO, -1, loSpliter.GetField(lu32Index));
        }
    }
    else
    {
        EncodeEncodeString(aoMemoryBlock, MM4_TO, -1, this->moRecipientAddress.c_str());
    }
    
    //SenderAddress: Encoded-string-value
    //From field: Value-length (Address-present-token Encoded-string-value | Insert-address-token )
    //EncodeFrom(aoMemoryBlock, MM4_FROM, this->moSenderAddress.c_str());
    EncodeEncodeString(aoMemoryBlock, MM4_FROM, -1, this->moSenderAddress.c_str());

    //MessageClass: Short-integer | Token-text
    if(!this->moMessageClass.empty())
    {
        apl_int32_t li32MessageClass = Get(MessageClassTable, this->moMessageClass.c_str());
        if(li32MessageClass >= 0)
        {
            EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_CLASS, (apl_uint8_t)li32MessageClass);
        }
        else
        {
            EncodeTextString(aoMemoryBlock, MM4_X_MMS_MESSAGE_CLASS, this->moMessageClass.c_str());
        }
    }

    //Date: Long-integer
    if(!this->moDate.empty())
    {
        EncodeLongint(aoMemoryBlock, MM4_DATE, apl_strtou32(this->moDate.c_str(), NULL, 10));
    }

    //Expiry: Long-integer
    if(!this->moExpiry.empty())
    {
        EncodeLongint(aoMemoryBlock, MM4_X_MMS_EXPIRY, apl_strtou32(this->moExpiry.c_str(), NULL, 10));
    }

    //DeliveryReport: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_DELIVERY_REPORT, TYPE_YES);

    //OriginatorDeliveryReport: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_ORIGINATOR_RS_DELIVERY_REPORT, TYPE_YES);

    //Priority: Short-integer
    if(!this->moPriority.empty())
    {
        apl_int32_t li32Priority = Get(MessagePriorityTable, this->moPriority.c_str());
        if(li32Priority >= 0)
        {
            EncodeShortint(aoMemoryBlock, MM4_X_MMS_PRIORITY, (apl_uint8_t)li32Priority);
        }
    }

    //SenderVisibility: Short-integer
    if(!this->moSenderVisibility.empty())
    {
        apl_int32_t li32SenderVisibility = Get(VisibilityTable, this->moSenderVisibility.c_str());
        if(li32SenderVisibility >= 0)
        {
            EncodeShortint(aoMemoryBlock, MM4_X_MMS_SENDER_VISIBILITY, (apl_uint8_t)li32SenderVisibility);
        }
    }

    //ReadReply: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_READ_REPLY, TYPE_NO);

    //Subject: Encoded-string-value
    std::string loCharSet;
    std::string loSubject;
    if(ParseCharSetText(this->moSubject, loCharSet, loSubject) == 0)
    {
        apl_int32_t li32Charset = Get(CharSetTable, loCharSet.c_str());
        if( li32Charset >= 0 )
        {
            EncodeEncodeString(aoMemoryBlock, MM4_SUBJECT, li32Charset, loSubject.c_str());
        }
        else
        {
            EncodeEncodeString(aoMemoryBlock, MM4_SUBJECT, loCharSet.c_str(), loSubject.c_str());
        }
    }
    else
    {
        EncodeEncodeString(aoMemoryBlock, MM4_SUBJECT, -1, this->moSubject.c_str());
    }
    
    //AckRequest: Short-integer
    if(!this->moAckRequest.empty())
    {
        apl_int32_t li32AckRequest = Get(TokenTable, this->moAckRequest.c_str());
        if(li32AckRequest >= 0)
        {
            EncodeShortint(aoMemoryBlock, MM4_X_MMS_ACK_REQUEST, (apl_uint8_t)li32AckRequest);
        }
    }

    //ForwardCounter: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_FORWARD_COUNTER, 1);

    //PreviouslySentBy
    //PreviouslySentDateAndTime
    if(!this->moPreviouslySentBy.empty())
    {
        EncodePreviouslySentBy(aoMemoryBlock, MM4_X_MMS_PREVIOUSLY_SENT_BY, this->moPreviouslySentBy.c_str());
        EncodePreviouslySentDateAndTime(aoMemoryBlock, MM4_X_MMS_PREVIOUSLY_SENT_DATE_AND_TIME);
    }

    //VASPID: Text-string
    if(!this->moVASPID.empty())
    {
        EncodeTextString(aoMemoryBlock, MM4_X_MMS_VASP_ID, this->moVASPID.c_str());
    }

    //VASID: Text-string
    if(!this->moVASID.empty())
    {   
        EncodeTextString(aoMemoryBlock, MM4_X_MMS_VAS_ID, this->moVASID.c_str());
    }
   
    //ServiceCode: Text-string
    if(!this->moServiceCode.empty())
    {   
        EncodeTextString(aoMemoryBlock, MM4_X_MMS_SERVICE_CODE, this->moServiceCode.c_str());
    }
   
    //ChargedParty: Text-string
    if(!this->moChargedPartyID.empty())
    {   
        EncodeTextString(aoMemoryBlock, MM4_X_MMS_CHARGED_PARTY_ID, this->moChargedPartyID.c_str());
    } 
  
    //LinkedID: Text-string
    if(!this->moLinkedID.empty())
    {
        EncodeTextString(aoMemoryBlock, MM4_X_MMS_LINKED_ID, this->moLinkedID.c_str());
    }
   
    //FeeType: Short-integer
    if(!this->moFeeType.empty())
    {
        EncodeShortint(aoMemoryBlock, MM4_X_MMS_FEE_TYPE, (apl_uint8_t)apl_strtou32(this->moFeeType.c_str(), NULL, 10));
    }

    //FeeCode: Integer-value
    if(!this->moFeeCode.empty())
    {
        EncodeInt(aoMemoryBlock, MM4_X_MMS_FEE_CODE, (apl_int32_t)apl_strtou32(this->moFeeCode.c_str(), NULL, 10));
    }

    //OriginatorSystem: Encoded-string-value
    //From field: Value-length (Address-present-token Encoded-string-value | Insert-address-token )
    //EncodeFrom(aoMemoryBlock, MM4_X_MMS_ORIGINATOR_SYSTEM, this->moOriginatorSystem.c_str());
    EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_ORIGINATOR_SYSTEM, -1, this->moOriginatorSystem.c_str());

    //RcptTo: Encoded-string-value
    if(apl_strchr(this->moRcptTo.c_str(), ',') != NULL)
    {
        acl::CSpliter loSpliter(",");
        apl_uint32_t lu32Num = loSpliter.Parse(this->moRcptTo);
        for(apl_uint32_t lu32Index = 0; lu32Index < lu32Num; lu32Index++)
        {
            EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_RCPT_TO, -1, loSpliter.GetField(lu32Index));
        }
    }
    else if(apl_strchr(this->moRcptTo.c_str(), ';') != NULL)
    {
        acl::CSpliter loSpliter(";");
        apl_uint32_t lu32Num = loSpliter.Parse(this->moRcptTo);
        for(apl_uint32_t lu32Index = 0; lu32Index < lu32Num; lu32Index++)
        {
            EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_RCPT_TO, -1, loSpliter.GetField(lu32Index));
        }
    }
    else
    {
        EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_RCPT_TO, -1, this->moRcptTo.c_str());
    }

    //last field: ContentType: Content-Type-value
    RETURN_ERR_IF((EncodeContentType(aoMemoryBlock, MM4_CONTENT_TYPE, this->moContentType) < 0), -1)

    //Content
    std::string loBoundary;
    if(this->moContentType.find("multipart") != std::string::npos && IsMultipart(this->moContentType, loBoundary)) //application/vnd.wap.multipart
    {
        std::list<stMIMEEntity> loMIMEEntity;

        RETURN_ERR_IF((ParseMultipart(this->moContent, loBoundary.c_str(), loMIMEEntity) != 0), -1);
        RETURN_ERR_IF((loMIMEEntity.empty()), -1);

        EncodeMultipart(aoMemoryBlock, loMIMEEntity);
    }
    else
    {       
        RESIZE_IF(aoMemoryBlock, this->moContent.size()); //memoryblock resize if need
        aoMemoryBlock.Write(this->moContent.data(), this->moContent.size());
    }

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ForwardREQ::Decode(std::list<stField> aoFieldList)
{   
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;

            case MM4_TO:
            case MM4_CC:
            case MM4_BCC:
                if(!this->moRecipientAddress.empty())
                {
                    this->moRecipientAddress += ",";
                }
                this->moRecipientAddress += loIter->moValue;
                break;
            
            case MM4_FROM:
                this->moSenderAddress = loIter->moValue;
                break;
            
            case MM4_CONTENT_TYPE:
                this->moContentType = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_CLASS:
                this->moMessageClass = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moDate = loIter->moValue;
                break;
            
            case MM4_X_MMS_EXPIRY:
                this->moExpiry = loIter->moValue;
                break;
            
            case MM4_X_MMS_DELIVERY_REPORT:
                this->moDeliveryReport = loIter->moValue;
                break;
            
            case MM4_X_MMS_ORIGINATOR_RS_DELIVERY_REPORT:
                this->moOriginatorDeliveryReport = loIter->moValue;
                break;
            
            case MM4_X_MMS_PRIORITY:
                this->moPriority = loIter->moValue;
                break;
            
            case MM4_X_MMS_SENDER_VISIBILITY:
                this->moSenderVisibility = loIter->moValue;
                break;
            
            case MM4_X_MMS_READ_REPLY:
                this->moReadReply = loIter->moValue;
                break;
            
            case MM4_SUBJECT:
                this->moSubject = loIter->moValue;
                break;
                
            case MM4_CONTENT:
                this->moContent = loIter->moValue;
                break;
            
            case MM4_X_MMS_ACK_REQUEST:
                this->moAckRequest = loIter->moValue;
                break;
            
            case MM4_X_MMS_FORWARD_COUNTER:
                this->moForwardCounter = loIter->moValue;
                break;
            
            case MM4_X_MMS_PREVIOUSLY_SENT_BY:
                this->moPreviouslySentBy = loIter->moValue;
                break;
            
            case MM4_X_MMS_PREVIOUSLY_SENT_DATE_AND_TIME:
                this->moPreviouslySentDateAndTime = loIter->moValue;
                break;
                        
            case MM4_X_MMS_VASP_ID:
                this->moVASPID = loIter->moValue;
                break;
            
            case MM4_X_MMS_VAS_ID:
                this->moVASID = loIter->moValue;
                break;
            
            case MM4_X_MMS_SERVICE_CODE:
                this->moServiceCode = loIter->moValue;
                break;
            
            case MM4_X_MMS_CHARGED_PARTY_ID:
                this->moChargedPartyID = loIter->moValue;
                break;
                        
            case MM4_X_MMS_LINKED_ID:
                this->moLinkedID = loIter->moValue;
                break;
            
            case MM4_X_MMS_FEE_TYPE:
                this->moFeeType = loIter->moValue;
                break;
            
            case MM4_X_MMS_FEE_CODE:
                this->moFeeCode = loIter->moValue;
                break;
            
            case MM4_X_MMS_ORIGINATOR_SYSTEM:
                this->moOriginatorSystem = loIter->moValue;
                break;
            
            case MM4_X_MMS_RCPT_TO:
                if(!this->moRcptTo.empty())
                {
                    this->moRcptTo += ",";
                }
                this->moRcptTo += loIter->moValue;
                break;
                
            default:
                break;
        }
    }

    return 0;
}

void CMM4ForwardREQ::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ForwardREQ/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "To",                                   this->moRecipientAddress.c_str(),
        "From",                                 this->moSenderAddress.c_str(),
        "X-Mms-Message-Class",                  this->moMessageClass.c_str(),
        "Date",                                 this->moDate.c_str(),
        "X-Mms-Expiry",                         this->moExpiry.c_str(),
        "X-Mms-Delivery-Report",                this->moDeliveryReport.c_str(),
        "X-Mms-Originator-R/S-Delivery-Report", this->moOriginatorDeliveryReport.c_str(),
        "X-Mms-Priority",                       this->moPriority.c_str(),
        "X-Mms-Sender-Visibility",              this->moSenderVisibility.c_str(),
        "X-Mms-Read-Reply",                     this->moReadReply.c_str(),
        "X-Mms-Ack-Request",                    this->moAckRequest.c_str(),
        "X-Mms-Forward-Counter",                this->moForwardCounter.c_str(),
        "X-Mms-Previously-sent-by",             this->moPreviouslySentBy.c_str(),
        "X-Mms-Previously-sent-date-and-time",  this->moPreviouslySentDateAndTime.c_str(),
        "X-Mms-VASP-ID",                        this->moVASPID.c_str(),
        "X-Mms-VAS-ID",                         this->moVASID.c_str(),
        "X-Mms-Service-Code",                   this->moServiceCode.c_str(),
        "X-Mms-Charged-Party-ID",               this->moChargedPartyID.c_str(),
        "X-Mms-Linked-ID",                      this->moLinkedID.c_str(),
        "X-Mms-Fee-Type",                       this->moFeeType.c_str(),
        "X-Mms-Fee-Code",                       this->moFeeCode.c_str(),
        "X-Mms-Originator-System",              this->moOriginatorSystem.c_str(),
        "X-Mms-Rcpt-To",                        this->moRcptTo.c_str(),
        "Content-Type",                         this->moContentType.c_str(),
        "Subject",                              this->moSubject.c_str()
        );
    
    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Forward.RES
*/
CMM4ForwardRES::CMM4ForwardRES()
{
    moMessageType = "MM4_forward.RES";
    miMessageType = MESSAGE_TYPE_FORWARD_RES;
}

CMM4ForwardRES::~CMM4ForwardRES()
{}

apl_ssize_t CMM4ForwardRES::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),              MM4_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                  MM4_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((Get(ReqStatusCodeTable, this->moRequestStatusCode.c_str()) < 0), MM4_ERROR_REQUEST_STATUS_CODE);
    
    aoMemoryBlock.Resize(1024);

    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_FORWARD_RES);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //MessageID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_MESSAGE_ID, this->moMessageID.c_str());

    //RequestStatusCode: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_REQUEST_STATUS_CODE, Get(ReqStatusCodeTable, this->moRequestStatusCode.c_str()));

    //StatusText: Encoded-string-value
    if(!this->moStatusText.empty())
    {
        std::string loCharSet;
        std::string loStatusText;
        if(ParseCharSetText(this->moStatusText, loCharSet, loStatusText) == 0)
        {
            apl_int32_t li32Charset = Get(CharSetTable, loCharSet.c_str());
            if( li32Charset >= 0 )
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, li32Charset, loStatusText.c_str());
            }
            else
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, loCharSet.c_str(), loStatusText.c_str());
            }
        }
        else
        {
            EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, -1, this->moStatusText.c_str());
        }
    }

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ForwardRES::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
            
            case MM4_X_MMS_REQUEST_STATUS_CODE:
                this->moRequestStatusCode = loIter->moValue;
                break;
            
            case MM4_X_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;
                
            default:
                break;
        }
    }

    return 0;
}

void CMM4ForwardRES::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ForwardRES/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "X-Mms-Request-Status-Code",            this->moRequestStatusCode.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Delivery_report.REQ
*/
CMM4DeliveryReportREQ::CMM4DeliveryReportREQ()
{
    moMessageType = "MM4_delivery_report.REQ";
    miMessageType = MESSAGE_TYPE_DELIVERY_REPORT_REQ;
}

CMM4DeliveryReportREQ::~CMM4DeliveryReportREQ()
{}

apl_ssize_t CMM4DeliveryReportREQ::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),              MM4_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                  MM4_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRecipientAddress.empty()),           MM4_ERROR_TO);
    RETURN_ERR_IF((this->moSenderAddress.empty()),              MM4_ERROR_FROM);
    RETURN_ERR_IF((this->moDate.empty()),                       MM4_ERROR_DATE);
    RETURN_ERR_IF((Get(TokenTable, this->moForwardToOriginatorUA.c_str()) < 0), MM4_ERROR_FORWARD_TO_ORIGINATOR_UA);
    RETURN_ERR_IF((Get(MmStatusCodeTable, this->moMMStatusCode.c_str()) < 0), MM4_ERROR_MM_STATUS_CODE);

    aoMemoryBlock.Resize(1024);

    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_DELIVERY_REPORT_REQ);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //MessageID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_MESSAGE_ID, this->moMessageID.c_str());

    //RecipientAddress: Encoded-string-value
    EncodeEncodeString(aoMemoryBlock, MM4_TO, -1, this->moRecipientAddress.c_str());
    
    //SenderAddress: Encoded-string-value
    //From field: Value-length (Address-present-token Encoded-string-value | Insert-address-token )
    //EncodeFrom(aoMemoryBlock, MM4_FROM, this->moSenderAddress.c_str());
    EncodeEncodeString(aoMemoryBlock, MM4_FROM, -1, this->moSenderAddress.c_str());

    //Date: Long-integer
    EncodeLongint(aoMemoryBlock, MM4_DATE, apl_strtou32(this->moDate.c_str(), NULL, 10));

    //AckRequest: Short-integer
    if(!this->moAckRequest.empty())
    {
        EncodeShortint(aoMemoryBlock, MM4_X_MMS_ACK_REQUEST, (apl_uint8_t)apl_strtou32(this->moAckRequest.c_str(), NULL, 10));
    }

    //ForwardToOriginatorUA: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_FORWARD_TO_ORIGINATOR_UA, Get(TokenTable, this->moForwardToOriginatorUA.c_str()));

    //MMStatusCode: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MM_STATUS_CODE, Get(MmStatusCodeTable, this->moMMStatusCode.c_str()));

    //MMStatusExtension: Short-integer
    if(!this->moMMStatusExtension.empty())
    {
        EncodeShortint(aoMemoryBlock, MM4_X_MMS_MM_STATUS_EXTENSION, Get(MmStatusExtensionTable, this->moMMStatusExtension.c_str()));
    }

    //StatusText: Encoded-string-value
    if(!this->moStatusText.empty())
    {
        std::string loCharSet;
        std::string loStatusText;
        if(ParseCharSetText(this->moStatusText, loCharSet, loStatusText) == 0)
        {
            apl_int32_t li32Charset = Get(CharSetTable, loCharSet.c_str());
            if( li32Charset >= 0 )
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, li32Charset, loStatusText.c_str());
            }
            else
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, loCharSet.c_str(), loStatusText.c_str());
            }
        }
        else
        {
            EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, -1, this->moStatusText.c_str());
        }
    }

    return aoMemoryBlock.GetLength();

}

apl_ssize_t CMM4DeliveryReportREQ::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
                        
            case MM4_FROM:
                this->moSenderAddress = loIter->moValue;
                break;
            
            case MM4_TO:
            case MM4_CC:
            case MM4_BCC:
                this->moRecipientAddress = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moDate = loIter->moValue;
                break;
            
            case MM4_X_MMS_ACK_REQUEST:
                this->moAckRequest = loIter->moValue;
                break;
            
            case MM4_X_MMS_FORWARD_TO_ORIGINATOR_UA:
                this->moForwardToOriginatorUA = loIter->moValue;
                break;
            
            case MM4_X_MMS_MM_STATUS_CODE:
                this->moMMStatusCode = loIter->moValue;
                break;
            
            case MM4_X_MMS_MM_STATUS_EXTENSION:
                this->moMMStatusExtension = loIter->moValue;
                break;

            case MM4_X_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;
            
            default:
                break;
        }
    }

    return 0;
}

void CMM4DeliveryReportREQ::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4DeliveryReportREQ/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "To",                                   this->moRecipientAddress.c_str(),
        "From",                                 this->moSenderAddress.c_str(),
        "Date",                                 this->moDate.c_str(),
        "X-Mms-Ack-Request",                    this->moAckRequest.c_str(),
        "X-Mms-Forward-To-Originator-UA",       this->moForwardToOriginatorUA.c_str(),
        "X-Mms-MM-Status-Code",                 this->moMMStatusCode.c_str(),
        "X-Mms-MM-Status-Extension",            this->moMMStatusExtension.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Delivery_report.RES
*/
CMM4DeliveryReportRES::CMM4DeliveryReportRES()
{
    moMessageType = "MM4_delivery_report.RES";
    miMessageType = MESSAGE_TYPE_DELIVERY_REPORT_RES;
}

CMM4DeliveryReportRES::~CMM4DeliveryReportRES()
{}

apl_ssize_t CMM4DeliveryReportRES::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),              MM4_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                  MM4_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((Get(ReqStatusCodeTable, this->moRequestStatusCode.c_str()) < 0), MM4_ERROR_REQUEST_STATUS_CODE);
    
    aoMemoryBlock.Resize(1024);

    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_DELIVERY_REPORT_RES);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //MessageID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_MESSAGE_ID, this->moMessageID.c_str());

    //RequestStatusCode: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_REQUEST_STATUS_CODE, Get(ReqStatusCodeTable, this->moRequestStatusCode.c_str()));

    //StatusText: Encoded-string-value
    if(!this->moStatusText.empty())
    {
        std::string loCharSet;
        std::string loStatusText;
        if(ParseCharSetText(this->moStatusText, loCharSet, loStatusText) == 0)
        {
            apl_int32_t li32Charset = Get(CharSetTable, loCharSet.c_str());
            if( li32Charset >= 0 )
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, li32Charset, loStatusText.c_str());
            }
            else
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, loCharSet.c_str(), loStatusText.c_str());
            }
        }
        else
        {
            EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, -1, this->moStatusText.c_str());
        }
    }

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4DeliveryReportRES::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
                        
            case MM4_X_MMS_REQUEST_STATUS_CODE:
                this->moRequestStatusCode = loIter->moValue;
                break;
            
            case MM4_X_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}

void CMM4DeliveryReportRES::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4DeliveryReportRES/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "X-Mms-Request-Status-Code",            this->moRequestStatusCode.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Read_reply_report.REQ
*/
CMM4ReadReplyReportREQ::CMM4ReadReplyReportREQ()
{
    moMessageType = "MM4_read_reply_report.REQ";
    miMessageType = MESSAGE_TYPE_READ_REPLY_REPORT_REQ;
}

CMM4ReadReplyReportREQ::~CMM4ReadReplyReportREQ()
{}

apl_ssize_t CMM4ReadReplyReportREQ::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),              MM4_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                  MM4_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRecipientAddress.empty()),           MM4_ERROR_TO);
    RETURN_ERR_IF((this->moSenderAddress.empty()),              MM4_ERROR_FROM);
    RETURN_ERR_IF((this->moDate.empty()),                       MM4_ERROR_DATE);
    RETURN_ERR_IF((Get(ReadStatusTable, this->moReadStatus.c_str()) < 0), MM4_ERROR_REQUEST_STATUS_CODE);

    aoMemoryBlock.Resize(1024);

    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_READ_REPLY_REPORT_REQ);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //MessageID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_MESSAGE_ID, this->moMessageID.c_str());

    //RecipientAddress: Encoded-string-value
    EncodeEncodeString(aoMemoryBlock, MM4_TO, -1, this->moRecipientAddress.c_str());
    
    //SenderAddress: Encoded-string-value
    //From field: Value-length (Address-present-token Encoded-string-value | Insert-address-token )
    //EncodeFrom(aoMemoryBlock, MM4_FROM, this->moSenderAddress.c_str());
    EncodeEncodeString(aoMemoryBlock, MM4_FROM, -1, this->moSenderAddress.c_str());

    //Date: Long-integer
    EncodeLongint(aoMemoryBlock, MM4_DATE, apl_strtou32(this->moDate.c_str(), NULL, 10));

    //AckRequest: Short-integer
    if(!this->moAckRequest.empty())
    {
        EncodeShortint(aoMemoryBlock, MM4_X_MMS_ACK_REQUEST, (apl_uint8_t)apl_strtou32(this->moAckRequest.c_str(), NULL, 10));
    }

    //ReadStatus: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_READ_STATUS, Get(ReadStatusTable, this->moReadStatus.c_str()));

    //StatusText: Encoded-string-value
    if(!this->moStatusText.empty())
    {
        std::string loCharSet;
        std::string loStatusText;
        if(ParseCharSetText(this->moStatusText, loCharSet, loStatusText) == 0)
        {
            apl_int32_t li32Charset = Get(CharSetTable, loCharSet.c_str());
            if( li32Charset >= 0 )
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, li32Charset, loStatusText.c_str());
            }
            else
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, loCharSet.c_str(), loStatusText.c_str());
            }
        }
        else
        {
            EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, -1, this->moStatusText.c_str());
        }
    }

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ReadReplyReportREQ::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
                                    
            case MM4_FROM:
                this->moSenderAddress = loIter->moValue;
                break;
            
            case MM4_TO:
            case MM4_CC:
            case MM4_BCC:
                this->moRecipientAddress = loIter->moValue;
                break;
            
            case MM4_X_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moDate = loIter->moValue;
                break;
                        
            case MM4_X_MMS_ACK_REQUEST:
                this->moAckRequest = loIter->moValue;
                break;
            
            case MM4_X_MMS_READ_STATUS:
                this->moReadStatus = loIter->moValue;
                break;
            
            case MM4_X_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;
                
            default:
                break;
        }
    }

    return 0;
}

void CMM4ReadReplyReportREQ::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ReadReplyReportREQ/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "To",                                   this->moRecipientAddress.c_str(),
        "From",                                 this->moSenderAddress.c_str(),
        "Date",                                 this->moDate.c_str(),
        "X-Mms-Ack-Request",                    this->moAckRequest.c_str(),
        "X-Mms-Read-Status",                    this->moReadStatus.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Read_reply_report.RES
*/
CMM4ReadReplyReportRES::CMM4ReadReplyReportRES()
{
    moMessageType = "MM4_read_reply_report.RES";
    miMessageType = MESSAGE_TYPE_READ_REPLY_REPORT_RES;
}

CMM4ReadReplyReportRES::~CMM4ReadReplyReportRES()
{}

apl_ssize_t CMM4ReadReplyReportRES::Encode(acl::CMemoryBlock& aoMemoryBlock) const
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),              MM4_ERROR_TRANSACTION_ID);  
    RETURN_ERR_IF((Get(ReqStatusCodeTable, this->moRequestStatusCode.c_str()) < 0), MM4_ERROR_REQUEST_STATUS_CODE);
    
    aoMemoryBlock.Resize(1024);

    //1. MessageType: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_MESSAGE_TYPE, MESSAGE_TYPE_READ_REPLY_REPORT_RES);

    //2. TransactionID: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_TRANSACTION_ID, this->moTransactionID.c_str());

    //3. MMSVersion: Text-string
    EncodeTextString(aoMemoryBlock, MM4_X_MMS_VERSION, MMS_VERSION);

    //RequestStatusCode: Short-integer
    EncodeShortint(aoMemoryBlock, MM4_X_MMS_REQUEST_STATUS_CODE, Get(ReqStatusCodeTable, this->moRequestStatusCode.c_str()));

    //StatusText: Encoded-string-value
    if(!this->moStatusText.empty())
    {
        std::string loCharSet;
        std::string loStatusText;
        if(ParseCharSetText(this->moStatusText, loCharSet, loStatusText) == 0)
        {
            apl_int32_t li32Charset = Get(CharSetTable, loCharSet.c_str());
            if( li32Charset >= 0 )
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, li32Charset, loStatusText.c_str());
            }
            else
            {
                EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, loCharSet.c_str(), loStatusText.c_str());
            }
        }
        else
        {
            EncodeEncodeString(aoMemoryBlock, MM4_X_MMS_STATUS_TEXT, -1, this->moStatusText.c_str());
        }
    }

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ReadReplyReportRES::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(loIter->miName)
        {
            case MM4_X_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_X_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_X_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
                        
            case MM4_X_MMS_REQUEST_STATUS_CODE:
                this->moRequestStatusCode = loIter->moValue;
                break;
            
            case MM4_X_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;

            default:
                break;
        }
    }

    return 0;
}

void CMM4ReadReplyReportRES::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ReadReplyReportRES/HTTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Request-Status-Code",            this->moRequestStatusCode.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}

CMM4Message* DecodeMessage(void* const apcPtr, const apl_size_t aiLen)
{
    RETURN_ERR_IF((aiLen <= 0), NULL);

    acl::CMemoryBlock   loMemoryBlock(aiLen);
    CMM4Message*        lpoMessage = NULL;
    std::list<stField>  loFieldList;
    apl_uint32_t        lu32MessageType = 0;
    apl_ssize_t         liErrorCode = 0;
    apl_uint8_t         lu8ErrField = 0;

    loMemoryBlock.Write(apcPtr, aiLen);

    if((liErrorCode = ParseMessage(loMemoryBlock, lu32MessageType, loFieldList, lu8ErrField)) != 0)
    {
        apl_errprintf("mm4/http error decode, field = 0x%02x(%s), err = %d\n", lu8ErrField, GetFieldName(lu8ErrField), liErrorCode);
        return NULL;
    }

    switch(lu32MessageType)
    {
        case MESSAGE_TYPE_CAPABILIT_REQ: 
            ACL_NEW_ASSERT(lpoMessage, CMM4CapabilityNegotiationREQ);
            break;

        case MESSAGE_TYPE_CAPABILIT_RES: 
            ACL_NEW_ASSERT(lpoMessage, CMM4CapabilityNegotiationRES);
            break;
            
        case MESSAGE_TYPE_FORWARD_REQ: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ForwardREQ);
            break;
        
        case MESSAGE_TYPE_FORWARD_RES: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ForwardRES); 
            break;
        
        case MESSAGE_TYPE_DELIVERY_REPORT_REQ: 
            ACL_NEW_ASSERT(lpoMessage, CMM4DeliveryReportREQ); 
            break;
        
        case MESSAGE_TYPE_DELIVERY_REPORT_RES: 
            ACL_NEW_ASSERT(lpoMessage, CMM4DeliveryReportRES); 
            break;
        
        case MESSAGE_TYPE_READ_REPLY_REPORT_REQ: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ReadReplyReportREQ); 
            break;
        
        case MESSAGE_TYPE_READ_REPLY_REPORT_RES: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ReadReplyReportRES); 
            break;
    
        default:
            return NULL;
    }
    
    if((liErrorCode = lpoMessage->Decode(loFieldList)) != 0)
    {
        apl_errprintf("mm4/http field error decode, err = %d\n", liErrorCode);
        ACL_DELETE(lpoMessage);
        return NULL;
    }

    return lpoMessage;
}

apl_ssize_t ParseMessage(acl::CMemoryBlock& aoMemoryBlock, apl_uint32_t &au32MessageType, std::list<stField> &aoFieldList, apl_uint8_t& au8ErrField)
{
    apl_uint8_t lu8Byte = 0;

    while(aoMemoryBlock.GetLength() > 0)
    {
        stField loField;
        
        lu8Byte = (apl_uint8_t)*aoMemoryBlock.GetReadPtr();
        aoMemoryBlock.SetReadPtr(1); //skip name

        RETURN_ERR_IF((lu8Byte < 0x80), MM4_DECODE_ERROR_NAME);

        //well_known_field: name-value
        {
            apl_uint32_t lu32Type = 0;
            apl_uint32_t lu32Value = 0;
            std::string  loCharSet;
            std::string  loTmp;
            char lsTmpBuf[128] = {0};
            
            loField.miName = lu8Byte&0x7F;

            au8ErrField = (apl_uint8_t)loField.miName;
            
            lu32Type = DecodeTypeValue(aoMemoryBlock, lu32Value);
            RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);

            if(lu32Type == VALUE_TYPE_NULL_END_STRING)
            {
                RETURN_ERR_IF((DecodeNullEndString(aoMemoryBlock, loField.moValue) <= 0), MM4_DECODE_ERROR_NULL_STRING);
            }
                        
            switch(loField.miName)
            {
                //Encoded-string-value
                case MM4_FROM:
                case MM4_X_MMS_ORIGINATOR_SYSTEM:
                case MM4_X_MMS_RCPT_TO:
                case MM4_TO:
                case MM4_BCC:
                case MM4_CC:
                case MM4_SUBJECT:
                case MM4_X_MMS_STATUS_TEXT:
                    if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
                    {
                        RETURN_ERR_IF((DecodeEncodeString(aoMemoryBlock, lu32Type, lu32Value, loCharSet, loField.moValue) <= 0), MM4_DECODE_ERROR_ENCODE_STRING);
                        if(!loCharSet.empty())
                        {
                            acl::CBase64Encoder loBase64Encoder;
                            if(loBase64Encoder.Final(loField.moValue.data(), loField.moValue.size()) == 0)
                            {
                                loField.moValue.assign(loBase64Encoder.GetOutput(), loBase64Encoder.GetLength());

                                loTmp += "=?";
                                loTmp += loCharSet;
                                loTmp += "?B?"; //use base64
                                loTmp += loField.moValue;
                                loTmp += "?=";
                                
                                loField.moValue = loTmp;
                            }
                        }
                    }
                    break;
                
                //Short-integer
                case MM4_X_MMS_MESSAGE_TYPE:
                    if(lu32Value != MESSAGE_TYPE_CAPABILIT_REQ && lu32Value != MESSAGE_TYPE_CAPABILIT_RES
                        && lu32Value != MESSAGE_TYPE_FORWARD_REQ && lu32Value != MESSAGE_TYPE_FORWARD_RES
                        && lu32Value != MESSAGE_TYPE_DELIVERY_REPORT_REQ && lu32Value != MESSAGE_TYPE_DELIVERY_REPORT_RES
                        && lu32Value != MESSAGE_TYPE_READ_REPLY_REPORT_REQ && lu32Value != MESSAGE_TYPE_READ_REPLY_REPORT_RES)
                    {
                        return MM4_DECODE_ERROR_MESSAGE_TYPE;
                    }
                    au32MessageType = lu32Value;
                    break;
                case MM4_X_MMS_PRIORITY:
                    if(lu32Type == VALUE_TYPE_SHORT_INT) loField.moValue = Get(MessagePriorityTable, (apl_int8_t)lu32Value);
                    break;
                case MM4_X_MMS_REQUEST_STATUS_CODE:
                    if(lu32Type == VALUE_TYPE_SHORT_INT) loField.moValue = Get(ReqStatusCodeTable, (apl_int8_t)lu32Value);
                    break;
                case MM4_X_MMS_SENDER_VISIBILITY:
                    if(lu32Type == VALUE_TYPE_SHORT_INT) loField.moValue = Get(VisibilityTable, (apl_int8_t)lu32Value);
                    break;
                case MM4_X_MMS_MM_STATUS_CODE:
                    if(lu32Type == VALUE_TYPE_SHORT_INT) loField.moValue = Get(MmStatusCodeTable, (apl_int8_t)lu32Value);
                    break;
                case MM4_X_MMS_READ_STATUS:
                    if(lu32Type == VALUE_TYPE_SHORT_INT) loField.moValue = Get(ReadStatusTable, (apl_int8_t)lu32Value);
                    break;
                case MM4_X_MMS_MM_STATUS_EXTENSION:
                    if(lu32Type == VALUE_TYPE_SHORT_INT) loField.moValue = Get(MmStatusExtensionTable, (apl_int8_t)lu32Value);
                    break;
                case MM4_X_MMS_DELIVERY_REPORT:
                case MM4_X_MMS_ORIGINATOR_RS_DELIVERY_REPORT:
                case MM4_X_MMS_READ_REPLY:
                case MM4_X_MMS_FORWARD_TO_ORIGINATOR_UA:
                case MM4_X_MMS_RERLY_CHARGING_SUPPORT:
                case MM4_X_MMS_PDU_READ_REPORT_SUPPORT:
                case MM4_X_MMS_ACK_REQUEST:
                    if(lu32Type == VALUE_TYPE_SHORT_INT) loField.moValue = Get(TokenTable, (apl_int8_t)lu32Value);
                    break;
                case MM4_X_MMS_FEE_TYPE:
                case MM4_X_MMS_FORWARD_COUNTER:
                    apl_snprintf(lsTmpBuf, sizeof(lsTmpBuf), "%"APL_PRIu32, lu32Value);
                    loField.moValue = std::string(lsTmpBuf);
                    break;
            
                //Long-integer
                case MM4_DATE:
                case MM4_X_MMS_EXPIRY:
                    RETURN_ERR_IF((aoMemoryBlock.GetLength() < lu32Value), MM4_DECODE_ERROR_LONG_INT_LENGTH);
                    apl_snprintf(lsTmpBuf, sizeof(lsTmpBuf), "%"APL_PRIu32, DecodeLongint(aoMemoryBlock, lu32Value));
                    loField.moValue = std::string(lsTmpBuf);
                    break;

                //integer
                case MM4_X_MMS_FEE_CODE:
                    if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
                    {
                        RETURN_ERR_IF((aoMemoryBlock.GetLength() < lu32Value), MM4_DECODE_ERROR_LONG_INT_LENGTH);
                    }
                    apl_snprintf(lsTmpBuf, sizeof(lsTmpBuf), "%"APL_PRIu32, DecodeInt(aoMemoryBlock, lu32Type, lu32Value));
                    loField.moValue = std::string(lsTmpBuf);
                    break;
            
                //Text-string
                case MM4_X_MMS_TRANSACTION_ID:
                case MM4_X_MMS_VERSION:
                case MM4_X_MMS_MESSAGE_ID:
                //case MM4_X_MMS_STATUS_TEXT:
                case MM4_X_MMS_SERVICE_CODE:
                case MM4_X_MMS_CHARGED_PARTY_ID:
                case MM4_X_MMS_VASP_ID:
                case MM4_X_MMS_VAS_ID:
                case MM4_X_MMS_LINKED_ID:
                    RETURN_ERR_IF((lu32Type != VALUE_TYPE_NULL_END_STRING), MM4_DECODE_ERROR_NOT_NULL_STRING);
                    break;
            
                //Content-Type-Value
                case MM4_CONTENT_TYPE:
                    if(lu32Value > 0)
                    { 
                        RETURN_ERR_IF((DecodeContentType(aoMemoryBlock, lu32Type, lu32Value, loField.moValue) < 0), MM4_DECODE_ERROR_CONTENT_TYPE);
                        if(loField.moValue.find("multipart") != std::string::npos)
                        {
                            loField.moValue += "; boundary=\"";
                            loField.moValue += BOUNDARY_NEXT_PART;
                            loField.moValue += "\"";
                        }
                    }
                    break;
            
                //From-value
                //case MM4_FROM:
                //case MM4_X_MMS_ORIGINATOR_SYSTEM:
                //    RETURN_ERR_IF((DecodeFrom(aoMemoryBlock, lu32Value, loField.moValue) <= 0), MM4_DECODE_ERROR_FROM);
                //    break;
                    
                case MM4_X_MMS_MESSAGE_CLASS:
                    if(lu32Type == VALUE_TYPE_SHORT_INT)
                    {
                        loField.moValue = Get(MessageClassTable, (apl_int8_t)lu32Value);
                        break;
                    }
                    RETURN_ERR_IF((lu32Type != VALUE_TYPE_NULL_END_STRING), MM4_DECODE_ERROR_NOT_NULL_STRING);
                    break;
    
                case MM4_X_MMS_PREVIOUSLY_SENT_BY:
                    RETURN_ERR_IF((DecodePreviouslySentBy(aoMemoryBlock, lu32Type, lu32Value, loField.moValue) < 0), MM4_DECODE_ERROR_PREVIOUSLY_SENT_BY);
                    break;
                    
                case MM4_X_MMS_PREVIOUSLY_SENT_DATE_AND_TIME:
                    RETURN_ERR_IF((DecodePreviouslySentDateAndTime(aoMemoryBlock, lu32Type, lu32Value, loField.moValue) < 0), MM4_DECODE_ERROR_PREVIOUSLY_SENT_DATE_AND_TIME);
                    break;
            
                default:
                    switch(lu32Type)
                    {
                        case VALUE_TYPE_OCTETS_WITH_LENGTH:
                            RETURN_ERR_IF((aoMemoryBlock.GetLength() < lu32Value), MM4_DECODE_ERROR_LONG_INT_LENGTH);
                            aoMemoryBlock.SetReadPtr(lu32Value); //unknow tag, so skip
                            break;
                        default:
                            break;
                    }
                    break;
            }
            aoFieldList.push_back(loField);
            
            //printf("Parse: Name = %d, Value = %s\n", loField.miName, loField.moValue.c_str());

            if(au32MessageType == MESSAGE_TYPE_FORWARD_REQ && loField.miName == MM4_CONTENT_TYPE) //break
            {
                au8ErrField = (apl_uint8_t)MM4_CONTENT;

                RETURN_ERR_IF((aoMemoryBlock.GetLength() <= 0), MM4_DECODE_ERROR_CONTENT);

                acl::CMemoryBlock loMemoryBlock;
                loMemoryBlock.Resize(aoMemoryBlock.GetLength());
                loMemoryBlock.Write(aoMemoryBlock.GetReadPtr(), aoMemoryBlock.GetLength());
                aoMemoryBlock.SetReadPtr(aoMemoryBlock.GetLength());

                if(loField.moValue.find("multipart") != std::string::npos) //application/vnd.wap.multipart
                {
                    loField.moValue = "";
                    RETURN_ERR_IF((DecodeMultipart(loMemoryBlock, loField.moValue) < 0), MM4_DECODE_ERROR_CONTENT);
                }
                else
                {
                    loField.moValue.assign(loMemoryBlock.GetReadPtr(), loMemoryBlock.GetLength());
                }

                loField.miName = MM4_CONTENT;
                aoFieldList.push_back(loField);

                break;
            }
        }
    }
    
    return 0;
}


/*ENCODE*/
apl_ssize_t EncodeShortint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Value)
{
    apl_uint8_t lu8Value = au8Value | 0x80;

    RESIZE_IF(aoMemoryBlock, 1); //memoryblock resize if need
    return aoMemoryBlock.Write(&lu8Value, 1);
}

apl_ssize_t EncodeFieldName(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Value)
{
    return EncodeShortint(aoMemoryBlock, au8Value);
}

apl_ssize_t EncodeShortint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint8_t au8Value)
{
    apl_ssize_t liRet = 0;
    liRet += EncodeFieldName(aoMemoryBlock, au8Name);
    liRet += EncodeShortint(aoMemoryBlock, au8Value);

    return liRet;   
}

apl_ssize_t EncodeLongint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value)
{
    apl_uint32_t    lu32Value = au32Value;
    unsigned char   lsOctet[4] = {0};
    apl_uint32_t    lu32Length;
    apl_uint8_t     lu8Length = 1;
    apl_ssize_t     liRet = 0;

    if(lu32Value == 0)
    {
        RESIZE_IF(aoMemoryBlock, 2); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(&lu8Length, 1);
        liRet += aoMemoryBlock.Write(lsOctet, 1);
        return liRet;
    }

    for(lu32Length = 0; lu32Value != 0; lu32Value >>= 8, lu32Length++)
    {   
        lsOctet[lu32Length] = lu32Value & 0xFF;
    }   
    lu8Length = (apl_uint8_t)lu32Length;

    RESIZE_IF(aoMemoryBlock, 1+lu32Length); //memoryblock resize if need
    liRet += aoMemoryBlock.Write(&lu8Length, 1); 

    for(; lu32Length > 0; lu32Length--)
    {
        liRet += aoMemoryBlock.Write(lsOctet+lu32Length-1, 1);
    }

    return liRet;
}

apl_ssize_t EncodeLongint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value)
{
    apl_ssize_t liRet = 0;
    liRet += EncodeFieldName(aoMemoryBlock, au8Name);
    liRet += EncodeLongint(aoMemoryBlock, au32Value);

    return liRet;
}

apl_ssize_t EncodeInt(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value)
{
    if(au32Value <= 127)
    {
        return EncodeShortint(aoMemoryBlock, (apl_uint8_t)au32Value);
    }

    return EncodeLongint(aoMemoryBlock, au32Value);
}

apl_ssize_t EncodeInt(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value)
{
    apl_ssize_t liRet = 0;
    liRet += EncodeFieldName(aoMemoryBlock, au8Name);
    liRet += EncodeInt(aoMemoryBlock, au32Value);

    return liRet;
}

apl_ssize_t EncodeUintvar(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value)
{
    apl_uint32_t lu32Value = au32Value;
    apl_uint32_t lu32Index;
    apl_uint32_t lu32Offset;
    unsigned char lsOctets[5];

    lsOctets[4] = lu32Value & 0x7F;
    lu32Value >>= 7;

    for(lu32Index = 3; lu32Value > 0 && lu32Index >= 0; lu32Index--)
    {
        lsOctets[lu32Index] = 0x80 | (lu32Value & 0x7F);
        lu32Value >>= 7;
    }
    lu32Offset = lu32Index + 1;

    RESIZE_IF(aoMemoryBlock, 5-lu32Offset); //memoryblock resize if need
    return aoMemoryBlock.Write(lsOctets+lu32Offset, 5-lu32Offset);
}

apl_ssize_t EncodeUintvar(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value)
{
    apl_ssize_t liRet = 0;
    liRet += EncodeFieldName(aoMemoryBlock, au8Name);
    liRet += EncodeUintvar(aoMemoryBlock, au32Value);

    return liRet;
}

apl_ssize_t EncodeValueLength(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value)
{
    apl_uint8_t lu8Value;
    apl_ssize_t liRet = 0;

    if(au32Value <= 30)
    {
        lu8Value = (apl_uint8_t)au32Value;
        RESIZE_IF(aoMemoryBlock, 1); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(&lu8Value, 1);
    }
    else
    {
        lu8Value = (apl_uint8_t)31;
        RESIZE_IF(aoMemoryBlock, 1); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(&lu8Value, 1);
        liRet += EncodeUintvar(aoMemoryBlock, au32Value);
    }

    return liRet;
}

apl_ssize_t EncodeValueLength(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value)
{
    apl_ssize_t liRet = 0;
    liRet += EncodeFieldName(aoMemoryBlock, au8Name);
    liRet += EncodeValueLength(aoMemoryBlock, au32Value);

    return liRet;
}

apl_ssize_t EncodeTextString(acl::CMemoryBlock& aoMemoryBlock, const char* apcText, const apl_size_t aiSize)
{
    apl_uint8_t lu8Quote = 127;
    apl_ssize_t liRet = 0;

    if((apl_uint8_t)apcText[0] < 32 || (apl_uint8_t)apcText[0] >= 128) //bin
    {
        RESIZE_IF(aoMemoryBlock, 1); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(&lu8Quote, 1);
    }

    if(aiSize == 0)
    {
        RESIZE_IF(aoMemoryBlock, apl_strlen(apcText)+1); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(apcText);
    }
    else
    {
        RESIZE_IF(aoMemoryBlock, aiSize); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(apcText, aiSize);
    }

    return liRet;
}

apl_ssize_t EncodeTextString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcText, const apl_size_t aiSize)
{
    apl_ssize_t liRet = 0;
    liRet += EncodeFieldName(aoMemoryBlock, au8Name);

    if(aiSize == 0)
    {
        liRet += EncodeTextString(aoMemoryBlock, apcText);
    }
    else
    {
        liRet += EncodeTextString(aoMemoryBlock, apcText, aiSize);
    }

    return liRet;
}

apl_ssize_t EncodeQuoteString(acl::CMemoryBlock& aoMemoryBlock, const char* apcText, const apl_size_t aiSize)
{
    apl_uint8_t lu8Quote = '"';
    apl_ssize_t liRet = 0;

    RESIZE_IF(aoMemoryBlock, 1); //memoryblock resize if need
    liRet += aoMemoryBlock.Write(&lu8Quote, 1);

    if(aiSize == 0)
    {
        RESIZE_IF(aoMemoryBlock, apl_strlen(apcText)+1); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(apcText);
    }
    else
    {
        RESIZE_IF(aoMemoryBlock, aiSize); //memoryblock resize if need
        liRet += aoMemoryBlock.Write(apcText, aiSize);
    }

    return liRet;
}

apl_ssize_t EncodeQuoteString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcText, const apl_size_t aiSize)
{
    apl_ssize_t liRet = 0;
    liRet += EncodeFieldName(aoMemoryBlock, au8Name);

    if(aiSize == 0)
    {   
        liRet += EncodeQuoteString(aoMemoryBlock, apcText);
    }
    else
    {
        liRet += EncodeQuoteString(aoMemoryBlock, apcText, aiSize);
    }

    return liRet;
}

apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_int_t aiCharSet, const char* apcText)
{
    apl_ssize_t liRet = 0;

    liRet += EncodeFieldName(aoMemoryBlock, au8Name);//name

    liRet += EncodeEncodeString(aoMemoryBlock, aiCharSet, apcText);
    
    return liRet;
}

apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcCharSet, const char* apcText)
{
    apl_ssize_t liRet = 0;

    liRet += EncodeFieldName(aoMemoryBlock, au8Name);//name

    liRet += EncodeEncodeString(aoMemoryBlock, apcCharSet, apcText);
    
    return liRet;
}

//Encoded-string-value = Text-string | Value-length Char-set Text-string
apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiCharSet, const char* apcText)
{
    apl_ssize_t liRet = 0;
    
    if(aiCharSet < 0)
    {
        return EncodeTextString(aoMemoryBlock, apcText);
    }

    acl::CMemoryBlock loCharSetMB(5);
    apl_uint32_t lu32Length = EncodeInt(loCharSetMB, (apl_uint32_t)aiCharSet);

    acl::CMemoryBlock loTextMB(128);
    lu32Length += EncodeTextString(loTextMB, apcText);
    RESIZE_IF(aoMemoryBlock, lu32Length); //memoryblock resize if need

    //Value-length
    liRet += EncodeValueLength(aoMemoryBlock, lu32Length);
    //Char-set 
    aoMemoryBlock.Write(loCharSetMB.GetReadPtr(), loCharSetMB.GetLength());
    liRet += loCharSetMB.GetLength();
    //Text-string
    aoMemoryBlock.Write(loTextMB.GetReadPtr(), loTextMB.GetLength());
    liRet += loTextMB.GetLength();

    return liRet;
}

//Encoded-string-value = Text-string | Value-length Char-set Text-string
apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const char* apcCharSet, const char* apcText)
{
    apl_ssize_t liRet = 0;
    
    if(apl_strlen(apcCharSet) <= 0)
    {
        return EncodeTextString(aoMemoryBlock, apcText);
    }

    acl::CMemoryBlock loCharSet(128);
    apl_uint32_t lu32Length = EncodeTextString(loCharSet, apcCharSet);

    acl::CMemoryBlock loTextMB(128);
    lu32Length += EncodeTextString(loTextMB, apcText);
    RESIZE_IF(aoMemoryBlock, lu32Length); //memoryblock resize if need

    //Value-length
    liRet += EncodeValueLength(aoMemoryBlock, lu32Length);
    //Char-set 
    aoMemoryBlock.Write(loCharSet.GetReadPtr(), loCharSet.GetLength());
    liRet += loCharSet.GetLength();
    //Text-string
    aoMemoryBlock.Write(loTextMB.GetReadPtr(), loTextMB.GetLength());
    liRet += loTextMB.GetLength();

    return liRet;
}

//Previously-sent-by-value = Value-length Forwarded-count-value Encoded-string-value
//Forwarded-count-value = Integer-value
apl_ssize_t EncodePreviouslySentBy(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcPreviouslySentBy)
{
    apl_ssize_t liRet = 0;

    acl::CMemoryBlock loMemoryBlock(1024);

    //Forwarded-count-value
    EncodeInt(loMemoryBlock, 1); //set Forwarded-count 1

    //Encoded-string-value
    EncodeEncodeString(loMemoryBlock, -1, apcPreviouslySentBy);

    liRet += EncodeFieldName(aoMemoryBlock, au8Name);//name

    liRet += EncodeValueLength(aoMemoryBlock, loMemoryBlock.GetLength());

    RESIZE_IF(aoMemoryBlock, loMemoryBlock.GetLength()); //memoryblock resize if need
    aoMemoryBlock.Write(loMemoryBlock.GetReadPtr(), loMemoryBlock.GetLength());

    liRet += loMemoryBlock.GetLength();

    return liRet;
}

//Previously-sent-date-value = Value-length Forwarded-count-value Date-value
apl_ssize_t EncodePreviouslySentDateAndTime(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name)
{
    apl_ssize_t liRet = 0;

    acl::CMemoryBlock loMemoryBlock(1024);

    //Forwarded-count-value
    EncodeInt(loMemoryBlock, 1); //set Forwarded-count 1

    //Date-value
    EncodeLongint(loMemoryBlock, (apl_uint32_t)(apl_time()/APL_TIME_SEC));

    liRet += EncodeFieldName(aoMemoryBlock, au8Name);//name

    liRet += EncodeValueLength(aoMemoryBlock, loMemoryBlock.GetLength());

    RESIZE_IF(aoMemoryBlock, loMemoryBlock.GetLength()); //memoryblock resize if need
    aoMemoryBlock.Write(loMemoryBlock.GetReadPtr(), loMemoryBlock.GetLength());

    liRet += loMemoryBlock.GetLength();

    return liRet;
}

apl_ssize_t EncodeFrom(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcFrom)
{
    apl_uint8_t lu8Token = 128;
    apl_uint32_t lu32Length = 2 + apl_strlen(apcFrom);
    apl_ssize_t liRet = 0;

    liRet += EncodeFieldName(aoMemoryBlock, au8Name);
    liRet += EncodeValueLength(aoMemoryBlock, lu32Length);
    liRet += aoMemoryBlock.Write(&lu8Token, 1);
    liRet += EncodeTextString(aoMemoryBlock, apcFrom);

    return liRet;
}

//Content-type-value = Constrained-media | Content-general-form
//Content-general-form = Value-length Media-type
//Media-type = (Well-known-media | Extension-Media) *(Parameter)
apl_ssize_t EncodeContentType(acl::CMemoryBlock& aoMemoryBlock, std::string const& aoContentType)
{
    std::string loType;
    std::list<stHeaderField> loParamList;
    apl_ssize_t liRet = 0;
    acl::CMemoryBlock loMemoryBlock(1024);

    ParseContentType(aoContentType, loType, loParamList);

    RETURN_ERR_IF((loType.empty()), -1);
	
    apl_uint32_t lu32Pos = 0;		
    if((lu32Pos = loType.find("multipart/")) != (apl_uint32_t)std::string::npos)
    {
        loType.replace(lu32Pos, apl_strlen("multipart/"), "application/vnd.wap.multipart.");
    }

    //type/subtype
    apl_int32_t li32Type = Get(ContentTypeTable, loType.c_str());
    if( li32Type >= 0 )
    {
        liRet += EncodeInt(loMemoryBlock, (apl_uint32_t)li32Type);
    }
    else
    {
        liRet += EncodeTextString(loMemoryBlock,loType.c_str());
    }

    // Constrained-media
    if( loParamList.empty() )
    {
        return aoMemoryBlock.Write(loMemoryBlock.GetReadPtr(), loMemoryBlock.GetLength());
    }
	
    //Content-general-form
    //if( !loParamList.empty() )
    {
        //param
        std::list<stHeaderField>::iterator loIter = loParamList.begin();
        for(; loIter != loParamList.end(); loIter++)
        {
            if(apl_strcasecmp(loIter->moName.c_str(), "boundary") == 0)
            {
                continue;
            }
            
            apl_int32_t li32Name = Get(ContentTypeParamTable, loIter->moName.c_str());
            if( li32Name < 0 )
            {
                EncodeTextString(loMemoryBlock, loIter->moName.c_str());
                EncodeTextString(loMemoryBlock, loIter->moValue.c_str());
            }
            else //Well-known-parameter
            {
                apl_int32_t li32Charset;
                apl_int32_t li32Type;
                
                switch(li32Name)
                {
                    case 0x01: //charset
                        li32Charset = Get(CharSetTable, loIter->moValue.c_str());
                        if( li32Charset >= 0 )
                        {
                            EncodeInt(loMemoryBlock, li32Name, (apl_uint32_t)li32Charset);
                        }
                        else
                        {
                            EncodeTextString(loMemoryBlock, li32Name, loIter->moValue.c_str());
                        }
                        break;
                    
                    case 0x09: //type
                        if((lu32Pos = loIter->moValue.find("multipart/")) != (apl_uint32_t)std::string::npos)
                        {
                            loIter->moValue.replace(lu32Pos, apl_strlen("multipart/"), "application/vnd.wap.multipart.");
                        }

                        li32Type = Get(ContentTypeTable, loIter->moValue.c_str());
                        if( li32Type >= 0 )
                        {
                            liRet += EncodeInt(loMemoryBlock, li32Name, (apl_uint32_t)li32Type);
                        }
                        else
                        {
                            liRet += EncodeTextString(loMemoryBlock, li32Name, loIter->moValue.c_str());
                        }
                        break;
                        
                    default:
                        EncodeTextString(loMemoryBlock, li32Name, loIter->moValue.c_str());
                        break;
                }
            }
        }
    }

    //1. Length
    liRet += EncodeValueLength(aoMemoryBlock, loMemoryBlock.GetLength());

    //2. type and param
    RESIZE_IF(aoMemoryBlock, loMemoryBlock.GetLength()); //memoryblock resize if need
    aoMemoryBlock.Write(loMemoryBlock.GetReadPtr(), loMemoryBlock.GetLength());

    liRet += loMemoryBlock.GetLength();
        
    return liRet;
}

apl_ssize_t EncodeContentType(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, std::string const& aoContentType)
{
    apl_ssize_t liRet = 0;

    //1. name
    EncodeFieldName(aoMemoryBlock, au8Name);

    //2. content type
    RETURN_ERR_IF((liRet = EncodeContentType(aoMemoryBlock, aoContentType)) < 0, -1);

    liRet += 1;
        
    return liRet;
}

apl_ssize_t EncodeMultipartHeader(acl::CMemoryBlock& aoMemoryBlock, std::list<stHeaderField> aoHeaderList, std::string& aoContentType)
{    
    std::list<stHeaderField>::iterator loIter = aoHeaderList.begin();
    for(; loIter != aoHeaderList.end(); loIter++)
    {
        if(apl_strcasecmp(loIter->moName.c_str(), "Content-Type") == 0)
        {
            aoContentType = loIter->moValue;
            continue;
        }
        
        apl_int32_t lu32Name = Get(MultipartHeaderTable, loIter->moName.c_str());
        if(lu32Name >= 0)
        {
            switch(lu32Name)
            {
                case 0x40: //Content-ID
                    EncodeQuoteString(aoMemoryBlock, (apl_uint8_t)lu32Name, loIter->moValue.c_str());
                    break;
                    
                default:
                    EncodeTextString(aoMemoryBlock, (apl_uint8_t)lu32Name, loIter->moValue.c_str());
                    break;
            }
            
        }
    }
    
    return 0;
}

apl_ssize_t EncodeMultipart(acl::CMemoryBlock& aoMemoryBlock, std::list<stMIMEEntity> aoMIMEEntityList)
{
    apl_ssize_t liRet = 0;
    
    if(aoMIMEEntityList.empty())
    {
        return 0;
    }
            
    //nEntries
    liRet += EncodeUintvar(aoMemoryBlock, aoMIMEEntityList.size());

    //Multipart Entry
    std::list<stMIMEEntity>::iterator loIter = aoMIMEEntityList.begin();
    for(; loIter != aoMIMEEntityList.end(); loIter++)
    {
        acl::CMemoryBlock   loHeaderMB(1024);
        acl::CMemoryBlock   loBodyMB(10*1024);
        std::string         loContentType;

        if(!loIter->moHeaderList.empty())
        {           
            EncodeMultipartHeader(loHeaderMB, loIter->moHeaderList, loContentType);
            
            if(!loContentType.empty())
            {
                if(!loIter->moMultipartList.empty())
                {
                    EncodeMultipart(loBodyMB, loIter->moMultipartList);
                }
                else
                {
                    RESIZE_IF(loBodyMB, loIter->moBody.size()); //memoryblock resize if need
                    loBodyMB.Write(loIter->moBody.data(), loIter->moBody.size());
                }

                //content type
                acl::CMemoryBlock loContentTypeMB(1024);
                apl_ssize_t liCTRet;

                RETURN_ERR_IF(((liCTRet = EncodeContentType(loContentTypeMB, loContentType)) < 0), -1)
                liRet +=  liCTRet;

                //memoryblock resize if need    
                RESIZE_IF(aoMemoryBlock, 10+loContentTypeMB.GetLength()+loHeaderMB.GetLength()+loBodyMB.GetLength());
                
                //1. headerslen
                liRet += EncodeUintvar(aoMemoryBlock, loContentTypeMB.GetLength()+loHeaderMB.GetLength());
                                
                //2. datalen
                liRet += EncodeUintvar(aoMemoryBlock, loBodyMB.GetLength());
                
                //3. content type
                liRet += aoMemoryBlock.Write(loContentTypeMB.GetReadPtr(), loContentTypeMB.GetLength());
                
                //4. header
                if(loHeaderMB.GetLength() > 0)
                {
                    liRet += aoMemoryBlock.Write(loHeaderMB.GetReadPtr(), loHeaderMB.GetLength());
                }
                
                //5. data
                liRet += aoMemoryBlock.Write(loBodyMB.GetReadPtr(), loBodyMB.GetLength());
            }
        }
    }
        
    return liRet;
}

apl_ssize_t ParseContentType(std::string const& aoContentType, std::string& aoType, std::list<stHeaderField>& aoPararmList)
{
    char const* lpcStart = aoContentType.c_str();
    char const * lpcEnd = lpcStart + apl_strlen(aoContentType.c_str());
    char* lpcSep = NULL;
    char* lpcSep2 = NULL;
    char* lpcSep3 = NULL;
    char lsName[256 + 1];
    char lsValue[256 + 1];

    apl_memset(lsName, 0, sizeof(lsName));
    apl_memset(lsValue, 0, sizeof(lsValue));
    
    if( (lpcSep = apl_strchr(lpcStart, ';')) == NULL )
    {
        aoType = aoContentType;
        acl::stralgo::TrimIf(aoType, IsQuote);
        return 0;
    }

    apl_memcpy(lsValue, lpcStart, lpcSep - lpcStart);
    acl::stralgo::TrimIf(lsValue, IsQuote);
    aoType = lsValue;
    
    lpcSep++;

    while(true)
    {
        if( (lpcEnd - lpcSep <= 0) || (lpcSep2 = (char *)apl_memchr(lpcSep, '=', lpcEnd - lpcSep)) == NULL )
        {
            break;
        }
        
        if( (lpcEnd - lpcSep2 - 1 <= 0) || (lpcSep3 = (char *)apl_memchr(lpcSep2+1, ';', lpcEnd - lpcSep2 - 1)) == NULL )
        {
            if( lpcEnd - lpcSep2 - 1 > 0 )
            {
                apl_memset(lsName, 0, sizeof(lsName));
                apl_memcpy(lsName, lpcSep, lpcSep2 - lpcSep);
                acl::stralgo::TrimIf(lsName, IsQuote);
                 
                apl_memset(lsValue, 0, sizeof(lsValue));
                apl_memcpy(lsValue, lpcSep2 + 1, lpcEnd - (lpcSep2 + 1));
                acl::stralgo::TrimIf(lsValue, IsQuote);

                stHeaderField loField;
                loField.moName = lsName;
                loField.moValue = lsValue;
                                
                aoPararmList.push_back(loField);
            }
            break;
        }

        apl_memset(lsName, 0, sizeof(lsName));
        apl_memcpy(lsName, lpcSep, lpcSep2 - lpcSep);
        acl::stralgo::TrimIf(lsName, IsQuote);
         
        apl_memset(lsValue, 0, sizeof(lsValue));
        apl_memcpy(lsValue, lpcSep2 + 1, lpcSep3 - (lpcSep2 + 1));
        acl::stralgo::TrimIf(lsValue, IsQuote);

        stHeaderField loField;
        loField.moName = lsName;
        loField.moValue = lsValue;
        
        aoPararmList.push_back(loField);

        lpcSep = lpcSep3+1;
    }
    
    return 0;
}

//=?charset?encoding?encoded-text?=
apl_ssize_t ParseCharSetText(std::string const& aoCharSetTextIn, std::string& aoCharSet, std::string& aoCharSetTextOut)
{
    apl_uint32_t lu32Start = 0;
    apl_uint32_t lu32End = 0;
    std::string  loCharSet;
    std::string  loEncodedText;
    apl_uint32_t lu32Length = 0;
    char*        lpcBuff = NULL;
    
    if((lu32End = aoCharSetTextIn.find("=?", lu32Start)) != (apl_uint32_t)std::string::npos)
    {
        lu32Start = lu32End+2;
        if((lu32End = aoCharSetTextIn.find_first_of("?", lu32Start)) != (apl_uint32_t)std::string::npos)
        {   
            if(lu32End != lu32Start)
            {
                loCharSet = aoCharSetTextIn.substr(lu32Start, lu32End-lu32Start);

                lu32Start = lu32End+1;
                if((lu32End = aoCharSetTextIn.find_first_of("?", lu32Start)) != (apl_uint32_t)std::string::npos)
                {
                    if(lu32End == lu32Start+1)
                    {
                        switch(*(aoCharSetTextIn.data()+lu32Start))
                        {
                            case 'Q': //quoted-printable
                            case 'q':
                                lu32Start = lu32End+1;
                                if((lu32End = aoCharSetTextIn.find("?=", lu32Start)) != (apl_uint32_t)std::string::npos)
                                {
                                    loEncodedText = aoCharSetTextIn.substr(lu32Start, lu32End-lu32Start);
                                    ACL_NEW_N(lpcBuff, char, 2*loEncodedText.size());
                                    if(QPDecode(loEncodedText.c_str(), lpcBuff, &lu32Length) == 0)
                                    {
                                        aoCharSet = loCharSet;
                                        aoCharSetTextOut.append(lpcBuff, lu32Length);
                                        ACL_DELETE_N(lpcBuff);
                                        return 0;
                                    }
                                    ACL_DELETE_N(lpcBuff);
                                }
                                break;

                            case 'B': //base64
                            case 'b':
                                lu32Start = lu32End+1;
                                if((lu32End = aoCharSetTextIn.find("?=", lu32Start)) != (apl_uint32_t)std::string::npos)
                                {
                                    acl::CBase64Decoder loBase64Decoder;
                                    loEncodedText = aoCharSetTextIn.substr(lu32Start, lu32End-lu32Start);
                                    if(loBase64Decoder.Final(loEncodedText.c_str()) == 0)
                                    {
                                        aoCharSet = loCharSet;
                                        aoCharSetTextOut.append(loBase64Decoder.GetOutput(), loBase64Decoder.GetLength());
                                        return 0;
                                    }
                                }
                                break;
                                
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }

    return -1;
}

bool IsMultipart(std::string const& aoContentType,  std::string& aoBoundary)
{
    apl_uint32_t lu32Start = 0;
    apl_uint32_t lu32End = 0;

    if((lu32End = aoContentType.find("boundary", lu32Start)) != (apl_uint32_t)std::string::npos)
    {
        std::string loBoundary;
        
        lu32Start = lu32End+apl_strlen("boundary");
        if((lu32End = aoContentType.find_first_of("=", lu32Start)) != (apl_uint32_t)std::string::npos)
        {
            lu32Start = lu32End+1;
            if((lu32End = aoContentType.find_first_of(";", lu32Start)) != (apl_uint32_t)std::string::npos)
            {
                loBoundary = aoContentType.substr(lu32Start, lu32End-lu32Start);
            }
            else
            {
                loBoundary.assign(aoContentType.c_str()+lu32Start);
            }
            acl::stralgo::TrimIf(loBoundary, IsQuote);
            
            aoBoundary += "--";
            aoBoundary += loBoundary.c_str();
                    
            return true;
        }
    }

    return false;
}

apl_ssize_t ParseMultipartHeader(std::string const& aoHeader, std::list<stHeaderField>& aoHeaderList)
{       
    char const* lpcStart = aoHeader.c_str();
    char const* lpcBeginOfLine = aoHeader.c_str();
    apl_uint32_t lu32Length = aoHeader.size();
    char const* lpcEndOfLine;
    char const* lpcSep;
    char lsName[MAX_LENGTH+1];
    char lsValue[MAX_LENGTH+1]; 

    apl_memset(lsName, 0, sizeof(lsName));
    apl_memset(lsValue, 0, sizeof(lsValue));

    while (true)
    {
        lpcEndOfLine = apl_strstr(lpcBeginOfLine, TOKEN_CRLF);

        RETURN_ERR_IF((NULL == lpcEndOfLine || lpcEndOfLine > lpcStart + lu32Length), -1);

        if (lpcEndOfLine == lpcBeginOfLine)
        {
            break; // reach end of buffer, break and return; 
        }

        assert(lpcEndOfLine > lpcBeginOfLine);

        RETURN_ERR_IF(((apl_size_t)(lpcEndOfLine - lpcBeginOfLine) > MAX_LENGTH), -1);

        if (apl_isspace(lpcBeginOfLine[0]))
        {
            RETURN_ERR_IF((apl_strlen(lsName) == 0), -1);

            apl_memset(lsValue, 0, sizeof(lsValue));
            apl_memcpy(lsValue, lpcBeginOfLine, lpcEndOfLine - lpcBeginOfLine);
            
            RETURN_ERR_IF((apl_strlen(lsValue) == 0), -1);

            RETURN_ERR_IF((aoHeaderList.empty()), -1);

            std::list<stHeaderField>::reverse_iterator loRIter = aoHeaderList.rbegin();
            RETURN_ERR_IF((apl_strcasecmp(loRIter->moName.c_str(),lsName) != 0), -1);

            loRIter->moValue.append(lsValue);
        }
        else
        {
            lpcSep = (char const*)apl_memchr(lpcBeginOfLine, ':', lpcEndOfLine - lpcBeginOfLine);

            RETURN_ERR_IF((NULL == lpcSep), -1);
           
            apl_memset(lsName, 0, sizeof(lsName));
            apl_memcpy(lsName, lpcBeginOfLine, lpcSep - lpcBeginOfLine);
            acl::stralgo::TrimIf(lsName, IsQuote);
             
            apl_memset(lsValue, 0, sizeof(lsValue));
            apl_memcpy(lsValue, lpcSep + 1, lpcEndOfLine - (lpcSep + 1));
            if(apl_strcasecmp(lsName, "Content-Type") != 0)
            {
                acl::stralgo::TrimIf(lsValue, IsQuote);
            }
           
            RETURN_ERR_IF((apl_strlen(lsName) == 0), -1);
            
            stHeaderField loField;
            loField.moName = lsName;
            loField.moValue = lsValue;            
            aoHeaderList.push_back(loField);
        }

        lpcBeginOfLine = lpcEndOfLine + apl_strlen(TOKEN_CRLF);
    }
    
    return 0;
}

apl_ssize_t ParseMultipart(std::string const& aoMultipart, const char* apcBoundary, std::list<stMIMEEntity>& aoMIMEEntityList)
{
    apl_uint32_t lu32Start = 0; 
    apl_uint32_t lu32End = 0;
    
    if((lu32Start = aoMultipart.find(apcBoundary,0)) == (apl_uint32_t)std::string::npos)
    {
        return -1;
    }

    lu32Start += apl_strlen(apcBoundary ) + 2; //boundary + "\r\n"

    while((lu32End = aoMultipart.find(apcBoundary,lu32Start)) != (apl_uint32_t)std::string::npos)
    {       
        std::string loPart = aoMultipart.substr(lu32Start, lu32End-lu32Start);
        
        apl_uint32_t lu32HeaderEnd = loPart.find(TOKEN_CRLFCRLF, 0);
        if( lu32HeaderEnd == (apl_uint32_t)std::string::npos )
        {
            return -1;
        }

        stMIMEEntity loMIMEEntity;
        
        std::string loHeader = loPart.substr(0, lu32HeaderEnd+4);
        std::string loBody = loPart.substr(lu32HeaderEnd+4, lu32End-lu32Start-(lu32HeaderEnd+4)-4); //trim "\r\n\r\n"
        
        std::list<stHeaderField> loHeaderList;
        if(ParseMultipartHeader(loHeader, loHeaderList) != 0 || loHeaderList.empty())
        {
            return -1;
        }
        
        std::list<stHeaderField>::iterator loIter = loHeaderList.begin();
        for(; loIter != loHeaderList.end(); loIter++)
        {
            if(apl_strcasecmp(loIter->moName.c_str(), "Content-Type") == 0)
            {
                std::string loBoundary;
                if(IsMultipart(loIter->moValue.c_str(), loBoundary))
                {
                    if(ParseMultipart(loBody, loBoundary.c_str(), loMIMEEntity.moMultipartList) != 0)
                    {
                        return -1;
                    }

                    loMIMEEntity.moBody = "";
                    break;
                }
            }
        }

        loMIMEEntity.moHeaderList = loHeaderList;
        loMIMEEntity.moBody = loBody;
        aoMIMEEntityList.push_back(loMIMEEntity);

        if( aoMultipart.compare(lu32End+apl_strlen(apcBoundary), 2, "--") == 0 ) //boundary + "--"(end token)
        {
            break;
        }

        lu32Start = lu32End + apl_strlen(apcBoundary ) + 2;
    }

    return 0;
}

/*DECODE*/
apl_ssize_t DecodeMultipartHeader(acl::CMemoryBlock& aoMemoryBlock, std::list<stHeaderField>& aoHeaderList)
{
    while(aoMemoryBlock.GetLength() > 0)
    {
        stHeaderField loHeader;
        apl_uint32_t lu32Type;
        apl_uint32_t lu32Value;

        //name
        lu32Type = DecodeTypeValue(aoMemoryBlock, lu32Value);
        RETURN_ERR_IF((lu32Type == 99), -1);
        switch(lu32Type)
        {
            case VALUE_TYPE_SHORT_INT:
            case VALUE_TYPE_OCTETS_WITH_LENGTH:
                if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
                {
                    RETURN_ERR_IF((aoMemoryBlock.GetLength() < lu32Value), -1);
                }
                loHeader.moName = Get(MultipartHeaderTable, DecodeInt(aoMemoryBlock, lu32Type, lu32Value));
                break;
                
            case VALUE_TYPE_NULL_END_STRING:
                RETURN_ERR_IF((DecodeNullEndString(aoMemoryBlock, loHeader.moName) <= 0), -1);
                break;
                
            default:
                return -1;
        }

        //value
        lu32Type = DecodeTypeValue(aoMemoryBlock, lu32Value);
        RETURN_ERR_IF((lu32Type == 99), -1);
        switch(lu32Type)
        {
            case VALUE_TYPE_SHORT_INT:
            case VALUE_TYPE_OCTETS_WITH_LENGTH:
                if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
                {
                    RETURN_ERR_IF((aoMemoryBlock.GetLength() < lu32Value), -1);
                }
                DecodeInt(aoMemoryBlock, lu32Type, lu32Value);//skip
                break;
                
            case VALUE_TYPE_NULL_END_STRING:
                RETURN_ERR_IF((DecodeNullEndString(aoMemoryBlock, loHeader.moValue) <= 0), -1);
                break;
                
            default:
                return -1;
        }
        
        if(!loHeader.moName.empty() && !loHeader.moValue.empty())
        {
            aoHeaderList.push_back(loHeader);
        }
    }
    
    return 0;
}

apl_ssize_t DecodeMultipart(acl::CMemoryBlock& aoMemoryBlock, std::string& aoContent)
{
    apl_uint32_t lu32BoundarySeq = 0;

    return DecodeMultipart(aoMemoryBlock, BOUNDARY_NEXT_PART, lu32BoundarySeq, aoContent);
}

apl_ssize_t DecodeMultipart(acl::CMemoryBlock& aoMemoryBlock, const std::string aoLastBoundary, apl_uint32_t& au32BoundarySeq, std::string& aoContent)
{   
    //nEntries
    apl_uint32_t lu32Entries = DecodeUintvar(aoMemoryBlock);
    RETURN_ERR_IF((lu32Entries <= 0), -1);

    while(lu32Entries > 0)
    {
        apl_uint32_t lu32HeaderLength = 0;
        apl_uint32_t lu32DataLenth = 0;
        apl_uint32_t lu32Type = 0;
        apl_uint32_t lu32Value = 0;

        std::list<stHeaderField> loHeaderList;
        std::string loContentType;
        std::string loBoundary;
        bool lbMulipart = false;

        //1. headerslen
        lu32HeaderLength = DecodeUintvar(aoMemoryBlock);

        //2. datalen
        lu32DataLenth = DecodeUintvar(aoMemoryBlock);

        RETURN_ERR_IF((aoMemoryBlock.GetLength() < lu32HeaderLength+lu32DataLenth), -1);

        acl::CMemoryBlock loMemoryBlock(lu32HeaderLength);
        loMemoryBlock.Write(aoMemoryBlock.GetReadPtr(), lu32HeaderLength);
        aoMemoryBlock.SetReadPtr(lu32HeaderLength);

        //3. content type
        lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
        //RETURN_ERR_IF((lu32Type != VALUE_TYPE_OCTETS_WITH_LENGTH), -1);
        RETURN_ERR_IF((DecodeContentType(loMemoryBlock, lu32Type, lu32Value, loContentType) < 0), -1);
        RETURN_ERR_IF((loContentType.empty()), -1);
        if(loContentType.find("multipart") != std::string::npos) //application/vnd.wap.multipart
        {
            lbMulipart = true;
        }
        
        //4. header
        RETURN_ERR_IF((DecodeMultipartHeader(loMemoryBlock, loHeaderList) < 0), -1);
        //RETURN_ERR_IF((loHeaderList.empty()), -1);

        //boundary
        aoContent += "--";
        aoContent += aoLastBoundary;
        aoContent += TOKEN_CRLF;

        //Content-Type
        aoContent += "Content-Type: ";
        aoContent += loContentType;
        if(lbMulipart)
        {
            char lsSubPart[128] = {0};
            apl_snprintf(lsSubPart, sizeof(lsSubPart), "%s_%"APL_PRIu32"", BOUNDARY_SUB_PART, au32BoundarySeq);
            loBoundary = lsSubPart;
            au32BoundarySeq++;
            
            aoContent += "; boundary=\"";
            aoContent += loBoundary;
            aoContent += "\"";
        }
        aoContent += TOKEN_CRLF;

        //HeaderList
        std::list<stHeaderField>::iterator loIter = loHeaderList.begin();
        for(; loIter != loHeaderList.end(); loIter++)
        {
            if(apl_strcasecmp("Content-Type",loIter->moName.c_str()) == 0)
            {
                continue;
            }
            
            aoContent += loIter->moName;
            aoContent += ": ";
            aoContent += loIter->moValue;
            aoContent += TOKEN_CRLF;
        }
        aoContent += TOKEN_CRLF;

        if(lbMulipart)
        {           
            RETURN_ERR_IF((DecodeMultipart(aoMemoryBlock, loBoundary, au32BoundarySeq, aoContent) < 0), -1);
        }
        else
        {
            //5. data
            aoContent.append(aoMemoryBlock.GetReadPtr(), lu32DataLenth);
            aoContent += TOKEN_CRLFCRLF;
            aoMemoryBlock.SetReadPtr(lu32DataLenth);
        }

        //next
        lu32Entries--;
    }

    //End
    aoContent += "--";
    aoContent += aoLastBoundary;
    aoContent += "--";
    if(apl_strcmp(BOUNDARY_NEXT_PART, aoLastBoundary.c_str()) != 0)
    {
        aoContent += TOKEN_CRLF;
    }
    
    return 0;
}

//Content-type-value = Constrained-media | Content-general-form
//Content-general-form = Value-length Media-type
//Media-type = (Well-known-media | Extension-Media) *(Parameter)
apl_ssize_t DecodeContentType(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Type, const apl_uint32_t au32Value, std::string& aoContentType)
{
    //Constrained-media
    switch(au32Type)
    {
        case VALUE_TYPE_SHORT_INT:
            aoContentType.append(Get(ContentTypeTable, (apl_int32_t)au32Value));
            return au32Value;

        case VALUE_TYPE_NULL_END_STRING:
            RETURN_ERR_IF((DecodeNullEndString(aoMemoryBlock, aoContentType) <= 0), MM4_DECODE_ERROR_NULL_STRING);
            return au32Value;

        default:
            break;
    }

    //Content-general-form
    apl_uint32_t lu32Length = au32Value;

    RETURN_ERR_IF((aoMemoryBlock.GetLength() < lu32Length), -1);
    
    apl_uint32_t lu32Type = 0;
    apl_uint32_t lu32Value = 0;
    acl::CMemoryBlock loMemoryBlock;

    loMemoryBlock.Resize(lu32Length);
    loMemoryBlock.Write(aoMemoryBlock.GetReadPtr(), lu32Length);
    aoMemoryBlock.SetReadPtr(lu32Length);

    lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
    RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);

    //type
    switch(lu32Type)
    {
        case VALUE_TYPE_SHORT_INT:
        case VALUE_TYPE_OCTETS_WITH_LENGTH:
            aoContentType.append(Get(ContentTypeTable, (apl_int32_t)DecodeInt(loMemoryBlock, lu32Type, lu32Value)));
            break;
            
        case VALUE_TYPE_NULL_END_STRING:
            RETURN_ERR_IF((DecodeNullEndString(loMemoryBlock, aoContentType) <= 0), MM4_DECODE_ERROR_NULL_STRING);
            break;
            
        default:
            return -1;
    }
        
    //param
    while(loMemoryBlock.GetLength() > 0)
    {
        std::string loParam;
        std::string loParamName;
        std::string loParamValue;
        apl_uint32_t lu32Name = 0;
        
        lu32Value = 0;

        //param name
        lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
        RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);

        switch(lu32Type)
        {
            case VALUE_TYPE_SHORT_INT:
            case VALUE_TYPE_OCTETS_WITH_LENGTH:
                if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
                {
                    RETURN_ERR_IF((loMemoryBlock.GetLength() < lu32Value), -1);
                }
                lu32Name = DecodeInt(loMemoryBlock, lu32Type, lu32Value);
                loParamName = Get(ContentTypeParamTable, (apl_int32_t)lu32Name);
                break;

            case VALUE_TYPE_NULL_END_STRING:
                RETURN_ERR_IF((DecodeNullEndString(loMemoryBlock, loParamName) <= 0), MM4_DECODE_ERROR_NULL_STRING);
                break;
                
            default:
                return -1;
        }

        //param value
        if(!loParamName.empty())
        {
            loParam += "; ";
            loParam += loParamName;
            loParam += "=";

            lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
            RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);
            
            switch(lu32Type)
            {
                case VALUE_TYPE_SHORT_INT:
                case VALUE_TYPE_OCTETS_WITH_LENGTH:
                    if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
                    {
                        RETURN_ERR_IF((loMemoryBlock.GetLength() < lu32Value), -1);
                    }

                    switch(lu32Name)
                    {
                        case 0x01: //charset
                            loParamValue = Get(CharSetTable, (apl_int32_t)DecodeInt(loMemoryBlock, lu32Type, lu32Value));
                            break;

                        case 0x03:
                        case 0x09: //type
                            loParamValue = Get(ContentTypeTable, (apl_int32_t)DecodeInt(loMemoryBlock, lu32Type, lu32Value));
                            break;

                        default:
                            DecodeInt(loMemoryBlock, lu32Type, lu32Value);//unsupport, so skip
                            break;
                    }
                    break;
                    
                case VALUE_TYPE_NULL_END_STRING:
                    RETURN_ERR_IF((DecodeNullEndString(loMemoryBlock, loParamValue) <= 0), MM4_DECODE_ERROR_NULL_STRING);
                    break;

                default:
                    return -1;
            }

            if(!loParamValue.empty())
            {
                loParam += "\"";
                loParam += loParamValue;
                loParam += "\"";

                if(apl_strcasecmp(loParamName.c_str(), "boundary") != 0)
                {
                    aoContentType += loParam;
                }
            }
        }
    }
    
    return lu32Length;
}

//From-value = Value-length (Address-present-token Encoded-string-value | Insert-address-token)
apl_ssize_t DecodeFrom(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Length, std::string& aoFrom)
{
    RETURN_ERR_IF((aoMemoryBlock.GetLength() <= 0), -1);
    
    apl_uint32_t lu32Type = 0;
    apl_uint32_t lu32Value = 0;
    std::string  loCharSet;

    if((apl_uint8_t)*aoMemoryBlock.GetReadPtr() == 128)
    {
        aoMemoryBlock.SetReadPtr(1); //skip Address-present-token
        
        lu32Type = DecodeTypeValue(aoMemoryBlock, lu32Value);
        RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);
        
        RETURN_ERR_IF((DecodeEncodeString(aoMemoryBlock, lu32Type, lu32Value, loCharSet, aoFrom) <= 0), -1);

        return (apl_ssize_t)aoFrom.size();
    }
    else if((apl_uint8_t)*aoMemoryBlock.GetReadPtr() == 129)
    {
        aoMemoryBlock.SetReadPtr(1); //skip Address-present-token

        return (apl_ssize_t)1;
    }
    
    return -1;
}

//Previously-sent-by-value = Value-length Forwarded-count-value Encoded-string-value
//Forwarded-count-value = Integer-value
apl_ssize_t DecodePreviouslySentBy(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiType, const apl_uint8_t au8Length, std::string& aoPreviouslySentBy)
{
    acl::CMemoryBlock loMemoryBlock;
    apl_uint32_t lu32Type = 0;
    apl_uint32_t lu32Value = 0;
    std::string  loCharSet;

    if(aiType == VALUE_TYPE_OCTETS_WITH_LENGTH)
    {
        RETURN_ERR_IF((aoMemoryBlock.GetLength() < au8Length), -1);
    }

    loMemoryBlock.Resize(au8Length);
    loMemoryBlock.Write(aoMemoryBlock.GetReadPtr(), au8Length);
    
    aoMemoryBlock.SetReadPtr(au8Length);

    //Forwarded-count-value
    lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
    RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);
    if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
    {
        RETURN_ERR_IF((loMemoryBlock.GetLength() < lu32Value), -1);
    }
    DecodeInt(loMemoryBlock, lu32Type, lu32Value); //skip

    //Encoded-string-value
    lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
    RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);
    RETURN_ERR_IF((DecodeEncodeString(loMemoryBlock, lu32Type, lu32Value, loCharSet, aoPreviouslySentBy) <= 0), -1);
    
    return 0;
}

//Previously-sent-date-value = Value-length Forwarded-count-value Date-value
apl_ssize_t DecodePreviouslySentDateAndTime(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiType, const apl_uint8_t au8Length, std::string& aoPreviouslySentDateAndTime)
{
    acl::CMemoryBlock loMemoryBlock;
    apl_uint32_t lu32Type = 0;
    apl_uint32_t lu32Value = 0;
    char lsTmpBuf[128] = {0};

    if(aiType == VALUE_TYPE_OCTETS_WITH_LENGTH)
    {
        RETURN_ERR_IF((aoMemoryBlock.GetLength() < au8Length), -1);
    }
    
    loMemoryBlock.Resize(au8Length);
    loMemoryBlock.Write(aoMemoryBlock.GetReadPtr(), au8Length);
    
    aoMemoryBlock.SetReadPtr(au8Length);

    //Forwarded-count-value
    lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
    RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);
    if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
    {
        RETURN_ERR_IF((loMemoryBlock.GetLength() < lu32Value), -1);
    }
    DecodeInt(loMemoryBlock, lu32Type, lu32Value); //skip

    //Date-value
    lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
    RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);
    if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
    {
        RETURN_ERR_IF((loMemoryBlock.GetLength() < lu32Value), -1);
    }
    apl_snprintf(lsTmpBuf, sizeof(lsTmpBuf), "%"APL_PRIu32, DecodeLongint(loMemoryBlock, lu32Value));
    aoPreviouslySentDateAndTime = std::string(lsTmpBuf);
    
    return 0;
}

//Long-integer = Short-length Multi-octet-integer
apl_uint32_t DecodeLongint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Length)
{   
    const char* lpcPtr = (const char*)aoMemoryBlock.GetReadPtr();
    apl_uint8_t  lu8Length = au8Length;
    apl_uint32_t lu32Value = 0;
    
    while(lu8Length > 0) 
    {
        lu32Value = lu32Value*256 + (apl_uint8_t)*lpcPtr++;
        lu8Length--;
    }

    aoMemoryBlock.SetReadPtr(au8Length);

    return lu32Value;
}

apl_uint32_t DecodeInt(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiType, const apl_uint8_t au8Length)
{   
    switch(aiType)
    {
        case VALUE_TYPE_SHORT_INT:
            return (apl_uint32_t)au8Length;
            
        case VALUE_TYPE_OCTETS_WITH_LENGTH:
            return DecodeLongint(aoMemoryBlock, au8Length);
            
        default:
            break;
    }

    return 0;   
}

apl_uint32_t DecodeUintvar(acl::CMemoryBlock& aoMemoryBlock)
{
    const char*  lpcValue  = (const char*)aoMemoryBlock.GetReadPtr();
    apl_uint8_t  lu8Value = 0;
    apl_uint32_t lu32Index = 0;
    apl_uint32_t lu32Value = 0;
    
    for(lu32Index = 0; lu32Index < 5; lu32Index++)
    {    
        lu8Value = (apl_uint8_t)*(lpcValue+lu32Index);
        lu32Value  = (lu32Value << 7) | (lu8Value & 0x7F);
        if( !(lu8Value & 0x80) )
        {    
            break;
        }    
    }

    aoMemoryBlock.SetReadPtr(lu32Index+1);

    return lu32Value;
}

apl_ssize_t DecodeNullEndString(acl::CMemoryBlock& aoMemoryBlock, std::string& aoText)
{
    RETURN_ERR_IF((aoMemoryBlock.GetLength() <= 0), -1);
    
    char const* lpcEnd = NULL;
    if((lpcEnd = (char const*)apl_memchr(aoMemoryBlock.GetReadPtr(), '\0', aoMemoryBlock.GetLength())) != NULL)
    {
        if(lpcEnd == aoMemoryBlock.GetReadPtr())
        {
            aoMemoryBlock.SetReadPtr(1);
            return (apl_ssize_t)1;
        }
        
        aoText.assign(aoMemoryBlock.GetReadPtr());
        aoMemoryBlock.SetReadPtr(lpcEnd-aoMemoryBlock.GetReadPtr()+1);
        
        return aoText.size()+1;
    }
    
    return 0;
}

//Encoded-string-value = Text-string | Value-length Char-set Text-string
apl_ssize_t DecodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Type, const apl_uint32_t au32Length, std::string& aoCharSet, std::string& aoText)
{
    apl_ssize_t liRet = 0;
    
    //Text-string
    if(au32Type == VALUE_TYPE_NULL_END_STRING)
    {
        RETURN_ERR_IF(((liRet = DecodeNullEndString(aoMemoryBlock, aoText)) <= 0), -1);
        return liRet;
    }

    acl::CMemoryBlock loMemoryBlock;
    loMemoryBlock.Resize(au32Length);
    loMemoryBlock.Write(aoMemoryBlock.GetReadPtr(), au32Length);
    aoMemoryBlock.SetReadPtr(au32Length);

    //Value-length Char-set Text-string
    
    apl_uint32_t lu32Type = 0;
    apl_uint32_t lu32Value = 0;
    lu32Type = DecodeTypeValue(loMemoryBlock, lu32Value);
    RETURN_ERR_IF((lu32Type == 99), MM4_DECODE_ERROR_TYPE_VALUE);

    //Char-set 
    switch(lu32Type)
    {
        case VALUE_TYPE_SHORT_INT:
        case VALUE_TYPE_OCTETS_WITH_LENGTH:
            if(lu32Type == VALUE_TYPE_OCTETS_WITH_LENGTH)
            {
                RETURN_ERR_IF((loMemoryBlock.GetLength() < lu32Value), -1);
            }
            aoCharSet = Get(CharSetTable, DecodeInt(loMemoryBlock, lu32Type, lu32Value));
            break;
            
        case VALUE_TYPE_NULL_END_STRING:
            RETURN_ERR_IF((DecodeNullEndString(loMemoryBlock, aoCharSet) <= 0), -1);
            break;
    }

    //Text-string
    RETURN_ERR_IF((DecodeNullEndString(loMemoryBlock, aoText) <= 0), -1);

    return au32Length;
}

/*Type, Value*/
apl_uint32_t DecodeTypeValue(acl::CMemoryBlock& aoMemoryBlock, apl_uint32_t& au32Value)
{
    RETURN_ERR_IF((aoMemoryBlock.GetLength() <= 0), 99);

    apl_uint8_t lu8Value = (apl_uint8_t)*aoMemoryBlock.GetReadPtr();
    apl_uint32_t lu32Type = 0;

    if(lu8Value > 0 && lu8Value < 31)
    {
        lu32Type = VALUE_TYPE_OCTETS_WITH_LENGTH;
        au32Value = lu8Value;
        aoMemoryBlock.SetReadPtr(1);
    }
    else if(lu8Value == 31)
    {
        lu32Type = VALUE_TYPE_OCTETS_WITH_LENGTH;

        aoMemoryBlock.SetReadPtr(1);
        au32Value = DecodeUintvar(aoMemoryBlock);
    }
    else if(lu8Value > 127)
    {
        lu32Type = VALUE_TYPE_SHORT_INT;

        au32Value = lu8Value - 128;
        aoMemoryBlock.SetReadPtr(1);
    }
    else if(lu8Value == 127/*Quote*/ || lu8Value == '"')
    {
        lu32Type = VALUE_TYPE_NULL_END_STRING;

        aoMemoryBlock.SetReadPtr(1);
    }
    else
    {
        lu32Type = VALUE_TYPE_NULL_END_STRING;
    }
    
    return lu32Type;
}

apl_int_t QPDecode(const char* apcIn, char* apcOut, apl_uint32_t* apcOutLen)
{
    apl_uint32_t lu32InPos, lu32OutPos, lu32LineLen, lu32NextLineStart, lu32NumChars, lu32CharsEnd;
    bool lbEolFound, lbSoftLineBrk, lbError;
    char lcChar, lcChar1, lcChar2;
    apl_uint32_t lu32InLength = apl_strlen(apcIn);

    if(!apcIn || !apcOut || !apcOutLen || lu32InLength <= 0)
    { 
        return -1;
    }
    lbError = false; lu32InPos = 0; lu32OutPos = 0;
    
    while(lu32InPos < lu32InLength)
    {
        /* Get line */
        lu32LineLen = 0;
        lbEolFound = false;

        while( !lbEolFound && lu32LineLen < lu32InLength - lu32InPos)
        {
            lcChar = apcIn[lu32InPos+lu32LineLen];
            ++lu32LineLen;
            if (lcChar == '\n')
            {
                lbEolFound = true;
            }
        }

        lu32NextLineStart = lu32InPos + lu32LineLen;
        lu32NumChars = lu32LineLen;

        /* Remove white space from end of line */
        while(lu32NumChars > 0)
        {
            lcChar = apcIn[lu32InPos+lu32NumChars-1] & 0xFF;
            if (lcChar != '\n' && lcChar != '\r' && lcChar != ' ' && lcChar != '\t')
            {
                break;
            }
            --lu32NumChars;
        }

        lu32CharsEnd = lu32InPos + lu32NumChars;
        /* Decode line */
        lbSoftLineBrk = false;
        while(lu32InPos < lu32CharsEnd)
        {
            lcChar = apcIn[lu32InPos++] & 0xFF;
            if(lcChar != '=') //Normal printable char
            {
                apcOut[lu32OutPos++] = lcChar;
            }
            else
            {
                /* Soft line break */
                if(lu32InPos >= lu32CharsEnd)
                {
                    lbSoftLineBrk = true;
                    break;
                }
                /* Non-printable char */
                else if(lu32InPos < lu32CharsEnd-1)
                {
                    lbError = false ;
                    lcChar1 = apcIn[lu32InPos++] & 0x7F;
                    if (lcChar1 != apcIn[lu32InPos-1])
                    {
                        lbError = true;
                    }
                    else
                    {
                        if ('0' <= lcChar1 && lcChar1 <= '9') lcChar1 -= '0';
                        else if ('A' <= lcChar1 && lcChar1 <= 'F') lcChar1 = lcChar1 - 'A' + 10;
                        else if ('a' <= lcChar1 && lcChar1 <= 'f') lcChar1 = lcChar1 - 'a' + 10;
                        else lbError = true;
                    } 
            
                    lcChar2 = apcIn[lu32InPos++] & 0x7F;
                    if( lcChar2 != apcIn[lu32InPos-1] )
                    {
                        lbError = true;
                    }
                    else
                    {
                        if ('0' <= lcChar2 && lcChar2 <= '9') lcChar2 -= '0';
                        else if ('A' <= lcChar2 && lcChar2 <= 'F') lcChar2 = lcChar2 - 'A' + 10;
                        else if ('a' <= lcChar2 && lcChar2 <= 'f') lcChar2 = lcChar2 - 'a' + 10;
                        else lbError = true;
                    }

                    if ( !lbError )
                    {
                        apcOut[lu32OutPos++] = (char) ((lcChar1 << 4) + lcChar2);
                    }
                    else
                    {
                        apcOut[lu32OutPos++] = apcIn[lu32InPos-3] ; 
                        apcOut[lu32OutPos++] = apcIn[lu32InPos-2] ; 
                        apcOut[lu32OutPos++] = apcIn[lu32InPos-1] ; 
                    } 
                }
                else
                {
                    lbError = true;
                }
            }
        }

        if(lbEolFound && !lbSoftLineBrk)
        {
            const char* lpcCRLF = "\r\n";
            apcOut[lu32OutPos++] = *lpcCRLF++;

            if(*lpcCRLF)
            {
                apcOut[lu32OutPos++] = *lpcCRLF;
            }
        }

        lu32InPos = lu32NextLineStart;
    }

    apcOut[lu32OutPos] = 0;
    *apcOutLen = lu32OutPos;

    return (lbError) ? -1 : 0;
}


}

ANF_NAMESPACE_END

