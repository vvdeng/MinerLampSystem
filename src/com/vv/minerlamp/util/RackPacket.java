package com.vv.minerlamp.util;

import java.lang.reflect.Array;
import java.util.Arrays;

import com.vv.minerlamp.comm.SerialComm;

public class RackPacket {
	private int addr;
	private static int[][] data = new int[SysConfiguration.rackCount + 1][];// �Ƽ�����0����
	private static byte[][] originData = new byte[SysConfiguration.rackCount + 1][];// ����ģ����ԭʼ����

	public RackPacket(byte[] ldata) {
		this.addr = ldata[0];
		System.out.println("addr="+addr);
		this.originData[addr] = new byte[SerialComm.DATA_TYPE_UNITS_INFO_LENTTH];
		data[addr] = new int[SerialComm.DATA_TYPE_UNITS_INFO_LENTTH - 1];//������0��ʼ����Ӧ����1
		// System.arraycopy(ldata, 0, originData[addr], 0,
		// originData[addr].length);
		
		this.originData[addr][0] = ldata[0];// ��ַλû�м�����ǰ׺����������
		for (int i = 1; i < SerialComm.DATA_TYPE_UNITS_INFO_LENTTH; i++) {
			if ((ldata[i] & 0xf0) == SerialComm.DATA_PRE_UNIT_INFO) {
				this.originData[addr][i] = (byte) (ldata[i] & 0x0f);
			}
			else{
				this.originData[addr][i] =0;
			}
			data[addr][i-1] = StaffState.convertUnitState(this.originData[addr][i]);
		}
//		System.out.println("slave addr=" + addr);

	}

	public int getAddr() {
		return addr;
	}

	public void setAddr(int addr) {
		this.addr = addr;
	}

	public int[] getData(int index) {
		return data[index];
	}

	public byte[] getOriginData(int index) {
		return originData[index];
	}

}