package com.asiainfo.aibc.ailibEx4j.CacheServer;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import com.asiainfo.aibc.ailibEx4j.AILib.AILogSys;

public class thDeleteTimeoutThread implements Runnable
{
	private ByteBuffer lclsSendBuf;
	private byte[] lclsKeyBuf;
	private byte[] lclsValueBuf;
	private int liKeySize;
	private int liValueSize;
	private int liTotalLen;
	private int liCmdId;
	private int liResult;
	private ByteBuffer lsResponse;
	private int liFlag;
	private int liLen;
	private int liCmd;

	private CacheClient cacheClient;

	public thDeleteTimeoutThread(CacheClient cacheClient)
	{
		this.cacheClient = cacheClient;

		lclsSendBuf = ByteBuffer.allocate(12);
		lclsSendBuf.order(ByteOrder.BIG_ENDIAN);

		lsResponse = ByteBuffer.allocate(CacheClient.MAX_PACKET_LEN);
		lsResponse.order(ByteOrder.BIG_ENDIAN);

		liTotalLen = 12;
		liCmdId = CacheDefine.REQ_DELTIMEOUT;
		liFlag = cacheClient.getCiTypeFlag();
	}

	public void run()
	{
		lclsSendBuf.putInt(liTotalLen);
		lclsSendBuf.putInt(liCmdId);
		lclsSendBuf.putInt(liFlag);
		lclsSendBuf.flip();
		
		boolean liShutdown = cacheClient.getCiShutdown();
		int liServerCnt = cacheClient.getCiServerCnt();
		String lsLogFile = cacheClient.getCsLogFile();
		long liDelTimeoutInterval = cacheClient.getCiDelTimeoutInterval();

		while (!liShutdown)
		{
			for (int i = 0; i < liServerCnt && !liShutdown; i++)
			{
				while (!liShutdown)
				{
					try
					{
						if (cacheClient.asyncSendPacket(cacheClient.getCpCacheServer(i).cpoAsyncClient, lclsSendBuf,
								lsResponse) != 0)
						{
							break;
						}
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
					}
					
					liLen = lsResponse.getInt();
					liCmd = lsResponse.getInt();
					liResult = lsResponse.getInt();
					
					System.out.println("liLen="+liLen);
					System.out.println("liCmd="+liCmd);
					System.out.println("liResult="+liResult);
					

					if (liResult != 0)
					{
						break;
					}
					else
					{
						liKeySize = lsResponse.getInt();
//						System.out.println("liKeySize="+liKeySize);
//						try
//						{
//							Thread.sleep(5000);
//						}
//						catch (InterruptedException e1)
//						{
//							e1.printStackTrace();
//						}
						
						lclsKeyBuf = new byte[liKeySize];
						lsResponse.get(lclsKeyBuf, 0, lclsKeyBuf.length);

						liValueSize = lsResponse.getInt();
						lclsValueBuf = new byte[liValueSize];
//						System.out.println("lsResponse.remaining="+lsResponse.remaining());
						lsResponse.get(lclsValueBuf, 0, lclsValueBuf.length);

						if (cacheClient.getCCBFun() != null)
						{
							cacheClient.getCCBFun().delTimeoutCBFun(lclsKeyBuf, lclsValueBuf, lclsValueBuf.length,
									cacheClient.getCpvParam());
						}

						try
						{
							System.out.println("new String(lclsKeyBuf)="+new String(lclsKeyBuf));
							AILogSys.AIWriteLOG(lsLogFile, AILogSys.AILOG_LEVEL_DEBUG, new Object[] {
									"Delete timeout data from CacheServer, key=", new String(lclsKeyBuf) });
							// usSleep(1);
							Thread.sleep(1);
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
				}
			}
			
			if (cacheClient.isCbStartBackupServer() && cacheClient.getCpBackupServer() != null)
			{
				while (!liShutdown)
				{
					try
					{
						if (cacheClient.asyncSendPacket(cacheClient.getCpBackupServer().cpoAsyncClient, lclsSendBuf,
								lsResponse) != 0)
						{
							break;
						}
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
					}

					liResult = lsResponse.getInt(8);

					if (liResult == CacheDefine.ERR_DATA_NOFOUND)
					{
						break;
					}
					else if (liResult == CacheDefine.ERR_DATABASE_EMPTY)
					{
						try
						{
							AILogSys.AIWriteLOG(lsLogFile, AILogSys.AILOG_LEVEL_INFO,
									new Object[] { "Stop using backup CacheServer" });
							cacheClient.setCbStartBackupServer(false);
							break;
						}
						catch (IOException e)
						{
							e.printStackTrace();
						}
					}
					else
					{
						liKeySize = lsResponse.getInt();
						lclsKeyBuf = new byte[liKeySize];
						lsResponse.get(lclsKeyBuf, 0, lclsKeyBuf.length);

						liValueSize = lsResponse.getInt();
						lclsValueBuf = new byte[liValueSize];
						lsResponse.get(lclsValueBuf, 0, lclsValueBuf.length);

						cacheClient.getCCBFun().delTimeoutCBFun(lclsKeyBuf, lclsValueBuf, lclsValueBuf.length,
								cacheClient.getCpvParam());

						try
						{
							AILogSys.AIWriteLOG(lsLogFile, AILogSys.AILOG_LEVEL_DEBUG, new Object[] {
									"Delete timeout data from CacheServer, key=", new String(lclsKeyBuf) });
							// usSleep(1);
							Thread.sleep(1);
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
				}
			}
			cacheClient.aiSleep(liDelTimeoutInterval, liShutdown);
		}
		return;

	}

}
