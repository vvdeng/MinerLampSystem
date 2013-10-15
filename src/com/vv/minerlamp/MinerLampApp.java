package com.vv.minerlamp;

import java.awt.EventQueue;
import java.io.IOException;

import javax.comm.PortInUseException;
import javax.swing.JOptionPane;

import com.vv.minerlamp.util.SysConfiguration;

public class MinerLampApp {
	public static void main(String[] args) {
		/*
		 * Runtime.getRuntime().addShutdownHook(new Thread() { public void run()
		 * { try { System.out.println("sysstate=" + SysConfiguration.sysState);
		 * if (SysConfiguration.sysState == SysConfiguration.SYS_STATE_REBOOT) {
		 * Runtime.getRuntime().exec("javaw -jar MinerLamp.jar"); } } catch
		 * (IOException e) {
		 * 
		 * e.printStackTrace(); } } });
		 */
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					MinerLampFrame frame = new MinerLampFrame();
				} catch (Exception e) {
					if (e instanceof PortInUseException) {
						JOptionPane
								.showMessageDialog(null,
										"             端口被占用， 启动失败              \n请检查是否有实例在运行");
					} else {
						JOptionPane
								.showMessageDialog(null,
										"              启动失败              \n请检查数据库运行或者网络连接是否正常");
					}
					e.printStackTrace();
				}
			}
		});
	}
}
