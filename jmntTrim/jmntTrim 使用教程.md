#                                                   jmntTrim 使用教程

### 一、使用环境

  1. 仅在使用USB外设时才可使用 `jmntTrim`。

  2. 配置KEIL工程如下图。

  ![img]( https://s3.bmp.ovh/imgs/2022/03/9816c694d7d06a2c.jpg ) 

### 二、占用资源

- SysTick 或者 TIM4 选其一即可

### 三、使用教程

  1. ##### 添加MHSI Trim程序。

​        在项目工程中添加 jmntTrim.c 和 jmntTrim.h 文件

​        [jmntTrim(SysTick).c](https://github.com/WestberryTech/Download/blob/master/jmntTrim/jmntTrim(SysTick).c) 

​        [jmntTrim(TIM4).c](https://github.com/WestberryTech/Download/blob/master/jmntTrim/jmntTrim(TIM4).c) 

​        [jmntTrim.h](https://github.com/WestberryTech/Download/blob/master/jmntTrim/jmntTrim.h)

2. ##### 配置USB 

- 使能 USB 的 SOF 中断

 ![img](https://s3.bmp.ovh/imgs/2022/03/9ab0ee729b3ce8ec.jpg ) 

- 在usbd_user.c里包含jmntTrim.h文件

```c
#include "jmntTrim.h"
```

- 在USBD_User_SOF函数里调用CheckTune函数

```c
/* This function was added in firmware library version 1.0.8. */
void USBD_User_SOF(void)
{
	CheckTune();
}
```

- USB中断抢占和子优先级都必须配置为最高。

- 其余配置根据用户使用场景自行配置。

3. ##### 在main 函数里调用jmntTrimInit函数

  ```c
int main(void)
{
    /* The clock configuration must be placed in the first line. */
	MAINCLKConfig_MHSI_48MHz(); 
	jmntTrimInit();
	
	while(1)
	{
		
	}
}
  ```

  注意需要包含jmntTrim.h文件

4. ##### 根据不同系统主频配置Trim参数

​      默认主频为48Mhz，用户主频为48Mhz时参数不用修改。

​      若主频不是48Mhz，用户可根据当前值线性修改。


```c
/* The range of sof frame interval TIMER counts. */
#define INR_HEAD 44500
#define INR_TAIL 51500
```

5. MHSI作为PLL时钟源，配置主频为48Mhz示例代码

```c
/**
 * @brief  Configures the Main clock frequency, HCLK, PCLK2 and PCLK1
 *         prescalers.
 * @return None
 */
void MAINCLKConfig_MHSI_48MHz(void)
{
  /* Disable ANCTL register write-protection */
  PWR_UnlockANA();

  /* Configure Flash prefetch, Cache and wait state */
  CACHE->CR = CACHE_CR_CHEEN | CACHE_CR_PREFEN_ON | CACHE_CR_LATENCY_1WS;

  /* AHBCLK = MAINCLK */
  RCC_AHBCLKConfig(RCC_MAINCLK_Div1);

  /* Enable APB1CLK. APB1CLK = MAINCLK */
  RCC_APB1CLKConfig(RCC_MAINCLK_Div1, ENABLE);

  /* Enable APB2CLK. APB2CLK = MAINCLK */
  RCC_APB2CLKConfig(RCC_MAINCLK_Div1, ENABLE);

  /* PLL configuration: PLLCLK = MHSI(8MHz) / 2 * 12 = 48 MHz */
  RCC_PLLSourceConfig(RCC_PLLSource_MHSI_Div2, ENABLE);
  ANCTL_PLLConfig(ANCTL_PLLMul_12);

  /* Enable PLL */
  ANCTL_PLLCmd(ENABLE);

  /* Wait till PLL is ready */
  while(ANCTL_GetFlagStatus(ANCTL_FLAG_PLLRDY) == RESET);

  /* Enable ANCTL register write-protection */
  PWR_LockANA();

  /* Select PLL as system clock source */
  RCC_MAINCLKConfig(RCC_MAINCLKSource_PLLCLK);
}
```

