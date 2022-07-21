#include "PID.H"



float PID_Control(Error *Element_sptr, PID *Control_pid, float Input, float Point)
{
	float Target_Error,
		  Realize;
	
	Target_Error = Point - Input;		//计算偏差
	Element_sptr -> Integral += Target_Error;		//求出偏差的积分
	Realize = Control_pid->P * Target_Error +  Control_pid->I * Element_sptr -> Integral + Control_pid->D * 
	(Target_Error - Element_sptr->Previous_Error);		//P*偏差积分 +I*偏差积分 +D * （偏差 - 上一次偏差）
	Element_sptr->Previous_Error = Target_Error;		//保存上一次偏差
	return Realize; //输出
}

