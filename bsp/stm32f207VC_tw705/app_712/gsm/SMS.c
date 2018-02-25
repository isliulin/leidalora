/*
     SMS.C
*/
#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include  "App_moduleConfig.h"
#include  "App_gsm.h"
#include  "SMS.h"


SMS_Style   SMS_Service;    //  ��Ϣ���


/*********************************************************************************
*��������:void SMS_timer(u8 *instr,u16 len)
*��������:���Ŵ����������������Ҫ��һ��1��Ķ�ʱ��������ã����ں���"SMS_Process"�Ķ�ʱ�����
*��    ��:none
*��    ��:none
*�� �� ֵ:none
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
void SMS_timer(void)
{
    if(SMS_Service.SMS_waitCounter)
        SMS_Service.SMS_waitCounter--;
}


/*********************************************************************************
*��������:void SMS_protocol(u8 *instr,u16 len)
*��������:���Ŵ����������������Ҫ��һ���߳�������ã�������ش���(���Ŷ�ȡ��ɾ�����Զ��������)
*��    ��:none
*��    ��:none
*�� �� ֵ:none
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
void SMS_Process(void)
{
    u16   	ContentLen = 0;
    //	u16 		i,j,k;
    char *pstrTemp;
    if(SMS_Service.SMS_waitCounter)
        return;
    //-----------  ���Ŵ������ --------------------------------------------------------
    //---------------------------------
    if(SMS_Service.SMS_read)	   // ��ȡ����
    {
        memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
        /*
        strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGR=" );
        if ( SMS_Service.SMIndex > 9 )
        {
        	SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex >> 4 ) + 0x30;
        	SMS_Service.SMSAtSend[9] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
        	SMS_Service.SMSAtSend[10] = 0x0d;
        	SMS_Service.SMSAtSend[11] = 0x0a;
        }
        else
        {
        	SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
        	SMS_Service.SMSAtSend[9] = 0x0d;
        	SMS_Service.SMSAtSend[10] = 0x0a;
        }
        rt_kprintf("\r\n%s",SMS_Service.SMSAtSend);
        */
        ///
        sprintf(SMS_Service.SMSAtSend, "AT+CMGR=%d\r\n", SMS_Service.SMIndex);
        rt_kprintf("%s", SMS_Service.SMSAtSend);
        ///
        rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
        SMS_Service.SMS_read--;
        SMS_Service.SMS_waitCounter = 3;
    }
    //-------------------------------
    //       ���Ͷ�Ϣȷ��
    else if(SMS_Service.SMS_sendFlag == 1)
    {
        //#ifdef SMS_TYPE_PDU
        if(SMS_Service.SMS_come == 1)
        {
            memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
            ///����400�ֽڿռ�
            pstrTemp = rt_malloc(400);
            memset(pstrTemp, 0, 400);
            ///���ַ�����ʽ��Ŀ�ĵ绰��������ΪPDU��ʽ�ĺ���
            SetPhoneNumToPDU(SMS_Service.Sms_Info.TPA, SMS_Service.SMS_destNum, sizeof(SMS_Service.Sms_Info.TPA));
            ///����PDU��ʽ��������
            ContentLen = AnySmsEncode_NoCenter(SMS_Service.Sms_Info.TPA, GSM_UCS2, SMS_Service.SMS_sd_Content, strlen(SMS_Service.SMS_sd_Content), pstrTemp);
            //ContentLen=strlen(pstrTemp);
            ///��Ӷ���β�����"esc"
            pstrTemp[ContentLen] = 0x1A;    // message  end
            //////
            sprintf( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", (ContentLen - 2) / 2);
            rt_kprintf("%s", SMS_Service.SMSAtSend);
            rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
            rt_thread_delay(50);
            //////
            //rt_kprintf("%s",pstrTemp);
            rt_device_write( &dev_vuart, 0, pstrTemp, strlen(pstrTemp) );
            rt_hw_gsm_output_Data( ( char * ) pstrTemp, ContentLen + 1);
            rt_free( pstrTemp );
            pstrTemp = RT_NULL;
        }
        //#else
        else
        {
            memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
            strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=\"" );
            //strcat(SMS_Service.SMSAtSend,"8613820554863");// Debug
            strcat(SMS_Service.SMSAtSend, SMS_Service.SMS_destNum);
            strcat(SMS_Service.SMSAtSend, "\"\r\n");

            rt_kprintf("\r\n%s", SMS_Service.SMSAtSend);
            rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );

            rt_thread_delay(50);
            ContentLen = strlen(SMS_Service.SMS_sd_Content);
            SMS_Service.SMS_sd_Content	[ContentLen] = 0x1A;    // message  end
            rt_kprintf("%s", SMS_Service.SMS_sd_Content);
            rt_hw_gsm_output_Data( ( char * ) SMS_Service.SMS_sd_Content, ContentLen + 1);
        }
        //#endif
        SMS_Service.SMS_sendFlag = 0; // clear
        SMS_Service.SMS_waitCounter = 3;
    }

}


///���ӷ��Ͷ�����������ݣ�����λ���Ͷ�Ϣ��ǣ��ɹ�����true��ʧ�ܷ���false
u8 Add_SMS_Ack_Content(char *instr, u8 ACKflag)
{
    if(ACKflag == 0)
        return false;

    if(strlen(instr) + strlen(SMS_Service.SMS_sd_Content) < sizeof(SMS_Service.SMS_sd_Content))
    {
        strcat((char *)SMS_Service.SMS_sd_Content, instr);
        SMS_Service.SMS_sendFlag = 1;
        return true;
    }
    return false;
}



/*********************************************************************************
*��������:u8 SMS_Rx_Text(char *instr,char *strDestNum)
*��������:���յ�TEXT��ʽ�Ķ��Ŵ�����
*��    ��:instr ԭʼ�������ݣ�strDestNum���յ�����Ϣ�ķ��ͷ�����
*��    ��:none
*�� �� ֵ:	1:������ɣ�
			0:��ʾʧ��
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
u8 SMS_Rx_Text(char *instr, char *strDestNum)
{
    u16 len;
    u8 ret = 0;
    len = strlen(strDestNum);
    memset( SMS_Service.SMS_destNum, 0, sizeof( SMS_Service.SMS_destNum ) );
    if(len > sizeof( SMS_Service.SMS_destNum ))
    {
        len = sizeof( SMS_Service.SMS_destNum );
    }
    memcpy(SMS_Service.SMS_destNum, strDestNum, len);
    rt_kprintf( "\r\n  ��Ϣ��Դ����:%s", SMS_Service.SMS_destNum );

    len = strlen(instr);
    rt_kprintf( "\r\n �����յ���Ϣ: " );
    rt_device_write( &dev_vuart, 0, instr, len);

    //  tw705 �ظ��ľ��� TW705  �յ�703 Ҳ��705
    if(( strncmp( (char *)instr, "TW705#", 6 ) == 0 ) || ( strncmp( (char *)instr, "TW703#", 6 ) == 0 ))                                            //�����޸�UDP��IP�Ͷ˿�
    {
        //-----------  �Զ��� ��Ϣ�����޸� Э�� ----------------------------------
      //  SMS_protocol( instr + 5, len - 5 , SMS_ACK_msg);
        ret = 1;
    }
    SMS_Service.SMS_read		= 0;
    SMS_Service.SMS_waitCounter = 0;
    SMS_Service.SMS_come		= 0;
    //SMS_Service.SMS_delALL		= 1;
    return ret;
}


/*********************************************************************************
*��������:u8 SMS_Rx_PDU(char *instr,u16 len)
*��������:���յ�PDU��ʽ�Ķ��Ŵ�����
*��    ��:instr ԭʼ�������ݣ�len���յ�����Ϣ���ȣ���λΪ�ֽ�
*��    ��:none
*�� �� ֵ:	1:������ɣ�
			0:��ʾʧ��
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
u8 SMS_Rx_PDU(char *instr, u16 len)
{
    char *pstrTemp;
    u8 ret = 0;

    //////
    memset( SMS_Service.SMS_destNum, 0, sizeof( SMS_Service.SMS_destNum ) );
    pstrTemp = (char *)rt_malloc(200);	///���Ž������������ݣ��������ΪGB��
    memset(pstrTemp, 0, 200);
    if(GB19056.workstate == 0)
    {
        rt_kprintf( "\r\n ����ԭʼ��Ϣ: " );
        rt_device_write( &dev_vuart, 0, GSM_rx, len );
    }

    len = GsmDecodePdu(GSM_rx, len, &SMS_Service.Sms_Info, pstrTemp);
    GetPhoneNumFromPDU( SMS_Service.SMS_destNum,  SMS_Service.Sms_Info.TPA, sizeof(SMS_Service.Sms_Info.TPA));

    //memcpy( SMS_Service.SMS_destNum, SMS_Service.Sms_Info.TPA,sizeof( SMS_Service.SMS_destNum ) );
    if(GB19056.workstate == 0)
    {
        rt_kprintf( "\r\n  ��Ϣ��Դ����:%s \r\n", SMS_Service.SMS_destNum );
        rt_kprintf( "\r\n ������Ϣ: " );
        rt_device_write( &dev_vuart, 0, pstrTemp, len );
    }
    //rt_hw_console_output(GSM_rx);
    if(( strncmp( (char *)pstrTemp, "TW705#", 6 ) == 0 ) || ( strncmp( (char *)pstrTemp, "TW703#", 6 ) == 0 ))                                            //�����޸�UDP��IP�Ͷ˿�
    {
        //-----------  �Զ��� ��Ϣ�����޸� Э�� ----------------------------------
     //   SMS_protocol( pstrTemp + 5, len - 5 , SMS_ACK_msg);
        ret = 1;
    }
    SMS_Service.SMS_read		= 0;
    SMS_Service.SMS_waitCounter = 3;
    SMS_Service.SMS_come		= 1;
    //SMS_Service.SMS_delALL		= 1;
    rt_free( pstrTemp );
    pstrTemp = RT_NULL;
    return ret;
}




#if 0
u8 SMS_Tx_Text(char *strDestNum, char *s)
{
    u16 len;
    char *pstrTemp;

    memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
    strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=\"" );
    //strcat(SMS_Service.SMSAtSend,"8613820554863");// Debug
    strcat(SMS_Service.SMSAtSend, strDestNum);
    strcat(SMS_Service.SMSAtSend, "\"\r\n");

    rt_kprintf("\r\n%s", SMS_Service.SMSAtSend);
    rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );

    rt_thread_delay(50);

    pstrTemp = rt_malloc(150);
    memset(pstrTemp, 0, 150);
    len = strlen(s);
    memcpy(pstrTemp, s, len);
    pstrTemp[len++] = 0x1A;    // message  end

    ///���͵�����Ϣ
    rt_device_write( &dev_vuart, 0, pstrTemp, len);
    ///���͵�GSMģ��
    rt_hw_gsm_output_Data(pstrTemp, len);
    rt_free( pstrTemp );
    pstrTemp = RT_NULL;
    return 1;
}
//FINSH_FUNCTION_EXPORT(SMS_Tx_Text, SMS_Tx_Text);



u8 SMS_Tx_PDU(char *strDestNum, char *s)
{
    u16 len;
    u16 i;
    char *pstrTemp;
    memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
    pstrTemp = rt_malloc(400);
    memset(pstrTemp, 0, 400);
    i = 0;
    SetPhoneNumToPDU(SMS_Service.Sms_Info.TPA, strDestNum, sizeof(SMS_Service.Sms_Info.TPA));
    len = AnySmsEncode_NoCenter(SMS_Service.Sms_Info.TPA, GSM_UCS2, s, strlen(s), pstrTemp);
    //len=strlen(pstrTemp);
    pstrTemp[len++] = 0x1A;    // message  end
    //////
    sprintf( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", (len - 2) / 2);
    rt_kprintf("%s", SMS_Service.SMSAtSend);
    rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
    rt_thread_delay(50);
    //////
    rt_device_write( &dev_vuart, 0, pstrTemp, strlen(pstrTemp) );
    //rt_hw_console_output(pstrTemp);
    rt_hw_gsm_output_Data(pstrTemp, len);
    rt_free( pstrTemp );
    pstrTemp = RT_NULL;
    return 1;
}
//FINSH_FUNCTION_EXPORT(SMS_Tx_PDU, SMS_Tx_PDU);

#endif

/*********************************************************************************
*��������:u8 SMS_Rx_Notice(u16 indexNum)
*��������:ģ���յ��¶���֪ͨ
*��    ��:�¶��ŵ�������
*��    ��:none
*�� �� ֵ:	1:������ɣ�
			0:��ʾʧ��
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
u8 SMS_Rx_Notice(u16 indexNum)
{
    SMS_Service.SMIndex = indexNum;
    if(GB19056.workstate == 0)
        rt_kprintf( " index=%d", SMS_Service.SMIndex );
    SMS_Service.SMS_read		= 3;
    SMS_Service.SMS_waitCounter = 1;
    return 1;
}

