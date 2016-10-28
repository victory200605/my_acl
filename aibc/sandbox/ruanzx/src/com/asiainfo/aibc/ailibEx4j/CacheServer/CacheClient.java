package com.asiainfo.aibc.ailibEx4j.CacheServer;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.LinkedList;
import java.util.Properties;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

import com.asiainfo.aibc.ailibEx4j.AIAsyncTcp.AsyncClient;
import com.asiainfo.aibc.ailibEx4j.AIAsyncTcp.AsyncMessage;
import com.asiainfo.aibc.ailibEx4j.AILib.AIString;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.TimeoutCBFun;

public class CacheClient
{
	public final static int ASYNC_SEND_FAIL = -100;
	public final static int ASYNC_RECV_FAIL = -200;

	public final static int MAX_PACKET_LEN = 4096;

	public class CacheServerInfo
	{
		public String csIpAddr;
		public int ciPort;
		public int ciConnection;
		public int ciMinRange;
		public int ciMaxRange;
		public AsyncClient cpoAsyncClient = null;
	}

	private boolean ciShutdown;
	private long ciDelTimeoutInterval;
	private long ciRetryDeleteInterval;
	private String csLogFile;
	private int ciServerCnt;
	private int ciTypeFlag;
	private CacheServerInfo[] cpCacheServer = null;
	private CacheServerInfo cpBackupServer = null;
	private boolean cbStartBackupServer;
	private LinkedList cpoRetryQueue = null;
	private Object cpvParam = null;
	private Logger CacheClientLog = null;
	private Properties properties;

	public CacheClient()
	{
		ciShutdown = false;
		ciDelTimeoutInterval = 0;
		ciRetryDeleteInterval = 0;
		ciServerCnt = 0;
		ciTypeFlag = 0;
		cbStartBackupServer = false;
		cpoRetryQueue = new LinkedList();

		properties = new Properties();
	}

	public int init(int aiTypeFlag, Object apvParam, int aiParamSize, String apcConfigFile) throws IOException,
			InterruptedException, ClassNotFoundException
	{
		String lsField;
		int liTimeout;

		CacheClientLog = Logger.getLogger("CacheClientLog");

		ciTypeFlag = aiTypeFlag;
		cpvParam = apvParam;

		FileInputStream inputFile = new FileInputStream(apcConfigFile);
		properties.load(inputFile);
		inputFile.close();

		csLogFile = properties.getProperty("com.asiainfo.aibc.ailibEx4j.CacheServer.Log.LogFile");

		int limit = 8000000; // 日志文件容量大小为8Mb，达到限制将被清空
		FileHandler myFileHandler = new FileHandler(csLogFile, limit, 1, true);
		CacheClientLog.addHandler(myFileHandler);

		liTimeout = Integer.parseInt(properties.getProperty(
				"com.asiainfo.aibc.ailibEx4j.CacheServer.CacheClient.ConnTimeout", "5"));

		for (int i = 0;; i++)
		{
			lsField = "Server" + new Integer(i + 1).toString();

			if (properties.getProperty("com.asiainfo.aibc.ailibEx4j.CacheServer." + lsField + ".ServerIP", "") == "")
			{
				break;
			}
			else
			{
				ciServerCnt = i + 1;
			}
		}

		cpCacheServer = new CacheServerInfo[ciServerCnt];
		for (int i = 0; i < ciServerCnt; i++)
		{
			cpCacheServer[i] = new CacheServerInfo();
			lsField = "Server" + new Integer(i + 1).toString();

			cpCacheServer[i].csIpAddr = properties.getProperty("com.asiainfo.aibc.ailibEx4j.CacheServer." + lsField
					+ ".ServerIP");

			cpCacheServer[i].ciPort = Integer.parseInt(properties.getProperty(
					"com.asiainfo.aibc.ailibEx4j.CacheServer." + lsField + ".ServerPort", "-1"));

			cpCacheServer[i].ciConnection = Integer.parseInt(properties.getProperty(
					"com.asiainfo.aibc.ailibEx4j.CacheServer." + lsField + ".Connection", "1"));
			cpCacheServer[i].ciMinRange = Integer.parseInt(properties.getProperty(
					"com.asiainfo.aibc.ailibEx4j.CacheServer." + lsField + ".MinRange", "0"));
			cpCacheServer[i].ciMaxRange = Integer.parseInt(properties.getProperty(
					"com.asiainfo.aibc.ailibEx4j.CacheServer." + lsField + ".MaxRange", "99"));

			cpCacheServer[i].cpoAsyncClient = new AsyncClient(cpCacheServer[i].csIpAddr, cpCacheServer[i].ciPort,
					cpCacheServer[i].ciConnection, liTimeout);

			if (cpCacheServer[i].cpoAsyncClient == null)
			{
				return 1;
			}
			cpCacheServer[i].cpoAsyncClient.connectTo();
		}

		if (properties.getProperty("com.asiainfo.aibc.ailibEx4j.CacheServer.BackupServer.ServerIP") != "")
		{
			cpBackupServer = new CacheServerInfo();

			cpBackupServer.csIpAddr = properties
					.getProperty("com.asiainfo.aibc.ailibEx4j.CacheServer.BackupServer.ServerIP");

			cpBackupServer.ciPort = Integer.parseInt(properties.getProperty(
					"com.asiainfo.aibc.ailibEx4j.CacheServer.BackupServer.ServerPort", "-1"));
			cpBackupServer.ciConnection = Integer.parseInt(properties.getProperty(
					"com.asiainfo.aibc.ailibEx4j.CacheServer.BackupServer.Connection", "1"));
			cpBackupServer.cpoAsyncClient = new AsyncClient(cpBackupServer.csIpAddr, cpBackupServer.ciPort,
					cpBackupServer.ciConnection, liTimeout);
			if (cpBackupServer.cpoAsyncClient == null)
			{
				return 1;
			}
			cpBackupServer.cpoAsyncClient.connectTo();
		}

		CacheClientLog.log(Level.INFO, "set delete timeout interval = " + ciDelTimeoutInterval);
		CacheClientLog.log(Level.INFO, "set retry delete interval = " + ciRetryDeleteInterval);

		for (int i = 0; i < ciServerCnt; i++)
		{
			CacheClientLog.log(Level.INFO, "set server" + (i + 1) + "(" + cpCacheServer[i].csIpAddr + ":"
					+ cpCacheServer[i].ciPort + ":" + cpCacheServer[i].ciConnection + ") range = "
					+ cpCacheServer[i].ciMinRange + "~" + cpCacheServer[i].ciMaxRange);
		}
		if (cpBackupServer != null)
		{
			CacheClientLog.log(Level.INFO, "set backup server = " + cpBackupServer.csIpAddr + ":"
					+ cpBackupServer.ciPort + ":" + cpBackupServer.ciConnection);
		}
		return 0;
	}

	public int saveData(String apcKey, String apvValue, int aiValueSize, int aiType) throws InterruptedException,
			IOException
	{
		int liServerPos = 0;
		int liRet;
		ByteBuffer lclsSendBuf;
		int liTotalLen;
		int liCmdId;
		int liType = aiType;
		int liKeySize = apcKey.length();
		int liValueSize = aiValueSize;
		ByteBuffer lsResponse = ByteBuffer.allocate(MAX_PACKET_LEN);
		lsResponse.order(ByteOrder.BIG_ENDIAN);

		if (ciServerCnt > 1)
		{
			liServerPos = getServerPos(apcKey);
			if (liServerPos < 0)
			{
				CacheClientLog.log(Level.SEVERE, "Fail to find server  pos for key " + apcKey);

				return CacheDefine.ERR_INVALID_PARAM;
			}
		}

		lclsSendBuf = ByteBuffer.allocate(20 + liKeySize + 1 + liValueSize);
		lclsSendBuf.order(ByteOrder.BIG_ENDIAN);

		liTotalLen = 20 + liKeySize + liValueSize;
		liCmdId = CacheDefine.REQ_INSERT;

		lclsSendBuf.putInt(liTotalLen);
		lclsSendBuf.putInt(liCmdId);
		lclsSendBuf.putInt(liType);
		lclsSendBuf.putInt(liKeySize);
		lclsSendBuf.put(apcKey.getBytes());
		lclsSendBuf.putInt(liValueSize);
		lclsSendBuf.put(apvValue.getBytes());

		lclsSendBuf.flip();

		if ((liRet = asyncSendPacket(cpCacheServer[liServerPos].cpoAsyncClient, lclsSendBuf, lsResponse)) == 0)
		{
			lsResponse.position(8);
			liRet = lsResponse.getInt();
		}

		if ((liRet == ASYNC_SEND_FAIL) && (cpBackupServer != null))
		{
			CacheClientLog.log(Level.WARNING, "Connect CacheServer(" + cpCacheServer[liServerPos].csIpAddr + ":"
					+ new Integer(cpCacheServer[liServerPos].ciPort) + ") fail, use backup CacheServer");
			if ((liRet = asyncSendPacket(cpBackupServer.cpoAsyncClient, lclsSendBuf, lsResponse)) == 0)
			{
				lsResponse.position(8);
				liRet = lsResponse.getInt();
				cbStartBackupServer = true;
			}
		}

		if (liRet != 0)
		{
			CacheClientLog.log(Level.SEVERE, "Fail to save data, err=" + new Integer(liRet) + ", key=" + apcKey);
		}
		else
		{
			CacheClientLog.log(Level.CONFIG, "Save data to CacheServer, key=" + apcKey);
		}
		return liRet;
	}

	public int deleteData(String apcKey, byte[] apoValueBuf, ifRetryDeleteCBFun aRetryDeleteCBFun, Object apvParam,
			int aiParamSize) throws InterruptedException, IOException
	{
		int liServerPos = 0;
		int liRet;
		ByteBuffer lclsSendBuf;
		int liTotalLen;
		int liCmdId;
		int liKeySize = apcKey.length();
		ByteBuffer lsResponse = ByteBuffer.allocate(MAX_PACKET_LEN);
		lsResponse.order(ByteOrder.BIG_ENDIAN);

		if (ciServerCnt > 1)
		{
			liServerPos = getServerPos(apcKey);

			if (liServerPos < 0)
			{
				CacheClientLog.log(Level.SEVERE, "Fail to find server pos for key " + apcKey);
				return CacheDefine.ERR_INVALID_PARAM;
			}
		}

		lclsSendBuf = ByteBuffer.allocate(12 + liKeySize);
		lclsSendBuf.order(ByteOrder.BIG_ENDIAN);

		liTotalLen = 12 + liKeySize;

		if (apoValueBuf == null)
		{
			liCmdId = CacheDefine.REQ_DELETE;
		}
		else
		{
			liCmdId = CacheDefine.REQ_FETCH;
		}

		lclsSendBuf.putInt(liTotalLen);
		lclsSendBuf.putInt(liCmdId);
		lclsSendBuf.putInt(liKeySize);
		lclsSendBuf.put(apcKey.getBytes(), 0, apcKey.length());

		lclsSendBuf.flip();

		if ((liRet = asyncSendPacket(cpCacheServer[liServerPos].cpoAsyncClient, lclsSendBuf, lsResponse)) == 0)
		{

			lsResponse.position(8);
			liRet = lsResponse.getInt();

		}

		if ((cbStartBackupServer) && ((liRet == ASYNC_SEND_FAIL) || (liRet == CacheDefine.ERR_DATA_NOFOUND))
				&& (cpBackupServer != null))
		{
			CacheClientLog.log(Level.INFO, "Try to delete data from backup CacheServer");
			if ((liRet = asyncSendPacket(cpBackupServer.cpoAsyncClient, lclsSendBuf, lsResponse)) == 0)
			{
				lsResponse.position(8);
				liRet = lsResponse.getInt();
			}
		}

		if (liRet != 0)
		{
			CacheClientLog.log(Level.SEVERE, "Fail to delete data, err=" + new Integer(liRet) + " ,key="
					+ new String(apcKey));
			if ((aRetryDeleteCBFun != null) && (liRet != ASYNC_SEND_FAIL))
			{
				RetryDeleteReq lstRetryDeleteReq = new RetryDeleteReq();
				lstRetryDeleteReq.setCpcKey(apcKey);
				lstRetryDeleteReq.setCpvParam(apvParam);
				lstRetryDeleteReq.setCRetryDeleteCBFun(aRetryDeleteCBFun);
				lstRetryDeleteReq.setCiInQueueTime(System.currentTimeMillis());

				cpoRetryQueue.add(lstRetryDeleteReq);

				CacheClientLog.log(Level.INFO, "Save delete request to delete later, key=" + apcKey + ", ret="
						+ new Integer(liRet));
			}
		}
		else
		{
			CacheClientLog.log(Level.CONFIG, "Delete data from CacheServer, key=" + apcKey);

			if (apoValueBuf != null)
			{
				int liValueSize;

				lsResponse.position(12);
				liValueSize = lsResponse.getInt();

				byte[] bpoValueBuf = new byte[liValueSize];

				lsResponse.get(bpoValueBuf);

				System.arraycopy(bpoValueBuf, 0, apoValueBuf, 0, bpoValueBuf.length);
			}
		}

		return liRet;
	}

	public int delTimeout()
	{
		ByteBuffer lclsSendBuf = ByteBuffer.allocate(12);
		lclsSendBuf.order(ByteOrder.BIG_ENDIAN);

		ByteBuffer lsResponse = ByteBuffer.allocate(CacheClient.MAX_PACKET_LEN);
		lsResponse.order(ByteOrder.BIG_ENDIAN);

		int liTotalLen = 12;
		int liCmdId = CacheDefine.REQ_DELTIMEOUT;
		int liFlag = ciTypeFlag;

		lclsSendBuf.putInt(liTotalLen);
		lclsSendBuf.putInt(liCmdId);

		lclsSendBuf.putInt(liFlag);

		int liResult;
		int liKeySize;
		int liValueSize;
		byte[] lclsKeyBuf;
		byte[] lclsValueBuf;

		TimeoutCBFun timeoutCBFun = new TimeoutCBFun();

		if (!ciShutdown)
		{
			for (int i = 0; i < ciServerCnt && !ciShutdown; i++)
			{
				while (!ciShutdown)
				{
					lclsSendBuf.flip();
					lsResponse.clear();

					try
					{
						if (asyncSendPacket(getCpCacheServer(i).cpoAsyncClient, lclsSendBuf, lsResponse) != 0)
						{
							break;
						}
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
					}

					lsResponse.position(8);
					liResult = lsResponse.getInt();

					if (liResult != 0)
					{
						break;
					}
					else
					{
						liKeySize = lsResponse.getInt();

						lclsKeyBuf = new byte[liKeySize];
						lsResponse.get(lclsKeyBuf, 0, lclsKeyBuf.length);

						liValueSize = lsResponse.getInt();
						lclsValueBuf = new byte[liValueSize];
						lsResponse.get(lclsValueBuf, 0, lclsValueBuf.length);

						timeoutCBFun.delTimeoutCBFun(lclsKeyBuf, lclsValueBuf, lclsValueBuf.length, cpvParam);

						try
						{
							CacheClientLog.log(Level.INFO, "Delete timeout data from CacheServer, key="
									+ new String(lclsKeyBuf));

							Thread.sleep(1);
						}
						catch (InterruptedException e)
						{
							e.printStackTrace();
						}
					}
				}
			}

			if (isCbStartBackupServer() && getCpBackupServer() != null)
			{
				while (!ciShutdown)
				{
					lclsSendBuf.flip();
					lsResponse.clear();

					try
					{
						if (asyncSendPacket(getCpBackupServer().cpoAsyncClient, lclsSendBuf, lsResponse) != 0)
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
						CacheClientLog.log(Level.INFO, "Stop using backup CacheServer");

						setCbStartBackupServer(false);
						break;
					}
					else
					{
						liKeySize = lsResponse.getInt();
						lclsKeyBuf = new byte[liKeySize];
						lsResponse.get(lclsKeyBuf, 0, lclsKeyBuf.length);

						liValueSize = lsResponse.getInt();
						lclsValueBuf = new byte[liValueSize];
						lsResponse.get(lclsValueBuf, 0, lclsValueBuf.length);

						timeoutCBFun.delTimeoutCBFun(lclsKeyBuf, lclsValueBuf, lclsValueBuf.length, cpvParam);

						try
						{
							CacheClientLog.log(Level.CONFIG, "Delete timeout data from CacheServer, key="
									+ new String(lclsKeyBuf));

							Thread.sleep(1);
						}
						catch (InterruptedException e)
						{
							e.printStackTrace();
						}
					}
				}
			}
			aiSleep(ciDelTimeoutInterval, ciShutdown);
		}
		return 0;
	}

	private int getServerPos(String apcKey)
	{
		int ciHashKey = AIString.GetStringHashKey1(apcKey.toCharArray(), CacheDefine.CACHE_HASH_MAX);

		int ciServerPos = -1;
		for (int i = 0; i < ciServerCnt; i++)
		{
			if ((ciHashKey >= cpCacheServer[i].ciMinRange) && (ciHashKey <= cpCacheServer[i].ciMaxRange))
			{
				ciServerPos = i;
				break;
			}
		}
		return ciServerPos;
	}

	public int asyncSendPacket(AsyncClient apoAsyncClient, ByteBuffer aoRequest, ByteBuffer apcResponse)
			throws InterruptedException
	{
		byte[] lbRequest = new byte[aoRequest.limit()];

		if (aoRequest.hasRemaining())
		{
			aoRequest.get(lbRequest, 0, lbRequest.length);

			AsyncMessage aMsg = new AsyncMessage();
			int liRet = apoAsyncClient.sendRequestAndGetResponse(lbRequest, aMsg);

			byte[] lbResponse = aMsg.getContent();

			apcResponse.put(lbResponse);
			apcResponse.flip();

			return liRet;
		}
		else
		{
			return -1;
		}
	}

	public void aiSleep(long aiSecond, boolean aiShutdown)
	{
		for (int i = 0; i < aiSecond && !aiShutdown; i++)
		{
			try
			{
				Thread.sleep(1000);
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
		}
	}

	public void shutdown() throws IOException
	{
		ciShutdown = true;
		for (int i = 0; i < ciServerCnt; i++)
		{
			cpCacheServer[i].cpoAsyncClient.shutdown();
		}
		if (cpBackupServer != null)
		{
			cpBackupServer.cpoAsyncClient.shutdown();
		}
		return;
	}

	public int getCiTypeFlag()
	{
		return ciTypeFlag;
	}

	public boolean getCiShutdown()
	{
		return ciShutdown;
	}

	public int getCiServerCnt()
	{
		return ciServerCnt;
	}

	public CacheServerInfo getCpCacheServer(int i)
	{
		return cpCacheServer[i];
	}

	public Object getCpvParam()
	{
		return cpvParam;
	}

	public boolean isCbStartBackupServer()
	{
		return cbStartBackupServer;
	}

	public void setCbStartBackupServer(boolean cbStartBackupServer)
	{
		this.cbStartBackupServer = cbStartBackupServer;
	}

	public CacheServerInfo getCpBackupServer()
	{
		return cpBackupServer;
	}

	public LinkedList getCpoRetryQueue()
	{
		return cpoRetryQueue;
	}

	public long getCiDelTimeoutInterval()
	{
		return ciDelTimeoutInterval;
	}

	public long getCiRetryDeleteInterval()
	{
		return ciRetryDeleteInterval;
	}

	public String getCsLogFile()
	{
		return csLogFile;
	}

}
