#define SETTINGS_KEY 0

Window *window;

GBitmap *ruler_base_outline, *bt_icon, *bt_icon_invert;

BitmapLayer *base_layer, *bt_icon_layer;

Layer *lines_layer, *bluetooth_circle_layer, *battery_lines_layer, *fix_pixels_layer;

TextLayer *hour_layer, *day_layer, *month_layer;
TextLayer *min_des_1, *min_des_2, *min_des_3, *actual_minute, *update_at_a_glance;

InverterLayer *highlighter, *theme;

GFont *impact;

AppTimer *charge_timer, *pixel_build_timer;

char hourBuffer[] = "12";
char day_buffer[] = "Wednesday.";
char month_buffer[] = "26.Sept. '14.";
char minute_buffer[] = "00";

int stage;
bool cancelled = 0;
bool bridge_is_right;
bool invert = 0;
bool animbooted = 0;
int placements[60] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
};
int minute;
int des_1_p_h[12] = {
	0, 1, 10, 11, 20, 21, 30, 31, 40, 41, 50, 51
};
int des_2_p_h[18] = {
	4, 5, 6, 14, 15, 16, 24, 25, 26, 34, 35, 36, 44, 45, 46, 54, 55, 56
};
int des_3_p_h[6] = {
	9, 19, 29, 39, 49, 59,
};
int battery_percent = 10;
bool phone_is_connected = 0;

GRect inverter_layer_location;

typedef struct persist {
	bool bticonhide;
	bool covernumbers;
	bool btdisalert;
	bool btrealert;
	bool theme;
	bool showactualmin;
	bool batterybarstyle;
	uint8_t lang;
	bool cleanerlook;
}persist;

persist settings = {
	.bticonhide = 1,
	.covernumbers = 1,
	.btdisalert = 1,
	.btrealert = 0,
	.theme = 1,
	.showactualmin = 0,
	.batterybarstyle = 0,
	.lang = 0,
	.cleanerlook = 0,
};

//Glance values, I think?
int persistvalue;
int currentAppVer = 4;
bool currentlyGlancing = 0;
int versionChecked = 0;
GRect finish01, start02;
bool booted = 0;
int boot = 0;
char glance_buffer[200];

char *months[2][12] = {
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"
	},
	{
		"Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul", "Aug", "Sept", "Okt", "Nov", "Dez"
	},
};

char *days[2][7] = {
	{
		"Sunday", "Monday", "Tuesday", "Wed.", "Thursday", "Friday", "Saturday",
	},
	{
		"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag",
	},
};

#define HIDEBATTERY_KEY 0
#define HIDEBTICON_KEY 1
#define THEME_KEY 2
#define COVERTEXT_KEY 3
#define BTDISALERT_KEY 4
#define BTREALERT_KEY 5
#define WATCHAPPVER_KEY 6
#define BATTERYBARSTYLE_KEY 7
#define SHOWMINUTE_KEY 8
#define LANGUAGE_KEY 9
#define CLEARNERLOOK_KEY 10

/*
GRect locations[10] = {
	GRect(11, 2, 12, 99), GRect(21, 2, 12, 99), GRect(31, 2, 12, 99), GRect(41, 2, 12, 99), GRect(51, 2, 12, 99),
	GRect(61, 2, 12, 99), GRect(71, 2, 12, 99), GRect(81, 2, 12, 99), GRect(91, 2, 12, 99), GRect(101, 2, 12, 99),
};
*/