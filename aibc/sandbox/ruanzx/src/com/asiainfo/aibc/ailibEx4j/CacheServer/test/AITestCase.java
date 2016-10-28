package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

public abstract class AITestCase
{
	//未使用多线程的count
	protected int ciRepeatCount;
	protected StStat coStat;

	protected class StStat
	{
		public int ciSuccess;
		public int ciReject;
		public long ciUseTime;

		public StStat()
		{
			ciSuccess = 0;
			ciReject = 0;
			ciUseTime = 0;
		}
	}

	public AITestCase()
	{
		ciRepeatCount = 1;
		coStat = new StStat();
	}

	public AITestCase(int aiCount)
	{
		ciRepeatCount = aiCount;
		coStat = new StStat();
	}

	public void SetRepeatCount(int aiCount)
	{
		ciRepeatCount = aiCount;
	}

	public abstract int DoIt(Object apParam);

	public synchronized void AddSuccess()
	{
		coStat.ciSuccess++;
	}

	public synchronized void AddReject()
	{
		coStat.ciReject++;
	}

	public void SetUseTime(long aiUseTime)
	{
		coStat.ciUseTime = aiUseTime;
	}

	public void IncreaseRepeatCount()
	{
		ciRepeatCount++;
	}

	public void DecreaseRepeatCount()
	{
		ciRepeatCount--;
	}

	public int GetRepeatCount()
	{
		return ciRepeatCount;
	}

	public int GetSuccess()
	{
		return coStat.ciSuccess;
	}

	public int GetReject()
	{
		return coStat.ciReject;
	}

	public long GetUseTime()
	{
		return coStat.ciUseTime;
	}

	public long GetFrequency()
	{
		return ((coStat.ciSuccess + coStat.ciReject) / (coStat.ciUseTime > 0 ? coStat.ciUseTime : 1));
	}
}
