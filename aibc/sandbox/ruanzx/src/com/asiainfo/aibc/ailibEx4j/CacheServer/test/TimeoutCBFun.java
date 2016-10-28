package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

import java.util.logging.Level;
import java.util.logging.Logger;


public class TimeoutCBFun 
{
	public static int giTimeCount = 0;

	private Logger DelTimeoutCBFunLog = null;

	public int delTimeoutCBFun(byte[] apcKey, byte[] apvValue, int aiValueSize, Object apvParam)
	{
		giTimeCount++;
		DelTimeoutCBFunLog = Logger.getLogger("DelTimeoutCBFunLog");

		DelTimeoutCBFunLog.log(Level.INFO, "giTimeCount="+giTimeCount+"Delete " + new String(apcKey) + "/" + new String(apvValue));

		return 0;
	}

}
