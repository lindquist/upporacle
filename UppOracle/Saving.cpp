#include "App.h"

void UppOracle::Settings::Serialize(Stream& s)
{
	const int this_version = 4;
	int version = this_version;
	
	String ident = "UppOracle"; // FIXME
	s % ident;
	if (s.IsLoading() && ident != "UppOracle")
		return;
	
	s % version;
	s % api_key;
	s % models;
	s % total_tokens;
	s % context_factor;
	s % token_limit;
	s % temperature;
	s % stream;
	s % model;
	
	s % screen_rect % maximized;
	s % horz_split % vert_split;
	s % array_cols;
	
	s % font_family;
	s % zoom;
	
	s % sans_serif_font;
	s % serif_font;
	s % monospace_font;
	s % enter_sends;
	s % default_prompt;
	
	s % activation_token;
	s % activation_signature;
	
	if (version >= 3)
	    s % eula_ok;
	else
	    return;
	
	if (version >= 4)
	    s % time_format;
	else
	    return;
}

void UppOracle::ModelUsage::Serialize(Stream& s)
{
    s % total_tokens % daily_tokens % token_1k_price_usd_cent % date;
}

void Conversation::Serialize(Stream& s)
{
	const int this_version = 3;
	int version = this_version;
	
	s % version;
	s % time_started;
	s % entries;
	s % token_usage;
	if (version >= 2) {
		s % favorite;
	}
	if (version >= 3) {
		s % last_modified;
	}
}

void Conversation::Entry::Serialize(Stream& s)
{
    const int this_version = 2;
	int version = this_version;
	s % version;
	s % request_time % first_reply % done_time;
	s % context;
	if (version < 2)
	    s % context; // sent_context - removed in v2
	s % request % reply;
	s % temperature % model;
}
