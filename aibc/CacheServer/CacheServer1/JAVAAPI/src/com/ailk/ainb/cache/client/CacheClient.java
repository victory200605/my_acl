/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client;

import java.util.Set;

import com.ailk.ainb.cache.client.server.ServerSocketAddress;



public interface CacheClient {

	/**
	 * 获取当前重连间隔配置值
	 *
	 * @return 连接超时值（毫秒）
	 */
	public long getReconnectInterval();

	/**
	 * 设置重连间隔值
	 *
	 * @param millis
	 *            连接超时值（毫秒）
	 */
	public void setReconnectInterval(long millis);

	/**
	 * 获取当前心跳间隔配置值
	 *
	 * @return 心跳间隔值（毫秒）
	 */
	public long getKeepAliveInterval();

	/**
	 * 设置心跳间隔值，当连续一段事件内没有任何消息发送到CacheServer时，则自动发送心跳包。
	 *
	 * 注意心跳间隔值必须比连接超时值小。
	 *
	 * @param millis
	 *            心跳间隔值（毫秒）
	 */
	public void setKeepAliveInterval(long millis);

	/**
	 * 获取当前连接超时配置值
	 *
	 * @return 连接超时值（毫秒）
	 */
	public long getConnectionIdle();

	/**
	 * 设置连接超时值，当连续一段时间内CacheServer没有发送任何消息时（包括心跳、心跳响应），则断开连接
	 *
	 * @param millis
	 *            连接超时值（毫秒）
	 */
	public void setConnectionIdle(long millis);

	/**
	 * 获取删除超时等待时间
	 *
	 * @return 超时值（秒）
	 */
	public long getDeleteDataTimeout();

	/**
	 * 设置删除超时等待时间
	 *
	 * @param millis
	 *            更新超时值（秒）
	 */
	public void setDeleteDataTimeout(long millis);


	/**
	 * 启动CacheClient，自动连接CacheServer并实时获取数据更新
	 */
	public void startup() throws Exception;

	/**
	 * 停止CacheClient，断开与所有CacheServer之间的连接
	 *
	 */
	public void shutdown();

	/**
	 * 插入数据
	 * @param key
	 * @param value
	 * @return
	 */
	public int insert(String key,String value);
	/**
	 * 删除保存的key
	 * @param key
	 * @return
	 */
	public int delete(String key);
	/**
	 * 获取key对应的value
	 * @param key
	 * @return
	 */
	public int fetch(String key);

	/**
	 * 设置连接的cacheServer地址
	 * @param address
	 */
	public void setSocketAddress(Set<ServerSocketAddress> address);

	/**
	 * 设置备份的cacheServer地址
	 * @param bak
	 */
	public void setBakSocketAddress(ServerSocketAddress bak);

	/**
	 * 增加监听器，获取数据变化事件。当获取到新版本的路由数据后，回调所有listener
	 *
	 * 注意：务必在启动网络连接之前加入listener，否则有可能丢失通知
	 *
	 * @param listener
	 *            监听器对象
	 */
	public void addListener(CacheClientListener listener);

	/**
	 * 移除指定的监听器
	 *
	 * @param listener
	 *            监听器对象
	 */
	public void removeListener(CacheClientListener listener);


	/**
	 * 设置删除超时时间返回要休眠时的休眠时间
	 * @return
	 */
	public Integer getDeleteTimeOut();

	/**
	 * 设置删除超时条数
	 *
	 */
	public void setDeleteTimeOutSize(long size);

	public long getDeleteTimeOutSize();

}
