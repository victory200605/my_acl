//随机字串产生

package com.asiainfo.aibc.ailibEx4j.AILib;

import java.util.Random;

public class AIRandomStringGenerater
{
	public static final String allChar = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	public static final String numberChar = "0123456789";

	public static String generateString(int length)
	{
		StringBuffer sb = new StringBuffer();
		Random random = new Random();
		for (int i = 0; i < length; i++)
		{
			sb.append(allChar.charAt(random.nextInt(allChar.length())));
		}
		return sb.toString();
	}

	public static String generateNumString(int length)
	{
		StringBuffer sb = new StringBuffer();
		Random random = new Random();
		for (int i = 0; i < length; i++)
		{
			sb.append(numberChar.charAt(random.nextInt(numberChar.length())));
		}
		return sb.toString();
	}

	public static String generateZeroString(int length)
	{
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < length; i++)
		{
			sb.append('0');
		}
		return sb.toString();
	}

	/** 
	 * 根据数字生成一个定长的字符串，长度不够前面补0 
	 * 
	 * @param num             数字 
	 * @param fixdlenth 字符串长度 
	 * @return 定长的字符串 
	 */
	public static String toFixdLengthString(long num, int fixdlenth)
	{
		StringBuffer sb = new StringBuffer();
		String strNum = String.valueOf(num);
		if (fixdlenth - strNum.length() >= 0)
		{
			sb.append(generateZeroString(fixdlenth - strNum.length()));
		}
		else
		{
			throw new RuntimeException("将数字" + num + "转化为长度为" + fixdlenth + "的字符串发生异常！");
		}
		sb.append(strNum);
		return sb.toString();
	}

	public static String toFixdLengthString(int num, int fixdlenth)
	{
		StringBuffer sb = new StringBuffer();
		String strNum = String.valueOf(num);
		if (fixdlenth - strNum.length() >= 0)
		{
			sb.append(generateZeroString(fixdlenth - strNum.length()));
		}
		else
		{
			throw new RuntimeException("将数字" + num + "转化为长度为" + fixdlenth + "的字符串发生异常！");
		}
		sb.append(strNum);
		return sb.toString();
	}

	public static void main(String[] args)
	{
		int a=1;
		
		System.out.println(toFixdLengthString(a,10));
	}
}
