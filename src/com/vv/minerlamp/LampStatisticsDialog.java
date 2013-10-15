package com.vv.minerlamp;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTable;

import com.vv.minerlamp.dao.LampChangeLogDAO;
import com.vv.minerlamp.dao.LampUnitDAO;
import com.vv.minerlamp.util.SysConfiguration;
import com.vv.minerlamp.util.TableUtil;
import com.vv.minerlamp.util.Util;

class LampStatisticsDialog extends JDialog {
	public LampStatisticsDialog(JFrame owner) {
		this(owner, 0);
	}

	public LampStatisticsDialog(JFrame owner, int selectedTabNo) {

		super(owner, "���ʹ�����", true);
		lampUnitDAO = new LampUnitDAO();
		lampChangeLogDAO = new LampChangeLogDAO();
		setIconImage(SysConfiguration.sysIcon);
		JLabel discLabel = new JLabel(new ImageIcon(
				"resources/statisticsTitle.png"));

		add(discLabel, BorderLayout.NORTH);

		JPanel panel = new JPanel();
		panel.setLayout(new GridBagLayout());

		add(panel, BorderLayout.CENTER);
		final JTabbedPane tabbedPane = new JTabbedPane();
		final JTable statisticsTable = new JTable();
		// statisticsTable.setCellEditor(TableUtil.getCellEditor());
		final JTable recordTable = new JTable();
		// recordTable.setCellEditor(TableUtil.getCellEditor());
		tabbedPane.addTab("���ʹ��ͳ��", new JScrollPane(statisticsTable));
		tabbedPane.addTab("��Ƹ�����¼", new JScrollPane(recordTable));
		tabbedPane.setSelectedIndex(selectedTabNo);
		JButton exit = new JButton("�˳�");
		exit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event) {

				dispose();
			}
		});
		panel.add(
				tabbedPane,
				new GBC(0, 0).setWeight(100, 100).setFill(
						GridBagConstraints.BOTH));
		JPanel functionPanel = new JPanel();
		functionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
		functionPanel.add(new JButton(new ButtonAction("ͳ��", null) {

			@Override
			public void actionPerformed(ActionEvent e) {
				if (tabbedPane.getSelectedIndex() == 0) {
					statisticsTable.setModel(TableUtil.getModel(
							lampUnitDAO.listAllForModel(),
							statisticsColumnNames));
				} else {
					recordTable.setModel(TableUtil.getModel(
							lampChangeLogDAO.listAllforModel(),
							recordColumnNames));
				}
			}
		}));
		functionPanel.add(new JButton(new ButtonAction("����", null) {

			@Override
			public void actionPerformed(ActionEvent e) {
				if (tabbedPane.getSelectedIndex() == 0) {
					Util.createExcelFile(TableUtil.LAMP_USE_FILENAME,
							TableUtil.LAMP_USE_TITLE,
							statisticsTable.getModel());
				} else {
					Util.createExcelFile(TableUtil.LAMP_CHANGE_FILENAME,
							TableUtil.LAMP_CHANGE_TITLE, recordTable.getModel());
				}

			}
		}));
		panel.add(
				functionPanel,
				new GBC(0, 1).setWeight(0, 0).setFill(
						GridBagConstraints.HORIZONTAL));
		JPanel bottomPanel = new JPanel();
		bottomPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
		bottomPanel.add(exit);
		add(bottomPanel, BorderLayout.SOUTH);
		setResizable(false);
		setSize(560, 320);
		setLocation(Util.calculatePosition(owner, this));
	}

	private String[] statisticsColumnNames = { "�Ƽܺ�", "��ƺ�", "�ۼƳ�����",
			"��ʼʹ��ʱ��", "����" };
	private String[] recordColumnNames = { "�Ƽܺ�", "��ƺ�", "Ա������", "Ա������", "����ʱ��" };
	private LampUnitDAO lampUnitDAO;
	private LampChangeLogDAO lampChangeLogDAO;
}
