package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

public class AITestAction
{
	protected AITestCase cpoCase;
	protected AITestAction cpParent;

	protected class StParam
	{
		public AITestAction cpoTestAction;
		public Object cpParam;
	}

	public void setTestCase(AITestCase apoCase)
	{
		cpoCase = apoCase;
	}

	public int doIt(Object apParam)
	{
		int liRet = 0;

		if ((liRet = cpoCase.DoIt(apParam)) == 0)
		{
			cpoCase.AddSuccess();
		}
		else
		{
			cpoCase.AddReject();
		}
		return liRet;
	}

	public int testDo(StParam aoParam)
	{
		while (cpoCase.GetRepeatCount() > 0)
		{
			aoParam.cpoTestAction.doIt(aoParam.cpParam);
			cpoCase.DecreaseRepeatCount();
		}
		return 0;
	}

	public AITestAction getParent()
	{
		return cpParent;
	}

	public int run(Object apParam)
	{
		long liStart = System.currentTimeMillis();

		StParam loParam = new StParam();
		loParam.cpoTestAction = this;
		loParam.cpParam = apParam;

		//		loParam.cpoTestAction.testDo(loParam);
		run(loParam);

		cpoCase.SetUseTime((System.currentTimeMillis() - liStart)/1000);

		return 0;
	}

	public int run(StParam aoParam)
	{
		aoParam.cpoTestAction.testDo(aoParam);
		return 0;
	}

}
