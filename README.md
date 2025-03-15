# 点灯大师
## 构建环境
本项目使用cubemx生成hal库项目，使用openocd烧录和仿真
## 硬件内容
- STM32F103ZET6中控
- 0.96'OLED(1306)：[驱动来源Keysking](https://github.com/BaudDance/LEDDrive/tree/main/library/TI-launchpad-M0G3507)
- AHT20温度传感器：[驱动来源Keysking](https://github.com/BaudDance/LEDDrive/tree/main/library/stm32_hal_aht20)
- MAX4466 ADC模块(不推荐，采样区间较窄，灵敏度不高)
- 电位器
- DCDC(5V)电源：[来自嘉立创](https://oshwhub.com/264xry/mp1584en)
- ws2012
  ![image](https://github.com/user-attachments/assets/fca507be-848b-4bdd-84ab-222da97098ea)

## 功能列表
- 指定颜色点灯
- 电位器阈值点灯
- 分贝阈值点灯
- 分贝控制灯亮度
- 温度点灯
- 长按点灯
- 松开点灯
- 轻触点灯
- 可调亮度
## 远程控制部分
- 0 255 255 255 ：指定颜色点灯，八位数据，GRB排列
- 4 2000 2000 ：闪烁灯，后面两位数据分别为红色和蓝色灯闪烁周期(ms)
- 5 2000 ：呼吸灯，后位数据为周期(ms)
- 8 1500 ： 电位器点灯（默认最高电压为3.3V），后为阈值*100
- 11 6000 ：分贝点灯，后为阈值*100

**PS：由于adruino老版本驱动控制对ws2812灯带的第一颗灯珠控制有信号问题，所以本示例以第二颗灯珠为例**
