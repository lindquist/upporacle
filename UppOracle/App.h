#ifndef _UppOracle_UppOracle_h
#define _UppOracle_UppOracle_h

#include <CtrlLib/CtrlLib.h>
#include <OpenAI/OpenAI.h>

using namespace Upp;

#define IMAGECLASS ImagesImg
#define IMAGEFILE <UppOracle/Images.iml>
#include <Draw/iml_header.h>

//----------------------------------------------------------------------------

#include <LogWindow/LogWindow.h>

//----------------------------------------------------------------------------

class InputDocEdit : public DocEdit {
public:
	typedef InputDocEdit CLASSNAME;
	
	InputDocEdit();
	
	bool Key(dword key, int count) override;
	
	void SetSendHotkey(dword key);
	
public:
	Event<> WhenSend;

protected:
	dword send_hotkey;
};

//----------------------------------------------------------------------------

class RichOutputView : public RichTextView {
public:
    Event<Bar&> WhenBar;

	void RightDown(Point p, dword keyflags) override;
};

//----------------------------------------------------------------------------

class MouseEventInfoCtrl : public InfoCtrl {
public:
	Event<Point, dword> WhenLeftDown;
	Event<Point, dword> WhenRightDown;
	
	virtual Image MouseEvent(int event, Point p, int zdelta, dword keyflags) override {
		bool used = true;
		switch (event) {
		case MouseEvents::LEFTDOWN:		WhenLeftDown(p, keyflags); break;
		case MouseEvents::RIGHTDOWN:	WhenRightDown(p, keyflags); break;
		default: used = false;
		}
		if (!used)
			return InfoCtrl::MouseEvent(event, p, zdelta, keyflags);
		return Image();
	}
};

//----------------------------------------------------------------------------

class InfoIconCtrl : public ImageCtrl {
public:
	InfoIconCtrl();
	void SetInfoText(const String& text);
	
protected:
	String info_text;
};

//----------------------------------------------------------------------------

String EasyTime(const Time& time, const Time& now = GetSysTime());

struct TimeDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
		               Color ink, Color paper, dword style) const;
};

//----------------------------------------------------------------------------

struct ToolerConvert : public Convert {
	typedef Function<Value(const Value&)> Func;
	Func func;
	// --
	ToolerConvert() {}
	ToolerConvert(Func fn) : func(fn) {}
	ToolerConvert& operator=(Func fn) { func = fn; return *this; }
	virtual Value  Format(const Value& q) const { return func(q); }
};

//----------------------------------------------------------------------------

struct SendButton : public Button {
public:
	void RightDown(Point p, dword keyflags) override;
	Event<> WhenRightDown;
};

//----------------------------------------------------------------------------

#define LAYOUTFILE <UppOracle/Layouts.lay>
#include <CtrlCore/lay.h>

#include "Preferences.h"

//----------------------------------------------------------------------------

struct Conversation : Moveable<Conversation> {
	struct Entry : Moveable<Entry> {
		Time request_time = Null;
		Time first_reply = Null;
		Time done_time = Null;
		String context;
		String request;
		String reply;
		double temperature = Null;
		String model;
		
		// not saved
		int pos_begin = -1; // begin/end pos in render
		int pos_end = -1;
		
		void Serialize(Stream& s);
	};
	
	// internal not saved
	int id;
	int context_marker = -1;
	bool modified = false;
	int mouse_entry = -1;
	
	// saved
	Time time_started = Null;
	Vector<Entry> entries;
	ValueMap token_usage;
	bool favorite = false;
	Time last_modified = Null;
	
	// methods
	
	void Serialize(Stream& s);
	
	void AddUsage(const String& model, int num_tokens);
	double TotalCost() const;
	String GetFileName() const;
	
	void Save();
	bool Load(const String& bin);
	
	const String& GetPrompt() const;
};

//----------------------------------------------------------------------------

enum {
	COMPLETION,
	CHAT_COMPLETION
};

struct ModelDef : Moveable<ModelDef> {
	int type;
	int cost; // real cost = USD(cost / 10000) * (N_tokens / 1000)
	int token_limit;
	
	ModelDef() = default;
	ModelDef(int t, int c, int m) : type(t), cost(c), token_limit(m) {}
};

//----------------------------------------------------------------------------

const String CONFIGFILE = "UppOracle.cfg";

enum {
	TIME_CALLBACK_ID_INIT,
	TIME_CALLBACK_ID_TIMER_FUNC
};

enum {
	ENTER_SENDS,
	CTRL_ENTER_SENDS,
};

enum {
    TIME_FORMAT_24_HR,
    TIME_FORMAT_12_HR
};

enum {
	COLUMN_STAR,
	COLUMN_TIME,
	COLUMN_PROMPT
};

const Id ID_ID("ID");
const Id ID_DATE("DATE");
const Id ID_PROMPT("PROMPT");
const Id ID_NAME("NAME");
const Id ID_TOTAL("TOTAL");
const Id ID_TODAY("TODAY");
const Id ID_COST_1KT("COST_1KT");
const Id ID_STAR("STAR");

// cost in cent per thousand tokens
const int GPT35TURBO_COST = 20;
const int DAVINCI_COST = 200;
const int CURIE_COST = 20;
const int BABBAGE_COST = 5;
const int ADA_COST = 4;
const double DOLLARCOST_PER_1KT = 0.0001;

const int GPT35TURBO_LIMIT = 3584;
const int DAVINCI_LIMIT = 3584;
const int DAVINCI_LIMIT2 = 1792;
const int CURIE_LIMIT = 896;
const int BABBAGE_LIMIT = 448;
const int ADA_LIMIT = 448;

//----------------------------------------------------------------------------

#include "appconfig.h"

struct SAppVersion {
    int major, minor, patch, build;
    
    void Serialize(Stream& s) {
        s % major % minor % patch % build;
    }
    
    bool operator==(const SAppVersion& other) const {
        return major == other.major
            && minor == other.minor
            && patch == other.patch
            && build == other.build;
    }
};
const SAppVersion AppVersion = { APP_VERSION_ARRAY };

//----------------------------------------------------------------------------


class UppOracle : public TopWindow {
public:
	typedef UppOracle CLASSNAME;
	
	UppOracle();
	virtual ~UppOracle();
	
	void StatusMessage(String message);
	bool IsActivated() const { return activated; }

	String GetLicensee();
	String GetLicenseKey();
	String GetLicenseToken();

protected:
	struct Settings : Moveable<Settings> {
		String api_key;
		Value models;
		int64 total_tokens = 0;
		int context_factor = 0;
		int token_limit = 50;
		int temperature = 90;
		bool stream;
		String model;
		
		Rect screen_rect;
		bool maximized;
		int horz_split;
		int vert_split;
		String array_cols;
		
		String font_family = "Arial";
		int zoom = 10;
		
		String sans_serif_font = "Arial";
		String serif_font = "Times New Roman";
		String monospace_font = "Consolas";
		
		bool enter_sends = true;
		
		String default_prompt;
		
		String activation_token;
		String activation_signature;
		
		bool eula_ok = false;
		
		int time_format = TIME_FORMAT_24_HR;
		
		void Serialize(Stream& s);
	};
	Settings settings;
	
	struct ModelUsage : Moveable<ModelUsage> {
		int64 total_tokens = 0;
		int64 daily_tokens = 0;
		int64 token_1k_price_usd_cent = 0;
		Date date;
		ModelUsage() : date(GetSysDate()) {}
		void Serialize(Stream& s);
	};
	
protected:
	void Close() override;
	void Paint(Draw& w) override;
	void Layout() override;
	bool Key(dword key, int count) override;
	
protected:
    bool LoadConfig();
    bool SaveConfig();
    
	bool RequestAPIKey();
	
	void Init();
	void MenuFunc(Bar& bar);
	void ModelMenu(Bar& bar);
	void HelpMenu(Bar& bar);
	void ToolBarFunc(Bar& bar);
	void UpdateToolBar();
	void UpdateStatus(int num_tokens);
	
	void ArrayBarFunc(Bar& bar);
	void DeleteConversation(int id);
	
	void SetFontFamily(const char* family);
	void UpdateOutput(bool scrollToEnd = true);
	
	void Send();
	void BuildContext();
	int  CountTokens(const String& s);
	
	void ThreadMain();
	void TimerFunc();
	void HandleResponse(Value input, Value response);
	void PostStream(int id, const String& s);
	void EndStream(int id);
	void AddGlobalUsage(const String& model, int num_tokens);
	void AddUsage(int id, int numtokens);
	
	void SetModel(const char* m);
	void NewConversation();
	
	void ContextFactorAction();
	void ArrayCursorAction();
	void ArrayCursorAction0();
	void SendAction();
	void FixupArraySel();
	void FavoriteAction();
	
	void ShowUsageDialog();
	void ShowPreferences();
	void ShowAboutDialog();
	void ShowSearch();
	void ShowEulaDialog();

	void LoadConversations();
	void SaveModifiedConversations();
	
	void ReorderConversations(int highlight);
	void RefreshConversationList(int select_id);
	void ToggleStar();
	void ShowSearchResults();
	
	void RichMouse(int pos);
	void RichMenu(Bar& bar);
	
private:
	MenuBar menubar;
	ToolBar toolbar;
	
	StatusBar statusbar;
	MouseEventInfoCtrl statusmodel;
	InfoCtrl statususage;
	InfoCtrl statusCurrentConvUsage;
	
	TrayIcon tray;
	
	Splitter horzsplit;
	Splitter vertsplit;
	
	DropList notebookDrop;
	ArrayCtrl array;
	
	RichOutputView richtext;
	int richMousePos = -1;
	
	WithInputLayout<StaticRect> inputbar;
	
	TimeDisplay timeDisp;
	ToolerConvert timeConvert;
	ToolerConvert promptConvert;
	ToolerConvert starConvert;
	
	String font_family;
	Font chatFont;
	const int zoomBase = 2;
	int zoom = 10;
	
	OpenAI openai;
	Thread requestThread;
	Semaphore requestSem;
	BiVector<Value> queue;
	BiVector<Value> results;
	Semaphore resultsSem;
	Time lastTime;
	
	String selectedmodel;
	VectorMap<String, ModelUsage> usageMap;
	Atomic token_counter;
	
	Atomic nextConvId;
	VectorMap<int, Conversation> conversations;
	Conversation* currentConv = nullptr;
	Conversation::Entry* currentEntry = nullptr;
	
	bool showOnlyStarred = false;
	Index<int> starred;
	
	String searchTerm;
	Vector<int> searchResults;
	bool showTimeStamps = true;
	
	// used to collect context while rendering the output
	String currentQtf;
	String current_context;
	int current_context_tokest = 0;
	
	bool activated = false;

public:
	static const VectorMap<String, ModelDef> ModelCostTable;
};

UppOracle& App();

#endif
