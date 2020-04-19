#include "controllers/engine/colormapperjsproxy.h"

#include <gtest/gtest.h>

#include <QJSEngine>

#include "test/mixxxtest.h"

namespace {

QJSEngine* createScriptEngine() {
    QJSEngine* pEngine = new QJSEngine();
    QJSValue mapper = pEngine->newQMetaObject(&ColorMapperJSProxy::staticMetaObject);
    pEngine->globalObject().setProperty("ColorMapper", mapper);
    return pEngine;
}

} // namespace

class ColorMapperJSProxyTest : public MixxxTest {};

TEST_F(ColorMapperJSProxyTest, Instantiation) {
    QJSEngine* pEngine = createScriptEngine();

    // Valid instantiation
    QJSValue jsval = pEngine->evaluate(
            R"JavaScript(
           var mapper = new ColorMapper({
               '#FF0000': 1,
               '#00FF00': 2,
               '#0000FF': 3,
           });
           )JavaScript");
    EXPECT_FALSE(jsval.isError());

    // Invalid instantiation: no arguments
    pEngine->evaluate("var mapper = new ColorMapper();");
    EXPECT_TRUE(jsval.isError());

    // Invalid instantiation: invalid argument
    jsval = pEngine->evaluate("var mapper = new ColorMapper('hello');");
    EXPECT_TRUE(jsval.isError());

    // Invalid instantiation: argument is an empty object
    jsval = pEngine->evaluate("var mapper = new ColorMapper({});");
    EXPECT_TRUE(jsval.isError());

    // Invalid instantiation: argument is an empty object
    jsval = pEngine->evaluate(
            R"JavaScript(
           var mapper = new ColorMapper({
               'not a color': 1
           });
           )JavaScript");
    EXPECT_TRUE(jsval.isError());
}

TEST_F(ColorMapperJSProxyTest, GetNearestColor) {
    QJSEngine* pEngine = createScriptEngine();
    QJSValue jsval = pEngine->evaluate(
            R"JavaScript(
           var mapper = new ColorMapper({
               '#C50A08': 1,
               '#32BE44': 2,
               '#42D4F4': 3,
               '#F8D200': 4,
               '#0044FF': 5,
               '#AF00CC': 6,
               '#FCA6D7': 7,
               '#F2F2FF': 8,
           });
           /* white */
           var color1 = mapper.getNearestColor(0xFFFFFF);
           if (color1.red != 0xF2 || color1.green != 0xF2 || color1.blue != 0xFF) {
               throw Error();
           };
           /* white */
           var color2 = mapper.getNearestColor(0xDCDCDC);
           if (color2.red != 0xF2 || color2.green != 0xF2 || color2.blue != 0xFF) {
               throw Error();
           };
           /* red */
           var color3 = mapper.getNearestColor(0xFF0000);
           if (color3.red != 0xC5 || color3.green != 0x0A || color3.blue != 0x08) {
               throw Error();
           };
           /* yellow */
           var color4 = mapper.getNearestColor(0x22CC22);
           if (color4.red != 0x32 || color4.green != 0xBE || color4.blue != 0x44) {
               throw Error();
           }
           )JavaScript");
    EXPECT_FALSE(jsval.isError());
}

TEST_F(ColorMapperJSProxyTest, GetNearestValue) {
    QJSEngine* pEngine = createScriptEngine();
    QJSValue jsval = pEngine->evaluate(
            R"JavaScript(
           var mapper = new ColorMapper({
               '#C50A08': 1,
               '#32BE44': 2,
               '#42D4F4': 3,
               '#F8D200': 4,
               '#0044FF': 5,
               '#AF00CC': 6,
               '#FCA6D7': 7,
               '#F2F2FF': 8,
           });
           /* red */
           if (mapper.getValueForNearestColor(0xFF0000) != 1) {
               throw Error();
           };
           /* blue */
           if (mapper.getValueForNearestColor(0x0000AA) != 5) {
               throw Error();
           };
           /* white */
           if (mapper.getValueForNearestColor(0xFFFFFF) != 8) {
               throw Error();
           };
           )JavaScript");
    EXPECT_FALSE(jsval.isError());
}
