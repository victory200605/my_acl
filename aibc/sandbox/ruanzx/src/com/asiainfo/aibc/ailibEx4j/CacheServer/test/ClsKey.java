package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

import com.asiainfo.aibc.ailibEx4j.AILib.AIRandomStringGenerater;

public class ClsKey
{
	public static int ciCount = 0;
	private int ciStep;

	public ClsKey()
	{
		ciCount = 0;
		ciStep = 1;
	}

	public ClsKey(int aiStart, int aiStep)
	{
		super();
		ciCount = aiStart;
		ciStep = aiStep;
	}

	public String getKey()
	{
		String lpsKeyBuffer = "0625091001" + AIRandomStringGenerater.toFixdLengthString(ciCount, 10);
		ciCount += ciStep;
		return lpsKeyBuffer;
	}

}
