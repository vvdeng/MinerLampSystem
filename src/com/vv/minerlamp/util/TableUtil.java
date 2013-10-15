package com.vv.minerlamp.util;

import java.awt.Component;
import java.util.Date;
import java.util.List;

import javax.swing.DefaultCellEditor;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableModel;

import com.vv.minerlamp.entity.LampChangeLog;

public class TableUtil {
	public static final String STAFFBASICINFO_FILENAME="Ա��������Ϣ";
	public static final String STAFFBASICINFO_TITLE="Ա��������Ϣ";
	public static final String HISTORY_FILENAME="Ա����ʷ������Ϣ";
	public static final String HISTORY_TITLE="Ա����ʷ������Ϣ";
	public static final String SINGLEDAY_FILENAME="���տ���δ��ʱ�Ͼ���Ϣ";
	public static final String SINGLEDAY_TITLE="���տ���δ��ʱ�Ͼ���Ϣ";
	public static final String UNDERGROUND_STAFF_FILENAME="�¾���Ա��Ϣ";
	public static final String UNDERGROUND_STAFF_TITLE="�¾���Ա��Ϣ";
	public static final String ALARM_FILENAME="��ʱ�澯��Ϣ";
	public static final String ALARM_TITLE="��ʱ�澯��Ϣ";
	public static final String LAMP_USE_FILENAME="���ʹ��ͳ��";
	public static final String LAMP_USE_TITLE="���ʹ��ͳ��";
	public static final String LAMP_CHANGE_FILENAME="��Ƹ�����¼";
	public static final String LAMP_CHANGE_TITLE="��Ƹ�����¼";
	public static TableModel getModel(final List list,
			final String columnNames[]) {

		final Object result[] = list != null ? list.toArray() : null;

		AbstractTableModel tm = new AbstractTableModel() {
			public int getColumnCount() {
				return columnNames.length;
			}

			public int getRowCount() {
				return list.size();
			}// ȡ�ñ������

			public Object getValueAt(int row, int column) {
				if (result == null || result.length == 0) {
					return null;
				}
				Object object = ((Object[]) (result[row]))[column];
				if (Date.class.isInstance(object)) {
					return Util.formatDateTime((Date) object);
				}
				return object;
			}

			public String getColumnName(int column) {
				return columnNames[column];
			}

			public void setValueAt(Object value, int row, int column) {
			}

			public Class getColumnClass(int c) {
				if (getValueAt(0, c) == null) {
					return Object.class;
				}
				return getValueAt(0, c).getClass();
			}

			public boolean isCellEditable(int row, int column) {
				return false;
			}
		};
		return tm;
	}

	public static TableCellEditor getCellEditor() {
		return new DefaultCellEditor(new JTextField()) {

			public Component getTableCellEditorComponent(JTable table,
					Object value, boolean isSelected, int row, int column) {

				JTextField editor = (JTextField) super
						.getTableCellEditorComponent(table, value, isSelected,
								row, column);

				if (value != null) {
					editor.setText(value.toString());
				}
				editor.setHorizontalAlignment(SwingConstants.CENTER);
				// if (column == 0) {
				// editor.setHorizontalAlignment(SwingConstants.CENTER);
				// editor.setFont(new Font("Serif", Font.BOLD, 14));
				// } else {
				// editor.setHorizontalAlignment(SwingConstants.RIGHT);
				// editor.setFont(new Font("Serif", Font.ITALIC, 12));
				// }
				return editor;
			}
		};
	}

}
