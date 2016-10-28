package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

import java.io.IOException;

import com.asiainfo.aibc.ailibEx4j.AILib.AIGetOpt;
import com.asiainfo.aibc.ailibEx4j.CacheServer.CacheDefine;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.AITest.AIDelTestCase;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.AITest.AIInsertTestCase;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.AITest.AITestAction;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.AITest.AITestCase;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.AITest.AITestControl;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.AITest.AITestParallel;
import com.asiainfo.aibc.ailibEx4j.CacheServer.test.AITest.AITestSnapshot;

public class MyMainInit
{
	public final static String CACHECLIENT_CONFIG_FILE = "./config/CacheClient.properties";

	public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException
	{
		boolean lbInsert = false;
		boolean lbDelete = false;
		boolean lbDeleteTimeOut = false;
		int liStatisticalTimeInterval = 0;
		int liMaxUnitFlow = 0;
		int liThreadCnt = 1;
		int liCount = 0;
		int liStartKey = 0;
		char liOptChar;
		int liStep = 1;

		ClsParam loParam = new ClsParam();
		loParam.setCpoKey(null);
		loParam.setCbIsTerminate(false);

		AIGetOpt go = new AIGetOpt("idl:s:f:t:n:k:poe:");
		while ((liOptChar = go.getopt(args)) != 0)
		{
			switch (liOptChar)
			{
			case 'i':
				lbInsert = true;
				break;
			case 'd':
				lbDelete = true;
				break;
			case 's':
				liStatisticalTimeInterval = Integer.parseInt(go.optarg());
				break;
			case 'f':
				loParam.setCpoKey(new ClsFileKey(go.optarg()));
				break;
			case 't':
				liThreadCnt = Integer.parseInt(go.optarg());
				break;
			case 'l':
				liCount = Integer.parseInt(go.optarg());
				break;
			case 'n':
				liMaxUnitFlow = Integer.parseInt(go.optarg());
				break;
			case 'k':
				liStartKey = Integer.parseInt(go.optarg());
				break;
			case 'p':
				loParam.setCbIsTerminate(true);
				break;
			case 'o':
				lbDeleteTimeOut = true;
				break;
			case 'e':
				liStep = Integer.parseInt(go.optarg());
				break;
			default:
				System.out.println("x");
				break;
			}
		}
		if (loParam.getCpoKey() == null)
		{
			loParam.setCpoKey(new ClsKey(liStartKey, liStep));
		}

		if (loParam.getCoCacheClient().init(CacheDefine.CACHE_TYPE_SM, null, 0, CACHECLIENT_CONFIG_FILE) != 0)
		{
			System.out.println("Cache client initialize fail, may be connect rejected");
		}

		AITestCase lpCase = null;
		if (lbInsert)
		{

			lpCase = new AIInsertTestCase();
		}
		else if (lbDelete)
		{
			lpCase = new AIDelTestCase();
		}
		else if (lbDeleteTimeOut)
		{
			loParam.getCoCacheClient().delTimeout();
			System.out.println("Success delete timeout data");
			System.exit(0);
		}

		lpCase.setRepeatCount(liCount);

		AITestAction lpTestAction = null;
		if (liThreadCnt > 1)
		{
			lpTestAction = new AITestParallel(liThreadCnt);
		}
		else
		{
			lpTestAction = new AITestAction();
		}

		AITestSnapshot lpSnapshot = null;
		if (liStatisticalTimeInterval > 0)
		{
			lpSnapshot = new AITestSnapshot(lpTestAction, liStatisticalTimeInterval);
			lpTestAction = lpSnapshot;
		}

		if (liMaxUnitFlow > 0)
		{
			lpTestAction = new AITestControl(lpTestAction, 1000, liMaxUnitFlow);
		}

		lpTestAction.setTestCase(lpCase);
		lpTestAction.run(loParam);

		System.out.println("--------------------------Complete Test--------------------------------\n");
		System.out.println(" Total Record count              = " + (lpCase.getSuccess() + lpCase.getReject()));
		System.out.println(" Success Record count            = " + lpCase.getSuccess());
		System.out.println(" Reject Record count             = " + lpCase.getReject());
		System.out.println(" TimeOut Record count            = " + TimeoutCBFun.giTimeCount);
		System.out.println(" Total use time(s)               = " + lpCase.getUseTime());
		if (lpSnapshot != null)
		{
			System.out.println(" Avg delay time(us)              = " + lpSnapshot.getAvgDelayTime());
			System.out.println(" Max delay time(us)              = " + lpSnapshot.getMaxDelayTime());
			System.out.println(" Min delay time(us)              = " + lpSnapshot.getMinDelayTime());
		}
		System.out.println(" Record count per                = " + lpCase.getFrequency() + "/s");

		//		return;
		System.exit(0);
	}
}
