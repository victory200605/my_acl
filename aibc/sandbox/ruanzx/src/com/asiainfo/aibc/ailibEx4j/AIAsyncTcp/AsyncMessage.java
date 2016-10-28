package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

public class AsyncMessage
{
	private AsyncHead ahead;
	private byte[] content;

	public AsyncMessage()
	{
		ahead = null;
		content = null;
	}

	public AsyncMessage(AsyncHead ahead, byte[] content)
	{
		this.ahead = ahead;
		this.content = content;
	}

	public void setAhead(AsyncHead ahead)
	{
		this.ahead = ahead;
	}

	public void setContent(byte[] content)
	{
		this.content = content;
	}

	public AsyncHead getAhead()
	{
		return ahead;
	}
	
	public int setAsyncMsg(AsyncMessage aMsg)
	{
		this.ahead=aMsg.getAhead();
		this.content=aMsg.getContent();
		return 0;
	}

	public byte[] getContent()
	{
		return content;
	}
}
