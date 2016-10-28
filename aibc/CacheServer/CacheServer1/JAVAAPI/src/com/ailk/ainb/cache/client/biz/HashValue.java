/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.biz;


public class HashValue {
	/**
	 * Time 33哈希算法
	 *
	 * @param key
	 * @param prime
	 * @return
	 */
	public static int hashKey(String key, int prime)

	{

		int liChar = 0;

		int liHashKey = 0;

		long liHashVal = 0;

		for (int liIt = 0; liIt < key.length(); liIt++)

		{

			liChar = key.charAt(liIt);

			liHashVal += liHashVal * 33 + liChar;

		}

		liHashKey = (int) (liHashVal % prime);

		return Math.abs(liHashKey);

	}

	public static void main(String[] args) {

	}
}
