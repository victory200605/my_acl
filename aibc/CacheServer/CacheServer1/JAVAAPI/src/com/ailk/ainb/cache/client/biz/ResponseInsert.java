/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.biz;

public class ResponseInsert extends Response{
	@Override
	public String toString() {
		StringBuilder buff = new StringBuilder();
		buff.append("insert response[msgId=").append(super.getMsgId()).append(",result=").append(super.getResult()).append("]");
		return buff.toString();
	}
}
