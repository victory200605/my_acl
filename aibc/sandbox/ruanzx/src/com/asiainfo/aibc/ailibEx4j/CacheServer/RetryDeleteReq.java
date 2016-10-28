package com.asiainfo.aibc.ailibEx4j.CacheServer;

public class RetryDeleteReq
{
		private String cpcKey;
		private ifRetryDeleteCBFun cRetryDeleteCBFun;
		private Object cpvParam;
		private long ciInQueueTime;
		public String getCpcKey()
		{
			return cpcKey;
		}
		public ifRetryDeleteCBFun getCRetryDeleteCBFun()
		{
			return cRetryDeleteCBFun;
		}
		public Object getCpvParam()
		{
			return cpvParam;
		}
		public long getCiInQueueTime()
		{
			return ciInQueueTime;
		}
		public void setCpcKey(String cpcKey)
		{
			this.cpcKey = cpcKey;
		}
		public void setCRetryDeleteCBFun(ifRetryDeleteCBFun retryDeleteCBFun)
		{
			cRetryDeleteCBFun = retryDeleteCBFun;
		}
		public void setCpvParam(Object cpvParam)
		{
			this.cpvParam = cpvParam;
		}
		public void setCiInQueueTime(long ciInQueueTime)
		{
			this.ciInQueueTime = ciInQueueTime;
		}
}
