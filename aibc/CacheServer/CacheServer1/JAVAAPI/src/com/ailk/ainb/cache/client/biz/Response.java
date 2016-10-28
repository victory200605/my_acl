/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.biz;

public class Response {
	private int msgId;

	private int result;

	public int getMsgId() {
		return msgId;
	}

	public void setMsgId(int msgId) {
		this.msgId = msgId;
	}

	public int getResult() {
		return result;
	}

	public void setResult(int result) {
		this.result = result;
	}

	@Override
	public String toString() {
		StringBuilder buff = new StringBuilder();
		buff.append("response[msgId=").append(msgId).append(",result=").append(result).append("]");
		return buff.toString();
	}


}
