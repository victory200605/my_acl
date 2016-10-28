package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

public class AITestParallel extends AITestAction
{
	private interface ifThreadFunc
	{
		public void threadFunc(Object apParam);
	}

	private class ThreadDoFunc implements ifThreadFunc
	{

		public void threadFunc(Object apParam)
		{
			StParam lpoParam = (StParam) apParam;
			lpoParam.cpoTestAction.testDo(lpoParam);
		}
	}

	protected int ciParallel;
	protected Thread[] cpThreadHandle;
	protected ifThreadFunc cpfThreadDo;

	public AITestParallel(int aiParallel)
	{
		ciParallel = aiParallel;

		setThreadDoFunc(new ThreadDoFunc());
		
		cpThreadHandle = new Thread[ciParallel];
		
	}

	protected void setThreadDoFunc(ifThreadFunc apThreadFunc)
	{
		cpfThreadDo = apThreadFunc;
	}
	
	
}
