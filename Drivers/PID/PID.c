#include "PID.H"



float PID_Control(Error *Element_sptr, PID *Control_pid, float Input, float Point)
{
	float Target_Error,
		  Realize;
	
	Target_Error = Point - Input;		//����ƫ��
	Element_sptr -> Integral += Target_Error;		//���ƫ��Ļ���
	Realize = Control_pid->P * Target_Error +  Control_pid->I * Element_sptr -> Integral + Control_pid->D * 
	(Target_Error - Element_sptr->Previous_Error);		//P*ƫ����� +I*ƫ����� +D * ��ƫ�� - ��һ��ƫ�
	Element_sptr->Previous_Error = Target_Error;		//������һ��ƫ��
	return Realize; //���
}

