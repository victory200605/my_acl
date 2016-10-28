/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client;

import com.ailk.ainb.cache.client.biz.Response;
import com.ailk.ainb.cache.client.biz.ResponseDelete;
import com.ailk.ainb.cache.client.biz.ResponseDeleteTimeOut;
import com.ailk.ainb.cache.client.biz.ResponseFetch;
import com.ailk.ainb.cache.client.biz.ResponseInsert;



public interface CacheClientListener {
	/**
	 * 回调函数处理插入后结果
	 * @param res
	 * @return
	 */
	public int insert(ResponseInsert res);
	/**
	 * 回调函数处理删除后结果
	 * @param res
	 * @return
	 */
	public int delete(ResponseDelete res);
	/**
	 * 回调函数处理获取后结果
	 * @param res
	 * @return
	 */
	public int fetch(ResponseFetch res);
	/**
	 * 回调函数处理删除超时后结果
	 * @param res
	 * @return
	 */
	public int deleteTimeOut(ResponseDeleteTimeOut res);
	/**
	 * 回调函数处理通讯错误
	 *
	 */
	public int errorCommunication(Response res);
	/**
	 * 当Api与某CacheServer建立连接之前回调此方法
	 *
	 *            建立连接的CacheServer地址信息
	 */
	public void preStartup();
	/**
	 * 当Api与某CacheServer建立连接之后回调此方法
	 *
	 *            建立连接的CacheServer地址信息
	 */
	public void postStartup();
	/**
	 * 当Api与某CacheServer连接断开之前回调此方法
	 */
	public void preShutdown();
	/**
	 * 当Api与某CacheServer连接断开之后回调此方法
	 *
	 *            断开连接的CacheServer地址信息
	 */
	public void postShutdown();
}
