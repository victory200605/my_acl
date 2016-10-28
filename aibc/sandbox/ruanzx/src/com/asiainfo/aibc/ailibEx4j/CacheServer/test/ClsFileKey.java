package com.asiainfo.aibc.ailibEx4j.CacheServer.test;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class ClsFileKey extends ClsKey
{
	private String lpsFileName;
	private BufferedReader br;

	public ClsFileKey(String apsFileName)
	{
		lpsFileName = apsFileName;
		try
		{
			br = new BufferedReader(new FileReader(lpsFileName));
		}
		catch (FileNotFoundException e)
		{
			e.printStackTrace();
		}
	}

	public String getKey()
	{
		String lsKeyBuffer = "";
		try
		{
			if ((lsKeyBuffer = br.readLine()) != null)
			{
				return lsKeyBuffer;
			}
			br.close();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return null;
	}
}
