#include <XPLMDefs.h>
#include <XPLMPlugin.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>
#include <XPLMMenus.h>
#include <XPLMUtilities.h>
#include <XPLMCamera.h>
#include <XPWidgets.h>
#include <XPStandardWidgets.h>

#include <string>
#include <stdio.h>
#include "serial\serial.h"

#include "Storage.h"
#include "QTermII.h"

// Variables
QTermII hc("COM1", 9600);


#if IBM
#include <windows.h>
BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

float FlightLoop(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon) {
	// Get information
	RadioData.transponderCode = XPLMGetDatai(RadioData.tCodeRef);
	RadioData.transponderId = XPLMGetDatai(RadioData.tIdRef);
	RadioData.transponderLight = XPLMGetDatai(RadioData.tLightRef);
	RadioData.transponderBrightness = XPLMGetDataf(RadioData.tBrightnessRef);
	RadioData.transponderMode = (TRANSPONDER_STATE) XPLMGetDatai(RadioData.tModeRef);

	FlightData.agl = XPLMGetDataf(FlightData.aglRef) * 3.2808399; // Convert to ft
	FlightData.msl = XPLMGetDataf(FlightData.mslRef) * 3.2808399; // Convert to ft
	FlightData.gndSpeed = XPLMGetDataf(FlightData.gndSpeedRef) * 1.94384449; // Convert to knots
	FlightData.airSpeed = XPLMGetDataf(FlightData.airSpeedRef); // Knots

#ifdef DEBUG
	char out[32];
	sprintf(out, "Squawk: %d\n", RadioData.transponderCode);
	XPLMDebugString(out);
#endif

	// Get autopilot information

	// Anything to get from the HC?
	size_t avail = hc.Available();
	std::string input;
	if (avail > 0) {
		hc.Read(input, avail);
		XPLMDebugString("[XPRC] Got char: "); XPLMDebugString(input.append("\n").c_str());

		if (input[0] == ':') {
			if (XPRCDat.curState == AUTO_PILOT) {
				XPRCDat.curState = RADIO;
			}
			else {
				XPRCDat.curState = (XPRC_STATE)(XPRCDat.curState + 1);
			}
			hc.Erase();
		}
	}

	// Send information
	char hcBuff[24];
	switch (XPRCDat.curState) {
	case INIT:
		XPRCDat.curState = SQUAWK;
		break;
	case RADIO:
		hc.WriteAt("Radio TODO", 0, 0);
		break;
	case AUTO_PILOT:
		sprintf(hcBuff, "Hdg: %d %s", 0, ("DISABLED"));
		hc.WriteAt(hcBuff, 0, 0);

		sprintf(hcBuff, "Alt: %d %s", 0, ("DISABLED"));
		hc.WriteAt(hcBuff, 1, 0);
		break;
	case FLIGHT_DISPLAY:
		sprintf(hcBuff, "ASpd: %dkts", (int)FlightData.airSpeed);
		hc.WriteAt(hcBuff, 0, 0);

		sprintf(hcBuff, "GSpd: %d", (int)FlightData.gndSpeed);
		hc.WriteAt(hcBuff, 1, 0);

		sprintf(hcBuff, "AGL: %dft", (int)FlightData.agl);
		hc.WriteAt(hcBuff, 2, 0);

		sprintf(hcBuff, "MSL: %dft", (int)FlightData.msl);
		hc.WriteAt(hcBuff, 3, 0);
		break;
	case SQUAWK:
		if (!input.empty()) {
			switch (input[0]) {
			case '>':
				RadioData.transponderMode = (RadioData.transponderMode == TRANSPONDER_STATE::Off ? TRANSPONDER_STATE::On : TRANSPONDER_STATE::Off);
				XPLMSetDatai(RadioData.tModeRef, (int)RadioData.transponderMode);
				break;
			case 'G':
				if (XPRCDat.changingValue == false) {
					hc.SetLED(3, QTermII::LEDState::Blinking);

					XPRCDat.changingValue = true;
					XPRCDat.tempBuff = std::to_string(RadioData.transponderCode);
					
					XPLMDebugString("Changing squawk from ");
					XPLMDebugString(XPRCDat.tempBuff.c_str());
					XPLMDebugString("\n");
				}
				else {
					hc.SetLED(3, QTermII::LEDState::Off);
					XPRCDat.changingValue = false;
					XPRCDat.cValueIdx = 0;
				}
				break;
			default:
				try {
					if (XPRCDat.changingValue) {
						std::atoi(input.c_str()); // We don't care about the value - just using this to rougly check for int value
						XPRCDat.tempBuff[XPRCDat.cValueIdx] = input.c_str()[0];
						XPLMSetDatai(RadioData.tCodeRef, std::atoi(XPRCDat.tempBuff.c_str()));
						if (XPRCDat.cValueIdx++ == 3) {
							hc.SetLED(3, QTermII::LEDState::Off);
							XPRCDat.changingValue = false;
							XPRCDat.cValueIdx = 0;
						}
					}
				}
				catch (std::exception err) {
					XPLMDebugString(err.what());
				}
				break;
			}
		}

		sprintf(hcBuff, "Squawk: %d %s %c", RadioData.transponderCode,
											(RadioData.transponderMode == Off ? "OFF" : "TXMT"),
											(RadioData.transponderId == true ? 'T' : ' '));
		hc.WriteAt(hcBuff, 0, 0);

		if (RadioData.transponderId) {
			hc.SetLED(4, QTermII::LEDState::On);
		}
		else {
			hc.SetLED(4, QTermII::LEDState::Off);
		}

		break;
	}

	return 0.1;
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc) {
	// Give the plugin some character
	strcpy(outName, "XP Remote Control");
	strcpy(outSig, "FerraraSolutions.remote_control");
	strcpy(outDesc, "Allows remote control of aircraft functions via QTERM-II terminal.");

	// Setup data refs
	RadioData.tCodeRef = XPLMFindDataRef("sim/cockpit/radios/transponder_code");
	RadioData.tIdRef = XPLMFindDataRef("sim/cockpit/radios/transponder_id");
	RadioData.tLightRef = XPLMFindDataRef("sim/cockpit/radios/transponder_light");
	RadioData.tBrightnessRef = XPLMFindDataRef("sim/cockpit/radios/transponder_brightness");
	RadioData.tModeRef = XPLMFindDataRef("sim/cockpit/radios/transponder_mode");

	FlightData.aglRef = XPLMFindDataRef("sim/flightmodel/position/y_agl"); // Meters
	FlightData.mslRef = XPLMFindDataRef("sim/flightmodel/position/elevation"); // Meters
	FlightData.airSpeedRef = XPLMFindDataRef("sim/cockpit2/gauges/indicators/airspeed_kts_pilot"); // Knots
	FlightData.gndSpeedRef = XPLMFindDataRef("sim/flightmodel/position/groundspeed"); // Meters

	// Make our serial connection
	hc.Connect();
	if (hc.Connected()) {
		XPLMDebugString("Connected to HC on COM1!\n");

		hc.Erase();
		hc.EnableBacklight();
		hc.SetCursorMode(QTermII::BlockOffUnderlineOff);

		hc.Write("Connected!");
		hc.WriteAt("Waiting for sim...", 1, 0);
	}
	else {
		XPLMDebugString("Could not connect to HC on COM1!\n");
		XPLMDisablePlugin(XPLMFindPluginBySignature("FerraraSolutions.remote_control"));
		return 0;
	}

	// Register our flight loop
	XPLMRegisterFlightLoopCallback(&FlightLoop, 1.0, NULL);

	return 1;
}

PLUGIN_API void XPluginStop(void) {
	hc.Erase();
	hc.WriteAt("Plugin disabled", 0, 0);
	hc.Disconnect();
}

PLUGIN_API void XPluginDisable(void) {
	hc.Erase();
	hc.WriteAt("Plugin disabled", 0, 0);
	hc.Disconnect();
}

PLUGIN_API void XPluginEnable(void) {
	hc.Connect();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) {

}