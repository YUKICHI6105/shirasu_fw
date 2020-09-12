/*
 * main.cpp
 *
 *  Created on: Aug 11, 2020
 *      Author: ryu
 */
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "microshell.h"
#include "msconf.h"
#include "mscmd.h"

#include "MotorCtrl.h"

MotorCtrl control;

extern "C" {
	void cdc_puts(char *str);
	char cdc_getc();
	void cdc_put(char c);
	uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
	TIM_HandleTypeDef htim15;
	TIM_HandleTypeDef htim3;

    /**
     * @brief
     * This is the workhorse of the md201x.
     * this handler is called @ 1 kHz.
     */
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
	  if(htim->Instance == TIM3)
	  {
		  control.Control();
	  }
	}
};

typedef struct {
    void (*puts)(char *str);
} USER_OBJECT;

static void action_hook(MSCORE_ACTION action)
{
}

static MSCMD_USER_RESULT usrcmd_system(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
    char buf[MSCONF_MAX_INPUT_LENGTH];
    int argc;
    int i;
    cdc_puts("[SYSTEM]\r\n");
    msopt_get_argc(msopt, &argc);
    for (i = 0; i < argc; i++) {
        msopt_get_argv(msopt, i, buf, sizeof(buf));
        cdc_puts(" '");
        cdc_puts(buf);
        cdc_puts("'\r\n");
    }
    return 0;
}

static MSCMD_USER_RESULT usrcmd_config(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
    char buf[MSCONF_MAX_INPUT_LENGTH];
    int argc;
    int i;
    cdc_puts("[CONFIG]\r\n");
    msopt_get_argc(msopt, &argc);
    for (i = 0; i < argc; i++) {
        msopt_get_argv(msopt, i, buf, sizeof(buf));
        cdc_puts(" '");
        cdc_puts(buf);
        cdc_puts("'\r\n");
    }
    return 0;
}

static MSCMD_USER_RESULT usrcmd_help(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
    USER_OBJECT *uo = (USER_OBJECT *)usrobj;
    uo->puts(
            "system : system command\r\n"
            "config : config command\r\n"
            "help   : help command\r\n"
    		"t_led   : toggle led\r\n"
            );
    return 0;
}

static MSCMD_USER_RESULT usrcmd_led_toggle(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
	USER_OBJECT *uo = (USER_OBJECT *)usrobj;
    char buf[MSCONF_MAX_INPUT_LENGTH];
    int argc;
    msopt_get_argc(msopt, &argc);
    for (int i = 0; i < argc; i++) {
        msopt_get_argv(msopt, i, buf, sizeof(buf));
        if (strcmp(buf, "RED") == 0) HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        else if (strcmp(buf, "GREEN") == 0) HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        else if (strcmp(buf, "YELLOW") == 0) HAL_GPIO_TogglePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin);
        else if (strcmp(buf, "CAN") == 0) HAL_GPIO_TogglePin(LED_CAN_GPIO_Port, LED_CAN_Pin);
    }
    return 0;
}

static MSCMD_USER_RESULT usrcmd_setduty(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
	USER_OBJECT *uo = (USER_OBJECT *)usrobj;
    char buf[MSCONF_MAX_INPUT_LENGTH];
    int argc;
    msopt_get_argc(msopt, &argc);
    for (int i = 0; i < argc; i++) {
        msopt_get_argv(msopt, i, buf, sizeof(buf));
        int duty;
        sscanf(buf,"%d",&duty);
        control.SetDuty(duty);
    }
    return 0;
}

static MSCMD_COMMAND_TABLE table[] = {
    {   "system",   usrcmd_system   },
    {   "config",   usrcmd_config   },
    {   "help",     usrcmd_help     },
    {   "?",        usrcmd_help     },
	{   "t_led",  usrcmd_led_toggle	},
	{ "setduty" ,   usrcmd_setduty	},
};

void main_cpp(void)
{
	  char buf[MSCONF_MAX_INPUT_LENGTH];
	  MICROSHELL ms;
	  MSCMD mscmd;
	  USER_OBJECT usrobj = {
			  .puts = cdc_puts,
	  };

	  microshell_init(&ms, cdc_put, cdc_getc, action_hook);
	  mscmd_init(&mscmd, table, sizeof(table) / sizeof(table[0]), &usrobj);


	  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
	  HAL_GPIO_WritePin(USB_PULLUP_GPIO_Port, USB_PULLUP_Pin,GPIO_PIN_SET);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//	  TIM15->CCR1 = 1800;
//	  TIM15->CCR2 = 1;
	  HAL_TIM_Base_Start_IT(&htim3);
//	  HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
//	  HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_2);
	  control.Init(&htim15);

	  uint8_t sendbuf[] = "shell>";
	while(1){
	      MSCMD_USER_RESULT r;
	      CDC_Transmit_FS(sendbuf,sizeof(sendbuf));
	      microshell_getline(&ms, buf, sizeof(buf));
	      mscmd_execute(&mscmd, buf, &r);
	}

}





