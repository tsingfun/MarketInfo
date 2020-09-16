#ifndef __COMMON_DEFINE_H_
#define __COMMON_DEFINE_H_

// 默认展示期货代码
#define DEFAULT_CODE	"IF1503"

// 前置地址
#define FRONT_ADDR		"tcp://180.169.124.109:41213"
// 经纪公司代码
#define BROKER_ID		"20000"

// caption_logo width
#define CAPTION_LOGO_WIDTH 90

// 列名
const TCHAR LIST_COLUMN_NAME[][10] = {_T("名称"), _T("买五"), _T("买四"), _T("买三"), _T("买二"), _T("买一"), 
									  _T("现价"), _T("卖一"), _T("卖二"), _T("卖三"), _T("卖四"), _T("卖五")};

typedef struct _STOCK_DATA {
	// 名称
	TThostFtdcInstrumentIDType Name;
	// 现价
	TThostFtdcPriceType Price;
	// 买一~五
	TThostFtdcPriceType Buy1;
	TThostFtdcPriceType Buy2;
	TThostFtdcPriceType Buy3;
	TThostFtdcPriceType Buy4;
	TThostFtdcPriceType Buy5;
	// 卖一~五
	TThostFtdcPriceType Sell1;
	TThostFtdcPriceType Sell2;
	TThostFtdcPriceType Sell3;
	TThostFtdcPriceType Sell4;
	TThostFtdcPriceType Sell5;
} STOCK_DATA, *PSTOCK_DATA;


// 颜色、字体
#define COLOR_TOOLBAR_BG	RGB(45, 50, 55)
#define COLOR_BLACK_BG		RGB(0, 0, 0)
#define COLOR_GRAY_BORDER	RGB(60, 60, 60)
#define COLOR_SEP_QUOTE		RGB(120, 120, 120)
#define COLOR_TITLE_BG		RGB(25, 25, 25)

// 文字颜色
#define COLOR_YELLOW_TEXT	RGB(255, 255, 0)
#define COLOR_GREEN_TEXT	RGB(82, 255, 82)
#define COLOR_RED_TEXT		RGB(255, 82, 82)
#define COLOR_WHITE_TEXT	RGB(192, 192, 192)
#define COLOR_CYAN_TEXT		RGB(82, 255, 255)

// 键盘宝对话框颜色
#define COLOR_JPB_BORDER	RGB(227, 227, 227)
#define COLOR_JPB_IN_BORDER	RGB(240, 240, 240)
#define COLOR_JPB_FILL		RGB(255, 255, 255)
#define COLOR_JPB_MARKET	RGB(107, 107, 107)
#define COLOR_JPB_NAME		RGB(0, 0, 0)

// 更新提示渐变背景色
// 灰色风格
//#define COLOR_SPLASH_4		RGB(24, 35, 69)
//#define COLOR_SPLASH_3		RGB(18, 26, 47)
//#define COLOR_SPLASH_2		RGB(12, 17, 35)
//#define COLOR_SPLASH_1		RGB(0, 0, 0)

// 稍淡蓝风格
#define COLOR_SPLASH_4		RGB(0, 0, 106)
#define COLOR_SPLASH_3		RGB(0, 0, 74)
#define COLOR_SPLASH_2		RGB(0, 0, 45)
#define COLOR_SPLASH_1		RGB(0, 0, 0)

// 蓝色风格
//#define COLOR_SPLASH_4		RGB(0, 0, 156)
//#define COLOR_SPLASH_3		RGB(0, 0, 106)
//#define COLOR_SPLASH_2		RGB(0, 0, 74)
//#define COLOR_SPLASH_1		RGB(0, 0, 0)




// 记录窗口位置
#define SECTION_DROPLOCATION        _T("DropLocation")
#define INI_KEY_MAIN_X              _T("MainX")
#define INI_KEY_MAIN_Y              _T("MainY")

#endif __COMMON_DEFINE_H_