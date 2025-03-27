/*
  此项目为 WouoUI 的精简和优化版本，又名 WouoUI - Lite_General_Pro_Plus_Ultra

  虽然起名 OS ，但其实只是一个简单的任务生命周期管理器。
  
  因为定义了硬件和编写框架，也因为对一些简单的设备来说这些足够用了，所以臭不要脸起名为 OS。
  
  与 WouoUI - Lite_General 相比，减少的功能和特性：

    * 主题切换：去除了白色主题遮罩
    * 列表循环：去除了选择框在头尾继续操作时，跳转至另一头的功能
    * 任意行高：去除了支持屏幕高度与行高度不能整除的特性，这个版本需要能够整除
    * 背景虚化：去除了弹窗背景虚化的可选项

  与 WouoUI - Lite_General 相比，增加和有区别的功能和特性有：

    * 展开列表动画从Y轴展开改为X轴排队进入
    * 进入更深层级时，选择框从头展开改为跳转
    * 增加选择时拖影，选择滚动越快拖影越长，XY轴都有拖影
    * 增加跳转时拖影，选择跳转越远拖影越长，XY轴都有拖影
    * 撞墙特性：滚动时当后续还有选择项时，有Y轴拖影，没有时没有Y轴拖影
  
  此项目使用的开源协议为 GPL ，以下简单解释权利和义务，详情请参考文档

    权利：

    * 不限制数量的复制
    * 不限制方式的传播
    * 允许增加，删除和修改代码
    * 允许收费，但以服务的形式

    义务：

    * 新项目也要开源，不能通过代码本身盈利
    * 新项目也要使用该许可证
    * 出售时要让买家知道可以免费获得

  请保留以下该信息以大致遵守开源协议

    * 作者：B站用户名：音游玩的人
    * 开源：https://github.com/RQNG/WoW-UI
*/
/************************************* 硬件连接 *************************************/
//#0.合宙版ESP32-C3简约版
//#1. 0.96 SSD1306 OLED
// I2C_SDA--GPIO04
// I2C_SCL--GPIO05
//#2. 5脚EC11编码器(无微动开关版，所以需要额外的按键)
// VCC--3.3V
// GND--GND
// A--GPIO07
// B--GPIO10
// C--GND
//#3. 按键
// GND--BTN--GPIO6
/************************************* 代码正文 *************************************/
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
/************************************* 屏幕驱动 *************************************/
// U8G2实例
// I2C_SDA--GPIO04
// I2C_SCL--GPIO05
#define   SDA   4
#define   SCL   5
#define   RST   U8X8_PIN_NONE
// 分辨率：128*64  驱动：SSD1306  接口：IIC（硬件）
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, RST);     
/************************************* 定义页面 *************************************/
void device_init();
void close();
//目录
enum 
{
  M_WIN,
  M_SLEEP,
    M_MAIN,
      M_F0,
      M_F1,
      M_F2,
      M_EDITOR,
        M_EDIT_F0,
      M_SETTING,
        M_ABOUT,
};

//过场
enum
{
  S_LAYER_IN,
  S_LAYER_OUT,
  S_FADE,
  S_MENU,
};

//菜单
typedef struct MENU
{
  char *m_select;
} M_SELECT;

/*********************************** 定义列表内容 ***********************************/

M_SELECT main_menu[]
{
  {"[ Main ]"},
  {"- Fidget Toy"},
  {"- 3D CUBE"},
  {"- Time Weather"},
  {"- Func 3"},
  {"- Func 4"},
  {"- Func 5"},
  {"- Editor"},
  {"- Setting"},
};

M_SELECT editor_menu[]
{
  {"[ Editor ]"},
  {"- Edit Fidget Toy"},
  {"- Edit 3D CUBE"},
  {"- Edit Time Weather"},
  {"- Edit Func 3"},
  {"- Edit Func 4"},
  {"- Edit Func 5"},
};

M_SELECT edit_f0_menu[]
{
  {"[ Edit Fidget Toy ]"},
  {"~ Box X OS"},
  {"~ Box Y OS"},
  {"~ Box Ani"},
};

M_SELECT setting_menu[]
{
  {"[ Setting ]"},
  {"~ Disp Bri"},
  {"~ Box X OS"},
  {"~ Box Y OS"},
  {"~ Win Y OS"},
  {"~ List Ani"},
  {"~ Win Ani"},
  {"~ Fade Ani"},
  {"~ Btn SPT"},
  {"~ Btn LPT"},
  {"+ Come Fm Scr"},
  {"+ Knob Rot Dir"},
  {"- [ About ]"},
};

M_SELECT about_menu[]
{
  {"[ WOW-OS ]"},
  {"- Version: v1.1"},
  {"- Creator: RQNG"},
  {"- Bili UID: 9182439"}, 
};

/************************************* 页面变量 *************************************/

//UI变量
#define   UI_DISP_H           64
#define   UI_DISP_W           128
#define   UI_DEPTH            10   
#define   UI_PARAM            11  
enum 
{
  DISP_BRI,
  BOX_X_OS,
  BOX_Y_OS,
  WIN_Y_OS,
  LIST_ANI,
  WIN_ANI,
  FADE_ANI,
  BTN_SPT,
  BTN_LPT,
  COME_SCR,
  KNOB_DIR,
};
struct 
{
  bool      init_flag;
  bool      oper_flag;

  uint16_t  buf_len;
  uint8_t   *buf_ptr;

  uint8_t   layer;
  uint8_t   fade = 1;

  uint8_t   index = M_SLEEP;
  uint8_t   state = S_MENU;

  uint8_t   select[UI_DEPTH];
  uint8_t   param[UI_PARAM];
} ui;

//列表变量
#define   LIST_FONT           u8g2_font_HelvetiPixel_tr 
#define   LIST_NUM            100
#define   LIST_LINE_H         16 
#define   LIST_TEXT_W         100 
#define   LIST_TEXT_H         8 
#define   LIST_TEXT_S         4
#define   LIST_BAR_W          3
#define   LIST_BOX_R          0.5f
struct
{
  uint8_t   select;

  int16_t   text_x_temp;
  int16_t   text_y_temp;
  int16_t   text_w_temp;

  float     text_x;
  float     text_x_trg;

  float     text_y;
  float     text_y_trg;

  float     box_y;
  float     box_y_trg[UI_DEPTH];

  float     box_W;
  float     box_w;
  float     box_w_trg;

  float     box_H;
  float     box_h;
  float     box_h_trg;

  float     bar_h;
  float     bar_h_trg;

  uint8_t   num[LIST_NUM];
} list;

//选择框变量
#define   CB_U                2
#define   CB_W                12
#define   CB_H                12
#define   CB_D                2
struct
{
  uint8_t *v;
  uint8_t *m;
  uint8_t *s;
  uint8_t *s_p;
} check_box;

//弹窗变量
#define   WIN_FONT            u8g2_font_HelvetiPixel_tr 
#define   WIN_H               27
#define   WIN_W               90
#define   WIN_TITLE_W         20
#define   WIN_TITLE_H         8 
#define   WIN_TITLE_S         5
#define   WIN_VALUE_S         67
#define   WIN_BAR_H           3 
struct
{
  char      title[WIN_TITLE_W];
  uint8_t   *value;
  uint8_t   max;
  uint8_t   min;
  uint8_t   step;
  uint8_t   index;
  MENU      *bg;

  bool      init_flag;
  bool      exit_flag;
  bool      oper_flag;

  float     box_x;

  float     box_y;
  float     box_y_trg;

  float     box_w;
  float     box_w_trg;

  float     box_H;
  float     box_h;
  float     box_h_trg;
  
  float     bar_x;
  float     bar_x_trg;
} win;

/********************************** 自定义功能变量 **********************************/
/********************************** 方块平移 **********************************/
//解压玩具变量
#define   F0_PARAM            3
#define   F0_BOX_H            20
#define   F0_BOX_W            20
#define   F0_POS_N            4
#define   F0_BOX_X            0
#define   F0_BOX_Y            1
#define   F0_BOX_R            2
enum 
{
  F0_X_OS,
  F0_Y_OS,
  F0_ANI
};
struct 
{
  float     box_x;
  float     box_x_trg;
  
  float     box_y; 
  float     box_y_trg;

  float     box_W = F0_BOX_W;
  float     box_w;
  float     box_w_trg;

  float     box_H = F0_BOX_H;
  float     box_h;
  float     box_h_trg;

  int8_t    select;
  uint8_t   pos[F0_POS_N][2] = {{                   0,                    0},   //pos1
                                {UI_DISP_W - F0_BOX_W,                    0},   //pos2
                                {UI_DISP_W - F0_BOX_W, UI_DISP_H - F0_BOX_H},   //pos3
                                {                   0, UI_DISP_H - F0_BOX_H}};  //pos4

  uint8_t   param[F0_PARAM];
} f0;
/************************************* 3D CUBE *************************************/
// 定义3D CUBE的参数
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4   
#define LOGO16_GLCD_HEIGHT 16 //定义显示高度  
#define LOGO16_GLCD_WIDTH  16 //定义显示宽度   

float cube[8][3]={{-15,-15,-15},{-15,15,-15},{15,15,-15},{15,-15,-15},{-15,-15,15},{-15,15,15},{15,15,15},{15,-15,15}}; // 立方体各点坐标
int lineid[]={1,2,2,3,3,4,4,1,5,6,6,7,7,8,8,5,8,4,7,3,6,2,5,1}; // 记录点之间连接顺序
/************************************* Time Weather *************************************/
void oled_show_image(int clockRadius,int centerX,int centerY,int dotSize);

// 时间同步相关
#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com"
#define NTP3 "ntp3.aliyun.com"

// WIFI配置
const char *ssid = "Redmi K30 Pro";   // WIFI账户
const char *password = "12345678";   // WIFI密码

// 星期和月份数组
const String WDAY_NAMES[] = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
const String MONTH_NAMES[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static tm timeInfo;
const String date = WDAY_NAMES[timeInfo.tm_wday];

//天气
String city="海口";
int temp;
String info;
String direct;
String power; 

//定义   此处是聚合数据官网给的，可以自己创建账号获得key参数
String url="http://apis.juhe.cn/simpleWeather/query";    //请求网址响应
String key="0a6ba44ad9da110116545ebccb2617f0";     

/************************************* 断电保存 *************************************/

#include <EEPROM.h>

//EEPROM变量
#define   EEPROM_CHECK        11
struct
{
  bool    init;
  bool    change;
  int     address;
  uint8_t check;
  uint8_t check_param[EEPROM_CHECK] = { 'a', 'b', 'c', 'd', 'e', 'f','g', 'h', 'i', 'j', 'k' }; 
} eeprom;

//EEPROM写数据，回到睡眠时执行一遍
void eeprom_write_all_data()
{
  eeprom.address = 0;
  for (uint8_t i = 0; i < EEPROM_CHECK; ++i)    EEPROM.write(eeprom.address + i, eeprom.check_param[i]);  eeprom.address += EEPROM_CHECK;
  for (uint8_t i = 0; i < UI_PARAM; ++i)        EEPROM.write(eeprom.address + i, ui.param[i]);            eeprom.address += UI_PARAM;
  for (uint8_t i = 0; i < F0_PARAM; ++i)        EEPROM.write(eeprom.address + i, f0.param[i]);            eeprom.address += F0_PARAM;
}

//EEPROM读数据，开机初始化时执行一遍
void eeprom_read_all_data()
{
  eeprom.address = EEPROM_CHECK;   
  for (uint8_t i = 0; i < UI_PARAM; ++i)        ui.param[i]   = EEPROM.read(eeprom.address + i);          eeprom.address += UI_PARAM;
  for (uint8_t i = 0; i < F0_PARAM; ++i)        f0.param[i]   = EEPROM.read(eeprom.address + i);          eeprom.address += F0_PARAM;
}

//开机检查是否已经修改过，没修改过则跳过读配置步骤，用默认设置
void eeprom_init()
{
  eeprom.check = 0;
  eeprom.address = 0; for (uint8_t i = 0; i < EEPROM_CHECK; ++i)  if (EEPROM.read(eeprom.address + i) != eeprom.check_param[i])  eeprom.check ++;
  if (eeprom.check <= 1) eeprom_read_all_data();  //允许一位误码
  else device_init();
}

/************************************* 旋钮相关 *************************************/

//旋钮引脚
#define   AIO   7
#define   BIO   10
#define   SW    6

//按键ID
#define   BTN_ID_CC           0 //逆时针旋转
#define   BTN_ID_CW           1 //顺时针旋转
#define   BTN_ID_SP           2 //SW短按
#define   BTN_ID_LP           3 //SW长按

//按键变量
struct
{
  uint8_t   id;
  bool      flag;
  bool      pressed;
  bool      CW_1;
  bool      CW_2;
  bool      val;
  bool      val_last;  
  bool      alv;  
  bool      blv;
  long      count;
} volatile btn;

void knob_inter() 
{
  btn.alv = digitalRead(AIO);
  btn.blv = digitalRead(BIO);
  if (!btn.flag && btn.alv == LOW) 
  {
    btn.CW_1 = btn.blv;
    btn.flag = true;
  }
  if (btn.flag && btn.alv) 
  {
    btn.CW_2 = !btn.blv;
    if (btn.CW_1 && btn.CW_2)
     {
      btn.id = ui.param[KNOB_DIR];
      btn.pressed = true;
    }
    if (btn.CW_1 == false && btn.CW_2 == false) 
    {
      btn.id = !ui.param[KNOB_DIR];
      btn.pressed = true;
    }
    btn.flag = false;
  }
}

void btn_scan() 
{
  btn.val = digitalRead(SW);
  if (btn.val != btn.val_last)
  {
    btn.val_last = btn.val;
    delay(ui.param[BTN_SPT]);
    btn.val = digitalRead(SW);
    if (btn.val == LOW)
    {
      btn.pressed = true;
      btn.count = 0;
      while (!digitalRead(SW))
      {
        btn.count++;
        delay(1);
      }
      if (btn.count < ui.param[BTN_LPT])  btn.id = BTN_ID_SP;
      else  btn.id = BTN_ID_LP;
    }
  }
}

void btn_init() 
{
  pinMode(AIO, INPUT);
  pinMode(BIO, INPUT);
  pinMode(SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(AIO), knob_inter, CHANGE);
}

/************************************ 初始化函数 ***********************************/

//单选框初始化
void check_box_s_init(uint8_t *param, uint8_t *param_p)
{
  check_box.s = param;
  check_box.s_p = param_p;
}

//多选框初始化
void check_box_m_init(uint8_t *param)
{
  check_box.m = param;
}

//数值初始化
void check_box_v_init(uint8_t *param)
{
  check_box.v = param;
}

//单选框处理函数
void check_box_s_select(uint8_t val, uint8_t pos)
{
  *check_box.s = val;
  *check_box.s_p = pos;
  eeprom.change = true;
}

//多选框处理函数
void check_box_m_select(uint8_t param)
{
  check_box.m[param] = !check_box.m[param];
  eeprom.change = true;
}

//弹窗数值初始化
void win_init(char title[], uint8_t *value, uint8_t max, uint8_t min, uint8_t step, uint8_t index, MENU *bg)
{
  ui.index = M_WIN;
  ui.state = S_MENU;
  strcpy(win.title, title);
  win.value = value;
  win.max = max;
  win.min = min;
  win.step = step;
  win.index = index;
  win.bg = bg;
}

/*********************************** UI 初始化函数 *********************************/

//列表行数
void list_init()
{
  list.num[M_MAIN]      = sizeof( main_menu     )   / sizeof(M_SELECT);
  list.num[M_EDITOR]    = sizeof( editor_menu   )   / sizeof(M_SELECT);
  list.num[M_EDIT_F0]   = sizeof( edit_f0_menu  )   / sizeof(M_SELECT);
  list.num[M_SETTING]   = sizeof( setting_menu  )   / sizeof(M_SELECT);
  list.num[M_ABOUT]     = sizeof( about_menu    )   / sizeof(M_SELECT);   
}

//默认设置
void device_init()
{
  //UI初始化
  ui.param[DISP_BRI]  = 255;
  ui.param[BOX_X_OS]  = 10;
  ui.param[BOX_Y_OS]  = 10;
  ui.param[WIN_Y_OS]  = 30;
  ui.param[LIST_ANI]  = 200;
  ui.param[WIN_ANI]   = 50;
  ui.param[FADE_ANI]  = 30;
  ui.param[BTN_SPT]   = 50;
  ui.param[BTN_LPT]   = 255;
  ui.param[COME_SCR]  = 0;
  ui.param[KNOB_DIR]  = 1;

  //功能初始化
  f0.param[F0_X_OS]  = 40;
  f0.param[F0_Y_OS]  = 40;
  f0.param[F0_ANI]   = 100;
}

/********************************* 分页面初始化函数 ********************************/

//F0编辑页初始化
void edit_f0_init()
{
  check_box_v_init(f0.param);
}

//设置页初始化
void setting_init()
{
  check_box_v_init(ui.param);
  check_box_m_init(ui.param);
}

/********************************** 通用初始化函数 *********************************/

/*
  页面层级管理逻辑是，把所有页面都先当作列表类初始化，不是列表类的按标签运行对应的初始化函数
  这样做会浪费一些资源，但跳转页面时只需要考虑页面层级，逻辑上更清晰
*/

//进入更深层级时的初始化
void layer_in()
{
  ui.layer ++;
  ui.state = S_FADE;
  ui.init_flag = false;
  ui.select[ui.layer] = 0;
  list.box_y_trg[ui.layer] = 0;
  list.box_w_trg += ui.param[BOX_X_OS] * (list.box_y_trg[ui.layer - 1] / LIST_LINE_H);
  list.box_h_trg += ui.param[BOX_Y_OS] * (list.box_y_trg[ui.layer - 1] / LIST_LINE_H);
  switch (ui.index)
  {   
    case M_EDIT_F0: edit_f0_init();   break;
    case M_SETTING: setting_init();   break;
  }
}

//进入更浅层级时的初始化
void layer_out()
{
  ui.layer --;
  ui.state = S_FADE;
  ui.init_flag = false;
  list.box_w_trg += ui.param[BOX_X_OS] * abs((list.box_y_trg[ui.layer] - list.box_y_trg[ui.layer + 1]) / LIST_LINE_H);
  list.box_h_trg += ui.param[BOX_Y_OS] * abs((list.box_y_trg[ui.layer] - list.box_y_trg[ui.layer + 1]) / LIST_LINE_H);
}

/************************************* 动画函数 *************************************/

//消失动画
void fade()
{
  delay(ui.param[FADE_ANI]);
  switch (ui.fade)
  {
    case 1: for (uint16_t i = 0; i < ui.buf_len; ++i)  if (i % 2 != 0) ui.buf_ptr[i] = ui.buf_ptr[i] & 0xAA; break;
    case 2: for (uint16_t i = 0; i < ui.buf_len; ++i)  if (i % 2 != 0) ui.buf_ptr[i] = ui.buf_ptr[i] & 0x00; break;
    case 3: for (uint16_t i = 0; i < ui.buf_len; ++i)  if (i % 2 == 0) ui.buf_ptr[i] = ui.buf_ptr[i] & 0x55; break;
    case 4: for (uint16_t i = 0; i < ui.buf_len; ++i)  if (i % 2 == 0) ui.buf_ptr[i] = ui.buf_ptr[i] & 0x00; break;
    default: ui.state = S_MENU; ui.fade = 0; break;
  }
  ui.fade++;
}

//渐近动画
void animation(float *a, float *a_trg, uint8_t n)
{
  if (*a != *a_trg)
  {
    if (fabs(*a - *a_trg) < 0.15f) *a = *a_trg;
    else *a += (*a_trg - *a) / (n / 10.0f);
  }
}

/************************************* 显示函数 *************************************/

/*********************************** 通用显示函数 ***********************************/

//绘制行末尾元素
void list_draw_val(int n) { u8g2.setCursor  (list.text_w_temp, LIST_TEXT_H + LIST_TEXT_S + list.text_y_temp); u8g2.print(check_box.v[n - 1]); }                 //数值
void list_draw_cbf()      { u8g2.drawRFrame (list.text_w_temp, CB_U + list.text_y_temp, CB_W, CB_H, 0.5f); }                                                    //外框
void list_draw_cbd()      { u8g2.drawBox    (list.text_w_temp + CB_D + 1, CB_U + CB_D + 1 + list.text_y_temp, CB_W - (CB_D + 1) * 2, CB_H - (CB_D + 1) * 2); }  //外框里面的点

//列表类页面通用显示函数
void list_show(uint8_t ui_index, struct MENU arr[])
{
  //在初始化时更新一次的参数
  if (!ui.init_flag)
  {
    ui.init_flag = true;
    ui.oper_flag = true;
    list.select = ui.select[ui.layer];
    list.text_x = - UI_DISP_W;
    list.text_y = list.box_y_trg[ui.layer] - LIST_LINE_H * ui.select[ui.layer];
    list.text_y_trg = list.text_y;
    list.box_H = LIST_LINE_H;
    u8g2.setFont(LIST_FONT);
    u8g2.setDrawColor(2);
  }

  //在每次操作后都会更新的参数
  if (ui.oper_flag)
  {
    ui.oper_flag = false;
    list.box_W = u8g2.getUTF8Width(arr[ui.select[ui.layer]].m_select) + LIST_TEXT_S * 2;
    list.box_w_trg += ui.param[BOX_X_OS];
    list.bar_h_trg = ceil((ui.select[ui.layer]) * ((float)UI_DISP_H / (list.num[ui_index] - 1)));
  }

  //计算动画过渡值
  animation(&list.text_x, &list.text_x_trg, ui.param[LIST_ANI]);
  animation(&list.text_y, &list.text_y_trg, ui.param[LIST_ANI]);
  animation(&list.box_y, &list.box_y_trg[ui.layer], ui.param[LIST_ANI]);
  animation(&list.box_w, &list.box_w_trg, ui.param[LIST_ANI]);
  animation(&list.box_w_trg, &list.box_W, ui.param[LIST_ANI]);
  animation(&list.box_h, &list.box_h_trg, ui.param[LIST_ANI]);
  animation(&list.box_h_trg, &list.box_H, ui.param[LIST_ANI]);
  animation(&list.bar_h, &list.bar_h_trg, ui.param[LIST_ANI]);

  //绘制列表文字和行末尾元素
  for (int i = 0; i < list.num[ui_index]; ++ i)
  {
    switch (ui.param[COME_SCR])
    {
      case 0: list.text_x_temp = list.text_x * (abs(list.select - i) + 1); break;
      case 1: list.text_x_temp = list.text_x * (i + 1); break;
    }
    list.text_y_temp = list.text_y + LIST_LINE_H * i;
    list.text_w_temp = list.text_x_temp + LIST_TEXT_W;
    u8g2.setCursor(list.text_x_temp + LIST_TEXT_S, LIST_TEXT_S + LIST_TEXT_H + list.text_y_temp);
    u8g2.print(arr[i].m_select);
    switch (arr[i].m_select[0])
    {
      case '~': list_draw_val(i); break;
      case '+': list_draw_cbf();  if (check_box.m[i - 1] == 1)  list_draw_cbd();  break;
      case '=': list_draw_cbf();  if (*check_box.s_p == i)      list_draw_cbd();  break;
    }
  }

  //绘制进度条和选择框
  u8g2.drawBox(UI_DISP_W - LIST_BAR_W, 0, LIST_BAR_W, list.bar_h);
  u8g2.drawRBox(0, list.box_y - (list.box_h - LIST_LINE_H) / 2, list.box_w, list.box_h, LIST_BOX_R);
}

//弹窗通用显示函数
void win_show()
{
  //在进场时更新的参数
  if (!win.init_flag)
  {
    win.init_flag = true;
    win.oper_flag = true;
    win.box_y = 0;
    win.box_y_trg = (UI_DISP_H - WIN_H) / 2;
    win.box_w = UI_DISP_W;
    win.box_w_trg = WIN_W;
    win.box_H = WIN_H;
    win.box_h = 0;
    win.box_h_trg = win.box_H + ui.param[WIN_Y_OS];
    win.bar_x = 0;
    u8g2.setFont(WIN_FONT);
  }

  //在离场时更新的参数
  if (win.exit_flag)
  {
    win.box_H = 0; 
    win.box_y_trg = 0;
    win.box_w_trg = UI_DISP_W;
    if (!win.box_y)
    {
      ui.index = win.index;
      win.init_flag = false;
      win.exit_flag = false;
    }
  }

  //在每次操作后都会更新的参数
  if (win.oper_flag)
  {
    win.oper_flag = false;
    win.bar_x_trg = (float)(*win.value - win.min) / (float)(win.max - win.min) * (win.box_w_trg - 2 * WIN_TITLE_S);
  }

  //计算动画过渡值
  animation(&win.box_y, &win.box_y_trg, ui.param[WIN_ANI]);
  animation(&win.box_w, &win.box_w_trg, ui.param[WIN_ANI]);
  animation(&win.box_h, &win.box_h_trg, ui.param[WIN_ANI]);
  animation(&win.box_h_trg, &win.box_H, ui.param[WIN_ANI]);
  animation(&win.bar_x, &win.bar_x_trg, ui.param[WIN_ANI]);

  //绘制背景列表和窗口
  list_show(win.index, win.bg);
  win.box_x = (UI_DISP_W - win.box_w) / 2;
  u8g2.setDrawColor(0); u8g2.drawBox  (win.box_x, win.box_y, win.box_w, win.box_h);  //绘制外框背景
  u8g2.setDrawColor(2); u8g2.drawFrame(win.box_x, win.box_y, win.box_w, win.box_h);  //绘制外框描边
  if (win.box_h > (WIN_TITLE_H + 2 * WIN_TITLE_S))
  {
    u8g2.setCursor(win.box_x + WIN_VALUE_S, win.box_y + WIN_TITLE_S + WIN_TITLE_H); u8g2.print(*win.value);             //绘制数值
    u8g2.setCursor(win.box_x + WIN_TITLE_S, win.box_y + WIN_TITLE_S + WIN_TITLE_H); u8g2.print(win.title);              //绘制标题
    u8g2.drawBox  (win.box_x + WIN_TITLE_S, win.box_y + win.box_h - WIN_TITLE_S - WIN_BAR_H - 1, win.bar_x, WIN_BAR_H); //绘制进度条
  }

  //需要在窗口修改参数时立即见效的函数
  if (!strcmp(win.title, "Disp Bri")) u8g2.setContrast(ui.param[DISP_BRI]);
}

/********************************** 分页面显示函数 **********************************/
/********************************** 方块平移相关 **********************************/
void f0_show()
{
  //在进场时更新的参数
  if (!ui.init_flag)
  {
    ui.init_flag = true;

    //进场时元素从屏幕外入场
    f0.box_x = - F0_BOX_W;
    f0.box_y = - F0_BOX_H;

    //进场时元素移动到初始位置
    f0.box_x_trg = 0;
    f0.box_y_trg = 0;

    //进场时元素效果
    f0.box_w_trg += f0.param[F0_X_OS];
    f0.box_h_trg += f0.param[F0_Y_OS];

    //其它初始化
    u8g2.setDrawColor(1);
  }

  //在每次操作后都会更新的参数
  if (ui.oper_flag)
  {
    ui.oper_flag = false;
    if (f0.box_x != f0.box_x_trg) f0.box_w_trg += f0.param[F0_X_OS];
    if (f0.box_y != f0.box_y_trg) f0.box_h_trg += f0.param[F0_Y_OS];
  }

  //计算动画过渡值
  animation(&f0.box_x, &f0.box_x_trg, f0.param[F0_ANI]);
  animation(&f0.box_y, &f0.box_y_trg, f0.param[F0_ANI]);
  animation(&f0.box_w, &f0.box_w_trg, f0.param[F0_ANI]);
  animation(&f0.box_w_trg, &f0.box_W, f0.param[F0_ANI]);
  animation(&f0.box_h, &f0.box_h_trg, f0.param[F0_ANI]);
  animation(&f0.box_h_trg, &f0.box_H, f0.param[F0_ANI]);

  //绘制元素
  u8g2.drawRBox((int16_t)f0.box_x, (int16_t)f0.box_y, f0.box_w, f0.box_h, F0_BOX_R);
}
/************************************* 3DCUBE相关 *************************************/
// 矩阵乘法函数
float* matconv(float* a, float b[3][3]) {
    static float res[3]; // 使用静态变量，避免栈内存丢失
    for (int i = 0; i < 3; i++) {
        res[i] = b[i][0] * a[0] + b[i][1] * a[1] + b[i][2] * a[2];
    }
    for (int i = 0; i < 3; i++) a[i] = res[i];
    return a;
}

// 旋转函数
void rotate(float* obj, float x, float y, float z) {
    x /= PI; y /= PI; z /= PI; // 将角度转换为弧度
    float rz[3][3] = {{cos(z), -sin(z), 0}, {sin(z), cos(z), 0}, {0, 0, 1}}; // Z轴旋转矩阵
    float ry[3][3] = {{1, 0, 0}, {0, cos(y), -sin(y)}, {0, sin(y), cos(y)}}; // Y轴旋转矩阵
    float rx[3][3] = {{cos(x), 0, sin(x)}, {0, 1, 0}, {-sin(x), 0, cos(x)}}; // X轴旋转矩阵
    matconv(matconv(matconv(obj, rz), ry), rx); // 顺序旋转
}
// 显示立方体的函数
void f1_show() {
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2.setFontDirection(0);
    u8g2.firstPage(); 
    do {
        u8g2.clearBuffer(); // 清空缓冲区

        for (int i = 0; i < 8; i++) {
            rotate(cube[i], 0.01f, 0.01f, 0.01f); // 旋转每个点（这里减小了旋转角度以减慢旋转速度）
        }

        for (int i = 0; i < 24; i += 2) { // 绘制立方体
            int x0 = cube[lineid[i] - 1][0];
            int y0 = cube[lineid[i] - 1][1];
            int x1 = cube[lineid[i + 1] - 1][0];
            int y1 = cube[lineid[i + 1] - 1][1];

            // 进行坐标映射，将坐标转为屏幕内坐标
            x0 += SCREEN_WIDTH / 2; // 将立方体居中显示
            y0 += SCREEN_HEIGHT / 2; // 将立方体居中显示
            x1 += SCREEN_WIDTH / 2;
            y1 += SCREEN_HEIGHT / 2;

            // 转换为整数并绘制
            u8g2.drawLine((int)x0, (int)(SCREEN_HEIGHT - y0), (int)x1, (int)(SCREEN_HEIGHT - y1)); // 绘制线条
        }
    } while (u8g2.nextPage());

    delay(10); // 延迟以减慢刷新速度
}
/************************************* Time Weather相关 *************************************/
//clockRadius 圆形时钟半径
//centerX     圆心X坐标
//centerY     圆心Y坐标
//dotSize     点的大小，1x1
void oled_show_image(int clockRadius,int centerX,int centerY,int dotSize){
  // 绘制表盘的12个整点位置（用点表示）
  for (int i = 0; i < 12; i++) {
    float angle = radians(360 / 12 * i); // 每个整点的角度间隔
    int x = centerX + cos(angle - M_PI_2) * clockRadius; // 整点位置X
    int y = centerY + sin(angle - M_PI_2) * clockRadius; // 整点位置Y
    //u8g2.drawPixel(x, y); // 在对应位置画一个小点
    u8g2.drawBox(x - dotSize / 2, y - dotSize / 2, dotSize, dotSize);// 使用drawBox绘制一个大点（1x1方块）
  }

  // 绘制中心的小实心圆表示指针固定点
  u8g2.drawDisc(centerX, centerY, 3);

  // 画时针（根据当前小时）
  float hourAngle = (timeInfo.tm_hour % 12 + timeInfo.tm_min / 60.0) * 30; // 每小时30度
  int hourX = centerX + cos(radians(hourAngle - 90)) * clockRadius * 0.6; // 时针长度60%半径
  int hourY = centerY + sin(radians(hourAngle - 90)) * clockRadius * 0.6;
  u8g2.drawLine(centerX, centerY, hourX, hourY);

  // 画分针（根据当前分钟）
  float minAngle = (timeInfo.tm_min + timeInfo.tm_sec / 60.0) * 6; // 每分钟6度
  int minX = centerX + cos(radians(minAngle - 90)) * clockRadius * 0.8; // 分针长度80%半径
  int minY = centerY + sin(radians(minAngle - 90)) * clockRadius * 0.8;
  u8g2.drawLine(centerX, centerY, minX, minY);

  // 画秒针（根据当前秒数）
  float secAngle = timeInfo.tm_sec * 6; // 每秒6度
  int secX = centerX + cos(radians(secAngle - 90)) * clockRadius * 0.9; // 秒针长度90%半径
  int secY = centerY + sin(radians(secAngle - 90)) * clockRadius * 0.9;
  u8g2.drawLine(centerX, centerY, secX, secY);
}
// 显示时间
void f2_show(){
  if (!getLocalTime(&timeInfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  String timeStr = String(timeInfo.tm_year + 1900) + "-" + String(timeInfo.tm_mon + 1) + "-" + String(timeInfo.tm_mday) + " "
                   + String(timeInfo.tm_hour) + ":" + String(timeInfo.tm_min) + ":" + String(timeInfo.tm_sec) + " "
                   + WDAY_NAMES[timeInfo.tm_wday - 1];
  Serial.println(timeStr.c_str());

  //在OLED上显示当前时间
  u8g2.clearBuffer();
      //设置字体
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  u8g2.setCursor(0,16);
  u8g2.print("城市");
  u8g2.setCursor(36,16);
  u8g2.print(city);
  u8g2.setCursor(0,32);
  u8g2.print("温度");
  u8g2.setCursor(36,32);
  u8g2.print(temp);
  u8g2.setCursor(0,48);
  u8g2.print("天气");
  u8g2.setCursor(36,48);
  u8g2.print(info);
  u8g2.setCursor(0,64);
  u8g2.print("风向");
  u8g2.setCursor(36,64);
  u8g2.print(direct);
  u8g2.setCursor(72,64);
  u8g2.print(power);
  oled_show_image(25,100,26,1);
  u8g2.sendBuffer();
  delay(1000);
}
/************************************* 处理函数 *************************************/

/*********************************** 通用处理函数 ***********************************/

//列表旋钮通用处理函数
void list_rot_sw()
{
  ui.oper_flag = true;
  switch (btn.id)
  {
    case BTN_ID_CC:
      if (ui.select[ui.layer] == 0) break;
      if (list.box_y_trg[ui.layer] == 0) list.text_y_trg += LIST_LINE_H;
      else list.box_y_trg[ui.layer] -= LIST_LINE_H;
      list.box_h_trg += ui.param[BOX_Y_OS];
      ui.select[ui.layer] -= 1;
      break;

    case BTN_ID_CW:
      if (ui.select[ui.layer] == (list.num[ui.index] - 1)) break;
      if (list.box_y_trg[ui.layer] == (UI_DISP_H - LIST_LINE_H)) list.text_y_trg -= LIST_LINE_H;
      else list.box_y_trg[ui.layer] += LIST_LINE_H;
      list.box_h_trg += ui.param[BOX_Y_OS];
      ui.select[ui.layer] += 1;
      break;
  }
}

//列表通用处理函数
void list_proc(struct MENU arr[], void (*Func)())
{
  list_show(ui.index, arr); 
  if (btn.pressed) 
  { 
    btn.pressed = false; 
    switch (btn.id) 
    { 
      case BTN_ID_CW: case BTN_ID_CC: list_rot_sw(); break; 
      case BTN_ID_LP: ui.select[ui.layer] = 0; 
      case BTN_ID_SP: Func();
    }
  }
}

//弹窗通用处理函数
void win_proc()
{
  win_show();
  if (btn.pressed && win.box_y == win.box_y_trg && win.box_y_trg != 0)
  {
    btn.pressed = false;
    win.oper_flag = true;
    switch (btn.id)
    {
      case BTN_ID_CW: if (*win.value < win.max) *win.value += win.step;  eeprom.change = true;  break;
      case BTN_ID_CC: if (*win.value > win.min) *win.value -= win.step;  eeprom.change = true;  break;  
      case BTN_ID_LP: case BTN_ID_SP: win.exit_flag = true;  break;
    }
  }
}

/********************************** 分页面处理函数 **********************************/

//睡眠页处理函数
void sleep_proc()
{
  if (!ui.init_flag)
  {
    ui.init_flag = true;
    u8g2.setPowerSave(1);
    if (eeprom.change)
    {
      eeprom_write_all_data();
      eeprom.change = false;
    }
  }
  btn_scan();
  if (btn.pressed) 
  { 
    btn.pressed = false; 
    switch (btn.id) 
    {
      case BTN_ID_CW: break;  //顺时针旋转执行的函数
      case BTN_ID_CC: break;  //逆时针旋转执行的函数
      case BTN_ID_SP: break;  //短按执行的函数
      case BTN_ID_LP: 
      
        ui.index = M_MAIN;
        ui.state = S_LAYER_IN;

        list.box_y = 0;
        list.box_w = 0;
        list.box_w_trg = 0;
        list.box_h = 0;
        list.box_h_trg = 0;
        list.bar_h = 0;
        u8g2.setPowerSave(0);
        break;
    }
  }
}

//主菜单处理函数
void main_proc()
{
  switch (ui.select[ui.layer]) 
  {
    case 0:   ui.index = M_SLEEP;   ui.state = S_LAYER_OUT; break;
    case 1:   ui.index = M_F0;      ui.state = S_LAYER_IN;  break;
    case 2:   ui.index = M_F1;      ui.state = S_LAYER_IN;  break;
    case 3:   ui.index = M_F2;      ui.state = S_LAYER_IN;  break;
    case 7:   ui.index = M_EDITOR;  ui.state = S_LAYER_IN;  break;
    case 8:   ui.index = M_SETTING; ui.state = S_LAYER_IN;  break;
  }
}

//解压玩具处理函数
void f0_proc()
{
  f0_show();
  btn_scan();
  if (btn.pressed) 
  { 
    btn.pressed = false; 
    ui.oper_flag = true;
    switch (btn.id) 
    {
      case BTN_ID_CW: f0.select ++; if (f0.select > F0_POS_N - 1) f0.select = 0;  break;
      case BTN_ID_CC: f0.select --; if (f0.select < 0) f0.select = F0_POS_N - 1;  break;
      case BTN_ID_SP: case BTN_ID_LP: ui.index = M_MAIN;  ui.state = S_LAYER_OUT; break;
    }
    f0.box_x_trg = f0.pos[f0.select][F0_BOX_X];
    f0.box_y_trg = f0.pos[f0.select][F0_BOX_Y];
  }
}

//3D立方体处理函数
void f1_proc()
{
  f1_show();
  btn_scan();
  if (btn.pressed) 
  { 
    btn.pressed = false; 
    ui.oper_flag = true;
    switch (btn.id) 
    {
      case BTN_ID_SP: case BTN_ID_LP: ui.index = M_MAIN;  ui.state = S_LAYER_OUT; break;
    }

  }
}
//Time Weather处理函数
void f2_proc()
{
  f2_show();
  btn_scan();
  if (btn.pressed) 
  { 
    btn.pressed = false; 
    ui.oper_flag = true;
    switch (btn.id) 
    {
      case BTN_ID_SP: case BTN_ID_LP: ui.index = M_MAIN;  ui.state = S_LAYER_OUT; break;
    }
  }
}
//编辑器菜单处理函数
void editor_proc()
{
  switch (ui.select[ui.layer]) 
  {
    case 0:   ui.index = M_MAIN;    ui.state = S_LAYER_OUT; break;
    case 1:   ui.index = M_EDIT_F0; ui.state = S_LAYER_IN;  break;
  }
}

//编辑解压玩具菜单
void edit_f0_proc()
{
  switch (ui.select[ui.layer]) 
  {
    case 0:   ui.index = M_EDITOR;  ui.state = S_LAYER_OUT; break;
    case 1:   win_init("F0 X OS",  &f0.param[F0_X_OS],   100,  0,  1, ui.index, edit_f0_menu);  break;
    case 2:   win_init("F0 Y OS",  &f0.param[F0_Y_OS],   100,  0,  1, ui.index, edit_f0_menu);  break;
    case 3:   win_init("F0 Ani",   &f0.param[F0_ANI],    255,  0,  1, ui.index, edit_f0_menu);  break;
  }
}

//设置菜单处理函数
void setting_proc()
{
  switch (ui.select[ui.layer]) 
  {
    //返回
    case 0:   ui.index = M_MAIN;    ui.state = S_LAYER_OUT; break;
        
    //弹出窗口
    case 1:   win_init("Disp Bri", &ui.param[DISP_BRI],  255,  0,  5, ui.index, setting_menu);  break;
    case 2:   win_init("Box X OS", &ui.param[BOX_X_OS],   40,  0,  1, ui.index, setting_menu);  break;
    case 3:   win_init("Box Y OS", &ui.param[BOX_Y_OS],   40,  0,  1, ui.index, setting_menu);  break;
    case 4:   win_init("Win Y OS", &ui.param[WIN_Y_OS],   40,  0,  1, ui.index, setting_menu);  break;
    case 5:   win_init("List Ani", &ui.param[LIST_ANI],  255, 20,  1, ui.index, setting_menu);  break;
    case 6:   win_init("Win Ani",  &ui.param[WIN_ANI],   255, 20,  1, ui.index, setting_menu);  break;
    case 7:   win_init("Fade Ani", &ui.param[FADE_ANI],  255,  0,  1, ui.index, setting_menu);  break;
    case 8:   win_init("Btn SPT",  &ui.param[BTN_SPT],   255,  0,  1, ui.index, setting_menu);  break;
    case 9:   win_init("Btn LPT",  &ui.param[BTN_LPT],   255,  0,  1, ui.index, setting_menu);  break;

    //多选框
    case 10:  check_box_m_select( COME_SCR );  break;
    case 11:  check_box_m_select( KNOB_DIR );  break;

    //关于本机
    case 12:  ui.index = M_ABOUT;   ui.state = S_LAYER_IN;  break;
  }
}

//关于本机
void about_proc()
{
  switch (ui.select[ui.layer]) 
  {
    case 0:   ui.index = M_SETTING; ui.state = S_LAYER_OUT; break;
  }
}

//总进程
void ui_proc()
{
  u8g2.sendBuffer();
  switch (ui.state)
  {
    case S_LAYER_IN:    layer_in();           break;
    case S_LAYER_OUT:   layer_out();          break;
    case S_FADE:        fade();               break;
    case S_MENU:        u8g2.clearBuffer();   switch (ui.index)
    {
      case M_WIN:             win_proc();                                         break;
      case M_SLEEP:           sleep_proc();                                       break;
        case M_MAIN:          list_proc(  main_menu,          main_proc     );    break;
          case M_F0:          f0_proc();                                          break;
          case M_F1:          f1_proc();                                          break;
          case M_F2:          f2_proc();                                          break;
          case M_EDITOR:      list_proc(  editor_menu,        editor_proc   );    break;
            case M_EDIT_F0:   list_proc(  edit_f0_menu,       edit_f0_proc  );    break;
          case M_SETTING:     list_proc(  setting_menu,       setting_proc  );    break;
            case M_ABOUT:     list_proc(  about_menu,         about_proc    );    break;
    }
  }
}

//OLED初始化函数
void oled_init()
{
  u8g2.setBusClock(10000000);
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setContrast(ui.param[DISP_BRI]);
  ui.buf_ptr = u8g2.getBufferPtr();
  ui.buf_len = 8 * u8g2.getBufferTileHeight() * u8g2.getBufferTileWidth();
}

//Time_Weather初始化函数
void Time_Weather_init(){
   //连接WiFi
  WiFi.begin(ssid,password);
  Serial.print("正在连接Wi-Fi");

  //检测是否连接成功
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("连接成功");
  Serial.print("IP地址:");
  Serial.println(WiFi.localIP());
  // 获取并配置时间
  configTime(8 * 3600, 0, NTP1, NTP2, NTP3);

  //get weather
  //创建HTTPCLient 对象
  HTTPClient http;
 
  //发送GET请求
  http.begin(url+"?city="+city+"&key="+key);
  int httpCode=http.GET();
  //获取响应状态码
  Serial.printf("HTTP 状态码:%d",httpCode);
  //获取响应正文
  String response=http.getString();
  Serial.println("响应数据");
  Serial.println(response);
 
  http.end();
  //创建DynamicJsonDocument对象
  DynamicJsonDocument doc(1024);
  //解析JSON数据
  deserializeJson(doc,response);
  //从解析后的JSON文档中获取值
  temp=doc["result"]["realtime"]["temperature"].as<int>();
  info=doc["result"]["realtime"]["info"].as<String>();
  direct=doc["result"]["realtime"]["direct"].as<String>();
  power=doc["result"]["realtime"]["power"].as<String>(); 
  //int aqi=doc["result"]["realtime"]["aqi"].as<int>();
 
  Serial.printf("温度：%d\n",temp);
  Serial.printf("天气：%s\n",info);
  //Serial.printf("空气指数：%d\n",aqi);

}
void close()
{
  u8g2.setDrawColor(1);
  u8g2.clear();
  u8g2.drawBox(0, 63 / 2 - 3, 127, 6);
  u8g2.drawBox(127 / 2 - 3, 0, 6, 63);
  u8g2.sendBuffer();
  delay(3);
  u8g2.clear();
  u8g2.drawBox(0, 63 / 2 - 2, 127, 4);
  u8g2.drawBox(127 / 2 - 2, 0, 4, 63);
  u8g2.sendBuffer();
  delay(3);
  u8g2.clear();
  u8g2.drawBox(0, 63 / 2 - 1, 127, 2);
  u8g2.drawBox(127 / 2 - 1, 0, 2, 63);
  u8g2.sendBuffer();
  delay(3);
  u8g2.clear();
}

void setup() 
{
  Serial.begin(115200);
  eeprom_init();
  list_init();
  oled_init();
  btn_init();
  Time_Weather_init();
}
 
void loop() 
{
  btn_scan();
  ui_proc();
}

