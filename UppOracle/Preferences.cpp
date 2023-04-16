#include "App.h"

// ---------------------------------------------------------------------------

namespace {

struct FontFaceDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
		               Color ink, Color paper, dword style) const
	{
		Font fnt = Font(q, r.Height() - 2);
		String txt = Font::GetFaceName(q);
		w.DrawRect(r, paper);
		w.DrawText(r.left + 2, r.top + (r.Height() - GetTextSize(txt, fnt).cy) / 2, txt, fnt, ink);
	}
};

} // anon

// ---------------------------------------------------------------------------

Preferences::Preferences()
{
	CtrlLayoutOKCancel(*this, "U++ Oracle Preferences");
	
	// list fonts
	
	Font arial;
	arial.FaceName("Arial");
	
	Font consolas;
	consolas.FaceName("Consolas");
	
	Font times;
	times.FaceName("Times New Roman");
	
	int ssi = arial.GetFace();
	int si = times.GetFace();
	int mi = consolas.GetFace();
	
	sans_serif_font.SetDisplay(Single<FontFaceDisplay>());
	for(int i = 0; i < Font::GetFaceCount(); i++)
		sans_serif_font.Add(i);
	sans_serif_font.Set(ssi);
	
	serif_font.SetDisplay(Single<FontFaceDisplay>());
	for(int i = 0; i < Font::GetFaceCount(); i++)
		serif_font.Add(i);
	serif_font.Set(si);
	
	monospace_font.SetDisplay(Single<FontFaceDisplay>());
	for(int i = 0; i < Font::GetFaceCount(); i++)
		monospace_font.Add(i);
	monospace_font.Set(mi);
	
	default_prompt.SetText("Can you tell me a random interesing fact?");
	
	enter_to_send.SetData(0);
}
