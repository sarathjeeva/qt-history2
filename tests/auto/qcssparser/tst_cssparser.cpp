#include <QtTest/QtTest>
#include <QtXml/QtXml>

#if QT_VERSION >= 0x040200
#include "private/qcssparser_p.h"

class tst_CssParser : public QObject
{
    Q_OBJECT
private slots:
    void scanner_data();
    void scanner();
    void term_data();
    void term();
    void expr_data();
    void expr();
    void import();
    void media();
    void page();
    void ruleset();
    void selector_data();
    void selector();
    void prio();
    void escapes();
    void malformedDeclarations_data();
    void malformedDeclarations();
    void invalidAtKeywords();
    void colorValue_data();
    void colorValue();
    void styleSelector_data();
    void styleSelector();
    void specificity_data();
    void specificity();
    void specificitySort_data();
    void specificitySort();
    void rulesForNode_data();
    void rulesForNode();
};

void tst_CssParser::scanner_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    QDir d(SRCDIR);
    d.cd("testdata");
    d.cd("scanner");
    foreach (QFileInfo test, d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dir = test.absoluteFilePath() + QDir::separator();
        QTest::newRow(qPrintable(test.baseName()))
            << dir + "input"
            << dir + "output"
            ;
    }
}

static void debug(const QVector<QCss::Symbol> &symbols, int index = -1)
{
    qDebug() << "all symbols:";
    for (int i = 0; i < symbols.count(); ++i)
        qDebug() << "(" << i << "); Token:" << QCss::Scanner::tokenName(symbols.at(i).token) << "; Lexem:" << symbols.at(i).lexem();
    if (index != -1)
        qDebug() << "failure at index" << index;
}

static void debug(const QCss::Parser &p) { debug(p.symbols); }

void tst_CssParser::scanner()
{
    QFETCH(QString, input);
    QFETCH(QString, output);

    QFile inputFile(input);
    QVERIFY(inputFile.open(QIODevice::ReadOnly|QIODevice::Text));
    QVector<QCss::Symbol> symbols = QCss::Scanner::scan(QCss::Scanner::preprocess(QString::fromUtf8(inputFile.readAll())));

    QVERIFY(symbols.count() > 1);
    QVERIFY(symbols.last().token == QCss::S);
    QVERIFY(symbols.last().lexem() == QLatin1String("\n"));
    symbols.remove(symbols.count() - 1, 1);

    QFile outputFile(output);
    QVERIFY(outputFile.open(QIODevice::ReadOnly|QIODevice::Text));
    QStringList lines;
    while (!outputFile.atEnd()) {
        QString line = QString::fromUtf8(outputFile.readLine());
        if (line.endsWith(QLatin1Char('\n')))
            line.chop(1);
        lines.append(line);
    }

    if (lines.count() != symbols.count()) {
        debug(symbols);
        QCOMPARE(lines.count(), symbols.count());
    }

    for (int i = 0; i < lines.count(); ++i) {
        QStringList l = lines.at(i).split(QChar::fromLatin1('|'));
        QCOMPARE(l.count(), 2);
        const QString expectedToken = l.at(0);
        const QString expectedLexem = l.at(1);
        QString actualToken = QString::fromLatin1(QCss::Scanner::tokenName(symbols.at(i).token));
        if (actualToken != expectedToken) {
            debug(symbols, i);
            QCOMPARE(actualToken, expectedToken);
        }
        if (symbols.at(i).lexem() != expectedLexem) {
            debug(symbols, i);
            QCOMPARE(symbols.at(i).lexem(), expectedLexem);
        }
    }
}

Q_DECLARE_METATYPE(QCss::Value)

void tst_CssParser::term_data()
{
    QTest::addColumn<bool>("parseSuccess");
    QTest::addColumn<QString>("css");
    QTest::addColumn<QCss::Value>("expectedValue");

    QCss::Value val;

    val.type = QCss::Value::Percentage;
    val.variant = QVariant(double(200));
    QTest::newRow("percentage") << true << "200%" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10px");
    QTest::newRow("px") << true << "10px" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10cm");
    QTest::newRow("cm") << true << "10cm" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10mm");
    QTest::newRow("mm") << true << "10mm" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10pt");
    QTest::newRow("pt") << true << "10pt" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10pc");
    QTest::newRow("pc") << true << "10pc" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("42in");
    QTest::newRow("inch") << true << "42in" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10deg");
    QTest::newRow("deg") << true << "10deg" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10rad");
    QTest::newRow("rad") << true << "10rad" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10grad");
    QTest::newRow("grad") << true << "10grad" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10ms");
    QTest::newRow("time") << true << "10ms" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10s");
    QTest::newRow("times") << true << "10s" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10hz");
    QTest::newRow("hz") << true << "10hz" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10khz");
    QTest::newRow("khz") << true << "10khz" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10myunit");
    QTest::newRow("dimension") << true << "10myunit" << val;

    val.type = QCss::Value::Percentage;

    val.type = QCss::Value::Percentage;
    val.variant = QVariant(double(-200));
    QTest::newRow("minuspercentage") << true << "-200%" << val;

    val.type = QCss::Value::Length;
    val.variant = QString("10em");
    QTest::newRow("ems") << true << "10em" << val;

    val.type = QCss::Value::String;
    val.variant = QVariant(QString("foo"));
    QTest::newRow("string") << true << "\"foo\"" << val;

    val.type = QCss::Value::Function;
    val.variant = QVariant(QStringList() << "myFunc" << "23, (nested text)");
    QTest::newRow("function") << true << "myFunc(23, (nested text))" << val;

    QTest::newRow("function_failure") << false << "myFunction((blah)" << val;
    QTest::newRow("function_failure2") << false << "+myFunc(23, (nested text))" << val;

    val.type = QCss::Value::Color;
    val.variant = QVariant(QColor("#12ff34"));
    QTest::newRow("hexcolor") << true << "#12ff34" << val;

    val.type = QCss::Value::Color;
    val.variant = QVariant(QColor("#ffbb00"));
    QTest::newRow("hexcolor2") << true << "#fb0" << val;

    QTest::ignoreMessage(QtWarningMsg, "QColor::setNamedColor: Could not parse color '#cafebabe'");
    QTest::newRow("hexcolor_failure") << false << "#cafebabe" << val;

    val.type = QCss::Value::Uri;
    val.variant = QString("www.kde.org");
    QTest::newRow("uri1") << true << "url(\"www.kde.org\")" << val;

    QTest::newRow("uri2") << true << "url(www.kde.org)" << val;

    val.type = QCss::Value::KnownIdentifier;
    val.variant = int(QCss::Value_Italic);
    QTest::newRow("italic") << true << "italic" << val;

    val.type = QCss::Value::KnownIdentifier;
    val.variant = int(QCss::Value_Italic);
    QTest::newRow("ItaLIc") << true << "ItaLIc" << val;
}

void tst_CssParser::term()
{
    QFETCH(bool, parseSuccess);
    QFETCH(QString, css);
    QFETCH(QCss::Value, expectedValue);

    QCss::Parser parser(css);
    QCss::Value val;
    QVERIFY(parser.testTerm());
    QCOMPARE(parser.parseTerm(&val), parseSuccess);
    if (parseSuccess) {
        QCOMPARE(int(val.type), int(expectedValue.type));
        if (val.variant != expectedValue.variant) {
            qDebug() << "val.variant:" << val.variant << "expectedValue.variant:" << expectedValue.variant;
            QCOMPARE(val.variant, expectedValue.variant);
        }
    }
}

Q_DECLARE_METATYPE(QVector<QCss::Value>)

void tst_CssParser::expr_data()
{
    QTest::addColumn<bool>("parseSuccess");
    QTest::addColumn<QString>("css");
    QTest::addColumn<QVector<QCss::Value> >("expectedValues");

    QVector<QCss::Value> values;
    QCss::Value val;

    QCss::Value comma;
    comma.type = QCss::Value::TermOperatorComma;

    val.type = QCss::Value::Identifier;
    val.variant = "foo";
    values << val;
    values << comma;
    val.variant = "bar";
    values << val;
    values << comma;
    val.variant = "baz";
    values << val;
    QTest::newRow("list") << true << "foo, bar, baz" << values;
    values.clear();
}

void tst_CssParser::expr()
{
    QFETCH(bool, parseSuccess);
    QFETCH(QString, css);
    QFETCH(QVector<QCss::Value>, expectedValues);

    QCss::Parser parser(css);
    QVector<QCss::Value> values;
    QVERIFY(parser.testExpr());
    QCOMPARE(parser.parseExpr(&values), parseSuccess);
    if (parseSuccess) {
        QCOMPARE(values.count(), expectedValues.count());

        for (int i = 0; i < values.count(); ++i) {
            QCOMPARE(int(values.at(i).type), int(expectedValues.at(i).type));
            QCOMPARE(values.at(i).variant, expectedValues.at(i).variant);
        }
    }
}

void tst_CssParser::import()
{
    QCss::Parser parser("@import \"plainstring\";");
    QVERIFY(parser.testImport());
    QCss::ImportRule rule;
    QVERIFY(parser.parseImport(&rule));
    QCOMPARE(rule.href, QString("plainstring"));

    parser = QCss::Parser("@import url(\"www.kde.org\") print/*comment*/,screen;");
    QVERIFY(parser.testImport());
    QVERIFY(parser.parseImport(&rule));
    QCOMPARE(rule.href, QString("www.kde.org"));
    QCOMPARE(rule.media.count(), 2);
    QCOMPARE(rule.media.at(0), QString("print"));
    QCOMPARE(rule.media.at(1), QString("screen"));
}

void tst_CssParser::media()
{
    QCss::Parser parser("@media print/*comment*/,screen /*comment to ignore*/{ }");
    QVERIFY(parser.testMedia());
    QCss::MediaRule rule;
    QVERIFY(parser.parseMedia(&rule));
    QCOMPARE(rule.media.count(), 2);
    QCOMPARE(rule.media.at(0), QString("print"));
    QCOMPARE(rule.media.at(1), QString("screen"));
    QVERIFY(rule.styleRules.isEmpty());
}

void tst_CssParser::page()
{
    QCss::Parser parser("@page :first/*comment to ignore*/{ }");
    QVERIFY(parser.testPage());
    QCss::PageRule rule;
    QVERIFY(parser.parsePage(&rule));
    QCOMPARE(rule.selector, QString("first"));
    QVERIFY(rule.declarations.isEmpty());
}

void tst_CssParser::ruleset()
{
    {
        QCss::Parser parser("p/*foo*/{ }");
        QVERIFY(parser.testRuleset());
        QCss::StyleRule rule;
        QVERIFY(parser.parseRuleset(&rule));
        QCOMPARE(rule.selectors.count(), 1);
        QCOMPARE(rule.selectors.at(0).basicSelectors.count(), 1);
        QCOMPARE(rule.selectors.at(0).basicSelectors.at(0).elementName, QString("p"));
        QVERIFY(rule.declarations.isEmpty());
    }

    {
        QCss::Parser parser("p/*comment*/,div{ }");
        QVERIFY(parser.testRuleset());
        QCss::StyleRule rule;
        QVERIFY(parser.parseRuleset(&rule));
        QCOMPARE(rule.selectors.count(), 2);
        QCOMPARE(rule.selectors.at(0).basicSelectors.count(), 1);
        QCOMPARE(rule.selectors.at(0).basicSelectors.at(0).elementName, QString("p"));
        QCOMPARE(rule.selectors.at(1).basicSelectors.count(), 1);
        QCOMPARE(rule.selectors.at(1).basicSelectors.at(0).elementName, QString("div"));
        QVERIFY(rule.declarations.isEmpty());
    }

    {
        QCss::Parser parser(":before, :after { }");
        QVERIFY(parser.testRuleset());
        QCss::StyleRule rule;
        QVERIFY(parser.parseRuleset(&rule));
        QCOMPARE(rule.selectors.count(), 2);

        QCOMPARE(rule.selectors.at(0).basicSelectors.count(), 1);
        QCOMPARE(rule.selectors.at(0).basicSelectors.at(0).pseudoClasses.count(), 1);
        QCOMPARE(rule.selectors.at(0).basicSelectors.at(0).pseudoClasses.at(0).name, QString("before"));

        QCOMPARE(rule.selectors.at(1).basicSelectors.count(), 1);
        QCOMPARE(rule.selectors.at(1).basicSelectors.at(0).pseudoClasses.count(), 1);
        QCOMPARE(rule.selectors.at(1).basicSelectors.at(0).pseudoClasses.at(0).name, QString("after"));

        QVERIFY(rule.declarations.isEmpty());
    }

}

Q_DECLARE_METATYPE(QCss::Selector)

void tst_CssParser::selector_data()
{
    QTest::addColumn<QString>("css");
    QTest::addColumn<QCss::Selector>("expectedSelector");

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "p";
        basic.relationToNext = QCss::BasicSelector::MatchNextSelectorIfPreceeds;
        sel.basicSelectors << basic;

        basic = QCss::BasicSelector();
        basic.elementName = "div";
        sel.basicSelectors << basic;

        QTest::newRow("comment") << QString("p/* */+ div") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = QString();
        sel.basicSelectors << basic;

        QTest::newRow("any") << QString("*") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        sel.basicSelectors << basic;

        QTest::newRow("element") << QString("e") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        basic.relationToNext = QCss::BasicSelector::MatchNextSelectorIfAncestor;
        sel.basicSelectors << basic;

        basic.elementName = "f";
        basic.relationToNext = QCss::BasicSelector::NoRelation;
        sel.basicSelectors << basic;

        QTest::newRow("descendant") << QString("e f") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        basic.relationToNext = QCss::BasicSelector::MatchNextSelectorIfParent;
        sel.basicSelectors << basic;

        basic.elementName = "f";
        basic.relationToNext = QCss::BasicSelector::NoRelation;
        sel.basicSelectors << basic;

        QTest::newRow("parent") << QString("e > f") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        QCss::PseudoClass pseudo;
        pseudo.name = "first-child";
        basic.pseudoClasses.append(pseudo);
        sel.basicSelectors << basic;

        QTest::newRow("first-child") << QString("e:first-child") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        QCss::PseudoClass pseudo;
        pseudo.name = "c";
        pseudo.function = "lang";
        basic.pseudoClasses.append(pseudo);
        sel.basicSelectors << basic;

        QTest::newRow("lang") << QString("e:lang(c)") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        basic.relationToNext = QCss::BasicSelector::MatchNextSelectorIfPreceeds;
        sel.basicSelectors << basic;

        basic.elementName = "f";
        basic.relationToNext = QCss::BasicSelector::NoRelation;
        sel.basicSelectors << basic;

        QTest::newRow("precede") << QString("e + f") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        QCss::AttributeSelector attrSel;
        attrSel.name = "foo";
        basic.attributeSelectors << attrSel;
        sel.basicSelectors << basic;

        QTest::newRow("attr") << QString("e[foo]") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        QCss::AttributeSelector attrSel;
        attrSel.name = "foo";
        attrSel.value = "warning";
        attrSel.valueMatchCriterium = QCss::AttributeSelector::MatchEqual;
        basic.attributeSelectors << attrSel;
        sel.basicSelectors << basic;

        QTest::newRow("attr-equal") << QString("e[foo=\"warning\"]") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        QCss::AttributeSelector attrSel;
        attrSel.name = "foo";
        attrSel.value = "warning";
        attrSel.valueMatchCriterium = QCss::AttributeSelector::MatchContains;
        basic.attributeSelectors << attrSel;
        sel.basicSelectors << basic;

        QTest::newRow("attr-contains") << QString("e[foo~=\"warning\"]") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        QCss::AttributeSelector attrSel;
        attrSel.name = "lang";
        attrSel.value = "en";
        attrSel.valueMatchCriterium = QCss::AttributeSelector::MatchBeginsWith;
        basic.attributeSelectors << attrSel;
        sel.basicSelectors << basic;

        QTest::newRow("attr-contains") << QString("e[lang|=\"en\"]") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "div";

        QCss::AttributeSelector attrSel;
        attrSel.name = "class";
        attrSel.valueMatchCriterium = QCss::AttributeSelector::MatchContains;
        attrSel.value = "warning";
        basic.attributeSelectors.append(attrSel);

        attrSel.value = "foo";
        basic.attributeSelectors.append(attrSel);

        sel.basicSelectors << basic;

        QTest::newRow("class") << QString("div.warning.foo") << sel;
    }

    {
        QCss::Selector sel;
        QCss::BasicSelector basic;

        basic.elementName = "e";
        basic.ids << "myid";
        sel.basicSelectors << basic;

        QTest::newRow("id") << QString("e#myid") << sel;
    }
}

void tst_CssParser::selector()
{
    QFETCH(QString, css);
    QFETCH(QCss::Selector, expectedSelector);

    QCss::Parser parser(css);
    QVERIFY(parser.testSelector());
    QCss::Selector selector;
    QVERIFY(parser.parseSelector(&selector));

    QCOMPARE(selector.basicSelectors.count(), expectedSelector.basicSelectors.count());
    for (int i = 0; i < selector.basicSelectors.count(); ++i) {
        const QCss::BasicSelector sel = selector.basicSelectors.at(i);
        const QCss::BasicSelector expectedSel = expectedSelector.basicSelectors.at(i);
        QCOMPARE(sel.elementName, expectedSel.elementName);
        QCOMPARE(int(sel.relationToNext), int(expectedSel.relationToNext));

        QCOMPARE(sel.pseudoClasses.count(), expectedSel.pseudoClasses.count());
        for (int i = 0; i < sel.pseudoClasses.count(); ++i) {
            QCOMPARE(sel.pseudoClasses.at(i).name, expectedSel.pseudoClasses.at(i).name);
            QCOMPARE(sel.pseudoClasses.at(i).function, expectedSel.pseudoClasses.at(i).function);
        }

        QCOMPARE(sel.attributeSelectors.count(), expectedSel.attributeSelectors.count());
        for (int i = 0; i < sel.attributeSelectors.count(); ++i) {
            QCOMPARE(sel.attributeSelectors.at(i).name, expectedSel.attributeSelectors.at(i).name);
            QCOMPARE(sel.attributeSelectors.at(i).value, expectedSel.attributeSelectors.at(i).value);
            QCOMPARE(int(sel.attributeSelectors.at(i).valueMatchCriterium), int(expectedSel.attributeSelectors.at(i).valueMatchCriterium));
        }
    }
}

void tst_CssParser::prio()
{
    {
        QCss::Parser parser("!important");
        QVERIFY(parser.testPrio());
    }
    {
        QCss::Parser parser("!impOrTAnt");
        QVERIFY(parser.testPrio());
    }
    {
        QCss::Parser parser("!\"important\"");
        QVERIFY(!parser.testPrio());
        QCOMPARE(parser.index, 0);
    }
    {
        QCss::Parser parser("!importbleh");
        QVERIFY(!parser.testPrio());
        QCOMPARE(parser.index, 0);
    }
}

void tst_CssParser::escapes()
{
    QCss::Parser parser("\\hello");
    parser.test(QCss::IDENT);
    QCOMPARE(parser.lexem(), QString("hello"));
}

void tst_CssParser::malformedDeclarations_data()
{
    QTest::addColumn<QString>("css");

    QTest::newRow("1") << QString("p { color:green }");
    QTest::newRow("2") << QString("p { color:green; color }  /* malformed declaration missing ':', value */");
    QTest::newRow("3") << QString("p { color:red;   color; color:green }  /* same with expected recovery */");
    QTest::newRow("4") << QString("p { color:green; color: } /* malformed declaration missing value */");
    QTest::newRow("5") << QString("p { color:red;   color:; color:green } /* same with expected recovery */");
    QTest::newRow("6") << QString("p { color:green; color{;color:maroon} } /* unexpected tokens { } */");
    QTest::newRow("7") << QString("p { color:red;   color{;color:maroon}; color:green } /* same with recovery */");
}

void tst_CssParser::malformedDeclarations()
{
    QFETCH(QString, css);
    QCss::Parser parser(css);
    QVERIFY(parser.testRuleset());
    QCss::StyleRule rule;
    QVERIFY(parser.parseRuleset(&rule));

    QCOMPARE(rule.selectors.count(), 1);
    QCOMPARE(rule.selectors.at(0).basicSelectors.count(), 1);
    QCOMPARE(rule.selectors.at(0).basicSelectors.at(0).elementName, QString("p"));

    QVERIFY(rule.declarations.count() >= 1);
    QCOMPARE(int(rule.declarations.last().propertyId), int(QCss::Color));
    QCOMPARE(rule.declarations.last().values.count(), 1);
    QCOMPARE(int(rule.declarations.last().values.at(0).type), int(QCss::Value::Identifier));
    QCOMPARE(rule.declarations.last().values.at(0).variant.toString(), QString("green"));
}

void tst_CssParser::invalidAtKeywords()
{
    QCss::Parser parser(""
    "@three-dee {"
    "  @background-lighting {"
    "    azimuth: 30deg;"
    "    elevation: 190deg;"
    "  }"
    "  h1 { color: red }"
    "}"
    "h1 { color: blue }");

    QCss::StyleSheet sheet;
    QVERIFY(parser.parse(&sheet));

    QCOMPARE(sheet.styleRules.count(), 1);
    QCss::StyleRule rule = sheet.styleRules.at(0);

    QCOMPARE(rule.selectors.count(), 1);
    QCOMPARE(rule.selectors.at(0).basicSelectors.count(), 1);
    QCOMPARE(rule.selectors.at(0).basicSelectors.at(0).elementName, QString("h1"));

    QCOMPARE(rule.declarations.count(), 1);
    QCOMPARE(int(rule.declarations.at(0).propertyId), int(QCss::Color));
    QCOMPARE(rule.declarations.at(0).values.count(), 1);
    QCOMPARE(int(rule.declarations.at(0).values.at(0).type), int(QCss::Value::Identifier));
    QCOMPARE(rule.declarations.at(0).values.at(0).variant.toString(), QString("blue"));
}

Q_DECLARE_METATYPE(QColor)

void tst_CssParser::colorValue_data()
{
    QTest::addColumn<QString>("css");
    QTest::addColumn<QColor>("expectedColor");

    QTest::newRow("identifier") << "color: black" << QColor("black");
    QTest::newRow("string") << "color: \"green\"" << QColor("green");
    QTest::newRow("hexcolor") << "color: #12af0e" << QColor(0x12, 0xaf, 0x0e);
    QTest::newRow("functional1") << "color: rgb(21, 45, 73)" << QColor(21, 45, 73);
    QTest::newRow("functional2") << "color: rgb(100%, 0%, 100%)" << QColor(0xff, 0, 0xff);
}

void tst_CssParser::colorValue()
{
    QFETCH(QString, css);
    QFETCH(QColor, expectedColor);

    QCss::Parser parser(css);
    QCss::Declaration decl;
    QVERIFY(parser.parseNextDeclaration(&decl));
    const QColor col = decl.colorValue();
    QVERIFY(col.isValid());
    QCOMPARE(col, expectedColor);
}

class DomStyleSelector : public QCss::StyleSelector
{
public:
    inline DomStyleSelector(const QDomDocument &doc, const QCss::StyleSheet &sheet)
        : doc(doc)
    {
        styleSheets.append(sheet);
    }

    virtual bool hasNodeName(NodePtr node, const QString& name) const { return reinterpret_cast<QDomElement *>(node.ptr)->tagName() == name; }
    virtual QString attribute(NodePtr node, const QString &name) const { return reinterpret_cast<QDomElement *>(node.ptr)->attribute(name); }
    virtual bool hasAttribute(NodePtr node, const QString &name) const { return reinterpret_cast<QDomElement *>(node.ptr)->hasAttribute(name); }
    virtual bool hasAttributes(NodePtr node) const { return reinterpret_cast<QDomElement *>(node.ptr)->hasAttributes(); }

    virtual bool isNullNode(NodePtr node) const {
        return reinterpret_cast<QDomElement *>(node.ptr)->isNull();
    }
    virtual NodePtr parentNode(NodePtr node) {
        NodePtr parent;
        parent.ptr = new QDomElement(reinterpret_cast<QDomElement *>(node.ptr)->parentNode().toElement());
        return parent;
    }
    virtual NodePtr duplicateNode(NodePtr node) {
        NodePtr n;
        n.ptr = new QDomElement(*reinterpret_cast<QDomElement *>(node.ptr));
        return n;
    }
    virtual NodePtr previousSiblingNode(NodePtr node) {
        NodePtr sibling;
        sibling.ptr = new QDomElement(reinterpret_cast<QDomElement *>(node.ptr)->previousSiblingElement());
        return sibling;
    }
    virtual void freeNode(NodePtr node) {
        delete reinterpret_cast<QDomElement *>(node.ptr);
    }

private:
    QDomDocument doc;
};

Q_DECLARE_METATYPE(QDomDocument);

void tst_CssParser::styleSelector_data()
{
    QTest::addColumn<bool>("match");
    QTest::addColumn<QString>("selector");
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("elementToCheck");

    QTest::newRow("plain") << true << QString("p") << QString("<p />") << QString();
    QTest::newRow("noplain") << false << QString("bar") << QString("<p />") << QString();

    QTest::newRow("class") << true << QString(".foo") << QString("<p class=\"foo\" />") << QString();
    QTest::newRow("noclass") << false << QString(".bar") << QString("<p class=\"foo\" />") << QString();

    QTest::newRow("attrset") << true << QString("[justset]") << QString("<p justset=\"bar\" />") << QString();
    QTest::newRow("notattrset") << false << QString("[justset]") << QString("<p otherattribute=\"blub\" />") << QString();

    QTest::newRow("attrmatch") << true << QString("[foo=bar]") << QString("<p foo=\"bar\" />") << QString();
    QTest::newRow("noattrmatch") << false << QString("[foo=bar]") << QString("<p foo=\"xyz\" />") << QString();

    QTest::newRow("contains") << true << QString("[foo~=bar]") << QString("<p foo=\"baz bleh bar\" />") << QString();
    QTest::newRow("notcontains") << false << QString("[foo~=bar]") << QString("<p foo=\"test\" />") << QString();

    QTest::newRow("beingswith") << true << QString("[foo|=bar]") << QString("<p foo=\"bar-bleh\" />") << QString();
    QTest::newRow("notbeingswith") << false << QString("[foo|=bar]") << QString("<p foo=\"bleh-bar\" />") << QString();

    QTest::newRow("attr2") << true << QString("[bar=foo]") << QString("<p bleh=\"bar\" bar=\"foo\" />") << QString();

    QTest::newRow("universal1") << true << QString("*") << QString("<p />") << QString();

    QTest::newRow("universal3") << false << QString("*[foo=bar]") << QString("<p foo=\"bleh\" />") << QString();
    QTest::newRow("universal4") << true << QString("*[foo=bar]") << QString("<p foo=\"bar\" />") << QString();

    QTest::newRow("universal5") << false << QString("[foo=bar]") << QString("<p foo=\"bleh\" />") << QString();
    QTest::newRow("universal6") << true << QString("[foo=bar]") << QString("<p foo=\"bar\" />") << QString();

    QTest::newRow("universal7") << true << QString(".charfmt1") << QString("<p class=\"charfmt1\" />") << QString();

    QTest::newRow("id") << true << QString("#blub") << QString("<p id=\"blub\" />") << QString();
    QTest::newRow("noid") << false << QString("#blub") << QString("<p id=\"other\" />") << QString();

    QTest::newRow("childselector") << true << QString("parent > child")
                                   << QString("<parent><child /></parent>")
                                   << QString("parent/child");

    QTest::newRow("nochildselector2") << false << QString("parent > child")
                                   << QString("<child><parent /></child>")
                                   << QString("child/parent");

    QTest::newRow("nochildselector3") << false << QString("parent > child")
                                   << QString("<parent><intermediate><child /></intermediate></parent>")
                                   << QString("parent/intermediate/child");

    QTest::newRow("childselector2") << true << QString("parent[foo=bar] > child")
                                   << QString("<parent foo=\"bar\"><child /></parent>")
                                   << QString("parent/child");

    QTest::newRow("nochildselector4") << false << QString("parent[foo=bar] > child")
                                   << QString("<parent><child /></parent>")
                                   << QString("parent/child");

    QTest::newRow("nochildselector5") << false << QString("parent[foo=bar] > child")
                                   << QString("<parent foo=\"bar\"><parent><child /></parent></parent>")
                                   << QString("parent/parent/child");

    QTest::newRow("childselectors") << true << QString("grandparent > parent > child")
                                   << QString("<grandparent><parent><child /></parent></grandparent>")
                                   << QString("grandparent/parent/child");

    QTest::newRow("descendant") << true << QString("grandparent child")
                                   << QString("<grandparent><parent><child /></parent></grandparent>")
                                   << QString("grandparent/parent/child");

    QTest::newRow("nodescendant") << false << QString("grandparent child")
                                   << QString("<other><parent><child /></parent></other>")
                                   << QString("other/parent/child");

    QTest::newRow("descendant2") << true << QString("grandgrandparent grandparent child")
                                   << QString("<grandgrandparent><inbetween><grandparent><parent><child /></parent></grandparent></inbetween></grandgrandparent>")
                                   << QString("grandgrandparent/inbetween/grandparent/parent/child");

    QTest::newRow("combined") << true << QString("grandparent parent > child")
                              << QString("<grandparent><inbetween><parent><child /></parent></inbetween></grandparent>")
                              << QString("grandparent/inbetween/parent/child");

    QTest::newRow("combined2") << true << QString("grandparent > parent child")
                              << QString("<grandparent><parent><inbetween><child /></inbetween></parent></grandparent>")
                              << QString("grandparent/parent/inbetween/child");

    QTest::newRow("combined3") << true << QString("grandparent > parent child")
                              << QString("<grandparent><parent><inbetween><child /></inbetween></parent></grandparent>")
                              << QString("grandparent/parent/inbetween/child");

    QTest::newRow("nocombined") << false << QString("grandparent parent > child")
                              << QString("<inbetween><parent><child /></parent></inbetween>")
                              << QString("inbetween/parent/child");

    QTest::newRow("nocombined2") << false << QString("grandparent parent > child")
                              << QString("<parent><child /></parent>")
                              << QString("parent/child");

    QTest::newRow("previoussibling") << true << QString("p1 + p2")
                                     << QString("<p1 /><p2 />")
                                     << QString("p2");

    QTest::newRow("noprevioussibling") << false << QString("p2 + p1")
                                     << QString("<p1 /><p2 />")
                                     << QString("p2");

    QTest::newRow("ancestry_firstmismatch") << false << QString("parent child[foo=bar]")
                                            << QString("<parent><child /></parent>")
                                            << QString("parent/child");
}

void tst_CssParser::styleSelector()
{
    QFETCH(bool, match);
    QFETCH(QString, selector);
    QFETCH(QString, xml);
    QFETCH(QString, elementToCheck);

    QString css = QString("%1 { background-color: green }").arg(selector);
    QCss::Parser parser(css);
    QCss::StyleSheet sheet;
    QVERIFY(parser.parse(&sheet));

    QDomDocument doc;
    xml.prepend("<!DOCTYPE test><test>");
    xml.append("</test>");
    QVERIFY(doc.setContent(xml));

    DomStyleSelector testSelector(doc, sheet);

    QDomElement e = doc.documentElement();
    if (elementToCheck.isEmpty()) {
        e = e.firstChildElement();
    } else {
        QStringList path = elementToCheck.split(QLatin1Char('/'));
        do {
            e = e.namedItem(path.takeFirst()).toElement();
        } while (!path.isEmpty());
    }
    QVERIFY(!e.isNull());
    QCss::StyleSelector::NodePtr n;
    n.ptr = &e;
    QVector<QCss::Declaration> decls = testSelector.declarationsForNode(n);

    if (match) {
        QCOMPARE(decls.count(), 1);
        QCOMPARE(int(decls.at(0).propertyId), int(QCss::BackgroundColor));
        QCOMPARE(decls.at(0).values.count(), 1);
        QCOMPARE(int(decls.at(0).values.at(0).type), int(QCss::Value::Identifier));
        QCOMPARE(decls.at(0).values.at(0).variant.toString(), QString("green"));
    } else {
        QVERIFY(decls.isEmpty());
    }
}

void tst_CssParser::specificity_data()
{
    QTest::addColumn<QString>("selector");
    QTest::addColumn<int>("specificity");

    QTest::newRow("universal") << QString("*") << 0;

    QTest::newRow("elements+pseudos1") << QString("foo") << 1;
    QTest::newRow("elements+pseudos2") << QString("foo *[blah]") << 1 + (1 * 0x10);

    // should strictly speaking be '2', but we don't support pseudo-elements yet,
    // only pseudo-classes
    QTest::newRow("elements+pseudos3") << QString("li:first-line") << 1 + (1 * 0x10);

    QTest::newRow("elements+pseudos4") << QString("ul li") << 2;
    QTest::newRow("elements+pseudos5") << QString("ul ol+li") << 3;
    QTest::newRow("elements+pseudos6") << QString("h1 + *[rel=up]") << 1 + (1 * 0x10);

    QTest::newRow("elements+pseudos7") << QString("ul ol li.red") << 3 + (1 * 0x10);
    QTest::newRow("elements+pseudos8") << QString("li.red.level") << 1 + (2 * 0x10);
    QTest::newRow("id") << QString("#x34y") << 1 * 0x100;
}

void tst_CssParser::specificity()
{
    QFETCH(QString, selector);

    QString css = QString("%1 { }").arg(selector);
    QCss::Parser parser(css);
    QCss::StyleSheet sheet;
    QVERIFY(parser.parse(&sheet));

    QCOMPARE(sheet.styleRules.count(), 1);
    QCOMPARE(sheet.styleRules.at(0).selectors.count(), 1);

    QTEST(sheet.styleRules.at(0).selectors.at(0).specificity(), "specificity");
}

void tst_CssParser::specificitySort_data()
{
    QTest::addColumn<QString>("firstSelector");
    QTest::addColumn<QString>("secondSelector");
    QTest::addColumn<QString>("xml");

    QTest::newRow("universal1") << QString("*") << QString("p") << QString("<p />");
    QTest::newRow("attr") << QString("p") << QString("p[foo=bar]") << QString("<p foo=\"bar\" />");
    QTest::newRow("id") << QString("p") << QString("#hey") << QString("<p id=\"hey\" />");
    QTest::newRow("id2") << QString("[id=hey]") << QString("#hey") << QString("<p id=\"hey\" />");
    QTest::newRow("class") << QString("p") << QString(".hey") << QString("<p class=\"hey\" />");
}

void tst_CssParser::specificitySort()
{
    QFETCH(QString, firstSelector);
    QFETCH(QString, secondSelector);
    QFETCH(QString, xml);

    firstSelector.append(" { color: green; }");
    secondSelector.append(" { color: red; }");

    QDomDocument doc;
    xml.prepend("<!DOCTYPE test><test>");
    xml.append("</test>");
    QVERIFY(doc.setContent(xml));

    for (int i = 0; i < 2; ++i) {
        QString css;
        if (i == 0)
            css = firstSelector + secondSelector;
        else
            css = secondSelector + firstSelector;

        QCss::Parser parser(css);
        QCss::StyleSheet sheet;
        QVERIFY(parser.parse(&sheet));

        DomStyleSelector testSelector(doc, sheet);

        QDomElement e = doc.documentElement().firstChildElement();
        QCss::StyleSelector::NodePtr n;
        n.ptr = &e;
        QVector<QCss::Declaration> decls = testSelector.declarationsForNode(n);

        QCOMPARE(decls.count(), 2);

        QCOMPARE(int(decls.at(0).propertyId), int(QCss::Color));
        QCOMPARE(decls.at(0).values.count(), 1);
        QCOMPARE(int(decls.at(0).values.at(0).type), int(QCss::Value::Identifier));
        QCOMPARE(decls.at(0).values.at(0).variant.toString(), QString("green"));

        QCOMPARE(int(decls.at(1).propertyId), int(QCss::Color));
        QCOMPARE(decls.at(1).values.count(), 1);
        QCOMPARE(int(decls.at(1).values.at(0).type), int(QCss::Value::Identifier));
        QCOMPARE(decls.at(1).values.at(0).variant.toString(), QString("red"));
    }
}

void tst_CssParser::rulesForNode_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("css");
    QTest::addColumn<int>("pseudoState");
    QTest::addColumn<int>("declCount");
    QTest::addColumn<QString>("value0");
    QTest::addColumn<QString>("value1");

    QTest::newRow("universal1") << QString("<p/>") << QString("* { color: red }") 
                                << (int)QCss::Enabled << 1 << "red" << "";

    QTest::newRow("basic") << QString("<p/>") << QString("p:enabled { color: red; bg:blue; }")
        << (int)QCss::Enabled << 2 << "red" << "blue";

    QTest::newRow("single") << QString("<p/>")
        << QString("p:enabled { color: red; } *:hover { color: white }")
        << (int)QCss::Hover << 1 << "white" << "";

    QTest::newRow("multisel") << QString("<p/>")
        << QString("p:enabled { color: red; } p:hover { color: gray } *:hover { color: white } ")
        << (int)QCss::Hover << 2 << "white" << "gray";

    QTest::newRow("multisel2") << QString("<p/>")
        << QString("p:enabled { color: red; } p:hover:focus { color: gray } *:hover { color: white } ")
        << int(QCss::Hover|QCss::Focus) << 1 << "gray" << "";

    QTest::newRow("multisel3-diffspec") << QString("<p/>")
        << QString("p:enabled { color: red; } p:hover:focus { color: gray } *:hover { color: white } ")
        << int(QCss::Hover) << 2 << "white" << "gray";
}

void tst_CssParser::rulesForNode()
{
    QFETCH(QString, xml);
    QFETCH(QString, css);
    QFETCH(int, pseudoState);
    QFETCH(int, declCount);
    QFETCH(QString, value0);
    QFETCH(QString, value1);

    QDomDocument doc;
    xml.prepend("<!DOCTYPE test><test>");
    xml.append("</test>");
    QVERIFY(doc.setContent(xml));

    QCss::Parser parser(css);
    QCss::StyleSheet sheet;
    QVERIFY(parser.parse(&sheet));

    DomStyleSelector testSelector(doc, sheet);
    QDomElement e = doc.documentElement().firstChildElement();
    QCss::StyleSelector::NodePtr n;
    n.ptr = &e;
    QVector<QCss::StyleRule> rules = testSelector.styleRulesForNode(n);

    QVector<QCss::Declaration> decls;
    for (int i = 0; i < rules.count(); i++) {
        if ((rules.at(i).selectors.at(0).pseudoState() & pseudoState) == pseudoState)
            decls += rules.at(i).declarations;
    }
    
    QVERIFY(decls.count() == declCount);

    if (declCount > 0)
        QCOMPARE(decls.at(0).values.at(0).variant.toString(), value0);
    if (declCount > 1)
        QCOMPARE(decls.at(1).values.at(0).variant.toString(), value1);
}

QTEST_MAIN(tst_CssParser)
#include "tst_cssparser.moc"
#else
QTEST_NOOP_MAIN
#endif
