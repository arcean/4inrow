#include <QtGui/QApplication>
#include <QDeclarativeContext>
#include "qmlapplicationviewer.h"
#include "engine.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    Engine engine;

    QmlApplicationViewer viewer;
    QDeclarativeContext *context = viewer.rootContext();
    context->setContextProperty("Engine", &engine);

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/4inrow/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
