#ifndef MP4_MANIPULATOR_ATOM_TREE_VIEW_H_
#define MP4_MANIPULATOR_ATOM_TREE_VIEW_H_

#include <QTreeView>

#include "Ap4.h"
#include "gui/atom_tree_model.h"

namespace mp4_manipulator {
// A QTreeView that also encapsulates the atom data displayed in the tree.
class AtomTreeView : public QTreeView {
  Q_OBJECT
 public:
  AtomTreeView(std::vector<std::unique_ptr<AtomOrDescriptorBase>>&&
                   top_level_inspected_atoms,
               std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms);

 private:
  // AP4 atoms that the AtomTreeModel will store references to. These are used
  // if we wish to do things like dumping atoms via AP4.
  std::vector<std::unique_ptr<AP4_Atom>> top_level_ap4_atoms_;

  AtomTreeModel* atom_tree_model_;

  // Begin context menu actions.
  // Actions to expand and collapse all items in our tree view.
  QAction* collapse_tree_action_;
  QAction* expand_tree_action_;
  // End QActions for `tree_view_` context menu.
 private slots:
  // Show a menu when right clicking on the tree model. This menu exposes
  // functionality to manipulate the tree and its contents.
  void ShowContextMenu(QPoint const& point);

  // Shows a file dialog and then dumps the passed atom to that file.
  void DumpAtom(AP4_Atom& atom);
};
}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_ATOM_TREE_VIEW_H_