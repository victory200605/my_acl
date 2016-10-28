package com.asiainfo.aibc.ailibEx4j.AILib;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public class AILogSys
{
	public static final int AILOG_LEVEL_SYSTEM = 0;
	public static final int AILOG_LEVEL_ERROR = 1;
	public static final int AILOG_LEVEL_WARN = 2;
	public static final int AILOG_LEVEL_INFO = 5;
	public static final int AILOG_LEVEL_DEBUG = 10;
	public static final int AILOG_LEVEL_TRACE = 11;
	public static final int AILOG_LEVEL_ANY = 12;

	public static final String AILOG_LEVEL_SYSTEM_STR = "SYS: ";
	public static final String AILOG_LEVEL_ERROR_STR = "ERR: ";
	public static final String AILOG_LEVEL_WARN_STR = "WARN: ";
	public static final String AILOG_LEVEL_INFO_STR = "INFO: ";
	public static final String AILOG_LEVEL_DEBUG_STR = "DEBUG: ";
	public static final String AILOG_LEVEL_TRACE_STR = "TRACE: ";
	public static final String AILOG_LEVEL_ANY_STR = "ANY: ";

	public static synchronized void AIWriteLOG(String apcLogName, int aiLogLevel, Object[] args) throws IOException
	{
		if (aiLogLevel >= 0)
		{
			String lpcLogName = apcLogName;
			String outputStr ;

			Date date = new Date();
			SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd.HH:mm:ss");
			String exceptionDate = dateFormat.format(date);

			DecimalFormat df = new DecimalFormat("00000");
			String currentThreadID = df.format(Thread.currentThread().getId());
			
			outputStr = exceptionDate + "#" + currentThreadID+" ";
			if(aiLogLevel <= AILOG_LEVEL_SYSTEM)
			{
				outputStr += AILOG_LEVEL_SYSTEM_STR;
			}
			else if(aiLogLevel <= AILOG_LEVEL_ERROR)
			{
				outputStr += AILOG_LEVEL_ERROR_STR;
			}
			else if(aiLogLevel<= AILOG_LEVEL_WARN)
			{
				outputStr += AILOG_LEVEL_WARN_STR;
			}
			else if(aiLogLevel<= AILOG_LEVEL_INFO)
			{
				outputStr += AILOG_LEVEL_INFO_STR;
			}
			else if(aiLogLevel<= AILOG_LEVEL_DEBUG)
			{
				outputStr += AILOG_LEVEL_DEBUG_STR;
			}
			else if(aiLogLevel<= AILOG_LEVEL_TRACE)
			{
				outputStr += AILOG_LEVEL_TRACE_STR;
			}
			else 
			{
				outputStr += AILOG_LEVEL_ANY_STR;
			}
			
			for(int i=0;i<args.length;i++)
			{
				outputStr+=args[i].toString();
			}
			outputStr+="\r\n";
			
			RandomAccessFile raf = new RandomAccessFile(lpcLogName, "rw");
			raf.seek(raf.length());
			raf.writeBytes(outputStr);
			raf.close();
		}
	}

//	public static void main(String[] args) throws IOException
//	{
//		AIWriteLOG("D:/xxxxxx.txt",5,new Object[] { "Fail to create DeleteTimeout thread when init CacheClient" });
//		
//	}
}
