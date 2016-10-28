//√¸¡Ó––π§æﬂ

package com.asiainfo.aibc.ailibEx4j.AILib;

public class AIGetOpt
{
	protected String pattern;
	protected int optind = 0;
	public static final int DONE = 0;
	protected boolean done = false;

	public int getOptInd()
	{
		return optind;
	}

	protected String optarg;

	public String optarg()
	{
		return optarg;
	}

	public AIGetOpt(String patt)
	{
		pattern = patt;
		rewind();
	}

	public void rewind()
	{
		done = false;
		optind = 0;
	}

	public char getopt(String argv[])
	{
		if (optind == (argv.length))
		{
			done = true;
		}
		if (done)
		{
			return DONE;
		}

		String thisArg = argv[optind++];

		if (thisArg.startsWith("-"))
		{
			optarg = null;
			for (int i = 0; i < pattern.length(); i++)
			{
				char c = pattern.charAt(i);
				if (thisArg.equals("-" + c))
				{
					if (i + 1 < pattern.length() && pattern.charAt(i + 1) == ':' && optind < argv.length)
						optarg = argv[optind++];
					return c;

				}
			}
			return '?';
		}
		else
		{
			optind--;
			done = true;
			return DONE;
		}
	}
}
