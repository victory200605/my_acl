package com.ailk.ainb.cache.client.config;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.io.xml.DomDriver;

public class Configuration {
	private final static Log logger = LogFactory.getLog(Configuration.class);

	public CacheClientConfig loadConfig(){
		CacheClientConfig cacheClientConfig=null;
		try{
		XStream xstream = new XStream(new DomDriver());
		xstream.processAnnotations(CacheClientConfig.class);// 显示声明使用注解
		xstream.autodetectAnnotations(true);

		cacheClientConfig = (CacheClientConfig) xstream
				.fromXML(Configuration.class.getClassLoader()
						.getResourceAsStream("cacheclient.xml"));

		}catch(Exception e){
			throw new RuntimeException("load cacheclient.xml error",e);
		}
		return cacheClientConfig;
	}

	public static void main(String[] args) throws Exception{
		Configuration config = new Configuration();
//		for(Server server:config.loadConfig().getList()){
//			System.out.println(server.getAddress()+":"+server.getPort()+":"+server.getMinRange()+":"+server.getMaxRange());
//		}
//		System.out.println(config.loadConfig().getDeleteTimeOutSize());
		List<Server> list = config.loadConfig().getList();
		config.validata(list);
	}
	public void validata(List<Server> list){
		if(list.isEmpty()){
			throw new IllegalArgumentException("cacheServer配置不正确");
		}

		Collections.sort(list,new  Comparator<Server>(){
			@Override
			public int compare(Server arg0, Server arg1) {
				if(arg0.getMinRange()>arg1.getMinRange()){
					return 1;
				}
				return 0;
			}

		});
		if(list.get(0).getMinRange()!=0){
			throw new IllegalArgumentException("IP:"+list.get(0).getAddress()+" port:"+list.get(0).getPort()+"的minRange值不正确,应为0");
		}
		if(list.get(list.size()-1).getMaxRange()!=99){
			throw new IllegalArgumentException("IP:"+list.get(list.size()-1).getAddress()+" port:"+list.get(list.size()-1).getPort()+"的maxRange值不正确,应为99");
		}
		Server s=null;
		for(Server server:list){
			if(s==null){
				s=server;
			}else{
				if((s.getMaxRange()+1)==server.getMinRange()){
					s=server;
				}else{
					throw new IllegalArgumentException("IP:"+server.getAddress()+" port:"+server.getPort()+"的minRange值不正确,应为"+(s.getMaxRange()+1));
				}
			}

		}
	}

}
