#ifndef MP4_MANIPULATOR_MAINWINDOW_H_
#define MP4_MANIPULATOR_MAINWINDOW_H_

#include <QAction>
#include <QMainWindow>

#include "Ap4.h"
#include "atom_tree_model.h"

QT_FORWARD_DECLARE_CLASS(QTreeView)

namespace mp4_manipulator {

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);

  void SetAtoms(std::vector<std::unique_ptr<AtomOrDescriptorBase>>&&
                    top_level_inspected_atoms,
                std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms);

 private:
  // Helpers for setting up specific UI elements.
  void SetupMenuBar();
  void SetupTreeView();

  QMenu* file_menu_;

  // TODO(bryce): refactor these members so that we can have multiple -- allow
  // for tabbed view.
  std::vector<std::unique_ptr<AP4_Atom>> top_level_ap4_atoms_;
  AtomTreeModel* atom_tree_model_;
  QTreeView* tree_view_;

  // Begin QActions for menu bar.
  QAction* open_file_action_;
  // End QActions for menu bar.

  // Begin QActions for `tree_view_` context menu.
  // Actions to expand and collapse all items in our tree view.
  QAction* collapse_tree_action_;
  QAction* expand_tree_action_;
  // End QActions for `tree_view_` context menu.

 private slots:
  // Open a file in the UI.
  void OpenFile();

  // Show a menu when right clicking on the tree model. This menu exposes
  // functionality to manipulate the tree and its contents.
  void ShowTreeMenu(QPoint const& point);

  void DumpAtom(AP4_Atom& atom);
};

}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_MAINWINDOW_H_
