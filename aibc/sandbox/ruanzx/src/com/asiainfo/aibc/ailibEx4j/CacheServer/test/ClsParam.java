package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

import com.asiainfo.aibc.ailibEx4j.CacheServer.CacheClient;

public class ClsParam
{
	private CacheClient coCacheClient = new CacheClient();
	private ClsKey cpoKey;
	private boolean cbIsTerminate;

	public CacheClient getCoCacheClient()
	{
		return coCacheClient;
	}

	public ClsKey getCpoKey()
	{
		return cpoKey;
	}

	public boolean isCbIsTerminate()
	{
		return cbIsTerminate;
	}

	public void setCoCacheClient(CacheClient coCacheClient)
	{
		this.coCacheClient = coCacheClient;
	}

	public void setCpoKey(ClsKey cpoKey)
	{
		this.cpoKey = cpoKey;
	}

	public void setCbIsTerminate(boolean cbIsTerminate)
	{
		this.cbIsTerminate = cbIsTerminate;
	}
}
