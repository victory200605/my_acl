package com.asiainfo.aibc.ailibEx4j.AILib;

public class AIString
{
	public static int GetStringHashKey1(char[] apcString, int aiMaxHashKey)
	{
		int liChar = 0;
		int liHashKey = 0;
		int liHashVal = 5381;

		for (int i = 0; i < apcString.length; ++i)
		{	
			liChar = Character.digit(apcString[i], 10);
			liHashVal = ((liHashVal << 5) + liHashVal) + liChar;
		}
		
		liHashKey = (int) (liHashVal % aiMaxHashKey);
		return (liHashKey);
	}
}
