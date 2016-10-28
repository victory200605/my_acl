/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.biz;

public class ResponseDelete extends Response{
	@Override
	public String toString() {
		StringBuilder buff = new StringBuilder();
		buff.append("delete response[msgId=").append(super.getMsgId()).append(",result=").append(super.getResult()).append("]");
		return buff.toString();
	}
}
