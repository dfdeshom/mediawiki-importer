#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QUrl>
#include <QDebug>

/*
 * There's a whole lot of static-constness in this file since we're itterating over
 * some of these things millions of times and want to avoid creating objects over
 * and over again.
 */

#define DISAMBIGS "disambigs.txt"
#define REDIRECTS "redirects.txt"

#define SETUP_STREAM(name, mode)                \
    QFile file(name);                           \
                                                \
    if(!file.open(mode))                        \
    {                                           \
        qDebug("Could not open file.");         \
        return;                                 \
    }                                           \
                                                \
    QTextStream stream(&file);                  \
    stream.setCodec("UTF-8")


static QString &expandEntities(QString &text)
{
    static QHash<QString, QChar> entities;

    if(entities.isEmpty())
    {
        entities["aacute"] = QChar(0xe1);    entities["acirc"] = QChar(0xe2);     entities["acute"] = QChar(0xb4);
        entities["aelig"] = QChar(0xe6);     entities["agrave"] = QChar(0xe0);    entities["alefsym"] = QChar(0x2135);
        entities["alpha"] = QChar(0x3b1);    entities["amp"] = QChar(0x26);       entities["and"] = QChar(0x22a5);
        entities["ang"] = QChar(0x2220);     entities["apos"] = QChar(0x27);      entities["aring"] = QChar(0xe5);
        entities["asymp"] = QChar(0x2248);   entities["atilde"] = QChar(0xe3);    entities["auml"] = QChar(0xe4);
        entities["bdquo"] = QChar(0x201e);   entities["beta"] = QChar(0x3b2);     entities["brvbar"] = QChar(0xa6);
        entities["bull"] = QChar(0x2022);    entities["cap"] = QChar(0x2229);     entities["ccedil"] = QChar(0xe7);
        entities["cedil"] = QChar(0xb8);     entities["cent"] = QChar(0xa2);      entities["chi"] = QChar(0x3c7);
        entities["circ"] = QChar(0x2c6);     entities["clubs"] = QChar(0x2663);   entities["cong"] = QChar(0x2245);
        entities["copy"] = QChar(0xa9);      entities["crarr"] = QChar(0x21b5);   entities["cup"] = QChar(0x222a);
        entities["curren"] = QChar(0xa4);    entities["dagger"] = QChar(0x2020);  entities["darr"] = QChar(0x2193);
        entities["deg"] = QChar(0xb0);       entities["delta"] = QChar(0x3b4);    entities["diams"] = QChar(0x2666);
        entities["divide"] = QChar(0xf7);    entities["eacute"] = QChar(0xe9);    entities["ecirc"] = QChar(0xea);
        entities["egrave"] = QChar(0xe8);    entities["empty"] = QChar(0x2205);   entities["emsp"] = QChar(0x2003);
        entities["ensp"] = QChar(0x2002);    entities["epsilon"] = QChar(0x3b5);  entities["equiv"] = QChar(0x2261);
        entities["eta"] = QChar(0x3b7);      entities["eth"] = QChar(0xf0);       entities["euml"] = QChar(0xeb);
        entities["euro"] = QChar(0x20ac);    entities["exist"] = QChar(0x2203);   entities["fnof"] = QChar(0x192);
        entities["forall"] = QChar(0x2200);  entities["frac12"] = QChar(0xbd);    entities["frac14"] = QChar(0xbc);
        entities["frac34"] = QChar(0xbe);    entities["frasl"] = QChar(0x2044);   entities["gamma"] = QChar(0x3b3);
        entities["ge"] = QChar(0x2265);      entities["gt"] = QChar(0x3e);        entities["harr"] = QChar(0x2194);
        entities["hearts"] = QChar(0x2665);  entities["hellip"] = QChar(0x2026);  entities["iacute"] = QChar(0xed);
        entities["icirc"] = QChar(0xee);     entities["iexcl"] = QChar(0xa1);     entities["igrave"] = QChar(0xec);
        entities["image"] = QChar(0x2111);   entities["infin"] = QChar(0x221e);   entities["int"] = QChar(0x222b);
        entities["iota"] = QChar(0x3b9);     entities["iquest"] = QChar(0xbf);    entities["isin"] = QChar(0x2208);
        entities["iuml"] = QChar(0xef);      entities["kappa"] = QChar(0x3ba);    entities["lambda"] = QChar(0x3bb);
        entities["lang"] = QChar(0x2329);    entities["laquo"] = QChar(0xab);     entities["larr"] = QChar(0x2190);
        entities["lceil"] = QChar(0x2308);   entities["ldquo"] = QChar(0x201c);   entities["le"] = QChar(0x2264);
        entities["lfloor"] = QChar(0x230a);  entities["lowast"] = QChar(0x2217);  entities["loz"] = QChar(0x25ca);
        entities["lrm"] = QChar(0x200e);     entities["lsaquo"] = QChar(0x2039);  entities["lsquo"] = QChar(0x2018);
        entities["lt"] = QChar(0x3c);        entities["macr"] = QChar(0xaf);      entities["mdash"] = QChar(0x2014);
        entities["micro"] = QChar(0xb5);     entities["middot"] = QChar(0xb7);    entities["minus"] = QChar(0x2212);
        entities["mu"] = QChar(0x3bc);       entities["nabla"] = QChar(0x2207);   entities["nbsp"] = QChar(0xa0);
        entities["ndash"] = QChar(0x2013);   entities["ne"] = QChar(0x2260);      entities["ni"] = QChar(0x220b);
        entities["not"] = QChar(0xac);       entities["notin"] = QChar(0x2209);   entities["nsub"] = QChar(0x2284);
        entities["ntilde"] = QChar(0xf1);    entities["nu"] = QChar(0x3bd);       entities["oacute"] = QChar(0xf3);
        entities["ocirc"] = QChar(0xf4);     entities["oelig"] = QChar(0x153);    entities["ograve"] = QChar(0xf2);
        entities["oline"] = QChar(0x203e);   entities["omega"] = QChar(0x3c9);    entities["omicron"] = QChar(0x3bf);
        entities["oplus"] = QChar(0x2295);   entities["or"] = QChar(0x22a6);      entities["ordf"] = QChar(0xaa);
        entities["ordm"] = QChar(0xba);      entities["oslash"] = QChar(0xf8);    entities["otilde"] = QChar(0xf5);
        entities["otimes"] = QChar(0x2297);  entities["ouml"] = QChar(0xf6);      entities["para"] = QChar(0xb6);
        entities["part"] = QChar(0x2202);    entities["percnt"] = QChar(0x25);    entities["permil"] = QChar(0x2030);
        entities["perp"] = QChar(0x22a5);    entities["phi"] = QChar(0x3c6);      entities["pi"] = QChar(0x3c0);
        entities["piv"] = QChar(0x3d6);      entities["plusmn"] = QChar(0xb1);    entities["pound"] = QChar(0xa3);
        entities["prime"] = QChar(0x2032);   entities["prod"] = QChar(0x220f);    entities["prop"] = QChar(0x221d);
        entities["psi"] = QChar(0x3c8);      entities["quot"] = QChar(0x22);      entities["radic"] = QChar(0x221a);
        entities["rang"] = QChar(0x232a);    entities["raquo"] = QChar(0xbb);     entities["rarr"] = QChar(0x2192);
        entities["rceil"] = QChar(0x2309);   entities["rdquo"] = QChar(0x201d);   entities["real"] = QChar(0x211c);
        entities["reg"] = QChar(0xae);       entities["rfloor"] = QChar(0x230b);  entities["rho"] = QChar(0x3c1);
        entities["rlm"] = QChar(0x200f);     entities["rsaquo"] = QChar(0x203a);  entities["rsquo"] = QChar(0x2019);
        entities["sbquo"] = QChar(0x201a);   entities["scaron"] = QChar(0x161);   entities["sdot"] = QChar(0x22c5);
        entities["sect"] = QChar(0xa7);      entities["shy"] = QChar(0xad);       entities["sigma"] = QChar(0x3c3);
        entities["sigmaf"] = QChar(0x3c2);   entities["sim"] = QChar(0x223c);     entities["spades"] = QChar(0x2660);
        entities["sub"] = QChar(0x2282);     entities["sube"] = QChar(0x2286);    entities["sum"] = QChar(0x2211);
        entities["sup"] = QChar(0x2283);     entities["sup1"] = QChar(0xb9);      entities["sup2"] = QChar(0xb2);
        entities["sup3"] = QChar(0xb3);      entities["supe"] = QChar(0x2287);    entities["szlig"] = QChar(0xdf);
        entities["tau"] = QChar(0x3c4);      entities["there4"] = QChar(0x2234);  entities["theta"] = QChar(0x3b8);
        entities["thetasym"] = QChar(0x3d1); entities["thinsp"] = QChar(0x2009);  entities["thorn"] = QChar(0xfe);
        entities["tilde"] = QChar(0x2dc);    entities["times"] = QChar(0xd7);     entities["trade"] = QChar(0x2122);
        entities["uacute"] = QChar(0xfa);    entities["uarr"] = QChar(0x2191);    entities["ucirc"] = QChar(0xfb);
        entities["ugrave"] = QChar(0xf9);    entities["uml"] = QChar(0xa8);       entities["upsih"] = QChar(0x3d2);
        entities["upsilon"] = QChar(0x3c5);  entities["uuml"] = QChar(0xfc);      entities["weierp"] = QChar(0x2118);
        entities["xi"] = QChar(0x3be);       entities["yacute"] = QChar(0xfd);    entities["yen"] = QChar(0xa5);
        entities["yuml"] = QChar(0xff);      entities["zeta"] = QChar(0x3b6);     entities["zwj"] = QChar(0x200d);
        entities["zwnj"] = QChar(0x200c);
    }

    static QRegExp entity("&(\\w+);");

    int offset = 0;

    while((offset = entity.indexIn(text, offset)) != -1)
    {
        text.replace(offset, entity.matchedLength(), entities[entity.cap(1).toLower()]);
    }

    return text;
}

static QString formatTitle(QString title)
{
    expandEntities(title);
    title.replace('_', ' ');
    title = title.trimmed();

    if(!title.isEmpty())
    {
        title[0] = title[0].toUpper();
    }

    return title;
}

static QStringList extractLinks(const QString &text)
{
    static QRegExp links("\\[\\[([^\\]]+)\\]\\]");

    QStringList values;

    int offset = 0;

    while((offset = links.indexIn(text, offset)) != -1)
    {
        offset += links.matchedLength();
        QString value = links.cap(1);

        if(!value.contains(':'))
        {
            value = value.section('|', 0, 0);
            value = value.section('#', 0, 0);
            value = formatTitle(value);

            if(!value.isEmpty())
            {
                values.append(value);
            }
        }
    }

    return values;
}

static bool isRedirect(const QString &text)
{
    static const QString redirect = "#REDIRECT";
    return text.contains(redirect, Qt::CaseInsensitive);
}

static bool isDisambig(const QString &text)
{
    static const QRegExp disambig("\\{\\{\\s*disambig\\s*\\}\\}", Qt::CaseInsensitive);
    return text.contains(disambig);
}

class RedirectBuilder
{
public:
    void operator()(const QString &title, const QString &text)
    {
        if(isRedirect(text))
        {
            QStringList links = extractLinks(text);

            if(!links.isEmpty())
            {
                m_redirects[title] = links.front();
            }
        }

	else {
	  m_redirects[title] = title;
	}
    }

    static void resolveRedirects(QString &target)
    {
        QSet<QString> checked;

        while(m_redirects.contains(target) && !checked.contains(target))
        {
            checked.insert(target);
            target = m_redirects[target];
        }
    }

    void print() const
    {
        SETUP_STREAM(REDIRECTS, QIODevice::WriteOnly);

        for(QHash<QString, QString>::ConstIterator it = m_redirects.begin();
            it != m_redirects.end(); ++it)
        {
            QString source = it.key();
            QString target = it.value();
            resolveRedirects(target);

            stream << source << "|" << target << "\n";
        }
    }

    static QHash<QString, QString> m_redirects;
};

QHash<QString, QString> RedirectBuilder::m_redirects;

class Infoboxes
{
public:
    Infoboxes()
    {
        add("actor", list("actor", "actress"));
        add("book");
        add("company");
        add("film");
        add("musician", list("musician", "musical artist"));
        add("author", list("writer"));
    }

    QStringList find(const QString &text) const
    {
        QRegExp clone(m_any);

        QStringList values;

        if(text.contains(clone))
        {
            values.append(m_map[clone.cap(1).toLower()]);
        }

        return values;
    }

private:
    void add(const QString &name, const QStringList &patterns = QStringList())
    {
        static const QString prefix = "\\{\\{\\s*Infobox[_ ]";
        static const QString postfix = "\\s*\\W";

        if(patterns.isEmpty())
        {
            m_map[name] = name;
        }
        else
        {
            for(QStringList::ConstIterator it = patterns.begin(); it != patterns.end(); ++it)
            {
                m_map[*it] = name;
            }
        }

        m_any.setPattern(prefix + join(m_map.keys()) + postfix);
        m_any.setCaseSensitivity(Qt::CaseInsensitive);
    }

    QStringList list(const QString &first, const QString &second = QString::null) const
    {
        QStringList l;
        l.append(first);
        if(!second.isEmpty())
        {
            l.append(second);
        }
        return l;
    }

    QString join(const QStringList &patterns)
    {
        return "(" + patterns.join("|") + ")";
    }

    QHash<QString, QString> m_map;
    QRegExp m_any;
};

class XMLWriter
{
public:

    XMLWriter() :
        m_file("mediawiki.xml"),
        m_xml(&m_file),
        m_disambigsFile(DISAMBIGS)
    {
        m_file.open(QIODevice::WriteOnly);

        m_xml.setAutoFormatting(true);

        m_xml.writeStartDocument();
        m_xml.writeStartElement("directededge");
        m_xml.writeAttribute("version", "0.1");

        m_disambigsFile.open(QIODevice::WriteOnly);
        m_disambigsStream.setDevice(&m_disambigsFile);
        m_disambigsStream.setCodec("UTF-8");
    }

    ~XMLWriter()
    {
        m_xml.writeEndElement();
        m_xml.writeEndDocument();
    }

    void operator()(const QString &title, const QString &text)
    {
        if(title.contains(':') || isRedirect(text))
        {
            return;
        }

        if(isDisambig(text))
        {
            m_disambigsStream << title << "\n";
            return;
        }

        static const QString itemToken = "item";
        static const QString idToken = "id";
        static const QString tagToken = "tag";
        static const QString linkToken = "link";
        static const Infoboxes infoboxes;

        QStringList tags = infoboxes.find(text);
        QStringList links = extractLinks(text);

        sanitizeLinks(links);

        m_xml.writeStartElement(itemToken);
        m_xml.writeAttribute(idToken, formatTitle(title));

        for(QStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it)
        {
            m_xml.writeTextElement(tagToken, *it);
        }

        for(QStringList::ConstIterator it = links.begin(); it != links.end(); ++it)
        {
            m_xml.writeTextElement(linkToken, *it);
        }

        m_xml.writeEndElement();
    }

private:

    void sanitizeLinks(QStringList &links)
    {
        for(QStringList::Iterator it = links.begin(); it != links.end(); ++it)
        {
            RedirectBuilder::resolveRedirects(*it);
        }

        links = links.toSet().toList();
        links.sort();
    }

    QFile m_file;
    QXmlStreamWriter m_xml;

    QFile m_disambigsFile;
    QTextStream m_disambigsStream;
};

template <class PageHandler> void parse(const QString &fileName, PageHandler &handler)
{
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("Could not open file.");
        return;
    }

    QXmlStreamReader xml(&file);

    QString title;
    QString text;

    int pageCount = 0;

    static const QString pageToken = "page";
    static const QString titleToken = "title";
    static const QString textToken = "text";
    
    while(!xml.atEnd())
    {
        QXmlStreamReader::TokenType type = xml.readNext();

        if(type == QXmlStreamReader::StartElement && xml.name() == titleToken)
        {
            title = xml.readElementText();
        }
        else if(type == QXmlStreamReader::StartElement && xml.name() == textToken)
        {
            text = xml.readElementText();
        }
        else if(type == QXmlStreamReader::EndElement && xml.name() == pageToken)
        {
            handler(title, text);

            if(++pageCount % 1000 == 0)
            {
                qDebug() << QString::number(pageCount) + " pages finished.";
            }
        }
	
    }
}

static void decode(QString &value)
{
    value = QUrl::fromPercentEncoding(value.toUtf8());
}

static void disambigsToDatabase()
{

    QSqlQuery idQuery("SELECT MAX(id) FROM pages");

    if(!idQuery.next())
    {
        qDebug("Could not fetch last id.");
        return;
    }

    int id = idQuery.value(0).toInt();

    SETUP_STREAM(DISAMBIGS, QIODevice::ReadOnly);

    int count = 0;

    for(QString line = stream.readLine(); !stream.atEnd(); line = stream.readLine())
    {
        decode(line);

        QSqlQuery query;
        query.prepare("INSERT INTO pages VALUES( ?, ? )");
        query.addBindValue(++id);
        query.addBindValue(line);
        query.exec();

        if(++count % 1000 == 0)
        {
            qDebug() << count << "disambigs processed.";
        }
    }
}

static void redirectsToDatabase()
{
    SETUP_STREAM(REDIRECTS, QIODevice::ReadOnly);

    int count = 0;

    for(QString line = stream.readLine(); !stream.atEnd(); line = stream.readLine())
    {
        QStringList fields = line.split('|');

        if(fields.size() < 2)
        {
            continue;
        }

        decode(fields[0]);
        decode(fields[1]);

        QSqlQuery idQuery;
        idQuery.prepare("SELECT id FROM pages WHERE name " /* COLLATE utf8_bin */ "= ?");
        idQuery.addBindValue(fields[1]);

        if(idQuery.exec() && idQuery.next())
        {
            QSqlQuery query("INSERT INTO redirects VALUES( ?, ? )");
            query.addBindValue(fields[0]);
            query.addBindValue(idQuery.value(0).toInt());
            query.exec();
        }
        else
        {
            qDebug() << "Could not find " << fields[1] << " in database."
                     << idQuery.lastError().text();
        }

        if(++count % 1000 == 0)
        {
            qDebug() << count << "redirects processed.";
        }
    }
}

static void database()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("related_wikipedia");
    db.setUserName("scott");

    if(db.open())
    {
        disambigsToDatabase();
        redirectsToDatabase();
    }
    else
    {
        qDebug("Could not connect to database.");
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments();

    arguments.removeFirst();

    if(arguments.contains("--database"))
    {
        database();
    }
    else if(!arguments.isEmpty())
    {
        RedirectBuilder redirectBuilder;
        parse(arguments.front(), redirectBuilder);

        redirectBuilder.print();

        XMLWriter xmlWriter;
        parse(arguments.front(), xmlWriter);
    }

    return 0;
}
