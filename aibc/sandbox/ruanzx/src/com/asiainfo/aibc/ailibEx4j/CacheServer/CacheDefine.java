package com.asiainfo.aibc.ailibEx4j.CacheServer;

public class CacheDefine
{
	public static final int CACHE_HASH_MAX = 100;

	public static final int ERR_DUPLICATE = 8001;
	public static final int ERR_DATA_NOFOUND = 8002;
	public static final int ERR_DATABASE_EMPTY = 8003;
	public static final int ERR_INVALID_PARAM = 8004;
	public static final int ERR_SYSTEM = 8005;
	public static final int ERR_INVALID_CMD = 8006;
	public static final int ERR_THREADPOOL_FULL = 8007;
	public static final int ERR_PACKET_ERROR = 8008;

	public static final int REQ_INSERT = 0x0001;
	public static final int RESP_INSERT = 0x8001;
	public static final int REQ_FETCH = 0x0002;
	public static final int RESP_FETCH = 0x8002;
	public static final int REQ_DELETE = 0x0003;
	public static final int RESP_DELETE = 0x8003;
	public static final int REQ_DELTIMEOUT = 0x0004;
	public static final int RESP_DELTIMEOUT = 0x8004;
	public static final int REQ_GET_COUNT = 0x0005;
	public static final int RESP_GET_COUNT = 0x8005;

	public static final int CACHE_TYPE_SM = 0x0001;
	public static final int CACHE_TYPE_MMS = 0x0002;
	public static final int CHE_TYPE_WAP = 0x0004;
}
