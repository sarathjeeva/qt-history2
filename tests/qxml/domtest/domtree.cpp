#include <qfile.h>
#include <qsplitter.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qsizepolicy.h>

#include "domtree.h"

//
// DomTree
//

DomTree::DomTree( const QString &fileName, QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    filename = fileName;

    // create menu bar
    QMenuBar *mb = new QMenuBar( this );
    QPopupMenu *pm;

    pm = new QPopupMenu( mb );
    mb->insertItem( "File", pm );
    pm->insertItem( "Reread with Namespace Processing", this, SLOT(withNSProc()) );
    pm->insertItem( "Reread without Namespace Processing", this, SLOT(withoutNSProc()) );
    pm->insertSeparator();
    pm->insertItem( "Close", this, SLOT(hide()) );

    pm = new QPopupMenu( mb );
    mb->insertItem( "Create node", pm );
    pm->insertItem( "Element", this, SLOT(createElement()) );
    pm->insertItem( "ElementNS", this, SLOT(createElementNS()) );
    pm->insertItem( "DocumentFragment", this, SLOT(createDocumentFragment()) );
    pm->insertItem( "TextNode", this, SLOT(createTextNode()) );
    pm->insertItem( "Comment", this, SLOT(createComment()) );
    pm->insertItem( "CDATASection", this, SLOT(createCDATASection()) );
    pm->insertItem( "ProcessingInstruction", this, SLOT(createProcessingInstruction()) );
    pm->insertItem( "Attribute", this, SLOT(createAttribute()) );
    pm->insertItem( "AttributeNS", this, SLOT(createAttributeNS()) );
    pm->insertItem( "EntityReference", this, SLOT(createEntityReference()) );

    // splitter with treeview
    QSplitter *split = new QSplitter( this );
    split->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    tree = new QListView( split );
    tree->addColumn( "Type" );
    tree->addColumn( "Name" );
    tree->setRootIsDecorated( TRUE );
    tree->setSorting( -1 );
    connect( tree, SIGNAL(selectionChanged(QListViewItem*)),
	    this, SLOT(selectionChanged(QListViewItem*)) );

    setContent( fileName, TRUE );

    text = new QTextView( split );
    text->setMinimumSize( 300, 400 );
    text->setTextFormat( RichText );

    split->setResizeMode( tree, QSplitter::KeepSize );
    split->setResizeMode( text, QSplitter::Stretch );

    resize( 700, 600 );
}

void DomTree::setContent( const QString &fileName, bool processNS )
{
    // read the XML file and create DOM tree
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
	return;
    }
    domTree = new QDomDocument;
    if ( !domTree->setContent( &file, processNS ) ) {
	file.close();
	return;
    }
    file.close();
    buildTree( processNS, 0, *domTree, QDomNamedNodeMap() );
}

DomTree::~DomTree()
{
    delete domTree;
}

void DomTree::buildTree( bool namespaces, QListViewItem *parentItem, const QDomNode &actNode, const QDomNamedNodeMap &attribs )
{
    static int depth = -1;
    QListViewItem *thisItem = 0;
    QDomNode node = actNode;
    depth++;

    // add attributes to the tree first
    for ( uint i=0; i< attribs.length(); i++ ) {
	if ( parentItem == 0 ) {
	    thisItem = new DomTreeItem( namespaces, attribs.item(i), tree, thisItem );
	} else {
	    thisItem = new DomTreeItem( namespaces, attribs.item(i), parentItem, thisItem );
	}
	// attributes have children
	buildTree( namespaces, thisItem, attribs.item(i).firstChild(),
		QDomNamedNodeMap() );
    }
    while ( !node.isNull() ) {
	if ( parentItem == 0 ) {
	    thisItem = new DomTreeItem( namespaces, node, tree, thisItem );
	} else {
	    thisItem = new DomTreeItem( namespaces, node, parentItem, thisItem );
	}
	if ( node.isElement() ) {
	    // add also attributes to the tree
	    buildTree( namespaces, thisItem, node.firstChild(),
		    node.toElement().attributes() );
	} else {
	    buildTree( namespaces, thisItem, node.firstChild(),
		    QDomNamedNodeMap() );
	}
	if ( depth <= 1 ) {
	    tree->setOpen( thisItem, TRUE );
	}
	node = node.nextSibling();
    }
    depth--;
}

void DomTree::selectionChanged( QListViewItem *it )
{
    text->setText( ((DomTreeItem*)it)->contentString() );
}

void DomTree::withNSProc()
{
    delete domTree;
    tree->clear();
    setContent( filename, TRUE );
}

void DomTree::withoutNSProc()
{
    delete domTree;
    tree->clear();
    setContent( filename, FALSE );
}

void DomTree::createElement()
{
    new DomTreeItem( TRUE, domTree->createElement( "element" ), tree, 0 );
}

void DomTree::createDocumentFragment()
{
    new DomTreeItem( TRUE, domTree->createDocumentFragment(), tree, 0 );
}

void DomTree::createTextNode()
{
    new DomTreeItem( TRUE, domTree->createTextNode( "text" ), tree, 0 );
}

void DomTree::createComment()
{
    new DomTreeItem( TRUE, domTree->createComment( "comment" ), tree, 0 );
}

void DomTree::createCDATASection()
{
    new DomTreeItem( TRUE, domTree->createCDATASection( "CDATA-Section" ), tree, 0 );
}

void DomTree::createProcessingInstruction()
{
    new DomTreeItem( TRUE, domTree->createProcessingInstruction( "target", "data" ), tree, 0 );
}

void DomTree::createAttribute()
{
    new DomTreeItem( TRUE, domTree->createAttribute( "attribute" ), tree, 0 );
}

void DomTree::createEntityReference()
{
    new DomTreeItem( TRUE, domTree->createEntityReference( "entity reference" ), tree, 0 );
}

void DomTree::createElementNS()
{
    new DomTreeItem( TRUE, domTree->createElementNS( "http://foo-bar", "pre:element" ), tree, 0 );
}

void DomTree::createAttributeNS()
{
    new DomTreeItem( TRUE, domTree->createAttributeNS( "http://foo-bar", "pre:attribute" ), tree, 0 );
}


//
// DomTreeItem
//

DomTreeItem::DomTreeItem( bool useNS, const QDomNode &node, QListView *parent, QListViewItem *after )
    : QListViewItem( parent, after )
{
    namespaces = useNS;
    _node = node;
    init();
}

DomTreeItem::DomTreeItem( bool useNS, const QDomNode &node, QListViewItem *parent, QListViewItem *after )
    : QListViewItem( parent, after )
{
    namespaces = useNS;
    _node = node;
    init();
}

DomTreeItem::~DomTreeItem()
{
}

void DomTreeItem::init()
{
    setText( 1, _node.nodeName() );

    switch ( _node.nodeType() ) {
	case QDomNode::ElementNode:
	    setText( 0, "Element" );
	    break;
	case QDomNode::AttributeNode:
	    setText( 0, "Attribute (no real child)" );
	    break;
	case QDomNode::CDATASectionNode:
	    setText( 0, "CDATA Section" );
	    break;
	case QDomNode::EntityReferenceNode:
	    setText( 0, "Entity Reference" );
	    break;
	case QDomNode::EntityNode:
	    setText( 0, "Entity" );
	    break;
	case QDomNode::ProcessingInstructionNode:
	    setText( 0, "PI" );
	    break;
	case QDomNode::CommentNode:
	    setText( 0, "Comment" );
	    break;
	case QDomNode::DocumentNode:
	    setText( 0, "Document" );
	    break;
	case QDomNode::DocumentTypeNode:
	    setText( 0, "Document Type" );
	    break;
	case QDomNode::DocumentFragmentNode:
	    setText( 0, "Document Fragment" );
	    break;
	case QDomNode::NotationNode:
	    setText( 0, "Notation" );
	    break;
	case QDomNode::TextNode:
	    setText( 0, "Character Data" );
	    break;
	default:
	    setText( 0, "" );
	    break;
    }
}

QString DomTreeItem::contentString()
{
    QString s;
    s += "<h3>Namespace</h3>";
    if ( _node.namespaceURI().isNull() ) {
	s += "no namespace";
	if ( !_node.prefix().isNull() )
	    s += "<br/>Error: Prefix is not Null!";
	if ( _node.nodeType()==QDomNode::ElementNode ||
		_node.nodeType()==QDomNode::AttributeNode ) {
	    if ( namespaces && _node.localName().isNull() ) {
		s += "<br/>Error: Local name is Null!";
	    } else {
		if ( namespaces ) {
		    s += "<br/><b>local name:</b> ";
		    s += _node.localName();
		} else if ( !_node.localName().isNull() ) {
		    s += "<br/>Error: Local name is not Null!";
		    s += _node.localName();
		}
	    }
	} else {
	    if ( !_node.localName().isNull() )
		s += "<br/>Error: Local name is not Null!";
	}
	s += "<br/>";
    } else {
	s += "<b>local name:</b> ";
	s += _node.localName();
	s += "<br/>";
	s += "<b>namespace URI:</b> ";
	s += _node.namespaceURI();
	s += "<br/>";
	s += "<b>prefix:</b> ";
	if ( _node.prefix().isEmpty() ) {
	    if ( _node.prefix().isNull() ) {
		s += "Error: Prefix is Null!";
	    } else {
		s += "default namespace";
	    }
	} else {
	    s += _node.prefix();
	}
	s += "<br/>";
    }
    s += "<hr/>";
    switch ( _node.nodeType() ) {
	case QDomNode::ElementNode:
	    {
		s += "<h3>Text</h3>";
		s += _node.toElement().text();
		s += "<hr/>";
		s += "<h3>Attributes</h3>";
		QDomNamedNodeMap attributes = _node.toElement().attributes();
		for ( uint i=0; i< attributes.length(); i++ ) {
		    s += attributes.item(i).toAttr().name();
		    s += " = '";
		    s += attributes.item(i).toAttr().value();
		    s += "'<br/>";
		}
	    }
	    break;
	case QDomNode::CDATASectionNode:
	    s += _node.toCDATASection().data();
	    break;
	case QDomNode::ProcessingInstructionNode:
	    s += "<b>Target: </b>";
	    s += _node.toProcessingInstruction().target();
	    s += "<br/>";
	    s += "<b>Value: </b>";
	    s += _node.toProcessingInstruction().data();
	    break;
	case QDomNode::CommentNode:
	    s += _node.toComment().data();
	    break;
	case QDomNode::DocumentNode:
	    {
		QDomDocumentType doctype = _node.toDocument().doctype();
		s += "<h2>Document Type</h2>";
		s += doctype.name();
		if ( !doctype.publicId().isNull() ) {
		    s += "<br/><b>public identifier:</b> ";
		    s += doctype.publicId();
		}
		if ( !doctype.systemId().isNull() ) {
		    s += "<br/><b>system identifier:</b> ";
		    s += doctype.systemId();
		}
		s += "<hr/><h3>Entities</h3>";
		QDomNamedNodeMap entities = doctype.entities();
		if ( entities.length() > 0 ) {
		    s += "<ul>";
		    for ( uint i=0; i< entities.length(); i++ ) {
			QDomEntity entity = entities.item(i).toEntity();
			s += "<li>";
			s += "<b>Name:</b> ";
			s += entity.nodeName();
			if ( !entity.publicId().isNull() ) {
			    s += "<br/>";
			    s += "<b>Public ID:</b> ";
			    s += entity.publicId();
			}
			if ( !entity.systemId().isNull() ) {
			    s += "<br/>";
			    s += "<b>System ID:</b> ";
			    s += entity.systemId();
			}
			if ( !entity.notationName().isNull() ) {
			    s += "<br/>";
			    s += "<b>Notation Name:</b> ";
			    s += entity.notationName();
			}
		    }
		    s += "</ul>";
		}
		s += "<hr/><h3>Notations</h3>";
		QDomNamedNodeMap notations = doctype.notations();
		if ( notations.length() > 0 ) {
		    s += "<ul>";
		    for ( uint i=0; i< notations.length(); i++ ) {
			QDomNotation notation = notations.item(i).toNotation();
			s += "<li>";
			s += "<b>Name:</b> ";
			s += notation.nodeName();
			if ( !notation.publicId().isNull() ) {
			    s += "<br/>";
			    s += "<b>Public:</b> ";
			    s += notation.publicId();
			}
			if ( !notation.systemId().isNull() ) {
			    s += "<br/>";
			    s += "<b>System:</b> ";
			    s += notation.systemId();
			}
		    }
		    s += "</ul>";
		}
	    }
	    break;
	case QDomNode::TextNode:
	    s += _node.toText().data();
	    break;
	case QDomNode::AttributeNode:
	    s += "<b>Value:</b> '";
	    s += _node.toAttr().value();
	    s += "'<br/>";
	    break;
	default:
	    break;
    }
    return s;
}
