package com.ailk.ainb.cache.client.netty;

public class MsgIdGenerator {
	private int sn = 0;

	public synchronized int next() {
		if (sn < Integer.MAX_VALUE) {
			++sn;
		} else {
			sn = 0;
		}
		return sn;
	}
}
