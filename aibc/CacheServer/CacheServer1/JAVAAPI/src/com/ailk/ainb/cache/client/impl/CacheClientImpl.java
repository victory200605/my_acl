/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.impl;

import java.net.InetSocketAddress;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.ailk.ainb.cache.client.CacheClient;
import com.ailk.ainb.cache.client.CacheClientListener;
import com.ailk.ainb.cache.client.biz.Constants;
import com.ailk.ainb.cache.client.config.CacheClientConfig;
import com.ailk.ainb.cache.client.config.Configuration;
import com.ailk.ainb.cache.client.config.Server;
import com.ailk.ainb.cache.client.netty.CacheClientNettyConnection;
import com.ailk.ainb.cache.client.server.CacheClientContext;
import com.ailk.ainb.cache.client.server.ServerSocketAddress;

public class CacheClientImpl implements CacheClient {
	private static final Log log = LogFactory.getLog(CacheClientImpl.class);
	private final CacheClientContext ctx;
	private Set<ServerSocketAddress> address;
	private ServerSocketAddress bak;

	private final CacheClientConfig config;

	public CacheClientImpl() {
		this.ctx = new CacheClientContext();

		config = new Configuration().loadConfig();

		setValue();
	}

	/**
	 * 初始化数据
	 */
	public void setValue() {
		long reconnectInterval = config.getReconnectInterval();
		//long keepAliveInterval = config.getKeepAliveInterval();
		long deleteDataTimeout = config.getDeleteDataTimeout();
		long deleteTimeOutSize = config.getDeleteTimeOutSize();

		if (reconnectInterval > 0)
			setReconnectInterval(reconnectInterval);
//		if (keepAliveInterval > 0)
//			setKeepAliveInterval(keepAliveInterval);
		if (deleteDataTimeout > 0)
			setDeleteDataTimeout(deleteDataTimeout);
		if (deleteTimeOutSize > 0)
			setDeleteTimeOutSize(deleteTimeOutSize);

		List<Server> list = config.getList();

		validata(list);

		Set<ServerSocketAddress> address = new HashSet<ServerSocketAddress>();
		for (Server server : list) {
			ServerSocketAddress add = new ServerSocketAddress(new InetSocketAddress(server.getAddress(), server.getPort()), server.getMinRange(),
					server.getMaxRange());
			address.add(add);
		}
		setSocketAddress(address);
	}

	/**
	 * 验证数据
	 *
	 * @param list
	 * @param server
	 */
	public void validata(List<Server> list) {
		if (list.isEmpty()) {
			throw new IllegalArgumentException("cacheServer配置不正确");
		}
		Map<String, Integer> map = new HashMap<String, Integer>();
		for (Server server : list) {
			String key = server.getAddress() + ":" + server.getPort();
			if (map.get(key) == null) {
				map.put(key, 0);
			} else {
				throw new IllegalArgumentException("cacheServer的IP端口 " + key + " 不能相同");
			}
		}

		Collections.sort(list, new Comparator<Server>() {
			@Override
			public int compare(Server arg0, Server arg1) {
				if (arg0.getMinRange() > arg1.getMinRange()) {
					return 1;
				}
				return 0;
			}

		});
		if (list.get(0).getMinRange() != 0) {
			throw new IllegalArgumentException("IP:" + list.get(0).getAddress() + " port:" + list.get(0).getPort() + "的minRange值不正确");
		}
		if (list.get(list.size() - 1).getMaxRange() != 99) {
			throw new IllegalArgumentException("IP:" + list.get(list.size() - 1).getAddress() + " port:" + list.get(list.size() - 1).getPort()
					+ "的maxRange值不正确");
		}
		Server s = null;
		for (Server server : list) {
			if (s == null) {
				s = server;
			} else {
				if ((s.getMaxRange() + 1) == server.getMinRange()) {
					s = server;
				} else {
					throw new IllegalArgumentException("IP:" + server.getAddress() + " port:" + server.getPort() + "的minRange值不正确");
				}
			}

		}
	}

	@Override
	public void addListener(CacheClientListener listener) {
		ctx.addCacheClientListener(listener);

	}

	@Override
	public long getConnectionIdle() {
		// TODO Auto-generated method stub
		return ctx.connectionIdle;
	}

	@Override
	public long getKeepAliveInterval() {
		// TODO Auto-generated method stub
		return ctx.keepAliveInterval;
	}

	@Override
	public long getReconnectInterval() {
		// TODO Auto-generated method stub
		return ctx.reconnectInterval;
	}

	@Override
	public long getDeleteDataTimeout() {
		// TODO Auto-generated method stub
		return ctx.deleteDataTimeout;
	}

	@Override
	public void removeListener(CacheClientListener listener) {
		ctx.removeCacheClientListenerr(listener);

	}

	@Override
	public void setConnectionIdle(long millis) {
		ctx.connectionIdle = millis;

	}

	@Override
	public void setKeepAliveInterval(long millis) {
		ctx.keepAliveInterval = millis;

	}

	@Override
	public void setReconnectInterval(long millis) {
		ctx.reconnectInterval = millis;

	}

	@Override
	public void setDeleteDataTimeout(long millis) {
		ctx.deleteDataTimeout = millis;

	}

	/**
	 *
	 */
	@Override
	public void shutdown() {
		ctx.close = true;
		for (CacheClientListener listener : ctx.listeners) {
			listener.preShutdown();
		}
		for (CacheClientNettyConnection conn : ctx.connMap.values()) {
			conn.close();
		}
		for (CacheClientListener listener : ctx.listeners) {
			listener.postShutdown();
		}
		ctx.connMap.clear();

	}

	@Override
	public void setBakSocketAddress(ServerSocketAddress bak) {
		this.bak = bak;
	}

	/**
	 * 启动API将连接到不同的cacheServer
	 */
	@Override
	public void startup() throws Exception {

		for (CacheClientListener listener : ctx.listeners) {
			listener.preStartup();
		}
		for (ServerSocketAddress addr : this.address) {
			connectToCacheServer(addr);
		}

		if (this.bak != null) {
			CacheClientNettyConnection conn = new CacheClientNettyConnection(ctx, this.bak);
			ctx.connMap.put(Constants.BAK_KEY, conn);
			conn.connect();
		}
		for (CacheClientListener listener : ctx.listeners) {
			listener.postStartup();
		}
	}

	private void connectToCacheServer(ServerSocketAddress serverAddress) {
		CacheClientNettyConnection conn = new CacheClientNettyConnection(ctx, serverAddress);
		ctx.connMap.put(serverAddress.getKey(), conn);
		conn.connect();
	}

	@Override
	public int delete(String key) {
		int msgId = ctx.msgIdGen.next();
		CacheClientNettyConnection ccnc = ctx.getProperConnection(key);
		boolean isWriteSucc = (ccnc != null && ccnc.write(ChannelBufferValue.getValue(key, Constants.REQ_DELETE, msgId)));
		if(log.isDebugEnabled()){
			log.debug("delete get CacheServer addr:"+ccnc.getAddr()+",key:"+key);
			log.debug("delete key:"+key+",msgId:"+msgId+",is isWriteSucc:"+isWriteSucc);
		}
		return isWriteSucc ? msgId : Constants.ERROR_FAIL;
	}

	@Override
	public int fetch(String key) {
		int msgId = ctx.msgIdGen.next();
		CacheClientNettyConnection ccnc = ctx.getProperConnection(key);
		boolean isWriteSucc = (ccnc != null && ccnc.write(ChannelBufferValue.getValue(key, Constants.REQ_FETCH, msgId)));
		if(log.isDebugEnabled()){
			log.debug("fetch get CacheServer addr:"+ccnc.getAddr()+",key:"+key);
			log.debug("fetch key:"+key+",msgId:"+msgId+",is isWriteSucc:"+isWriteSucc);
		}
		return isWriteSucc ? msgId : Constants.ERROR_FAIL;
	}

	@Override
	public int insert(String key, String value) {
		int msgId = ctx.msgIdGen.next();
		CacheClientNettyConnection conn = ctx.getProperConnection(key);
		boolean isWriteSucc = (conn != null && conn.write(ChannelBufferValue.getValue(key, value, msgId)));
		if(log.isDebugEnabled()){
			log.debug("insert get CacheServer addr:"+conn.getAddr()+",key:"+key);
			log.debug("insert key:"+key+",value:"+value+",msgId:"+msgId+",is isWriteSucc:"+isWriteSucc);
		}
		return isWriteSucc ? msgId : Constants.ERROR_FAIL;
	}

	@Override
	public void setSocketAddress(Set<ServerSocketAddress> address) {
		this.address = address;
		ctx.serverSocketAddressSet.clear();
		ctx.serverSocketAddressSet.addAll(address);
	}

	@Override
	public Integer getDeleteTimeOut() {
		return ctx.deleteTimeOutType;
	}

	@Override
	public long getDeleteTimeOutSize() {
		return ctx.deleteTimeOutSize;
	}

	@Override
	public void setDeleteTimeOutSize(long size) {
		ctx.deleteTimeOutSize = size;
	}
}
