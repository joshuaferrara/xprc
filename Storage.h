#pragma once

enum TRANSPONDER_STATE {
	Off,
	Standby,
	On,
	Test
};

enum XPRC_STATE {
	INIT,
	RADIO,
	SQUAWK,
	FLIGHT_DISPLAY,
	AUTO_PILOT
};

struct {
	XPRC_STATE curState = INIT;

	bool changingValue = false; // If a value is being changed
	int cValueIdx = 0; // The current index of the value we're changing
	std::string tempBuff; // Temporary buffer for storing values being changed
} XPRCDat;

struct {
	float altitude;
} AutopilotData;

struct {
	XPLMDataRef airSpeedRef;
	XPLMDataRef gndSpeedRef;
	XPLMDataRef aglRef;
	XPLMDataRef mslRef;

	float airSpeed;
	float gndSpeed;
	float agl;
	float msl;
} FlightData;

struct {
	XPLMDataRef tCodeRef = NULL;
	XPLMDataRef tIdRef = NULL;
	XPLMDataRef tLightRef = NULL;
	XPLMDataRef tBrightnessRef = NULL;
	XPLMDataRef tModeRef = NULL;

	int transponderCode = 0;
	int transponderId = 0;
	int transponderLight = 0;
	float transponderBrightness = 0.0;
	TRANSPONDER_STATE transponderMode = Off;
} RadioData;