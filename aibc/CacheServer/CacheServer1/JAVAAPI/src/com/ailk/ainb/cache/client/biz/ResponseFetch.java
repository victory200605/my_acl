/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.biz;

public class ResponseFetch extends Response{

	private String value;

	public String getValue() {
		return value;
	}
	public void setValue(String value) {
		this.value = value;
	}
	@Override
	public String toString() {
		StringBuilder buff = new StringBuilder();
		buff.append("fetch response[msgId=").append(super.getMsgId()).append(
				",result=").append(super.getResult()).append(",value=").append(value).append("]");
		return buff.toString();
	}
}
