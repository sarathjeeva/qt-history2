#include <qapplication.h>
#include <qlabel.h>
#include <qthread.h>


class MyWidget : public QLabel {


public:

    MyWidget() : QLabel(0,0,0) {}
    ~MyWidget() {}

protected:

    void customEvent(QCustomEvent * c) {
      qDebug("Got customevent! %d %d",c->type(),QThread::currentThread());
      setText(QString::number((int)c->data()));
    }

};

MyWidget * mywidget;

void * test_thread(void *)
{
    int n=0;
    while(1) {
	QCustomEvent * wibble=new QCustomEvent(6666);
	wibble->setData((void *)n);
	QThread::postEvent(mywidget,wibble);
	sleep(1);
	n++;
    }
}

int main(int argc,char ** argv)
{
  QApplication app(argc,argv);
  mywidget=new MyWidget();
  mywidget->show();
  qDebug("Main thread is %d",QThread::currentThread());
  pthread_t foo;
  pthread_create(&foo,0,test_thread,0);
  qDebug("Made thread %d",foo);
  app.exec();
}
