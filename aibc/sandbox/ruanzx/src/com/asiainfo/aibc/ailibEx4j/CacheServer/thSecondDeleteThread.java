package com.asiainfo.aibc.ailibEx4j.CacheServer;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.LinkedList;

import com.asiainfo.aibc.ailibEx4j.AILib.AILogSys;

public class thSecondDeleteThread implements Runnable
{
	private long liTimeDiff;
	private RetryDeleteReq lstRetryDeleteReq = null;
	private byte[] lstValue;
	private int liRet;

	private CacheClient lCacheClient;

	public thSecondDeleteThread(CacheClient cacheClient)
	{
		lCacheClient = cacheClient;
		
	}

	public void run()
	{
		LinkedList lpoRetryQueue = lCacheClient.getCpoRetryQueue();
		boolean liShutdown = lCacheClient.getCiShutdown();
		String lsLogFile = lCacheClient.getCsLogFile();
		long liRetryDeleteInterval = lCacheClient.getCiDelTimeoutInterval();
		
		while (!liShutdown)
		{
			if (lpoRetryQueue.size() > 0)
			{
				lstRetryDeleteReq = (RetryDeleteReq) lpoRetryQueue.removeFirst();
				liTimeDiff = System.currentTimeMillis() - lstRetryDeleteReq.getCiInQueueTime();

				if (liTimeDiff < liRetryDeleteInterval)
				{
					lCacheClient.aiSleep(liRetryDeleteInterval - liTimeDiff, liShutdown);
				}

				if (liShutdown)
				{
					// AI_DELETE_N(lstRetryDeleteReq.cpcKey);
					break;
				}

				try
				{
					if ((liRet = lCacheClient.deleteData(lstRetryDeleteReq.getCpcKey(), lstValue, null, null, 0)) == 0)
					{
						AILogSys.AIWriteLOG(lsLogFile, AILogSys.AILOG_LEVEL_DEBUG, new Object[] {
								"Delete data second time success, key=", new String(lstRetryDeleteReq.getCpcKey()) });
						lstRetryDeleteReq.getCRetryDeleteCBFun().retryDeleteCBFun(0, lstRetryDeleteReq.getCpcKey(),
								lstValue, lstValue.length, lstRetryDeleteReq.getCpvParam());
					}
					else
					{
						AILogSys.AIWriteLOG(lsLogFile, AILogSys.AILOG_LEVEL_ERROR, new Object[] {
								"Fail to delete data second time, key=", new String(lstRetryDeleteReq.getCpcKey()),
								" ret=", new Integer(liRet) });
						lstRetryDeleteReq.getCRetryDeleteCBFun().retryDeleteCBFun(liRet, lstRetryDeleteReq.getCpcKey(),
								null, 0, lstRetryDeleteReq.getCpvParam());
					}
					// AI_DELETE_N(lstRetryDeleteReq.cpcKey);
				}
				catch (IOException e)
				{
					e.printStackTrace();
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
				}
			}
			else
			{
				lCacheClient.aiSleep(liRetryDeleteInterval, liShutdown);
			}
		}
		return;
	}

}
