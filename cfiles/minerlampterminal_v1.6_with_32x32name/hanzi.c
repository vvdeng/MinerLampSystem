#include "hanzi.h"
//---------------------------------------------------------
//�������ƣ�u8 GB_HZZF_len(u8 *hzStr)
//˵����    �ɺ���������㺺������ռ�õ��ֽ���
//����˵����hzStr ��������ָ��
//���أ�	���������ռ�õ��ֽ���
//---------------------------------------------------------
//---------------------------------------------------------
u8 GB_HZZF_len(u8 *hzStr)//���GB������ռ�õ��ֽ���
{
	u8 c1,c2;
	c1 = *hzStr++;
	if( c1 < 0x20 )
		return(0);
	if( c1 < 0x80)
		return(1);
	c2= *hzStr++;
	if( (c1==0x81 && c2>=0x39)||(c1==0x82) )
		return(4);
	return(2);

}
//----------------------------------------------------------
//----------------------------------------------------------
//�������ƣ�u32  GB_ADDR(u8 *HZ,u8 HZ_type)
//˵����    �ɺ���������㺺�ֵ�ַ
//����˵����
//			HZ_type  ֧��8�㣬12�㣬16�㣬24�㣬32�㺺��
//			HZ       ��������ǵ��ֽڣ�˫�ֽڣ����ֽ�
//			BaseAddr �����͵ĵ������ʼ��ַ
//���أ�	���ֵ������ֿ��еľ��Ե�ַ
//----------------------------------------------------------
u32  GB_ADDR(u8 *HZ,u8 HZ_type)//��ú��ֵ���ĵ�ַ
{
	u32 temp;
	u8 c1,c2;
	if(HZ[0]<0x80)//ASCII ,���
	{

            if( HZ[0] < ' ' )
				HZ[0] = ' ';
			temp = HZ[0] - ' ';
			if( HZ_type == TYPE_8 )
				temp = temp*8 +  ASC0808YZF_ADDR;
			else if( HZ_type == TYPE_12 )
				temp = temp*12 + ASC0812YZF_ADDR;
			else if( HZ_type == TYPE_16 )
				temp = temp*16 + ASC0816YZF_ADDR;
			else if( HZ_type == TYPE_24 )
				temp = temp*48 + ASC1224YZF_ADDR;
			else if( HZ_type == TYPE_32 )
				temp = temp*64 + ASC1632YZF_ADDR;

	}
    else //chinese  for BG2312,GBK,GB18030,GB12345
    {
        	c1=HZ[0];
        	c2=HZ[1];
       		//������,ȫ�� 846+96 = 1038
        	if( (c1>=0xA1 && c1 <= 0xAB && c2>=0xa1)||(c1>=0xa8 && c1 <= 0xa9 && c2<0xa1) )
        	{
                if( c1>=0xA1 && c1 <= 0xAB && c2>=0xa1 )//˫�ֽ��ַ�1�� 846
					temp = (c1 - 0xA1) * 94 + (c2 - 0xA1);
				else									//˫�ֽ��ַ�5�� 96
					{if(c2>0x7f) c2--;temp=(c1-0xa8)*96 +(c2-0x40)+846;}

                if( HZ_type == TYPE_12 )
					return( temp *24 + ZF1212Y_ADDR);
				else if( HZ_type == TYPE_16 )
					return( temp *32 + ZF1616Y_ADDR);
				else if( HZ_type == TYPE_24 )
					return( temp *72 + ZF2424Y_ADDR);
				else if( HZ_type == TYPE_32 )
					return( temp *128 + ZF3232Y_ADDR);
				else
					return(0x00);
        	}

        	//GBK˫�ֽ�2�� 6768
        	else if(c1>=0xb0 && c1 <= 0xf7 && c2>=0xa1)
        	{
                	temp= (c1 - 0xB0) * 94 + (c2 - 0xA1);
        	}
		if( HZ_type == TYPE_12 )
			temp = (temp+846-184)*24 + ZF1212Y_ADDR;
		else if( HZ_type == TYPE_16 )
			temp = (temp+846-184)*32 + ZF1616Y_ADDR;
		else if( HZ_type == TYPE_24 )
			temp = (temp+846-184)*72 + ZF2424Y_ADDR;
		else if( HZ_type == TYPE_32 )
			temp=(temp+846-184)*128+ZF3232Y_ADDR;
		else
			return(0x00);
	}
	return temp;
}
