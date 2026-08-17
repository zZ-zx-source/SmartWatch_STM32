#include "stm32f10x.h"                  // Device header
#include "app.h"

int main(void)
{	
	App_Init();

	while(1)
	{
		App_Process();
	}
}
