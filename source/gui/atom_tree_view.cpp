#include "gui/atom_tree_view.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

#include "parsing/file_utils.h"


namespace mp4_manipulator {
AtomTreeView::AtomTreeView(std::unique_ptr<AtomHolder>&& atom_holder)
    : atom_tree_model_{new AtomTreeModel{this}},
      collapse_tree_action_{new QAction{"&Collapse all", this}},
      expand_tree_action_{new QAction{"&Expand all", this}} {
  atom_tree_model_->SetAtoms(std::move(atom_holder));

  // Avoid warnings/footguns for virtual call in ctor, don't call on `this`,
  // explicitly use the QTreeView func.
  QTreeView::setModel(atom_tree_model_);

  // This helps optimize performance -- our rows should all be the same height
  // as they're text. If we violate this we'll need to scrap this and implement
  // size hints instead.
  setUniformRowHeights(true);

  // Setup context menu items.
  setContextMenuPolicy(Qt::CustomContextMenu);
  [[maybe_unused]] bool ok =
      connect(this, &QTreeView::customContextMenuRequested, this,
              &AtomTreeView::ShowContextMenu);
  assert(ok);
  ok = connect(collapse_tree_action_, &QAction::triggered, this,
               &QTreeView::collapseAll);
  assert(ok);
  ok = connect(expand_tree_action_, &QAction::triggered, this,
               &QTreeView::expandAll);
  assert(ok);
}

void AtomTreeView::ShowContextMenu(QPoint const& point) {
  QModelIndex const index = indexAt(point);

  if (atom_tree_model_->rowCount() <= 0) {
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

    AtomOrDescriptorBase* atom_or_descriptor = item->underlying_item;

    AP4_Atom* ap4_atom = item->underlying_item != nullptr
                             ? item->underlying_item->GetAp4Atom()
                             : nullptr;

    if (ap4_atom != nullptr) {
      QAction* dump_action = new QAction("&Dump atom", &menu);
      // These need to be on the same thread so the connection below will use
      // a direct connection, otherwise this isn't thread safe.
      assert(dump_action->thread() == this->thread());

      [[maybe_unused]] bool ok =
          connect(dump_action, &QAction::triggered,
                  [this, ap4_atom]() { DumpAtom(*ap4_atom); });
      assert(ok);
      menu.addAction(dump_action);
    }

    if (atom_or_descriptor != nullptr &&
        atom_or_descriptor->GetType() == AtomOrDescriptorBase::Type::kAtom) {
      Atom* atom = static_cast<Atom*>(atom_or_descriptor);
      QAction* remove_action = new QAction("&Remove atom", &menu);
      // These need to be on the same thread so the connection below will use
      // a direct connection, otherwise this isn't thread safe.
      assert(remove_action->thread() == this->thread());

      auto remove_lambda = [this, atom]() {
        // TODO(bryce): check the returned result. This shouldn't fail,
        // but will assert if it does so this should be handled.
        RemoveAtom(atom);
      };
      [[maybe_unused]] bool ok = connect(remove_action, &QAction::triggered, remove_lambda);
      assert(ok);
      menu.addAction(remove_action);
    }
  }

  menu.exec(mapToGlobal(point));
}

void AtomTreeView::DumpAtom(AP4_Atom& atom) {
  QString const file_name = QFileDialog::getSaveFileName(this);
  QByteArray file_name_bytes = file_name.toLocal8Bit();
  char const* c_str_file_name = file_name_bytes.data();

  utility::DumpAtom(c_str_file_name, atom);
}

void AtomTreeView::SaveAtoms() {
  QString const file_name = QFileDialog::getSaveFileName(this);

  Result<std::monostate, std::string> result = atom_tree_model_->SaveAtoms(file_name);
  if (result.IsErr()) {
    result.MarkErrorHandled();

    QMessageBox message_box;
    message_box.setText("Saving atoms failed.");
    message_box.setDetailedText(QString::fromStdString(result.GetErr()));
    message_box.exec();
  }
}

Result<std::monostate, std::string> AtomTreeView::RemoveAtom(Atom* atom) {
  return atom_tree_model_->RemoveAtom(atom);
}

}  // namespace mp4_manipulator
