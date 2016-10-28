/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.config;

import java.util.List;

import com.thoughtworks.xstream.annotations.XStreamAlias;
import com.thoughtworks.xstream.annotations.XStreamImplicit;

@XStreamAlias("CacheClientConfig")
public class CacheClientConfig {

	private long reconnectInterval;
	//private long keepAliveInterval;
	private long deleteDataTimeout;
	private long deleteTimeOutSize;
	@XStreamImplicit(itemFieldName="Server")
	private List<Server> list;

	public List<Server> getList() {
		return list;
	}
	public void setList(List<Server> list) {
		this.list = list;
	}
	public long getReconnectInterval() {
		return reconnectInterval;
	}
	public void setReconnectInterval(long reconnectInterval) {
		this.reconnectInterval = reconnectInterval;
	}
//	public long getKeepAliveInterval() {
//		return keepAliveInterval;
//	}
//	public void setKeepAliveInterval(long keepAliveInterval) {
//		this.keepAliveInterval = keepAliveInterval;
//	}
	public long getDeleteDataTimeout() {
		return deleteDataTimeout;
	}
	public void setDeleteDataTimeout(long deleteDataTimeout) {
		this.deleteDataTimeout = deleteDataTimeout;
	}
	public long getDeleteTimeOutSize() {
		return deleteTimeOutSize;
	}
	public void setDeleteTimeOutSize(long deleteTimeOutSize) {
		this.deleteTimeOutSize = deleteTimeOutSize;
	}

}
