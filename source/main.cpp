#include <QApplication>
#include <QSettings>
// Could try and move these later, but import order seems to matter for some
// reason

#include "gui/main_window.h"
#include "parsing/file_utils.h"

int main(int argc, char* argv[]) {
  // TODO(bryce): save and load window dimensions. See
  // https://doc.qt.io/qt-6/restoring-geometry.html
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "",
                     "mp4-manipulator");

  QApplication app(argc, argv);
  QCoreApplication::setApplicationVersion(QT_VERSION_STR);

  mp4_manipulator::MainWindow main_window;
  main_window.show();

  return app.exec();
}