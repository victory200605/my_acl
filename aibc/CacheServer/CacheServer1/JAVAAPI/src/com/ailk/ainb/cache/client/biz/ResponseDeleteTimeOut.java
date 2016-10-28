/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.biz;

public class ResponseDeleteTimeOut extends Response {

	private String key;
	private String value;

	public String getKey() {
		return key;
	}

	public void setKey(String key) {
		this.key = key;
	}

	public String getValue() {
		return value;
	}

	public void setValue(String value) {
		this.value = value;
	}

	@Override
	public String toString() {
		StringBuilder buff = new StringBuilder();
		buff.append("deleteTimeOut response[msgId=").append(super.getMsgId()).append(
				",result=").append(super.getResult()).append(",key=").append(
				key).append(",value=").append(value).append("]");
		return buff.toString();
	}

}
