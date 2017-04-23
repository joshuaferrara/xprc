#pragma once
#include <string>
#include <map>
#include "serial\serial.h"

#include <XPLMUtilities.h>

class QTermII
{
public:
	enum CursorMode {
		BlockOffUnderlineOff,
		BlockOffUnderlineOn,
		BlockOnUnderlineOff,
		BlockOnUnderlineOn
	};

	enum LEDState {
		Off,
		On,
		Blinking
	};

	QTermII(std::string port, int baudRate);
	void Connect();
	void Disconnect();
	size_t Available();
	size_t Read(std::string & buffer, size_t amount);
	bool Connected();
	~QTermII();
	void SendCommand(std::string command);
	void EnableBacklight();
	void DisableBacklight();
	void ToggleBacklight();
	void Beep();
	void SetCursorPosition(int row, int col);
	void SetCursorMode(CursorMode mode);
	void Write(std::string text);
	void SetLED(int led, QTermII::LEDState newState);
	void WriteAt(std::string text, int row, int col);
	void Reset();
	void Erase();
	void EraseRow();
	void EraseRow(int row);
private:
	serial::Serial hc;
	std::map<LEDState, char*> ledToChars;

	char cursorPositions[20] = { '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S' };
	char ledsOff[6] = { '@', 'A', 'B', 'C', 'D', 'E' };
	char ledsOn[6] = { 'H', 'I', 'J', 'K', 'L', 'M' };
	char ledsBlink[6] = { 'P', 'Q', 'R', 'S', 'T', 'U' };
	char ledsToggle[6] = { 'X', 'Y', 'Z', '[', '\\', ']' };
};

