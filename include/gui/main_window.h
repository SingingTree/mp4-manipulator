#ifndef MP4_MANIPULATOR_MAINWINDOW_H_
#define MP4_MANIPULATOR_MAINWINDOW_H_

#include <QAction>
#include <QMainWindow>

#include "Ap4.h"
#include "gui/atom_tree_model.h"

QT_FORWARD_DECLARE_CLASS(QTreeView)

namespace mp4_manipulator {

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

 private:
  void RemoveTab(int tab_index);
  // Helpers for setting up specific UI elements.
  void SetupMenuBar();
  void SetupTabbedWidget();

  void SetupNewTab(
      QString const& file_name,
      std::vector<std::unique_ptr<AtomOrDescriptorBase>>&&
          top_level_inspected_atoms,
      std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms);

  void OpenFile(QString const& file_name);

  QMenu* file_menu_;

  QTabWidget* tabbed_widget_;

  // Begin QActions for menu bar.
  QAction* open_file_action_;
  // End QActions for menu bar.

 private slots:
  // Open a file in the UI.
  void OpenFileUsingDialog();
};

}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_MAINWINDOW_H_
