#include "javadocgenerator.h"

enum JavaSignatureSyntax {
    GeneratedJdocFile,
    JavadocRef
};

static QString javaSignature(const FunctionNode *func, JavaSignatureSyntax syntax)
{
    QString result;

    if (syntax == GeneratedJdocFile) {
        if (func->access() == Node::Public) {
            result += "public ";
        } else if (func->access() == Node::Protected) {
            result += "protected ";
        } else {
            result += "private ";
        }

        if (func->isConst())
            result += "final ";

        if (func->isStatic())
            result += "static ";

        result += func->returnType();
        result += ' ';
    }

    result += func->name();
    result += '(';
    for (int i = 0; i < func->parameters().count(); ++i) {
        if (i != 0)
            result += ", ";
        result += func->parameters().at(i).leftType();
        if (syntax == GeneratedJdocFile) {
            result += ' ';
            result += func->parameters().at(i).name();
        }
    }
    result += ')';

    return result;
}

JavadocGenerator::JavadocGenerator()
    : oldDevice(0), currentDepth(0)
{
}

JavadocGenerator::~JavadocGenerator()
{
}

void JavadocGenerator::initializeGenerator(const Config &config)
{
    HtmlGenerator::initializeGenerator(config);
}

void JavadocGenerator::terminateGenerator()
{
    HtmlGenerator::terminateGenerator();
}

QString JavadocGenerator::format()
{
    return "javadoc";
}

void JavadocGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    HtmlGenerator::generateTree(tree, marker);
}

QString JavadocGenerator::fileExtension(const Node *node)
{
    if (node->type() == Node::Fake) {
        return "html";
    } else {
        return "jdoc";
    }
}

void JavadocGenerator::startText(const Node *relative, CodeMarker *marker)
{
    Q_ASSERT(!oldDevice);
    oldDevice = out().device();
    Q_ASSERT(oldDevice);
    out().setString(&buffer);
    HtmlGenerator::startText(relative, marker);
}

void JavadocGenerator::endText(const Node *relative, CodeMarker *marker)
{
    HtmlGenerator::endText(relative, marker);
    Q_ASSERT(oldDevice);
    out().setDevice(oldDevice);
    oldDevice = 0;

    buffer.replace("&", "&amp;");
    buffer.replace("\"", "&quot;");
    out() << buffer;
    buffer.clear();
}

int JavadocGenerator::generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    return HtmlGenerator::generateAtom(atom, relative, marker);
}

void JavadocGenerator::generateClassLikeNode(const InnerNode *inner, CodeMarker *marker)
{
    generateIndent();
    out() << "<class name=\"" << protect(inner->name()) << "\"";
    generateDoc(inner, marker);
    out() << ">\n";

    ++currentDepth;
    foreach (Node *node, inner->childNodes()) {
        if (node->isInnerNode()) {
            generateClassLikeNode(static_cast<InnerNode *>(node), marker);
        } else {
            if (node->type() == Node::Enum) {
                generateIndent();
                out() << "<enum name=\"" << protect(node->name()) << "\"";
                generateDoc(node, marker);
                out() << "/>\n";
            } else if (node->type() == Node::Function) {
                generateIndent();
                out() << "<method name=\""
                      << protect(javaSignature(static_cast<FunctionNode *>(node),
                                               GeneratedJdocFile))
                      << "\"";
                generateDoc(node, marker);
                out() << "/>\n";
            }
        }
    }
    --currentDepth;
    
    generateIndent();
    out() << "</class>\n";
}

void JavadocGenerator::generateFakeNode(const FakeNode *fake, CodeMarker *marker)
{
    return HtmlGenerator::generateFakeNode(fake, marker);
}

void JavadocGenerator::generateText(const Text& text, const Node *relative, CodeMarker *marker)
{
    return HtmlGenerator::generateText(text, relative, marker);
}

void JavadocGenerator::generateBody(const Node *node, CodeMarker *marker)
{
    generateText(node->doc().body(), node, marker);
}

void JavadocGenerator::generateAlsoList(const Node * /* node */, CodeMarker * /* marker */)
{
    // ###
}

QString JavadocGenerator::refForNode( const Node *node )
{
    if (node->type() == Node::Function)
        return javaSignature(static_cast<const FunctionNode *>(node), JavadocRef);

    return HtmlGenerator::refForNode(node);
}

QString JavadocGenerator::linkForNode( const Node *node, const Node *relative )
{
    if (node->type() == Node::Fake) {
        return node->name();
    } else {
        if (!node->isInnerNode()) {
            return linkForNode(node->parent(), relative) + "#" + refForNode(node);
        } else {
            return node->name() + ".html";
        }
    }
}

QString JavadocGenerator::refForAtom(Atom *atom, const Node *node)
{
    return HtmlGenerator::refForAtom(atom, node);
}

/*
    Neutralize dumb functions called from HtmlGenerator.
*/
void JavadocGenerator::generateDcf(const QString & /* fileBase */, const QString & /* startPage */,
                                   const QString & /* title */, DcfSection & /* dcfRoot */)
{
}

void JavadocGenerator::generateIndex(const QString & /* fileBase */, const QString & /* url */,
                                     const QString & /* title */)
{
}

void JavadocGenerator::generateIndent()
{
    for (int i = 0; i < currentDepth; ++i)
        out() << "    ";
}

void JavadocGenerator::generateDoc(const Node *node, CodeMarker *marker)
{
    if (!node->doc().body().isEmpty()) {
        out() << " doc=\"/**\n";
        generateText(node->doc().body(), node, marker); // ### handle '*/'
        out() << " */\"";
    }
}
