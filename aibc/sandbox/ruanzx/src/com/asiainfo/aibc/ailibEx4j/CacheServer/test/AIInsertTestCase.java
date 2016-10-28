package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

import java.io.IOException;

import com.asiainfo.aibc.ailibEx4j.CacheServer.CacheDefine;

public class AIInsertTestCase extends AITestCase
{
	public int DoIt(Object apParam)
	{
//		byte[] lcKey = new byte[40];
		String lcKey = "";
//		byte[] lcValue = new byte[512];
//
//		for (int i = 0; i < lcValue.length; i++)
//		{
//			lcValue[i] = 'A';
//		}
		String lcValue = "";
		for(int i=0;i<512;i++)
		{
			lcValue+='A';
		}
		
		ClsParam lpoParam = (ClsParam) apParam;
		if ((lcKey=lpoParam.getCpoKey().getKey()) == null)
		{
			return -1;
		}
		try
		{
			int liRet = lpoParam.getCoCacheClient().saveData(lcKey, lcValue, lcValue.length(), CacheDefine.CACHE_TYPE_SM);
			
			if (liRet != 0)
			{
				if (lpoParam.isCbIsTerminate())
				{
					System.out.println("Insert data fail, MSGCODE:" + liRet + " [key=" + lcKey + "]");
				}
				return -1;
			}
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		return 0;
	}

}
