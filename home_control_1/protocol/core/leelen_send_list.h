/********************************************************************************
**
** �ļ���:     leelen_send_list.h
** ��Ȩ����:   (c) 2015 �������ֿƼ����޹�˾
** �ļ�����:   ʵ��ͨѶЭ�����ݻ��巢�Ͷ���
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����       | ����   |  �޸ļ�¼
**===============================================================================
**| 2015/09/28 | zzh    |  �������ļ�
**
*********************************************************************************/



#ifndef LEELEN_SEND_LIST_H
#define LEELEN_SEND_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
** ������:     apply_send_list
** ��������:   ������Э������
** ����:       [in]  keyword:       �����͵����ݰ��ؼ���, ͨ�����Э��������
**             [in]  pkt:           �����͵����ݰ�
**             [in]  over_time:     ָ�����ͳ�ʱʱ��(��λ:��)
**             [in]  max_send:      �ڽ��ղ���Ӧ������µ�����ʹ���
**             [in]  informer:      ���ͽ���ص�����
** ����:       ������ʧ��, �򷵻�false;  �ɹ�, �򷵻�һ����Ϊ0�ı�ʶ
** ע��:       1. ���ݰ�pkt���ڱ�ģ���ڲ������ͷ�, һ�������˸ýӿ�, ���۳ɹ���ʧ��, �����߶������ͷ�pkt;
**             2. ��over_timeΪ0, ���ʾ����ȱʡ��ʱʱ��(��15��)
**             3. ��max_sendΪ0, ���ʾ����ȱʡ���ʹ���(��3��);
********************************************************************/
bool_t apply_send_list(uint16_t keyword, packet_t *pkt, uint8_t over_time, uint8_t max_send, void (*informer)(RESULT_E));

/*******************************************************************
** ������:     cancel_send_list
** ��������:   ɾ�����������е�ָ���ڵ�
** ����:       [in]  keyword:    ɾ��������
** ����:       ��
********************************************************************/
void cancel_send_list(uint16_t keyword);

/*******************************************************************
** ������:     init_send_list
** ��������:   ��ʼ������
** ����:       ��
** ����:       ��
********************************************************************/
void init_send_list(void);

#ifdef __cplusplus
}
#endif


#endif      /* end of LEELEN_SEND_LIST_H */
