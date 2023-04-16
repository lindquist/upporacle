#include "App.h"

void UppOracle::ThreadMain()
{
	// runs in request thread
	
	for (;;) {
		if (Thread::IsShutdownThreads())
			return;
		if (requestSem.Wait(40)) {
			Value request = queue.PopHead();
			Value input = request["input"];
			int id = request["id"];
			
			LOG("starting request:");
			DUMP(input);
			
			// build value for the result
			ValueMap m;
			m("id", id);
			
			// streaming callback
			auto streamer = [&](Value v) {
				AtomicInc(token_counter);
				ValueMap m;
				m("id", id)("data", pick(v));
				results.AddTail(pick(m));
				resultsSem.Release();
			};
			
			bool useStream = input["stream"];
			if (useStream) {
				auto events = openai.CreateCompletion(input, streamer);
				if (events.IsError())
					streamer(events);
			}
			else {
				auto response = openai.CreateCompletion(input);
				ValueMap m;
				m("id", id)("data", pick(response));
				results.AddTail(pick(m));
				resultsSem.Release();
			}
		}
	}
}

void UppOracle::TimerFunc()
{
	// runs in gui thread
	
	while (resultsSem.Wait(0)) {
		auto event = results.PopHead();
		auto json = event["data"];
		int id = event["id"];
		
		if (json.IsError()) {
			String s;
			s << "[ERROR: " << GetErrorText(json) << "]\n";
			PostStream(id, s);
			EndStream(id);
			continue;
		}
		auto& error = json["error"];
		if (!error.IsNull()) {
			String s;s << error["type"] << ": " << error["message"];
			PostStream(id, s);
			EndStream(id);
			continue;
		}
		if (json.IsNull()) {
			EndStream(id);
			continue;
		}
		
		auto& text = json["choices"][0]["text"];
		auto& finish_reason = json["choices"][0]["finish_reason"];
		if (!text.IsNull())
			PostStream(id, text);
		if (!finish_reason.IsNull()) {
			if (finish_reason == "stop") {
			}
			else if (finish_reason == "length") {
				PostStream(id, "<< cut off due to length >>");
			}
			else {
				PostStream(id, String("<< finish_reason = ") << finish_reason << ">>");
			}
			EndStream(id);
		}
	}
	
	auto prevTime = lastTime;
}

void UppOracle::HandleResponse(Value input, Value response)
{
	auto sinput = AsJSON(input, true);
	DUMP(sinput);
	auto sresponse = AsJSON(response, true);
	DUMP(sresponse);
	
	auto data = response["data"];
	int id = response["id"];

	if (IsError(response)) {
		String s = GetErrorText(response);
		PostStream(id, s);
	}
	else if (IsString(data)) {
		String s = TrimBoth(data);
		PostStream(id, s);
	}
	else if (IsValueArray(data["choices"])) {
		for (auto& choice : data["choices"]) {
			String s = TrimBoth(choice["text"]);
			PostStream(id, s);
		}
	}
	else {
		// nothing?
		String s;
		s << "Unknown format: " << data;
		PostStream(id, s);
	}
	
	// collect usage if possible
	if (IsValueMap(response["usage"])) {
		token_counter = response["usage"]["total_tokens"];
	}
	else {
		StatusMessage("no token usage was returned");
		token_counter = 0;
	}
	
	// end the stream
	EndStream(id);
}

void UppOracle::PostStream(int id, const String& s)
{
	int i = conversations.Find(id);
	if (i < 0) {
		LOG("received token '" << s << "' with invalid conversation id '" << id << "'");
		return;
	}
	auto& conv = conversations[i];
	if (conv.entries.IsEmpty()) {
		LOG("received token '" << s << "' but conversation has zero entries");
		return;
	}
	auto& entry = conv.entries.Top();
	if (IsNull(entry.first_reply))
		entry.first_reply = GetSysTime();
	entry.reply << s;
	conv.modified = true;
	UpdateOutput();
}

void UppOracle::EndStream(int id)
{
	// finalize conversation
	int i = conversations.Find(id);
	if (i < 0)
		return;
	auto& conv = conversations[i];
	if (conv.entries.IsEmpty())
		return;
	auto& entry = conv.entries.Top();
	entry.done_time = GetSysTime();
	conv.modified = true;
	UpdateOutput();
	
	// update usage
	if (token_counter > 0) {
		AddUsage(id, token_counter);
		UpdateStatus(token_counter);
		token_counter = 0;
	}
	
	// finally re-enable the send button
	inputbar.sendbutton.Enable();
}

void UppOracle::AddUsage(int id, int numtokens)
{
	int i = conversations.Find(id);
	if (i < 0)
		return;
	
	auto& conv = conversations[i];
	if (conv.entries.IsEmpty())
		return;
	
	auto& entry = conv.entries.Top();
	conv.AddUsage(entry.model, numtokens);
	AddGlobalUsage(entry.model, numtokens);
}

void UppOracle::AddGlobalUsage(const String& model, int num_tokens)
{
	int i = usageMap.FindAdd(model);
	auto& use = usageMap[i];
	use.daily_tokens += num_tokens;
	use.total_tokens += num_tokens;
}
