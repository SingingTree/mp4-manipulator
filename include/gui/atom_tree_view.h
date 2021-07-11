#ifndef MP4_MANIPULATOR_ATOM_TREE_VIEW_H_
#define MP4_MANIPULATOR_ATOM_TREE_VIEW_H_

#include <QTreeView>

#include "Ap4.h"
#include "gui/atom_tree_model.h"

namespace mp4_manipulator {
// A QTreeView that also encapsulates the atom data displayed in the tree.
// This widget is what shows the contents of each tab in the UI.
class AtomTreeView : public QTreeView {
  Q_OBJECT
 public:
  AtomTreeView(std::unique_ptr<AtomHolder>&& atom_holder);

  // Shows a file dialog and then saves (dumps all) atoms to the file.
  void SaveAtoms();

 private:
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

  // Shows a file dialog and then dumps the passed atom to the file.
  void DumpAtom(AP4_Atom& atom);

  // Start processing methods.
  // These methods result in an AP4 processor running, meaning the atom tree
  // is reprocessed. Tries to ensure the atoms are well formed (no bad sizes,
  // offsets, etc.), but can be costly.
  bool RemoveAtom(Atom* atom);

  // TODO(bryce) ReplaceAtom
  // TODO(bryce) InsertAtom(figure out how to handle location)
  // End processing methods.
};
}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_ATOM_TREE_VIEW_H_