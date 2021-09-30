package cn.com.xishanju.qyq.util;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Locale;

import android.content.Context;

class LogStruct
{
	public int m_level;
	public String m_app;
	public String m_tag;
	public String m_text;
}

public class TcpLog
{
	public static final int LOG_LEVEL_DEFAULT = 0;
	public static final int LOG_LEVEL_VERBOSE = 1;
	public static final int LOG_LEVEL_DEBUG = 2;
	public static final int LOG_LEVEL_INFO = 3; 
	public static final int LOG_LEVEL_WARN = 4; 
	public static final int LOG_LEVEL_ERROR = 5;

	String m_app;
	Socket m_socket;
	OutputStream m_outStream;
	String m_serverIp;
	int m_serverPort;
	ArrayList<LogStruct> m_queue = new ArrayList<LogStruct>();
	boolean m_threadRunning;
	Runnable m_run = new Runnable()
	{
		@Override
		public void run()
		{
			if(!tryConnect())
			{
				//重试
			}
			//等待发送log
//			doSendLog(TcpLog.LOG_LEVEL_DEBUG, "tcplog", "tcplog", "socket thread started");
			while(m_threadRunning)
			{
				synchronized (m_queue)
				{
					try
					{
						m_queue.wait();
					} catch (InterruptedException e)
					{
						e.printStackTrace();
						break;
					}
//					doSendLog(TcpLog.LOG_LEVEL_DEBUG, "tcplog", "tcplog", "running:"+m_threadRunning);
					if(m_threadRunning)
					{
						for(LogStruct log : m_queue)
						{
							doSendLog(log.m_level, log.m_app, log.m_tag, log.m_text);
						}
						m_queue.clear();
					}
				}
			}
//			doSendLog(TcpLog.LOG_LEVEL_DEBUG, "tcplog", "tcplog", "socket thread terminated");
//			try
//			{
//				Thread.sleep(2000);
//			} catch(Exception e)
//			{
//				e.printStackTrace();
//			}
			doTerminate();
		}
	};
	public void init(Context ctx, String serverIp, int serverPort)
	{
		if(ctx != null)
		{
			m_app = ctx.getApplicationContext().getPackageName();
		}
		m_serverIp = serverIp;
		m_serverPort = serverPort;
		m_queue.clear();
		m_threadRunning = true;
		new Thread(m_run).start();
	}
	public void terminate()
	{
		m_threadRunning = false;
		synchronized (m_queue)
		{
			m_queue.notify();
		}
	}
	public void sendLog(int level, String app, String tag, String text)
	{
		synchronized (m_queue)
		{
			LogStruct log = new LogStruct();
			log.m_level = level;
			log.m_app = app;
			log.m_tag = tag;
			log.m_text = text;
			m_queue.add(log);
			m_queue.notify();
		}
	}
	public void d(String tag, String msg)
	{
		sendLog(TcpLog.LOG_LEVEL_DEBUG, getApp(), tag, msg);
	}
	public void e(String tag, String msg)
	{
		sendLog(TcpLog.LOG_LEVEL_ERROR, getApp(), tag, msg);
	}
	public void i(String tag, String msg)
	{
		sendLog(TcpLog.LOG_LEVEL_INFO, getApp(), tag, msg);
	}
	public void v(String tag, String msg)
	{
		sendLog(TcpLog.LOG_LEVEL_VERBOSE, getApp(), tag, msg);
	}
	public void w(String tag, String msg)
	{
		sendLog(TcpLog.LOG_LEVEL_WARN, getApp(), tag, msg);
	}
	
	public void printException(Exception e)
	{
		int level = TcpLog.LOG_LEVEL_WARN;
		String app = getApp();
		String tag = "System.err";
		String msg = e.getMessage();
		String exceptionName = e.getClass().getName();
		this.sendLog(level, app, tag, exceptionName + ": " + msg);
		StackTraceElement[] trace = e.getStackTrace();
		for(StackTraceElement t : trace)
		{
			String str = String.format(Locale.US, "at %s.%s(%s:%d)", t.getClassName(), t.getMethodName(), t.getFileName(), t.getLineNumber());
			this.sendLog(level, app, tag, str);
		}
	}

	private String getApp()
	{
		if(m_app != null)
			return m_app;
		return "app";
	}
	private boolean tryConnect()
	{
		try
		{
			m_socket = new Socket(m_serverIp, m_serverPort);
			m_outStream = m_socket.getOutputStream();
			return true;
		} catch (UnknownHostException e)
		{
			e.printStackTrace();
		} catch (IOException e)
		{
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return false;
	}
	private void doTerminate()
	{
		try
		{
			if(m_outStream != null)
				m_outStream.close();
			if(m_socket != null)
				m_socket.close();
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	private void doSendLog(int level, String app, String tag, String text)
	{
		if(m_outStream == null) return;
		ByteArrayOutputStream buff = new ByteArrayOutputStream();
		DataOutputStream dataOut = new DataOutputStream(buff);
		try
		{
			dataOut.writeByte(level);
			dataOut.write(app.getBytes()); dataOut.writeByte(0);
			dataOut.write(tag.getBytes()); dataOut.writeByte(0);
			dataOut.write(text.getBytes()); dataOut.writeByte(0);
			dataOut.flush();
			byte[] data = buff.toByteArray();
			m_outStream.write(toLH(data.length));
			m_outStream.write(data);
//			m_outStream.flush();
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	/**
	 * int转为低字节在前，高字节在后的byte数组 VC
	 * @param n
	 * @return byte[]
	 */
	private static byte[] toLH(int n)
	{
		byte[] b = new byte[4];
		b[0] = (byte) (n & 0xff);
		b[1] = (byte) (n >> 8 & 0xff);
		b[2] = (byte) (n >> 16 & 0xff);
		b[3] = (byte) (n >> 24 & 0xff);
		return b;
	}
}
