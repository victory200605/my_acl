package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

public class AITestControl extends AITestAction
{
	protected AITestAction cpoParentAction;
	protected int ciUnitTime;
	protected int ciUnitRepeat;
	protected int ciUnitRepeatConst;

	// AIMutexLock coControlLock;
	Object coControlCond = new Object();

	protected class ThreadControlFunc implements Runnable
	{
		private AITestControl lpTestControl;
		private StParam lpParam;

		public ThreadControlFunc(Object apParam)
		{
			lpParam = (StParam) (apParam);
			lpTestControl = (AITestControl) (lpParam.cpoTestAction);
		}

		public void run()
		{
			while (true)
			{
				lpTestControl.ciUnitRepeat = lpTestControl.ciUnitRepeatConst;
				synchronized (coControlCond)
				{
					coControlCond.notifyAll();
				}
				try
				{
					Thread.sleep(lpTestControl.ciUnitTime);
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
				}
			}
		}
	}

	public AITestControl(AITestAction apoParentAction, int aiUnitTime, int aiUnitRepeat)
	{
		cpoParentAction = apoParentAction;
		ciUnitTime = aiUnitTime;
		ciUnitRepeat = aiUnitRepeat;
		ciUnitRepeatConst = aiUnitRepeat;
	}

	public int doIt(Object apParam)
	{
		return cpoParentAction.doIt(apParam);
	}

	public int run(StParam aoParam)
	{
		cpParent = aoParam.cpoTestAction;

		StParam loParam = aoParam;
		loParam.cpoTestAction = this;

		new Thread(new ThreadControlFunc(loParam)).start();

		return cpoParentAction.run(aoParam);
	}

	public int testDo(StParam aoParam)
	{
		synchronized (coControlCond)
		{
			try
			{
				coControlCond.wait();
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
		}

		while (cpoCase.GetRepeatCount() > 0)
		{
			synchronized (coControlCond)
			{
				while (ciUnitRepeat-- > 0 && cpoCase.GetRepeatCount() > 0)
				{
					aoParam.cpoTestAction.doIt(aoParam.cpParam);
					cpoCase.DecreaseRepeatCount();
				}

				try
				{
					coControlCond.wait(ciUnitTime);
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
				}
			}
		}

		return 0;
	}

	public void setTestCase(AITestCase apoCase)
	{
		cpoCase = apoCase;
		if (cpoParentAction != null)
		{
			cpoParentAction.setTestCase(apoCase);
		}
	}
	
	public AITestCase getTestCase()
	{
		return cpoCase;
	}
}
