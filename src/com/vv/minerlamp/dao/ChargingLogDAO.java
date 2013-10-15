package com.vv.minerlamp.dao;

import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import org.hibernate.HibernateException;
import org.hibernate.Query;
import org.hibernate.Session;
import org.hibernate.Transaction;

import com.vv.minerlamp.entity.ChargingLog;
import com.vv.minerlamp.entity.RackStatistics;
import com.vv.minerlamp.entity.Staff;
import com.vv.minerlamp.util.HibernateUtil;
import com.vv.minerlamp.util.StaffAction;
import com.vv.minerlamp.util.StaffState;
import com.vv.minerlamp.util.SysConfiguration;
import com.vv.minerlamp.util.Util;

public class ChargingLogDAO {
	private static final int ONE_DAY_MILLONSECONDS = 24 * 3600 * 1000;

	public ChargingLogDAO() {

	}

	public Long save(Staff staff, Integer oldState, Integer action,
			Integer newState) {
		ChargingLog chargingLog = new ChargingLog();
		Session session = HibernateUtil.getSessionFactory().openSession();
		Transaction transaction = null;
		Long chargingLogId = null;
		try {
			chargingLog.setWorkId(staff.getWorkId());
			chargingLog.setName(staff.getName());
			chargingLog.setRackId(staff.getRackId());
			chargingLog.setLampNo(staff.getLampNo());
			chargingLog.setProfession(staff.getProfession());
			chargingLog.setDepartment(staff.getDepartment());
			chargingLog.setClazzId(staff.getClazzId());
			chargingLog.setClazz(staff.getClazz());
			chargingLog.setOldState(oldState);
			chargingLog.setAction(action);
			chargingLog.setNewState(newState);
			chargingLog.setUndergroundBeginTime(staff.getChargingLastTime());
			// 0�����¾�ʱ����ܿ�Խ����
			//������״ζ���ʱ chargingLastTime�ֶ�Ϊ�գ�Ϊ�˷���ͳ�ƣ�������������Ϊ��ǰ����
			if(staff.getChargingLastTime()==null){
				chargingLog.setClazzDay(Util.formatDate(new Date()));
			}
			else if (staff.getChargingLastTime().getHours() > 22
					&& staff.getClazz().equals(SysConfiguration.clazz0)) {
				chargingLog.setClazzDay(Util.formatDate(new Date(staff
						.getChargingLastTime().getTime()
						+ ONE_DAY_MILLONSECONDS)));
			} else {
				chargingLog.setClazzDay(Util.formatDate(staff
						.getChargingLastTime()));
			}
			if (action.equals(StaffAction.PUT_ON)) {
				chargingLog.setUndergroundEndTime(new Date());
				chargingLog.setDescription("��λ���");
			} else if (action.equals(StaffAction.TAKE_AWAY)) {
				chargingLog.setDescription("�¾�����");
			} else if (action.equals(StaffAction.CHARGING_OK)) {
				chargingLog.setDescription("��س���");
			} else if (action.equals(StaffAction.OVER_TIME)) {
				chargingLog.setDescription("�Ͼ���ʱ");
			} else if (action.equals(StaffAction.ERROR_HAPPENS)) {
				chargingLog.setDescription("��������");
			} else if (action.equals(StaffAction.ERROR_REPARED)) {
				chargingLog.setDescription("�����޸�");
			} else if (action.equals(StaffAction.FIRSTTIME_CHARGING)) {
				chargingLog.setDescription("�״γ��");
			}else if (action.equals(StaffAction.OFFLINE)) {
				chargingLog.setDescription("����");
			}else if (action.equals(StaffAction.ONLINE)) {
				chargingLog.setDescription("��������");
			}else if (action.equals(StaffAction.TIME_TOO_SHORT)) {
				chargingLog.setDescription("�����λ");
			}
			else if (action.equals(StaffAction.BAT_ALREADY_FULL)) {
				chargingLog.setDescription("�������");
			}
			transaction = session.beginTransaction();

			chargingLogId = (Long) session.save(chargingLog);

			transaction.commit();
		} catch (HibernateException e) {
			transaction.rollback();
			e.printStackTrace();
		} finally {
			session.close();
		}
		return chargingLogId;
	}

	public List<Object[]> queryHistoryLogModel(Date beginTime, Date endTime) {
		Session session = HibernateUtil.getSessionFactory().openSession();
		List<Object[]> result;
		StringBuilder sb = new StringBuilder(
				"select chargingLog.workId,chargingLog.name,chargingLog.rackId,chargingLog.lampNo,chargingLog.profession,chargingLog.department,chargingLog.lastUpdateTime,chargingLog.description from ChargingLog chargingLog");
		StringBuilder querySb = new StringBuilder();
		if (beginTime != null) {
			querySb.append("chargingLog.lastUpdateTime>='"
					+ Util.formatDate(beginTime) + "'");
		}
		if (endTime != null) {
			if (querySb.length() > 0) {
				querySb.append(" and ");
			}
			querySb.append("chargingLog.lastUpdateTime<='"
					+ Util.formatDate(new Date(endTime.getTime()
							+ ONE_DAY_MILLONSECONDS)) + "'");
		}
		if (querySb.length() > 0) {
			sb.append(" where ").append(querySb);
		}
		Query query = session.createQuery(sb.toString());
		result = query.list();
		session.close();
		return result;
	}

	public List<Object[]> querySingleDayStatisticsModel(Date date) {
		Session session = HibernateUtil.getSessionFactory().openSession();
		Object[][] result = new Object[SysConfiguration.rackCount][7];

		for (int i = 0; i < result.length; i++) {
			for (int j = 0; j < result[i].length; j++) {
				if (j == 0) {
					result[i][j] = new Long(i + 1);
				} else {
					result[i][j] = new Long(0);
				}
			}

		}

		List<Object[]> clazzList;
		Query query = session.createQuery(getStatisticsQuerySql(
				SysConfiguration.clazz0, date));
		clazzList = query.list();

		if (clazzList != null && clazzList.size() > 0) {

			for (int i = 0; i < clazzList.size(); i++) {
				Object[] clazzStatistics = (Object[]) clazzList.get(i);

				Integer rackId = ((Long) clazzStatistics[0]).intValue();
				if (clazzStatistics[1].equals(StaffAction.TAKE_AWAY)) {
					result[rackId - 1][1] = clazzStatistics[2];
				} else if (clazzStatistics[1].equals(StaffAction.PUT_ON)) {
					result[rackId - 1][2] = (Long) result[rackId - 1][1]
							- (Long) clazzStatistics[2];
				}

			}
		}

		query = session.createQuery(getStatisticsQuerySql(
				SysConfiguration.clazz8, date));
		clazzList = query.list();
		if (clazzList != null && clazzList.size() > 0) {

			for (int i = 0; i < clazzList.size(); i++) {
				Object[] clazzStatistics = (Object[]) clazzList.get(i);

				Integer rackId = ((Long) clazzStatistics[0]).intValue();
				if (clazzStatistics[1].equals(StaffAction.TAKE_AWAY)) {
					result[rackId - 1][3] = clazzStatistics[2];
				} else if (clazzStatistics[1].equals(StaffAction.PUT_ON)) {
					result[rackId - 1][4] = (Long) result[rackId - 1][3]
							- (Long) clazzStatistics[2];
				}

			}
		}

		query = session.createQuery(getStatisticsQuerySql(
				SysConfiguration.clazz16, date));
		clazzList = query.list();

		if (clazzList != null && clazzList.size() > 0) {

			for (int i = 0; i < clazzList.size(); i++) {
				Object[] clazzStatistics = (Object[]) clazzList.get(i);

				Integer rackId = ((Long) clazzStatistics[0]).intValue();
				if (clazzStatistics[1].equals(StaffAction.TAKE_AWAY)) {
					result[rackId - 1][5] = clazzStatistics[2];
				} else if (clazzStatistics[1].equals(StaffAction.PUT_ON)) {
					result[rackId - 1][6] = (Long) result[rackId - 1][5]
							- (Long) clazzStatistics[2];
				}

			}
		}
		session.close();
		return Arrays.asList(result);
	}

	

	private String getStatisticsQuerySql(String clazz, Date date) {
		return "SELECT chargingLog.rackId,chargingLog.action,COUNT(*) FROM ChargingLog chargingLog WHERE chargingLog.clazz='"
				+ clazz
				+ "' and chargingLog.clazzDay ='"
				+ Util.formatDate(date)
				+ "' GROUP BY chargingLog.rackId,chargingLog.action order by chargingLog.action asc";
	}

	public List<Object[]> querySingleDayTimeoutModel(Date date) {
		Session session = HibernateUtil.getSessionFactory().openSession();
		List<Object[]> result = null;

		StringBuilder sb = new StringBuilder(
				"SELECT chargingLog.clazz,chargingLog.workId,chargingLog.name,chargingLog.rackId,chargingLog.lampNo,chargingLog.department FROM ChargingLog chargingLog WHERE chargingLog.action="
						+ StaffAction.OVER_TIME
						+ " and DATE_FORMAT(chargingLog.lastUpdateTime,'%Y-%m-%d') ='"
						+ Util.formatDate(date) + "'");

		Query query = session.createQuery(sb.toString());
		result = query.list();
		session.close();
		return result;
	}

	public void deleteAll() {
		Session session = HibernateUtil.getSessionFactory().openSession();
		Transaction transaction = null;
		try {
			transaction = session.beginTransaction();
			Query query = session.createQuery("delete from ChargingLog");
			query.executeUpdate();
			transaction.commit();
		} catch (HibernateException e) {
			transaction.rollback();
			e.printStackTrace();
		} finally {
			session.close();
		}
	}
	public void deleteAllInCurrentSession(Session session) {
		Query query = session.createQuery("delete from ChargingLog");
		query.executeUpdate();
	}
}
