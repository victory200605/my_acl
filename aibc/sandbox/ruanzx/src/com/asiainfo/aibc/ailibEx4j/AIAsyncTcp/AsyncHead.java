package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class AsyncHead
{
	public static final int HEADER_SIZE = 12;

	private int ciMsgID;
	private int ciResult;
	private int ciLength;

	public AsyncHead()
	{
		ciMsgID = 0;
		ciResult = 0;
		ciLength = 0;
	}

	public AsyncHead(int aiMsgID, int aiResult, int aiLength)
	{
		this.ciMsgID = aiMsgID;
		this.ciResult = aiResult;
		this.ciLength = aiLength;
	}

	public synchronized void parseHeader(ByteBuffer bufHead)
	{
		ciMsgID = bufHead.getInt();
		ciResult = bufHead.getInt();
		ciLength = bufHead.getInt();
	}

	public synchronized ByteBuffer buildHeader(int msgID, int result, int length)
	{
		ByteBuffer bufHead = ByteBuffer.allocate(HEADER_SIZE);
		bufHead.clear();

		bufHead.order(ByteOrder.BIG_ENDIAN);

		bufHead.putInt(msgID);
		bufHead.putInt(result);
		bufHead.putInt(length);

		return bufHead;
	}

	public int getCiMsgID()
	{
		return ciMsgID;
	}

	public int getCiResult()
	{
		return ciResult;
	}

	public int getCiLength()
	{
		return ciLength;
	}
}