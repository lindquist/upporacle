#include "App.h"

#define IMAGECLASS ImagesImg
#define IMAGEFILE <UppOracle/Images.iml>
#include <Draw/iml_source.h>

#include <OpenIconic/OpenIconic.h>

//----------------------------------------------------------------------------

const VectorMap<String, ModelDef> UppOracle::ModelCostTable({
//  { "davinci",                ModelDef({COMPLETION, DAVINCI_COST, DAVINCI_LIMIT2  }) },
//  { "curie",                  ModelDef({COMPLETION, CURIE_COST,   CURIE_LIMIT     }) },
//  { "babbage",                ModelDef({COMPLETION, BABBAGE_COST, BABBAGE_LIMIT   }) },
//  { "ada",                    ModelDef({COMPLETION, ADA_COST,     ADA_LIMIT       }) },
//	{ "gpt-3.5-turbo",			ModelDef({CHAT_COMPLETION, GPT35TURBO_COST, GPT35TURBO_LIMIT}) },
    { "text-davinci-003",       ModelDef({COMPLETION, DAVINCI_COST, DAVINCI_LIMIT   }) },
    { "text-davinci-002",       ModelDef({COMPLETION, DAVINCI_COST, DAVINCI_LIMIT2  }) },
    { "text-davinci-001",       ModelDef({COMPLETION, DAVINCI_COST, DAVINCI_LIMIT2  }) },
    { "davinci-instruct-beta",  ModelDef({COMPLETION, DAVINCI_COST, DAVINCI_LIMIT2  }) },
    { "code-davinci-002",       ModelDef({COMPLETION, DAVINCI_COST, DAVINCI_LIMIT2  }) },
    { "code-cushman-001",       ModelDef({COMPLETION, DAVINCI_COST, DAVINCI_LIMIT2  }) },
    { "text-curie-001",         ModelDef({COMPLETION, CURIE_COST,   CURIE_LIMIT     }) },
    { "curie-instruct-beta",    ModelDef({COMPLETION, CURIE_COST,   CURIE_LIMIT     }) },
    { "text-babbage-001",       ModelDef({COMPLETION, BABBAGE_COST, BABBAGE_LIMIT   }) },
    { "text-ada-001",           ModelDef({COMPLETION, ADA_COST,     ADA_LIMIT       }) },
});

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

InputDocEdit::InputDocEdit()
{
    send_hotkey = 0;
}

bool InputDocEdit::Key(dword key, int count)
{
    if (send_hotkey == K_RETURN) {
        if (key == K_RETURN) {
            WhenSend();
            return true;
        }
        else if (key == K_CTRL_RETURN) {
            return DocEdit::Key(K_RETURN, count); // inject ENTER on Ctrl+ENTER
        }
    }
    else if (send_hotkey == K_CTRL_RETURN) {
        if (key == K_CTRL_RETURN) {
            WhenSend();
            return true;
        }
        // no need to inject anything
    }
    else if (send_hotkey && send_hotkey == key) {
        WhenSend();
        return true;
    }
    // fall through
    return DocEdit::Key(key, count);
}

void InputDocEdit::SetSendHotkey(dword key)
{
    send_hotkey = key;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void RichOutputView::RightDown(Point p, dword keyflags)
{
    if (WhenBar) {
        MenuBar::Execute(WhenBar);
    }
    else {
        RichTextView::RightDown(p, keyflags);
    }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void Conversation::AddUsage(const String& model, int num_tokens)
{
    int i = token_usage.Find(model);
    int64 count = num_tokens;
    if (i >= 0) {
        count += int64(token_usage[i]);
        token_usage.SetAt(i, count);
    }
    else {
        token_usage.Set(model, count);
    }
}

double Conversation::TotalCost() const
{
    int N = token_usage.GetCount();
    int64 centils = 0;
    for (int i = 0; i < N; ++i) {
        auto& key = token_usage.GetKey(i);
        auto& val = token_usage.GetValue(i);
        int j = UppOracle::ModelCostTable.Find(key);
        if (j >= 0) {
            centils += UppOracle::ModelCostTable[i].cost * int64(val);
        }
    }
    return (centils * 0.001) * DOLLARCOST_PER_1KT;
}

String Conversation::GetFileName() const
{
    int64 ts = time_started.Get();
    String s;s << FormatInt64(ts) << "_conversation.bin";
    auto path = AppendFileName("conversations", s);
    return ConfigFile(path);
}

void Conversation::Save()
{
    if (IsNull(time_started))
        return;
    auto path = GetFileName();
    RealizePath(path);
    StoreToFile(*this, path);
    modified = false;
}

bool Conversation::Load(const String& bin)
{
    if (LoadFromString(*this, bin))
        modified = false;
    return !modified;
}

const String& Conversation::GetPrompt() const
{
    static const String bad = String::GetVoid();
    if (entries.IsEmpty())
        return bad;
    return entries[0].request;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

InfoIconCtrl::InfoIconCtrl()
{
    SetImage(OpenIconic::info_2x());
}

void InfoIconCtrl::SetInfoText(const String& text)
{
    info_text = text;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

String EasyTime(const Time& time, const Time& now)
{
    if (time > now)
        return "future";
    
    Date date = time;
    Date today = now;
    
    int _;
    int dateWeek = GetWeek(date, _);
    int todayWeek = GetWeek(today, _);
    int Ddate = date.Get();
    int Dtoday = today.Get();
    int64 Ttime = time.Get();
    int64 Tnow = now.Get();
    
    auto timeMin = Ttime / 60;
    auto nowMin = Tnow / 60;
    auto dMin = nowMin - timeMin;
    
    String txt;
    
    if (date == today) {
        if (time.hour == now.hour) {
            if (time.minute == now.minute)
                txt = "now";
            else
                txt << (now.minute - time.minute) << "min";
        }
        else if (dMin < 4*60) {
            int hrs = dMin/60;
            txt << hrs << (hrs>1 ? "hrs " : "hr ") << (dMin % 60) << "min";
        }
        else if (dMin < 12*60) {
            txt << dMin/60 << " hrs";
        }
        else
            txt = "today";
    }
    else if (date.year == today.year) {
        if (Ddate + 1 == Dtoday)
            txt = "1 day";
        else if (dateWeek == todayWeek)
            txt << (Dtoday - Ddate) << " days";
        else if (dateWeek + 1 == todayWeek)
            txt = "1 week";
        else
            txt << (todayWeek - dateWeek) << " weeks";
    }
    else {
        txt = Format("%Mon %d", date.month, date.year);
    }
    
    return txt;
}

String EasyDetailedTime(const Time& time)
{
    String s;
    s << FormatTime(time, "hh:mm") << Format(" %d-%mon-%d", time.day, time.month, time.year);
    return pick(s);
}

//----------------------------------------------------------------------------

void TimeDisplay::Paint(Draw& w, const Rect& r, const Value& q, Color ink,
                        Color paper, dword style) const
{
    w.DrawRect(r, paper);
    if (!IsDateTime(q))
        return;
    Font fnt = StdFont();
    Time time = q;
    String txt;
    if (style & Display::CURSOR)
        txt = EasyDetailedTime(time);
    else
        txt = EasyTime(time);
    w.DrawText(r.left + 2, r.top + (r.Height() - GetTextSize(txt, fnt).cy) / 2, txt, fnt, ink);
}

struct StarDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
		               Color ink, Color paper, dword style) const
    {
        w.DrawRect(r, paper);
        String txt = q;
        bool starred = (q == "★");
        
        Color color = starred ? ink : SLtGray();
        
        Font fnt = StdFont();
        Size textSize = GetTextSize(txt, fnt);
        w.DrawText(r.left + 2, r.top + (r.Height() - textSize.cy) / 2, txt, fnt, color);
    }
};

static StarDisplay TheStarDisplay;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static int CountRichChars(const String& str) {
    WString s = ToUtf32(str);
    // fixme use some appropriate filter function
    return s.GetCount() - Count(s, '\r');
#if 0
    int N = s.GetCount();
    if (N == 0)
        return 0;
    int n = N;
    int i = 0;
    auto p = s.begin();
    auto e = s.end();
    bool in = false;
    while (p != e) {
        if (*p == '\n') {
            if (in)
                --n;
            else
                in = true;
        }
        else if (*p == '\r') {
            // ignore
        }
        else {
            in = false;
        }
        ++p;
    }
    return n;
#endif
}

struct QtfBuilder {
    int pos;
    String qtf;
    bool first_cell;
    
    QtfBuilder() {
        pos = 0;
    }
    
    void Begin(const char* fmt) {
        qtf << "[" << fmt << " ";
    }
    void End() {
        qtf << "]";
    }
    void Add(const char* fmt, const String& deqtf) {
        qtf << "[" << fmt << " " << DeQtf(deqtf) << "]";
        pos += CountRichChars(deqtf);
    }
    void Add(const String& deqtf) {
        qtf << DeQtf(deqtf);
        pos += CountRichChars(deqtf);
    }
    
    String FontFamily(const String& fontFam) const {
        String s;
        s << "!" << fontFam << "!";
        return pick(s);
    }
    
    void BeginTable(const char* cols, const char* fmt = "") {
        first_cell = true;
        qtf << "{{" << cols << fmt;
    }
    void TableCell(const char* fmt = "") {
        if (first_cell) {
            first_cell = false;
            qtf << fmt << " ";
        }
        else
            qtf << "::" << fmt << " ";
        ++pos;
    }
    void EndTable() {
        qtf << "}}";
    }
    
    operator const char* () const { return ~qtf; }
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

UppOracle::UppOracle()
{
    Title("U++ Oracle");
    Icon(ImagesImg::icon());
    
    Sizeable().Zoomable();
    SetMinSize(Size(640,480));
    
    AddFrame(menubar);
    AddFrame(toolbar);
    AddFrame(statusbar);
    
    //statusbar.AddFrame(statusmessage.Left(600));
    statusbar.AddFrame(statusCurrentConvUsage.Right(200));
    statusbar.AddFrame(statususage.Right(200));
    statusbar.AddFrame(statusmodel.Right(200));
    statusmodel.WhenLeftDown = [this](Point,dword){
        MenuBar::Execute(THISFN(ModelMenu));
    };
    
    CtrlLayout(inputbar);
    auto size = inputbar.GetLayoutSize();
    
    vertsplit.Vert();
    vertsplit.Add(richtext.SizePos());
    vertsplit.Add(inputbar.SizePos());
    
    inputbar.SetMinSize(size);
        
    horzsplit.Horz();
    horzsplit.Add(array.SizePos());
    horzsplit.Add(vertsplit.SizePos());
    
    // finally add the primary (horz) splitter
    Add(horzsplit);
    
    menubar.Set(THISFN(MenuFunc));
    
    chatFont = StdFont(20);
    
    inputbar.sendbutton << THISFN(SendAction);
    inputbar.sendbutton.SetImage(OpenIconic::chat_4x());
    
    inputbar.inputedit.WhenSend = THISFN(SendAction);
    
    // Load configuration file
    if (LoadConfig()) {
        if (settings.maximized)
            Maximize(true);
        else
            SetRect(settings.screen_rect);
        horzsplit.SetPos(settings.horz_split);
        vertsplit.SetPos(settings.vert_split);
        zoom = settings.zoom;
        font_family = settings.font_family;
    }
    else {
        settings.maximized = IsMaximized();
        settings.screen_rect = GetRect();
        settings.screen_rect.SetSize(800,600);
        SetRect(settings.screen_rect);
        horzsplit.SetPos(2500);
        vertsplit.SetPos(6600);
        settings.array_cols = "22 68 79";
    }
    
    richtext.SetZoom(Zoom(zoomBase,zoom));
    richtext.WhenMouseMove = THISFN(RichMouse);
    richtext.WhenBar = THISFN(RichMenu);
    
    inputbar.temperature.MinMax(0, 100).Step(10, false).SetData(settings.temperature);
    inputbar.token_limit.MinMax(0,100).Step(10, false).SetData(settings.token_limit);
    inputbar.context.MinMax(0,10).Step(1, false).SetData(settings.context_factor);
    inputbar.context.WhenAction = THISFN(ContextFactorAction);
    inputbar.stream.Set(1);
    
    inputbar.context_info.Tip("More context will help the AI understand indirect questions, but will use more tokens");
    inputbar.token_limit_info.Tip("Limit the maximum amount of tokens used per request as a percentage of the model maximum");
    inputbar.temperature_info.Tip("High temperature will let the AI take more risks, low temperature will result in more conservative answers");
    
    inputbar.inputedit.SetSendHotkey(settings.enter_sends ? K_ENTER : K_CTRL_ENTER);
    inputbar.inputedit.SetData(settings.default_prompt);
    
    lastTime = GetSysTime();
    if (settings.model.IsEmpty())
        settings.model = "text-davinci-003";
    SetModel(settings.model);
    
    if (font_family.IsEmpty())
        font_family = "Arial";
    chatFont.FaceName(font_family);
    inputbar.inputedit.SetFont(chatFont);
    
    // set up the notebook drop
    int notebookId = 1;
    notebookDrop.Add(notebookId, "My Notebook");
    notebookDrop.SetIndex(0);
    
    // now the fonts are known update the toolbar
    UpdateToolBar();
    
    timeConvert = [this](const Value& q) -> Value {
        int index = q;
        int id = array.Get(index, ID_ID);
        return conversations.Get(id).last_modified;
    };
    promptConvert = [this](const Value& q) -> Value {
        int index = q;
        int id = array.Get(index, ID_ID);
        auto p = conversations.Get(id).GetPrompt();
        String s = p.Left(512);
        if (p.GetCount() > 512)
            s << "...";
        return Nvl(s, "nothing yet");
    };
    starConvert = [this](const Value& q) -> Value {
        int index = q;
        int id = array.Get(index, ID_ID);
        auto fav = conversations.Get(id).favorite;
        return fav ? "★" : "☆";
    };
    
    array.AddIndex(ID_ID);
    array.AddRowNumColumn("★").SetConvert(starConvert).SetDisplay(TheStarDisplay); // COLUMN_STAR
    array.AddRowNumColumn("Last active").Sorting().SetConvert(timeConvert).SetDisplay(timeDisp); // COLUMN_TIME
    // fixme .SortDefault() does not work on virtual column - is it a bug?
    array.AddRowNumColumn("Prompt").Sorting().SetConvert(promptConvert); // COLUMN_PROMPT
    array.WhenCursor = THISFN(ArrayCursorAction);
    array.WhenLeftDouble = THISFN(FavoriteAction);
    array.WhenBar = THISFN(ArrayBarFunc);
    array.SetSortColumn(COLUMN_TIME, true);
    
    /*
    array.AddColumn(ID_STAR, "Star");
    array.AddColumn(ID_DATE, "Last active").Sorting().SortDefault(true).SetDisplay(timeDisp);
    array.AddColumn(ID_PROMPT, "Prompt").Sorting();
    //array.WhenColumnSorted = THISFN(FixupArraySel);
    */
    
    
    // restore column widths
    if (!settings.array_cols.IsEmpty())
        array.ColumnWidths(~settings.array_cols);
    
    // load all conversations
    nextConvId = 0;
    LoadConversations();
    
    token_counter = 0;
    if (conversations.IsEmpty())
        NewConversation();
    else
        ArrayCursorAction();
    
    requestThread.Start(THISFN(ThreadMain));
    
    SetTimeCallback(0, THISFN(Init), TIME_CALLBACK_ID_INIT);
}

UppOracle::~UppOracle()
{
    KillTimeCallback(TIME_CALLBACK_ID_INIT);
    KillTimeCallback(TIME_CALLBACK_ID_TIMER_FUNC);
}

void UppOracle::Close()
{
    SaveConfig();
    
    SaveModifiedConversations();
    
    TopWindow::Close();
}

bool UppOracle::LoadConfig()
{
    String filename = ConfigFile(CONFIGFILE);
    RLOG("Loading config: " << filename);
    
    FileIn f(filename);
    if (!f.IsOK()) {
        RLOG("Config not found.");
        return false;
    }
    Load(settings, f);
    Load(usageMap, f);
    return f.IsOK();
}

bool UppOracle::SaveConfig()
{
    String filename = ConfigFile(CONFIGFILE);
    RLOG("Saving config: " << filename);
    
    settings.temperature = ~inputbar.temperature;
    settings.token_limit = ~inputbar.token_limit;
    settings.context_factor = ~inputbar.context;
    settings.model = selectedmodel;
    
    settings.maximized = IsMaximized();
    if (!settings.maximized)
        settings.screen_rect = GetRect();
    
    settings.horz_split = horzsplit.GetPos();
    settings.vert_split = vertsplit.GetPos();
    
    settings.array_cols = array.GetColumnWidths();
    
    settings.font_family = font_family;
    settings.zoom = zoom;
    
    FileOut f(filename);
    if (!f.IsOK()) {
        RLOG("Failed to save config.");
        return false;
    }
    Store(settings, f) && Store(usageMap, f);
    return f.IsError();
}

bool UppOracle::RequestAPIKey()
{
    while (!EditText(settings.api_key, "OpenAI API Key required", "OpenAI API key")) {
        if (ErrorAbortRetry(DeQtf("This application cannot function without a valid OpenAI API key.")))
            return false;
    }
    return true;
}

void UppOracle::Init()
{
#if 0
    if (!settings.eula_ok) {
        ShowEulaDialog();
        if (!settings.eula_ok)
            Exit();
    }
#endif
    
    if (settings.api_key.IsEmpty()) {
        // api key check
        for (;;) {
            if (!RequestAPIKey())
                Exit();
            
            Progress prog("Checking OpenAI API key");
            prog.Create();
            
            openai.SetSecret(~settings.api_key);
            String message;
            if (!openai.CheckSecret(message)) {
                ErrorOK(DeQtf("Error: " + message));
                settings.api_key.Clear();
                if (ErrorAbortRetry(DeQtf("The API key you provided is not working. Please enter a valid OpenAI API key.")))
                    Exit();
                continue;
            }
            
            settings.models = openai.GetModels();
            break;
        }
    }
    else {
        openai.SetSecret(~settings.api_key);
    }
    
    SetTimeCallback(-80, THISFN(TimerFunc), TIME_CALLBACK_ID_TIMER_FUNC);
}

void UppOracle::MenuFunc(Bar& bar)
{
    bar.Sub("Home", [&](Bar& bar){
        bar.Add("New session", OpenIconic::chat_2x(), THISFN(NewConversation));
        bar.Separator();
        bar.Add("Show usage report", OpenIconic::graph_2x(), THISFN(ShowUsageDialog));
        bar.Separator();
        bar.Add("Exit", OpenIconic::account_logout_2x(), THISFN(Close));
    });
    
    bar.Sub("Model", THISFN(ModelMenu));
    
    bar.Sub("Settings", [this](Bar& bar) {
        bar.Add("Preferences", OpenIconic::wrench_2x(), THISFN(ShowPreferences));
        bar.Add("Set API key", OpenIconic::key_2x(), [this]{ RequestAPIKey(); });
        //bar.Add("Activate", OpenIconic::circle_check_2x(), THISFN(ShowActivationDialog));
    });
    
#if 0
    bar.Sub("Theme", [this](Bar& bar) {
        //DarkTheme()
        bar.Add("Host theme",     OpenIconic::image_2x(), [this] {
            //SetDarkThemeEnabled(true);
            SetSkin(ChHostSkin);
        });
        bar.Add("Standard theme", OpenIconic::image_2x(), [this] {
            //SetDarkThemeEnabled(false);
            SetSkin(ChStdSkin);
        });
        bar.Add("Classic theme",  OpenIconic::image_2x(), [this] {
            //SetDarkThemeEnabled(false);
            SetSkin(ChClassicSkin);
        });
    });
#endif
    
    bar.Sub("Help", THISFN(HelpMenu));
}

void UppOracle::ModelMenu(Bar& bar)
{
    for (auto& modelname : ModelCostTable.GetKeys()) {
        bar.Add(modelname, [this,modelname]{
            SetModel(modelname);
        }).Radio(modelname == selectedmodel);
    }
}

void UppOracle::HelpMenu(Bar& bar)
{
#if 0
    bar.Add("Review EULA", OpenIconic::document_2x(), [this]{
        ShowEulaDialog();
        if (!settings.eula_ok)
            Close();
    });
#endif

    bar.Add("About ...", ImagesImg::icon_2x(), THISFN(ShowAboutDialog));
}

void UppOracle::ToolBarFunc(Bar& bar)
{
    bar.Add("New session", OpenIconic_3x::chat(), THISFN(NewConversation))
        .Key(K_CTRL_N);
    bar.Add("Show usage report", OpenIconic_3x::graph(), THISFN(ShowUsageDialog));
    bar.Add("Preferences", OpenIconic_3x::wrench(), THISFN(ShowPreferences));
    
    bar.Separator();
    
    bar.Add("Sans Serif font", ImagesImg::arial_3x(), [this]{
        SetFontFamily(settings.sans_serif_font);
        UpdateToolBar();
    }).Radio(font_family == settings.sans_serif_font);
    
    bar.Add("Serif font", ImagesImg::times_3x(), [this]{
        SetFontFamily(settings.serif_font);
        UpdateToolBar();
    }).Radio(font_family == settings.serif_font);
    
    bar.Add("Monospace font", ImagesImg::monospace_3x(), [this]{
        SetFontFamily(settings.monospace_font);
        UpdateToolBar();
    }).Radio(font_family == settings.monospace_font);
    
    bar.Separator();
    
    bar.Add("Show only favorites", OpenIconic_3x::star(), [this]{
        ToggleStar();
        UpdateToolBar();
    })
    .Check(showOnlyStarred)
    .Key(K_CTRL_R);
    
    bar.Add("Show timestamps", OpenIconic_3x::clock(), [this]{
        showTimeStamps = !showTimeStamps;
        UpdateOutput();
        UpdateToolBar();
    })
    .Check(showTimeStamps)
    .Key(K_CTRL_T);
    
    bar.Separator();
    
    bar.Add("Larger text", OpenIconic_3x::zoom_in(), [this]{
        if (zoom > 3) {
            richtext.SetZoom(Zoom(zoomBase,--zoom));
            richtext.ScrollEnd();
        }
    })
    .Key(K_CTRL_UP);
    
    bar.Add("Smaller text", OpenIconic_3x::zoom_out(), [this]{
        if (zoom < 24) {
            richtext.SetZoom(Zoom(zoomBase,++zoom));
            richtext.ScrollEnd();
        }
    })
    .Key(K_CTRL_DOWN);
    
    bar.Separator();
    
    bar.Add("About", OpenIconic_3x::info(), THISFN(ShowAboutDialog));
    
    bar.Separator();
    
    bar.Add("Search", OpenIconic_3x::magnifying_glass(), THISFN(ShowSearch))
        .Key(K_CTRL_F);
        
    bar.Separator();
    
    //bar.Add(notebookDrop, Size(300, StdFont().GetLineHeight()));
}

void UppOracle::UpdateToolBar()
{
    toolbar.Set(THISFN(ToolBarFunc));
}

void UppOracle::ArrayBarFunc(Bar& bar)
{
    int i = array.GetCursor();
    if (i < 0)
        return;
    int id = array.Get(i, ID_ID);
    bar.Add("Delete conversation", OpenIconic::trash_2x(), [this,id]{ DeleteConversation(id); });
}

void UppOracle::DeleteConversation(int id)
{
    int i = conversations.Find(id);
    if (i < 0)
        return;
    auto& conv = conversations[i];
    String fn = conv.GetFileName();
    String qtf;
    qtf << "Are you sure you want to delete the conversation below?`:";
    qtf << "&&[$1/ " << DeQtf(conv.GetPrompt()) << " ...&...&...]";
    qtf << "&&Filename: " << DeQtf(fn);
    qtf << "&&[* This operation cannot be undone!]";
    
    if (PromptYesNo(qtf)) {
        FileDelete(fn);
        conversations.Unlink(i);
        int j = array.Find(id, ID_ID);
        if (j >= 0)
            array.Remove(j);
    }
}

void UppOracle::SetFontFamily(const char *family)
{
    font_family = family;
    chatFont.FaceName(font_family);
    inputbar.inputedit.SetFont(chatFont);
    UpdateOutput();
}

void UppOracle::UpdateStatus(int num)
{
    if (num > 0) {
        String status;
        status << "Num. tokens: " << num;
        statususage.Set(status);
    }
    
    String s;
    s << "Cost ≅ $" << currentConv->TotalCost();
    statusCurrentConvUsage.Set(s);
}

void UppOracle::UpdateOutput(bool scrollToEnd)
{
    if (!currentConv)
        return;
    
    QtfBuilder qtf;
    qtf.Begin(qtf.FontFamily(font_family));
    
    qtf.BeginTable("1", "~");
    
    // adds text to the output, highlights search term if any
    auto add_text = [this,&qtf](const String& s) {
        int i = 0;
        int len = searchTerm.GetCount();
        if (len == 0) {
            qtf.Add(s);
        }
        else while (i < s.GetCount()) {
            int j = s.Find(searchTerm, i);
            if (j >= 0) {
                if (j > i) {
                    qtf.Add(s.Mid(i, j-i));
                }
                
                qtf.Add("$9", s.Mid(j, len));
                
                i = j + len;
            }
            else {
                qtf.Add(s.Mid(i));
                break;
            }
        }
    };
    
    int N = currentConv->entries.GetCount();
    if (N == 0) {
        qtf.TableCell();
        qtf.Begin("@1");
        add_text("empty");
        qtf.End();
    }
    else for (int i = 0; i < N; ++i) {
        auto& entry = currentConv->entries[i];
        
        entry.pos_begin = qtf.pos;
        
        bool mouseEntry = i == currentConv->mouse_entry;
        qtf.TableCell(mouseEntry ? "@7" : "!");
        
        if (i == currentConv->context_marker) {
            if (showTimeStamps) {
                qtf.Add("\n\n");
                qtf.Begin("H1");
                qtf.Add("1@1", "\n>> Context marker");
                qtf.End(); // H1
                qtf.Add("\n");
            }
            else {
                qtf.Add("\n");
                qtf.Begin("H1");
                qtf.Add("1@1", "\n ");
                qtf.End();
            }
        }
        
        //qtf.Add("\n\n");
        
        qtf.Begin("@K");
        add_text(entry.request);
        qtf.End();
        
        if (entry.reply.IsEmpty()) {
            if (showTimeStamps)
                qtf.Add("1@1/", "\nprocessing ...");
        }
        else {
            //qtf.Add("\n");
            qtf.Begin("@0");
            add_text(entry.reply);
            qtf.End();
        }

        if (showTimeStamps && !IsNull(entry.done_time)) {
            String s;
            s << "<< done " << entry.done_time;
            qtf.Add("\n");
            qtf.Add("1@1", s);
        }
        
        entry.pos_end = qtf.pos;
    }
    
    qtf.EndTable();
    
    qtf.End(); // font
    
    int scrollPos = richtext.GetSb();
    richtext.SetQTF(qtf);
    richtext.GetLength();
    
    if (scrollToEnd)
        richtext.ScrollEnd();
    else {
        richtext.SetSb(scrollPos);
    }
}

void UppOracle::Paint(Draw& w)
{
    TopWindow::Paint(w);
}

void UppOracle::Layout()
{
    TopWindow::Layout();
}

bool UppOracle::Key(dword key, int count)
{
    if (!searchTerm.IsEmpty() && key == K_ESCAPE) {
        searchTerm.Clear();
        searchResults.Clear();
        int id = -1;
        int i = array.GetCursor();
        if (i >= 0)
            id = array.Get(i, ID_ID);
        RefreshConversationList(id);
        return true;
    }
    
    return TopWindow::Key(key, count);
}

void UppOracle::Send()
{
    ASSERT(currentConv);
    
    int i = ModelCostTable.Find(selectedmodel);
    if (i < 0) {
        String s;
        s << "invalid model requested: " << selectedmodel;
        ErrorOK(DeQtf(s));
        return;
    }
    
    ModelDef model = ModelCostTable[i];
    
    inputbar.sendbutton.Disable();
    bool useStream = inputbar.stream.Get() != 0;
    
    currentConv->modified = true;
    currentConv->last_modified = GetSysTime();
    
    String prompt = ~inputbar.inputedit;
    // fixme make into method - also used in BuildContext
    int maxlimit = (int(~inputbar.token_limit) * model.token_limit) / inputbar.token_limit.GetMax();
    
    double temperature = int(~inputbar.temperature) / 100.0;
    
    Conversation::Entry entry;
    entry.request_time = GetSysTime();
    entry.first_reply = Null;
    entry.done_time = Null;
    entry.context = current_context;
    entry.request = prompt;
    entry.reply = Null;
    entry.temperature = temperature;
    entry.model = selectedmodel;
    currentEntry = &currentConv->entries.Add(pick(entry)); // picks entry
    
    int promptTokens = CountTokens(prompt);
    int estUsage = current_context_tokest + promptTokens;
    DUMP(estUsage);
    int limit = maxlimit - estUsage; // the request limit should exclude the prompt - not easy
    ASSERT(limit > 0);
    
    ValueMap m;
    m("model", selectedmodel);
    
    if (current_context.IsEmpty())
        m("prompt", prompt);
    else {
        String send_string;
        send_string << current_context << prompt; // newlines included in context
        m("prompt", send_string);
    }

    m("temperature", temperature)
        ("stream", useStream)
        ("echo", false)
        ("max_tokens", limit);
        
    ValueMap wrap;
    wrap("id", currentConv->id)
        ("input", pick(m));

    queue.AddTail(pick(wrap));
    requestSem.Release();
    
    ReorderConversations(currentConv->id);
    UpdateOutput();
}

int UppOracle::CountTokens(const String& s) {
    auto words = Split(~s, CharFilterWhitespace, true);
    int tokens = 0;
    for (auto& word : words) {
        tokens += (word.GetCharCount() * 10) / 25; //  n_tok = wordlen / 2.5 🤷️
    }
    return tokens;
}

void UppOracle::BuildContext()
{
    ASSERT(currentConv != nullptr);
    
    int i = ModelCostTable.Find(selectedmodel);
    if (i < 0)
        return;
    
    ModelDef model = ModelCostTable[i];
    // sliderPct * modelTokenLimit
    int limit = (int(~inputbar.token_limit) * model.token_limit) / inputbar.token_limit.GetMax();
    int maxNum = ~inputbar.context;
    
    int contextMaxTok = limit / 2; // max half for context
    int maxtok = (contextMaxTok * 3) / 4; // 25% safety margin
    
    auto& conv = *currentConv;
    conv.context_marker = conv.entries.GetCount();
    
    // for each entry in reverse
    BiVector<String> bits;
    int tokest = 0;
    
    for (auto& e : ReverseRange(conv.entries)) {
        if (maxNum-- <= 0)
            break;
        
        int ntok = CountTokens(e.reply);
        ntok += CountTokens(e.request);
        
        // if adding this request+reply exceeds the token limit, then abandon
        if (tokest + ntok > maxtok)
            break;
        
        tokest += ntok;
        bits.AddHead(e.reply);
        bits.AddHead(e.request);
        --conv.context_marker;
    }
    
    // build the context
    current_context.Clear();
    for (auto& bit : bits)
        current_context << bit << "\n\n";
    
    current_context_tokest = tokest;
}

void UppOracle::SetModel(const char *m)
{
    selectedmodel = m;
    statusmodel.Set(selectedmodel);
}

void UppOracle::NewConversation()
{
    if (currentConv && currentConv->modified) {
        currentConv->modified = false;
        // save the current conv
        auto path = currentConv->GetFileName();
        RealizePath(path);
        StoreToFile(*currentConv, path);
    }
    
    int id = AtomicInc(nextConvId);
    currentConv = &conversations.Add(id);
    currentConv->id = id;
    
    auto t = GetSysTime();
    currentConv->time_started = t;
    currentConv->last_modified = t;
    currentEntry = nullptr;
    
    array.Add(id);
    ReorderConversations(id);
    
    inputbar.inputedit.SelectAll();
    inputbar.inputedit.SetFocus();
}

void UppOracle::ContextFactorAction()
{
    BuildContext();
    UpdateOutput();
}

void UppOracle::ArrayCursorAction()
{
    if (array.GetCount() == 0)
        return;
    int i = min(0, array.GetCursor());
    ArrayCursorAction0();
}

void UppOracle::ArrayCursorAction0()
{
    int i = array.GetCursor();
    if (i < 0)
        return;
    
    // ..
    // now find the selection
    int j = conversations.Find(array.Get(i, ID_ID));
    if (j < 0)
        Panic("Bad conversion table");
    
    // and update state
    currentConv = &conversations[j];
    currentEntry = nullptr; // always start a new entry
    
    SaveModifiedConversations();
    BuildContext();
    UpdateOutput();
    UpdateStatus(0);
}

void UppOracle::SendAction()
{
    if (!inputbar.sendbutton.IsEnabled())
        return;

	int i = ModelCostTable.Find(selectedmodel);
    if (i < 0)
        return;
    
    ModelDef model = ModelCostTable[i];
    
    if (model.type == CHAT_COMPLETION) {
    }
	else if (model.type == COMPLETION) {
		Send();
	    inputbar.inputedit.SelectAll();
	    inputbar.inputedit.SetFocus();
	}
}

void UppOracle::FixupArraySel()
{
    if (!currentConv)
        return;
    int i = array.Find(currentConv->id, ID_ID);
    if (i >= 0) {
        array.SetCursor(i);
    }
}

void UppOracle::FavoriteAction()
{
    int i = array.GetCursor();
    if (i < 0)
        return;
    int j = conversations.Find(array.Get(i, ID_ID));
    if (j < 0)
        return;
    auto& conv = conversations[j];
    conv.favorite = !conv.favorite;
    if (conv.favorite) {
        starred.FindAdd(conv.id);
    }
    else {
        starred.UnlinkKey(conv.id);
    }
    conv.modified = true;
    array.RefreshRow(i);
}

void UppOracle::LoadConversations()
{
    conversations.Clear();
    array.Clear();
    
    int lastId = -1;
    
    auto path = ConfigFile(AppendFileName("conversations", "*_conversation.bin"));
    FindFile ff(path);
    while (ff) {
        if (ff.IsFile()) {
            Conversation conv;
            if (conv.Load(LoadFile(ff.GetPath()))) {
                // get a unique id
                conv.id = lastId = AtomicInc(nextConvId);
                
                // track starred
                if (conv.favorite)
                    starred.FindAdd(conv.id);
                
                // finally add it
                conversations.Add(lastId) = pick(conv);
                array.Add(lastId);
            }
        }
        ff.Next();
    }
    
    ReorderConversations(lastId);
}

void UppOracle::SaveModifiedConversations()
{
    for (auto& conv : conversations) {
        if (conv.modified)
            conv.Save();
    }
}

void UppOracle::ReorderConversations(int highlight)
{
    array.ClearSelection(false);
    array.DoColumnSort();
    
    if (highlight >= 0) {
        int i = array.Find(highlight, ID_ID);
        if (i >= 0)
            array.SetCursor(i);
    }
    
    array.Refresh();
}

void UppOracle::RefreshConversationList(int select_id)
{
    array.ClearSelection(false);
    array.Clear();
    
    if (showOnlyStarred) {
        int N = starred.GetCount();
        for (int i = 0; i < N; ++i) {
            if (starred.IsUnlinked(i))
                continue;
            array.Add(starred[i]);
        }
    }
    else {
        for (auto& conv : conversations) {
            array.Add(conv.id);
        }
    }
    
    ReorderConversations(select_id);
    array.Refresh();
}

void UppOracle::ToggleStar()
{
    int id = -1;
    int i = array.GetCursor();
    if (i >= 0)
        id = array.Get(i, ID_ID);
    
    showOnlyStarred = !showOnlyStarred;
    
    RefreshConversationList(id);
}

void UppOracle::ShowSearchResults()
{
    if (searchResults.IsEmpty())
        return;
    
    array.ClearSelection(false);
    array.Clear();
    
    for (int id : searchResults) {
        array.Add(id);
    }
    
    ReorderConversations(-1);
    if (array.GetCount() > 0)
        array.SetCursor(0);
    array.Refresh();
}

void UppOracle::StatusMessage(String message)
{
    statusbar.Temporary(message);
}

// String GetLicensee();
//  String GetLicenseKey();
//  String GetLicenseToken();

String UppOracle::GetLicensee()
{
    return "not saved";//settings.licensee;
}

String UppOracle::GetLicenseKey()
{
    return "not saved";//settings.license_key;
}

String UppOracle::GetLicenseToken()
{
    return settings.activation_token;
}

void UppOracle::RichMouse(int pos)
{
    if (!currentConv || currentConv->entries.IsEmpty())
        return;
    
    richMousePos = pos;
    
    int oldMouseEntry = currentConv->mouse_entry;

#if 0
    // breakpoint handle
    bool mouse = false;
    if (GetMouseFlags() & K_MOUSELEFT) {
        mouse = true;
    }
#endif
    
    if (pos < 0)
        return;
    
    // debug
    int entryLen = -1;
    int dPos = -1;
    int begin_ = -1;
    int end_ = -1;
    
    for (auto& entry : currentConv->entries) {
        if (pos >= entry.pos_begin && pos < entry.pos_end) {
            // found the entry with position 'pos'
            currentConv->mouse_entry = &entry - currentConv->entries.begin();
            
            // debug
            entryLen = CountRichChars(entry.request) + CountRichChars(entry.reply);
            dPos = pos - entry.pos_begin;
            begin_ = entry.pos_begin;
            end_ = entry.pos_end;
            break;
        }
    }
    
    if (oldMouseEntry != currentConv->mouse_entry) {
        UpdateOutput(false);
    }
    
#if 0
    // debug
    statusbar.Set(Format("pos = %d , entryLen = %d, dPos = %d, begin = %d, end = %d", pos, entryLen, dPos, begin_, end_));
#endif
}

void UppOracle::RichMenu(Bar& bar)
{
    if (!currentConv || currentConv->mouse_entry < 0)
        return;
    Conversation::Entry* entry = &currentConv->entries[currentConv->mouse_entry];
    
    bar.Add("Delete entry", OpenIconic::trash_2x(), [this, entry]{
        String s;
        s << "Are you sure you want to delete the entry`:";
        s << "&&Query`:-|[@K " << DeQtf(entry->request) << "]&&Reply`:-|[@K " << DeQtf(entry->reply) << "]";
        s << "&&[* This action cannot be undone`!]";
        if (PromptYesNo(s) == 1) {
            currentConv->entries.Remove(currentConv->mouse_entry);
            currentConv->mouse_entry = -1;
            currentConv->modified = true;
            UpdateOutput(false);
        }
    });
}
















