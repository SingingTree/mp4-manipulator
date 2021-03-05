#include "main_window.h"

#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QTreeView>

#include "file_utils.h"

namespace mp4_manipulator {

// cppcoreguidelines-pro-type-member-init gives false positives for members
// initialized in methods, so disable it here.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      atom_tree_model_{new AtomTreeModel{this}},
      tree_view_{new QTreeView{this}},
      open_file_action_{new QAction{"&Open file", this}},
      collapse_tree_action_{new QAction{"&Collapse all", this}},
      expand_tree_action_{new QAction{"&Expand all", this}} {
  SetupMenuBar();
  SetupTreeView();
}

void MainWindow::SetAtoms(
    std::vector<std::unique_ptr<AtomOrDescriptorBase>>&&
        top_level_inspected_atoms,
    std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms) {
  atom_tree_model_->SetAtoms(std::move(top_level_inspected_atoms));
  top_level_ap4_atoms_ = std::move(top_level_ap4_atoms);

  tree_view_->repaint();
}

void MainWindow::SetupMenuBar() {
  file_menu_ = menuBar()->addMenu("&File");
  file_menu_->addAction(open_file_action_);
  [[maybe_unused]] bool ok = connect(open_file_action_, &QAction::triggered,
                                     this, &MainWindow::OpenFile);
  assert(ok);
}

void MainWindow::SetupTreeView() {
  tree_view_->setModel(atom_tree_model_);

  // This helps optimize performance -- our rows should all be the same height
  // as they're text. If we violate this we'll need to scrap this and implement
  // size hints instead.
  tree_view_->setUniformRowHeights(true);

  // Setup the tree view menu.
  tree_view_->setContextMenuPolicy(Qt::CustomContextMenu);
  [[maybe_unused]] bool ok =
      connect(tree_view_, &QTreeView::customContextMenuRequested, this,
              &MainWindow::ShowTreeMenu);
  assert(ok);
  setCentralWidget(tree_view_);
  ok = connect(collapse_tree_action_, &QAction::triggered, tree_view_,
               &QTreeView::collapseAll);
  assert(ok);
  ok = connect(expand_tree_action_, &QAction::triggered, tree_view_,
               &QTreeView::expandAll);
  assert(ok);
  // Done setting up tree view menu.
}

void MainWindow::OpenFile() {
  QString const file_name = QFileDialog::getOpenFileName(this);
  QByteArray file_name_bytes = file_name.toLocal8Bit();
  char const* c_str_file_name = file_name_bytes.data();

  std::optional<utility::ParsedAtomHolder> possible_atoms =
      mp4_manipulator::utility::ReadAtoms(c_str_file_name);
  if (!possible_atoms.has_value()) {
    // TODO(bryce): warn on error.
    return;
  }
  mp4_manipulator::utility::ParsedAtomHolder holder{
      std::move(possible_atoms.value())};

  SetAtoms(std::move(holder.top_level_inspected_atoms),
           std::move(holder.top_level_ap4_atoms));

  tree_view_->resizeColumnToContents(0);
  tree_view_->resizeColumnToContents(1);
  tree_view_->resizeColumnToContents(2);
  tree_view_->resizeColumnToContents(3);
  tree_view_->resizeColumnToContents(4);
}

void MainWindow::ShowTreeMenu(QPoint const& point) {
  QModelIndex const index = tree_view_->indexAt(point);

  if (top_level_ap4_atoms_.empty()) {
    // Don't show the menu if we haven't got any atoms loaded.
    return;
  }

  // Create our menu on the stack for ease of deletion. If we ever put this on
  // the heap, remember to `menu->setAttribute(Qt::WA_DeleteOnClose);` so we
  // don't end up amassing menus in memory.
  QMenu menu(this);

  // Add collapse and expand actions.
  menu.addAction(collapse_tree_action_);
  menu.addAction(expand_tree_action_);
  menu.addSeparator();

  if (index.isValid()) {
    // Add item manipulation actions.
    // These are created on demand so they can reference the appropriate atoms.
    ModelItem* item = static_cast<ModelItem*>(index.internalPointer());

    AP4_Atom* ap4_atom = item->underlying_item != nullptr
                             ? item->underlying_item->GetAp4Atom()
                             : nullptr;

    QAction* dump_action = new QAction("&Dump atom", &menu);
    if (ap4_atom != nullptr) {
      // These need to be on the same thread so the connection below will use
      // a direct connection, otherwise this isn't thread safe.
      assert(dump_action->thread() == this->thread());

      [[maybe_unused]] bool ok =
          connect(dump_action, &QAction::triggered,
                  [this, ap4_atom]() { DumpAtom(*ap4_atom); });
      assert(ok);
      menu.addAction(dump_action);
    }
  }

  menu.exec(mapToGlobal(point));
}

void MainWindow::DumpAtom(AP4_Atom& atom) {
  QString const file_name = QFileDialog::getSaveFileName(this);
  QByteArray file_name_bytes = file_name.toLocal8Bit();
  char const* c_str_file_name = file_name_bytes.data();

  utility::DumpAtom(c_str_file_name, atom);
}

}  // namespace mp4_manipulator
