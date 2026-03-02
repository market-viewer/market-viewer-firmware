#include "BaseScreen.h"
#include "AiTextScreen.h"
#include "StockScreen.h"
#include "CryptoScreen.h"
#include "ClockScreen.h"
#include "HttpRequestManager.h"
#include "messageDisplay.h"
#include "Arduino.h"
#include "TimerScreen.h"

void BaseScreen::update() {
    display_message("Fetching new data...", MessageSeverity::INFO);

    JsonDocument doc;

    if (!fetch_screen_data(position, doc)) {
        handleUpdateError("Error fetching data...");
        return;
    }

    JsonObject screenData = doc.as<JsonObject>();

    if (screenData.size() == 0) {
        handleUpdateError("Error - empty response...");
        return;
    }

    destroy_message();
    show_success_message("Screen data updated");

    parseData(screenData);
    render();

    lastFetchTime = millis();
}

void BaseScreen::handleUpdateError(String message) {
    destroy_message();
    show_error_message(message);

    // fetch failed try again in a minute
    lastFetchTime = millis();
}

BaseScreen* createScreenFromType(const String& type, JsonObject& data) {
    int position = data["position"];

    // if (type == "AI_TEXT") {
    //     return new AiTextScreen(
    //         position,
    //         data["fetchIntervalHours"],
    //         data["prompt"]
    //     );
    // }
    
    if (type == "CLOCK") {
        ClockType clockType = ClockType::ANALOG_CLOCK;
        String clockTypeStr = data["clockType"];
        if (clockTypeStr == "DIGITAL") {
            clockType = ClockType::DIGITAL_CLOCK;
        }
        
        return new ClockScreen(
            position,
            data["timezone"],
            data["timezoneCode"],
            data["timeFormat"] == "TWENTY_FOUR_HOUR",
            clockType
        );
    }
    
    if (type == "CRYPTO") {
        GraphType graphType = data["graphType"] == "CANDLE" ? GraphType::CANDLE : GraphType::LINE;
        return new CryptoScreen(
            position,
            data["assetName"],
            data["currency"],
            data["fetchInterval"] | 10,
            data["timeFrame"],
            data["displayGraph"],
            data["simpleDisplay"],
            graphType
        );
    }
    
    if (type == "STOCK") {
        GraphType graphType = data["graphType"] == "CANDLE" ? GraphType::CANDLE : GraphType::LINE;
        return new StockScreen(
            position,
            data["symbol"],
            data["fetchInterval"] | 10,
            data["timeFrame"],
            data["displayGraph"],
            data["simpleDisplay"],
            graphType
        );
    }

    if (type == "TIMER") {
        return new TimerScreen(
            position,
            data["name"] | "Timer"
        );
    }

    Serial.println("Unknown screen type: " + type);
    return nullptr;
}