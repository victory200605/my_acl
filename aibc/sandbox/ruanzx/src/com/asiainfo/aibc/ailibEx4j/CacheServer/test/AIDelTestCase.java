package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

import java.io.IOException;
import java.nio.ByteBuffer;

public class AIDelTestCase extends AITestCase
{

	public int DoIt(Object apParam)
	{
		String lcKey = "";
		byte[] loValue = new byte[512];

		ClsParam lpoParam = (ClsParam) apParam;
		if ((lcKey = lpoParam.getCpoKey().getKey()) == null)
		{
			return -1;
		}
		int liRet = 0;
		try
		{
			liRet = lpoParam.getCoCacheClient().deleteData(lcKey, loValue, null, null, 0);

			if (liRet != 0)
			{
				if (lpoParam.isCbIsTerminate())
				{
					System.out.println("Delete data fail, MSGCODE:" + liRet + " [key=" + lcKey + "]");
				}
				return -1;
			}
			else
			{
				if (lpoParam.isCbIsTerminate())
				{
					String strTemp="";
					for(int i=0;i<loValue.length;i++)
					{
						strTemp+=(char)loValue[i];
					}
					System.out.println("Delete data success, [key=" + lcKey + "]/[value=" + strTemp + "]");
				}
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

		return liRet;
	}

}
