package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

public class MatchNode
{
	private int msgID; //message id
	private int result; //message status
	private int length; //message body length
	private byte[] content; //message body content     

	public MatchNode()
	{
		msgID = 0;
		length = 0;
		result = 0;
		content = null;
	}

	public synchronized int getMsgID()
	{
		return msgID;
	}

	public synchronized int getLength()
	{
		return length;
	}

	public synchronized int getResult()
	{
		return result;
	}

	public synchronized byte[] getContent()
	{
		return content;
	}

	public synchronized void setMsgID(int msgID)
	{
		this.msgID = msgID;
	}

	public synchronized void setLength(int length)
	{
		this.length = length;
	}

	public synchronized void setResult(int result)
	{
		this.result = result;
	}

	public synchronized void setContent(byte[] content)
	{
		this.content = content;
	}

}
