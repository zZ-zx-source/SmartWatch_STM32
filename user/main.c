#include "stm32f10x.h"                  // Device header
#include "app.h"
#include "power.h"

int main(void)
{	
	Power_Init();
	App_Init();

	while(1)
	{
		App_Process();
	}
}
