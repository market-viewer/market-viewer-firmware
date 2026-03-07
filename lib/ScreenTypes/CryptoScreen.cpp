#include "ui.h" // Import the SquareLine generated UI
#include "CryptoScreen.h"
#include "messageDisplay.h"
#include "HttpRequestManager.h"
#include "utils.h"
#include "string"
#include "graphDrawer.h"
#include "colors.h"


void CryptoScreen::render() {
    //update current price and asset name (same for simple display and normal)
    std::string formattedPrice = build_price_label(price, currency).c_str();

    String tempName = String(assetName.c_str());
    tempName.toUpperCase();
    std::string assetPlusCurrency = (tempName + " " + currency).c_str();

    //select right font size based on price label size
    const lv_font_t* correctFont = select_correct_font_size(formattedPrice, 450);
    lv_obj_set_style_text_font(ui_cryptoPriceLabel, correctFont, LV_PART_MAIN);

    //set values
    lv_label_set_text(ui_cryptoPriceLabel, formattedPrice.c_str());
    lv_label_set_text(ui_cryptoTimeFrameLabel, timeFrame.c_str());
    lv_label_set_text(ui_cryptoAssetCurrencyLabel, assetPlusCurrency.c_str());
    
    //update colors
    if (priceChange >= 0) {
        lv_obj_set_style_bg_img_src(ui_cryptoScreen, &ui_img_green_background_png, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_img_src(ui_cryptoScreen, &ui_img_red_background_png, LV_PART_MAIN);
    }

    //handle simple display and normal display
    if (simpleDisplay) {
        renderSimple();
    } else { 
        renderNormal();
    }

    //clear graph data
    lv_obj_clean(get_screen_panel_from_type(ScreenType::CRYPTO));

    //display graph data
    if (displayGraph) {
        renderGraph();
    }
}

void CryptoScreen::renderNormal() {
    //show all widgets
    lv_obj_clear_flag(ui_cryptoPriceChangeLabel, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_clear_flag(ui_cryptoAthArc, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_clear_flag(ui_cryptoAthLabel, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_clear_flag(ui_cryptoAthChangeLabel, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_clear_flag(ui_cryptoZeroLabel1, LV_OBJ_FLAG_HIDDEN);   

    //update widgets values
    std::string formattedPriceChange = build_price_change_label(priceChange).c_str();
    std::string formattedAthChange = build_price_change_label(athChange).c_str();
    
    double athBarValue = 100 + athChange;

    lv_label_set_text(ui_cryptoPriceChangeLabel, formattedPriceChange.c_str());
    lv_label_set_text(ui_cryptoAthChangeLabel, formattedAthChange.c_str());
    lv_arc_set_value(ui_cryptoAthArc, (int32_t)athBarValue);    

    if (priceChange >= 0) {
        lv_obj_set_style_bg_color(ui_cryptoPriceChangeLabel, lv_color_hex(greenColor), LV_PART_MAIN);
        lv_obj_set_style_bg_color(ui_cryptoAthArc, lv_color_hex(greenColorKnob), LV_PART_KNOB);
    } else {
        lv_obj_set_style_bg_color(ui_cryptoPriceChangeLabel, lv_color_hex(redColor), LV_PART_MAIN);
        lv_obj_set_style_bg_color(ui_cryptoAthArc, lv_color_hex(redColorKnob), LV_PART_KNOB);
    }
 
}
void CryptoScreen::renderSimple() {
    //hide widgets for simple display
    lv_obj_add_flag(ui_cryptoPriceChangeLabel, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_add_flag(ui_cryptoAthArc, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_add_flag(ui_cryptoAthLabel, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_add_flag(ui_cryptoAthChangeLabel, LV_OBJ_FLAG_HIDDEN);    
    lv_obj_add_flag(ui_cryptoZeroLabel1, LV_OBJ_FLAG_HIDDEN);    
}


void CryptoScreen::parseData(JsonObject& data) {
    price = data["price"] | 0.0f;
    priceChange = data["priceChange"] | 0.0f;
    ath = data["allTimeHigh"] | 0.0f;
    athChange = data["allTimeHighChange"] | 0.0f;
    
    // Parse graph data array manually
    graphData.clear();
    graphData.shrink_to_fit(); // Release memory

    // Parse new graph data
    JsonArray graphArray = data["graphData"].as<JsonArray>();
    if (graphArray.size() > 0) {
        // Reserve memory to prevent reallocation
        graphData.reserve(graphArray.size());
        
        for (JsonVariant value : graphArray) {
            graphData.push_back(value.as<double>());
        }        
    }
}

bool CryptoScreen::needsUpdate() {
    unsigned long refreshIntervalMillis = refreshIntervalMinutes * 60 * 1000;
    
    return (millis() - lastFetchTime >= refreshIntervalMillis);
}

void CryptoScreen::renderGraph() {
    if (graphData.empty()) return;

    lv_color_t graphColor = priceChange >= 0 ? lv_color_hex(greenColorGraph) : lv_color_hex(redColorGraph);
    draw_graph_on_canvas(ScreenType::CRYPTO, graphData, graphColor, graphType == GraphType::CANDLE);
}