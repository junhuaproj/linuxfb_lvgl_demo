# linuxfb_lvgl_demo
运行与Linux系统的LVGL功能练习，显示天气预报，折线图，pthread后台线程，MQTT收发
* X86下模拟用X11,ARM嵌入式下用fb
* lvgl 9.1.1-dev
* http请求使用curl
* MQTT使用paho.mqtt.c
* arm linux，t113,rgb 7寸 1024X600
## 只是LVGL功能的测试，不稳定，网络延迟等原因可能导致卡死
## 启动
<p align="center">
  <img src="https://github.com/junhuaproj/linuxfb_lvgl_demo/blob/45c1a59030fd267ba734f7bd5b30e1c791a532d7/image/menu.png?raw=true" alt="start"/>
</p>
![image](https://github.com/junhuaproj/linuxfb_lvgl_demo/blob/45c1a59030fd267ba734f7bd5b30e1c791a532d7/image/menu.png)
## 股票
![image](image/stock_today.png "股票")
![image](image/stock_plot.png "股票")
## 传感器测量(MQTT收到)
![image](image/sensor.png "stm32h7")
![image](image/sensor2.png "tm32h7")
![image](image/system.png "Linux")
## 天气预报
![image](image/today1.png "天气与日期，温度，气压")
![image](image/today2.png "天气与日期，温度，气压")
![image](image/weather_station.png "天气站点")
![image](image/predict.png "预报")
