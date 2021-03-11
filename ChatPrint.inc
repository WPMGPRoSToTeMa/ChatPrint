#if !defined _CHATPRINT_INCLUDED
#define _CHATPRINT_INCLUDED

#include <amxmodx>
#include <fakemeta>

// Hack hack hack
#if !defined SetGlobalTransTarget
	#define SetGlobalTransTarget ChatPrint_SetGlobalTransTarget
	
	stock ChatPrint_SetGlobalTransTarget(clientIndex) {
		new oldFlags = pev(clientIndex, pev_flags);
		set_pev(clientIndex, pev_flags, FL_FAKECLIENT); // Emulate bot
		console_cmd(clientIndex, ""); // When player is bot this function call only SetDefLang function
		set_pev(clientIndex, pev_flags, oldFlags); // Restore flags
	}
#endif

#if !defined MAX_PLAYERS
	const MAX_PLAYERS = 32;
#endif

#define CHATPRINT_COLOR_CTRLCHAR_CODE   0x5C // '\'
#define CHATPRINT_DEFAULT_COLOR_CODE    1
#define CHATPRINT_TEAM_COLOR_CODE       3
#define CHATPRINT_GREEN_COLOR_CODE      4
#define CHATPRINT_RED_COLOR_CODE        5
#define CHATPRINT_BLUE_COLOR_CODE       6
#define CHATPRINT_GREY_COLOR_CODE       7
#define CHATPRINT_DEFAULT_COLOR_LETTER  'd' // default
#define CHATPRINT_DEFAULT_COLOR_LETTER2 'y' // yellow
#define CHATPRINT_DEFAULT_COLOR_LETTER3 'n' // none
#define CHATPRINT_DEFAULT_COLOR_NUMBER  '1'
#define CHATPRINT_TEAM_COLOR_LETTER     't' // team
#define CHATPRINT_TEAM_COLOR_NUMBER     '3'
#define CHATPRINT_GREEN_COLOR_LETTER    'g' // green
#define CHATPRINT_GREEN_COLOR_NUMBER    '4'
#define CHATPRINT_RED_COLOR_LETTER      'r' // red
#define CHATPRINT_RED_COLOR_NUMBER      '5'
#define CHATPRINT_BLUE_COLOR_LETTER     'b' // blue
#define CHATPRINT_BLUE_COLOR_NUMBER     '6'
#define CHATPRINT_GREY_COLOR_LETTER     'w' // white
#define CHATPRINT_GREY_COLOR_NUMBER     '7'

#define CHATPRINT_USERMSG_MAXBYTES      192
#define CHATPRINT_CHATMSG_MAXBYTES      (CHATPRINT_USERMSG_MAXBYTES - 1 - sizeof(ChatPrint_PercentS1Phrase)) // 1 is for player id byte
#define CHATPRINT_RAWCHATMSG_MAXBYTES   ((CHATPRINT_CHATMSG_MAXBYTES - 1) * 2 + 1) // (CHATPRINT_CHATMSG_MAXBYTES - sizeof(char(EOS))) * 2 + sizeof(char(EOS))

#define CHATPRINT_PLAYERINFO_MAXINDEX   63
#define CHATPRINT_RED_COLOR_PLINDEX     (CHATPRINT_PLAYERINFO_MAXINDEX - 2)
#define CHATPRINT_BLUE_COLOR_PLINDEX    (CHATPRINT_PLAYERINFO_MAXINDEX - 1)
#define CHATPRINT_GREY_COLOR_PLINDEX    CHATPRINT_PLAYERINFO_MAXINDEX

static stock const CHATPRINT_LIBRARY_NAME[] = "ChatPrint";
static stock const ChatPrint_PercentS1Phrase[] = "#Spec_PlayerItem";

static stock g_msgidSayText;

stock ChatPrint(clientIndex, const format[], any:...) {
	ChatPrint_Initialize();
	
	static raw[CHATPRINT_RAWCHATMSG_MAXBYTES];
	if (clientIndex > 0) {
		SetGlobalTransTarget(clientIndex);
		vformat(raw, charsmax(raw), format, 3);
		
		ChatPrint_PrepareAndSendMessage(clientIndex, raw);
	} else {
		static players[MAX_PLAYERS];
		new playersCount;
		get_players(players, playersCount, "c"); // Don't include bots
		
		for (new n = 0; n < playersCount; n++) {
			if (clientIndex < 0 && players[n] == -clientIndex) { // Send all exclude this player
				continue;
			}
			
			SetGlobalTransTarget(players[n]);
			vformat(raw, charsmax(raw), format, 3);
			
			ChatPrint_PrepareAndSendMessage(players[n], raw);
		}
	}
}

stock ChatPrint_EscapeString(dst[], maxLen, const src[]) {
	new n = 0;
	for (new i = 0; src[i] != EOS && n < maxLen; i++) {
		if (0 < src[i] <= 0x1F || src[i] == 0x7F) {
			continue;
		}
		
		if (src[i] == CHATPRINT_COLOR_CTRLCHAR_CODE) {
			if (maxLen - n < 2) {
				break;
			}
			
			dst[n++] = CHATPRINT_COLOR_CTRLCHAR_CODE;
			dst[n++] = CHATPRINT_COLOR_CTRLCHAR_CODE;
		} else {
			dst[n++] = src[i];
		}
	}
	dst[n] = EOS;
}

static stock ChatPrint_PrepareAndSendMessage(clientIndex, const raw[]) {
	static message[CHATPRINT_CHATMSG_MAXBYTES];
	new bool:useColors = false;
	new msgLen = 0;
	new colorPlayerIndex = clientIndex;
	for (new rawPos = 0; raw[rawPos] != EOS && (msgLen + 1) <= charsmax(message); rawPos++, msgLen++) {
		switch (raw[rawPos]) {
			case CHATPRINT_DEFAULT_COLOR_CODE, CHATPRINT_GREEN_COLOR_CODE: {
				useColors = true;
				message[msgLen] = raw[rawPos];
			}
			case CHATPRINT_TEAM_COLOR_CODE: {
				useColors = true;
				colorPlayerIndex = clientIndex;
				message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
			}
			case CHATPRINT_RED_COLOR_CODE: {
				useColors = true;
				colorPlayerIndex = CHATPRINT_RED_COLOR_PLINDEX;
				message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
			}
			case CHATPRINT_BLUE_COLOR_CODE: {
				useColors = true;
				colorPlayerIndex = CHATPRINT_BLUE_COLOR_PLINDEX;
				message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
			}
			case CHATPRINT_GREY_COLOR_CODE: {
				useColors = true;
				colorPlayerIndex = CHATPRINT_GREY_COLOR_PLINDEX;
				message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
			}
			case CHATPRINT_COLOR_CTRLCHAR_CODE: {
				switch (raw[++rawPos]) {
					case CHATPRINT_DEFAULT_COLOR_LETTER, CHATPRINT_DEFAULT_COLOR_LETTER2, CHATPRINT_DEFAULT_COLOR_LETTER3, CHATPRINT_DEFAULT_COLOR_NUMBER: {
						useColors = true;
						message[msgLen] = CHATPRINT_DEFAULT_COLOR_CODE;
					}
					case CHATPRINT_TEAM_COLOR_LETTER, CHATPRINT_TEAM_COLOR_NUMBER: {
						useColors = true;
						colorPlayerIndex = clientIndex;
						message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
					}
					case CHATPRINT_GREEN_COLOR_LETTER, CHATPRINT_GREEN_COLOR_NUMBER: {
						useColors = true;
						message[msgLen] = CHATPRINT_GREEN_COLOR_CODE;
					}
					case CHATPRINT_RED_COLOR_LETTER, CHATPRINT_RED_COLOR_NUMBER: {
						useColors = true;
						colorPlayerIndex = CHATPRINT_RED_COLOR_PLINDEX;
						message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
					}
					case CHATPRINT_BLUE_COLOR_LETTER, CHATPRINT_BLUE_COLOR_NUMBER: {
						useColors = true;
						colorPlayerIndex = CHATPRINT_BLUE_COLOR_PLINDEX;
						message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
					}
					case CHATPRINT_GREY_COLOR_LETTER, CHATPRINT_GREY_COLOR_NUMBER: {
						useColors = true;
						colorPlayerIndex = CHATPRINT_GREY_COLOR_PLINDEX;
						message[msgLen] = CHATPRINT_TEAM_COLOR_CODE;
					}
					default: {
						message[msgLen] = raw[rawPos];
					}
				}
			}
			default: {
				message[msgLen] = raw[rawPos];
			}
		}
	}
	
	message_begin(MSG_ONE, g_msgidSayText, _, clientIndex);
	write_byte(colorPlayerIndex);
	write_string(ChatPrint_PercentS1Phrase);
	
	// If message contains color codes, but doesn't begin with color code it will be uncolored
	// If message begins with # it will be localized, so we can put color code at the beginning which will disable localizing, it is invisible so it won't affect the displayed message
	// TODO: condition for localizability can be more narrow, but this complicates things and also this condition can be fully omitted, so we will always put this char in message, this solution will always eat 1 byte, but this makes the max message length constant and makes code simpler, so we can easily add new features such as splitting up long messages
	if ((useColors && message[0] > CHATPRINT_GREEN_COLOR_CODE) || message[0] == '#') {
		write_char(CHATPRINT_DEFAULT_COLOR_CODE);
		if (msgLen > (charsmax(message) - 1)) { // charsmax(message) - sizeof(char(CHATPRINT_DEFAULT_COLOR_CODE))
			msgLen = (charsmax(message) - 1);
		}
	}
	
	// UTF-8 validation of the message ending
	// It is needed since we are truncating message as bytes, not as unicode codepoints
	// TODO: looks like we need to consider not only codepoints, but visible glyphs
	if (msgLen >= 2 && message[msgLen-1] & 0x80) {
		if (message[msgLen-1] & 0x40) {
			msgLen--;
		} else {
			new n = 2;
			for (new i = msgLen-2; i != -1 && message[i] & 0x80; i--, n++) {
				if (message[i] & 0x40) {
					new k;
					
					if (message[i] & 0x20) {
						// Only 1-4 successive bytes are valid in utf-8
						// But goldsrc windows client can use only codepoints from 0 to 0xFFFF, so we can omit 4-byte sequences
						// TODO: can linux client use codepoints higher than 0xFFFF?
						/*if (message[i] & 0x10) {
							k = 4;
						} else {*/
							k = 3;
						/*}*/
					} else {
						k = 2;
					}
					
					if (n != k) {
						msgLen = i;
					}
					
					break;
				}
			}
		}
	}
	
	message[msgLen] = EOS;
	
	write_string(message);
	message_end();
}

static stock ChatPrint_Initialize() {
	static bool:initialized = false;
	
	if (!initialized) { // Is locally initialized? (this plugin)
		if (!LibraryExists(CHATPRINT_LIBRARY_NAME, LibType_Library)) { // Is globally initialized? (all plugins)
			new msgidTeamInfo = get_user_msgid("TeamInfo");
			
			// Precaching teaminfo's
			engfunc(EngFunc_MessageBegin, MSG_INIT, msgidTeamInfo, 0, 0);
			write_byte(CHATPRINT_RED_COLOR_PLINDEX);
			write_string("TERRORIST");
			message_end();
			engfunc(EngFunc_MessageBegin, MSG_INIT, msgidTeamInfo, 0, 0);
			write_byte(CHATPRINT_BLUE_COLOR_PLINDEX);
			write_string("CT");
			message_end();
			engfunc(EngFunc_MessageBegin, MSG_INIT, msgidTeamInfo, 0, 0);
			write_byte(CHATPRINT_GREY_COLOR_PLINDEX);
			write_string(""); // or SPECTATOR
			message_end();
			
			// And send it now for already connected players
			// Use MSG_ONE with for-loop instead of MSG_ALL, because MSG_ALL messages sends after MSG_ONE (ChatPrint sends chat messages via MSG_ONE)
			static players[MAX_PLAYERS];
			new playersCount;
			get_players(players, playersCount, "c"); // Don't include bots
			
			for (new n = 0; n < playersCount; n++) {
				message_begin(MSG_ONE, msgidTeamInfo, _, players[n]);
				write_byte(CHATPRINT_RED_COLOR_PLINDEX);
				write_string("TERRORIST");
				message_end();
				message_begin(MSG_ONE, msgidTeamInfo, _, players[n]);
				write_byte(CHATPRINT_BLUE_COLOR_PLINDEX);
				write_string("CT");
				message_end();
				message_begin(MSG_ONE, msgidTeamInfo, _, players[n]);
				write_byte(CHATPRINT_GREY_COLOR_PLINDEX);
				write_string(""); // or SPECTATOR
				message_end();
			}
			
			register_library(CHATPRINT_LIBRARY_NAME);
		}
		
		g_msgidSayText = get_user_msgid("SayText");
		
		initialized = true;
	}
}

#undef CHATPRINT_COLOR_CTRLCHAR_CODE
#undef CHATPRINT_DEFAULT_COLOR_CODE
#undef CHATPRINT_TEAM_COLOR_CODE
#undef CHATPRINT_GREEN_COLOR_CODE
#undef CHATPRINT_RED_COLOR_CODE
#undef CHATPRINT_BLUE_COLOR_CODE
#undef CHATPRINT_GREY_COLOR_CODE
#undef CHATPRINT_DEFAULT_COLOR_LETTER
#undef CHATPRINT_DEFAULT_COLOR_LETTER2
#undef CHATPRINT_DEFAULT_COLOR_LETTER3
#undef CHATPRINT_DEFAULT_COLOR_NUMBER
#undef CHATPRINT_TEAM_COLOR_LETTER
#undef CHATPRINT_TEAM_COLOR_NUMBER
#undef CHATPRINT_GREEN_COLOR_LETTER
#undef CHATPRINT_GREEN_COLOR_NUMBER
#undef CHATPRINT_RED_COLOR_LETTER
#undef CHATPRINT_RED_COLOR_NUMBER
#undef CHATPRINT_BLUE_COLOR_LETTER
#undef CHATPRINT_BLUE_COLOR_NUMBER
#undef CHATPRINT_GREY_COLOR_LETTER
#undef CHATPRINT_GREY_COLOR_NUMBER

#undef CHATPRINT_USERMSG_MAXBYTES
#undef CHATPRINT_CHATMSG_MAXBYTES
#undef CHATPRINT_RAWCHATMSG_MAXBYTES

#undef CHATPRINT_PLAYERINFO_MAXINDEX
#undef CHATPRINT_RED_COLOR_PLINDEX
#undef CHATPRINT_BLUE_COLOR_PLINDEX
#undef CHATPRINT_GREY_COLOR_PLINDEX

#endif