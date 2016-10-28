package com.ailk.ainb.cache.client.biz;

/**
 * Created with IntegerelliJ IDEA.
 * User: Administrator
 * Date: 13-1-28
 * Time: 下午10:21
 */
public interface Constants {

	/*
	 * 命令或响应类型
	 */
	Integer REQ_INSERT		= 0x0001;
	Integer RESP_INSERT		= 0x8001;
	Integer REQ_FETCH		= 0x0002;
	Integer RESP_FETCH		= 0x8002;
	Integer REQ_DELETE		= 0x0003;
	Integer RESP_DELETE		= 0x8003;
	Integer REQ_DELTIMEOUT	= 0x0004;
	Integer RESP_DELTIMEOUT	= 0x8004;

	/*
	 * 处理结果定义
	 */
	int CORRECT= 0	  	;//：正确

	int AI_WARN_NO_FOUND  =  70001;

	int AI_ERROR_OPEN_FILE            = -70001;

	int AI_ERROR_WRITE_FILE           = -70002;

	int AI_ERROR_READ_FILE            = -70003;

	int AI_ERROR_SYNC_FILE            = -70004;

	int AI_ERROR_INVALID_FILE         = -70005;

	int AI_ERROR_OUT_OF_RANGE         = -70006;

	int AI_ERROR_DEL_FREE_NODE        = -70007;

	int AI_ERROR_INVALID_DATA         = -70008;

	int AI_ERROR_NO_ENOUGH_SPACE      = -70009;

	int AI_ERROR_UNOPENED             = -70010;

	int AI_ERROR_DUP_RECORD           = -70011;

	int AI_ERROR_UNCOMPATIBLE_VERSION = -70012;

	int AI_ERROR_MAP_FILE             = -70013;

	int ERROR_FAIL = -1;


	String BAK_KEY= "bak_key";

}
