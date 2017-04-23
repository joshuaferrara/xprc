#include "QTermII.h"

QTermII::QTermII(std::string port, int baudRate)
{
	ledToChars.insert_or_assign(LEDState::Off, ledsOff);
	ledToChars.insert_or_assign(LEDState::On, ledsOn);
	ledToChars.insert_or_assign(LEDState::Blinking, ledsBlink);

	hc.setPort(port);
	hc.setBaudrate(baudRate);
	Connect();
}

QTermII::~QTermII()
{
	Disconnect();
}

void QTermII::SendCommand(std::string command) {
	try {
		hc.write("\x001b" + command);
	}
	catch (serial::SerialException err) {
		XPLMDebugString("QTermII Error sending command!\n");
		XPLMDebugString(err.what());
	}
}

void QTermII::EnableBacklight() {
	SendCommand("VA");
}

void QTermII::DisableBacklight() {
	SendCommand("V@");
}

void QTermII::ToggleBacklight() {
	SendCommand("VB");
}

void QTermII::Beep() {
	SendCommand("OB");
}

void QTermII::SetCursorPosition(int row, int col) {
	char out[4];
	sprintf(out, "I%c%c", cursorPositions[row], cursorPositions[col]);
	SendCommand(out);
}

void QTermII::SetCursorMode(CursorMode mode) {
	char out[3];
	sprintf(out, "b%c", ledsOff[(int)mode]);
	SendCommand(out);
}

void QTermII::Write(std::string text) {
	try {
		hc.write(text);
	}
	catch (serial::SerialException err) {
		XPLMDebugString("QTermII Error writing!\n");
		XPLMDebugString(err.what());
	}
}

void QTermII::SetLED(int led, QTermII::LEDState newState) {
	char out[3];
	sprintf(out, "P%c", ledToChars[newState][led]);
	SendCommand(out);
}

void QTermII::WriteAt(std::string text, int row, int col) {
	SetCursorPosition(row, col);
	hc.write(text);
}

void QTermII::Reset() {
	SendCommand("M");
}

void QTermII::Erase() {
	SendCommand("E");
}

void QTermII::EraseRow() {
	SendCommand("K");
}

void QTermII::EraseRow(int row) {
	SetCursorPosition(row, 0);
	EraseRow();
}

void QTermII::Connect() {
	if (Connected()) return;
	try {
		hc.open();
	}
	catch (serial::SerialException ex) {
		XPLMDebugString("QTermII Error connecting!\n");
		XPLMDebugString(ex.what());
	}
}

void QTermII::Disconnect() {
	hc.close();
}

size_t QTermII::Available() {
	return hc.available();
}

size_t QTermII::Read(std::string &buffer, size_t size) {
	return hc.read(buffer, size);
}

bool QTermII::Connected() {
	return hc.isOpen();
}