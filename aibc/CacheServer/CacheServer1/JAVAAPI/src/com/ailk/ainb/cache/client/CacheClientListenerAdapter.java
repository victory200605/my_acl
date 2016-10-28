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

public class CacheClientListenerAdapter implements CacheClientListener {

	@Override
	public int delete(ResponseDelete res) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int deleteTimeOut(ResponseDeleteTimeOut res) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int fetch(ResponseFetch res) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int insert(ResponseInsert res) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public void postShutdown() {
		// TODO Auto-generated method stub

	}

	@Override
	public void postStartup() {
		// TODO Auto-generated method stub

	}

	@Override
	public void preShutdown() {
		// TODO Auto-generated method stub

	}

	@Override
	public void preStartup() {
		// TODO Auto-generated method stub

	}

	@Override
	public int errorCommunication(Response res) {
		// TODO Auto-generated method stub
		return 0;
	}

}
