function QAxWidget2::Click()
{
	QAxWidget2.lineWidth++;
}

function fatLines()
{
	QAxWidget2.lineWidth = 25;
}

function thinLines()
{
	QAxWidget2.lineWidth = 1;
}

function setLineWidth(width)
{
	QAxWidget2.lineWidth = width;
}

function getLineWidth()
{
	return(QAxWidget2.lineWidth)
}
