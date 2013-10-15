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
										"             �˿ڱ�ռ�ã� ����ʧ��              \n�����Ƿ���ʵ��������");
					} else {
						JOptionPane
								.showMessageDialog(null,
										"              ����ʧ��              \n�������ݿ����л������������Ƿ�����");
					}
					e.printStackTrace();
				}
			}
		});
	}
}
