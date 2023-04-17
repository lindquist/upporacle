#include "App.h"

String QtfTable(const char* header, const Vector<String>& cells)
{
	String qtf;
	qtf << "{{" << header << " " << Join(cells, "||") << "}}";
	return pick(qtf);
}

String QtfImage(int w, int h, const char* image)
{
	String qtf;
	qtf << "@@iml:" << w << "*" << h << "`" << image << "`";
	return pick(qtf);
}

String Obfuscate(const String& text)
{
    return String('*', 8) + text.Right(6);
}

void UppOracle::ShowAboutDialog()
{
	WithAboutLayout<TopWindow> dialog;
	CtrlLayout(dialog, "About U++ Oracle");
	
	String header = "[4*/ U`+`+ Oracle]"
					"&Copyright `© 2023 by Tomas Lindquist `(tomas.l.olsen`@gmail.com`)"
					"&"
					"&All rights reserved.";
	
	String copyright; // fixme
	String url;
	
	String versionInfo;
	versionInfo << "&Version`: " << AppVersion.major << "." << AppVersion.minor << "." << AppVersion.patch << "`-" << AppVersion.build;

	String opensource;
	opensource << "&[H2 U`+`+ Oracle makes use of open`-source software: [^https://www.ultimatepp.org^ U`+`+].]"
			      "&Open source license details can be found on the [^https://github.com/lindquist/upporacle^ GitHub site].";
	
	String icon;
	icon << "[= &" << QtfImage(384,384, "ImagesImg:icon") << "]";
	
	String qtftable = QtfTable("1:6", {
		icon, header,
		"",	copyright,
		"",	url,
		"", versionInfo,
		"", opensource
	});
	
	dialog.rich.SetZoom(Zoom(zoomBase,zoom));
	dialog.rich.SetQTF(qtftable);
	
	dialog.RunAppModal();
}

void UppOracle::ShowUsageDialog()
{
	WithUsageLayout<TopWindow> dialog;
	CtrlLayout(dialog, "Usage report (not yet working)");
	
	dialog.array.AddColumn(ID_NAME, "Model name");
	dialog.array.AddColumn(ID_TOTAL, "Total tokens");
	dialog.array.AddColumn(ID_TODAY, "Tokens today");
	
	for (auto& model: ModelCostTable.GetKeys()) {
		int i = usageMap.Find(model);
		if (i < 0 || usageMap.IsUnlinked(i))
			continue;
		auto& usage = usageMap[i];
		dialog.array.Add(model, usage.total_tokens, usage.daily_tokens);
	}
	
	dialog.RunAppModal();
}

void UppOracle::ShowPreferences()
{
	Preferences prefs;
	
	prefs.sans_serif_font.SetData(Font::FindFaceNameIndex(settings.sans_serif_font));
	prefs.serif_font.SetData(Font::FindFaceNameIndex(settings.serif_font));
	prefs.monospace_font.SetData(Font::FindFaceNameIndex(settings.monospace_font));
	prefs.enter_to_send.SetData(settings.enter_sends ? ENTER_SENDS : CTRL_ENTER_SENDS);
	prefs.default_prompt.SetData(settings.default_prompt);
	prefs.time_format.SetData(settings.time_format);
	
	if (prefs.RunAppModal() == IDOK) {
		settings.sans_serif_font = Font::GetFaceName(~prefs.sans_serif_font);
		settings.serif_font = Font::GetFaceName(~prefs.serif_font);
		settings.monospace_font = Font::GetFaceName(~prefs.monospace_font);
		settings.enter_sends = int(~prefs.enter_to_send) == ENTER_SENDS ? true : false;
		settings.default_prompt = ~prefs.default_prompt;
		settings.time_format = ~prefs.time_format;
	}
	
	inputbar.inputedit.SetSendHotkey(settings.enter_sends ? K_ENTER : K_CTRL_ENTER);
	
	// maybe something changes (like time format)
	UpdateOutput();
}

void UppOracle::ShowSearch()
{
    String toFind;
    if (!EditText(toFind, "Search ...", "Search terms:") || toFind.IsEmpty())
        return;
    
    // try to show some progress
    int total = 0;
    for (auto& conv : conversations) {
        total += conv.entries.GetCount();
    }
    
    int p = 0;
    Progress prog(this, "Searching ...", total);
    
    searchResults.Clear();
    
    int N = conversations.GetCount();
    for (int ci = 0; ci < N; ++ci) {
        if (conversations.IsUnlinked(ci))
            continue;
        auto& conv = conversations[ci];
        
        for (auto& entry : conv.entries) {
            int p2 = p;
            int i = entry.request.Find(toFind);
            if (i >= 0) {
                searchResults << conv.id;
                break;
            }
            prog.Set(p2, total);
        }
        p += conv.entries.GetCount();
        prog.Set(p, total);
    }
    
    if (searchResults.IsEmpty()) {
        String msg;
        msg << "No conversations containing \"" << toFind << "\" were found";
        PromptOK(DeQtf(msg));
        searchTerm.Clear();
    }
    else {
        // show results
        searchTerm = toFind;
        ShowSearchResults();
    }
}

void UppOracle::ShowEulaDialog()
{
    auto topic = GetTopic("UppOracle", "app", "EULA_en-us");
    
    WithEULALayout<TopWindow> wnd;
    CtrlLayoutOKCancel(wnd, topic.title);
    wnd.Zoomable().Sizeable();
    wnd.SetMinSize(Size(800,600));
    
    wnd.eula.SetQTF(topic.text);
    wnd.eula.Margins(16);
    
    wnd.agreeCheck.Set(false);
    wnd.ok.Enable(false);
    
    wnd.agreeCheck.WhenAction = [&]{
        wnd.ok.Enable(wnd.agreeCheck.Get() != 0);
    };
    
    settings.eula_ok = false;
    if (wnd.RunAppModal()) {
        if (wnd.agreeCheck.Get()) {
            settings.eula_ok = true;
        }
    }
}










