/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.server;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.ailk.ainb.cache.client.CacheClientListener;
import com.ailk.ainb.cache.client.biz.HashValue;
import com.ailk.ainb.cache.client.netty.CacheClientNettyConnection;
import com.ailk.ainb.cache.client.netty.MsgIdGenerator;

public class CacheClientContext {
	private static final Log log = LogFactory.getLog(CacheClientContext.class);
	private final Integer HASH_MAX = 100;
	
	public Set<ServerSocketAddress> serverSocketAddressSet = new HashSet<ServerSocketAddress>();
	public Map<String, CacheClientNettyConnection> connMap = new HashMap<String, CacheClientNettyConnection>();

	/* 注册监听器，当最新的数据获取到后，就调用通知方法 */
	public Set<CacheClientListener> listeners = new HashSet<CacheClientListener>();

	/* 连接失败后，重连间隔，单位：秒 */
	public long reconnectInterval = TimeUnit.SECONDS.toSeconds(5);
	/* 等待心跳超时时长，单位：秒 */
	public long connectionIdle = TimeUnit.SECONDS.toSeconds(5);
	/* 发送心跳间隔时长，单位：秒 */
	public long keepAliveInterval = TimeUnit.SECONDS.toSeconds(10);
	/* 超时等待，单位：秒 */
	public long deleteDataTimeout = TimeUnit.SECONDS.toSeconds(20);

	public long deleteTimeOutSize = 100;

	public boolean close = false;

	public MsgIdGenerator msgIdGen = new MsgIdGenerator();

	/* 需要删除的记录的类型 */

	public Integer deleteTimeOutType = 1;

	public void addCacheClientListener(CacheClientListener listener) {
		listeners.add(listener);
	}

	public void removeCacheClientListenerr(CacheClientListener listener) {
		listeners.remove(listener);
	}

	public CacheClientNettyConnection getProperConnection(String key) {
		int hash = HashValue.hashKey(key, HASH_MAX);
		for (ServerSocketAddress addr : serverSocketAddressSet) {

			if (hash <= addr.getMaxRange() && hash >= addr.getMinRange()) {
				return  connMap.get(addr.getKey());
			}
		}

		return null;
	}
}
