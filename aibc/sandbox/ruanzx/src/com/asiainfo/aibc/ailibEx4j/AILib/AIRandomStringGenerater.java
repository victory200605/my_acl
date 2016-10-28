//����ִ�����

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
	 * ������������һ���������ַ��������Ȳ���ǰ�油0 
	 * 
	 * @param num             ���� 
	 * @param fixdlenth �ַ������� 
	 * @return �������ַ��� 
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
			throw new RuntimeException("������" + num + "ת��Ϊ����Ϊ" + fixdlenth + "���ַ��������쳣��");
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
			throw new RuntimeException("������" + num + "ת��Ϊ����Ϊ" + fixdlenth + "���ַ��������쳣��");
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
