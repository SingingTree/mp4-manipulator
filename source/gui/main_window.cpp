#include "gui/main_window.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QTreeView>

#include "gui/atom_tree_view.h"
#include "parsing/file_utils.h"

namespace mp4_manipulator {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      file_menu_{menuBar()->addMenu("&File")},
      tabbed_widget_{new QTabWidget{this}},
      open_file_action_{new QAction{"&Open file", this}},
      save_file_action_{new QAction{"&Save file as", this}} {
  SetupMenuBar();
  SetupTabbedWidget();
  setAcceptDrops(true);  // Accept drag and drop to open files.
}

void MainWindow::RemoveTab(int tab_index) {
  assert(tabbed_widget_ != nullptr);
  // These should always be AtomTreeViews, but it doesn't matter so don't bother
  // casting.
  QWidget* removed_widget = tabbed_widget_->widget(tab_index);
  tabbed_widget_->removeTab(tab_index);
  if (tabbed_widget_->count() == 0) {
    // Disable saving if no tabs exist.
    save_file_action_->setDisabled(true);
  }
  // When removing widgets, Qt doesn't handle deletion, so we manually delete.
  delete removed_widget;
}

void MainWindow::SetupMenuBar() {
  file_menu_->addAction(open_file_action_);
  [[maybe_unused]] bool ok = connect(open_file_action_, &QAction::triggered,
                                     this, &MainWindow::OpenFileUsingDialog);
  // Disable the action until a file is opened.
  save_file_action_->setDisabled(true);
  file_menu_->addAction(save_file_action_);
  ok = connect(save_file_action_, &QAction::triggered, this,
               &MainWindow::SaveFile);
  assert(ok);
}

void MainWindow::SetupTabbedWidget() {
  setCentralWidget(tabbed_widget_);
  tabbed_widget_->setTabsClosable(true);

  [[maybe_unused]] bool ok =
      connect(tabbed_widget_, &QTabWidget::tabCloseRequested, this,
              &MainWindow::RemoveTab);
  assert(ok);
}

void MainWindow::SetupNewTab(QString const& file_name,
                             std::unique_ptr<AtomHolder>&& atom_holder) {
  AtomTreeView* atom_tree_view = new AtomTreeView(std::move(atom_holder));

  save_file_action_->setEnabled(true);
  tabbed_widget_->addTab(atom_tree_view, file_name);
}

void MainWindow::OpenFile(QString const& file_name) {
  QByteArray file_name_bytes = file_name.toLocal8Bit();
  char const* c_str_file_name = file_name_bytes.data();

  std::optional<std::unique_ptr<AtomHolder>> possible_atoms =
      mp4_manipulator::utility::ReadAtoms(c_str_file_name);
  if (!possible_atoms.has_value()) {
    // TODO(bryce): warn on error.
    return;
  }
  std::unique_ptr<AtomHolder> holder{std::move(possible_atoms.value())};

  SetupNewTab(file_name, std::move(holder));
}

void MainWindow::OpenFileUsingDialog() {
  QString const file_name = QFileDialog::getOpenFileName(this);
  OpenFile(file_name);
}

void MainWindow::SaveFile() {
  assert(save_file_action_->isEnabled());
  assert(tabbed_widget_->count() > 0);

  QWidget* current_tab = tabbed_widget_->currentWidget();
  AtomTreeView* current_tree_view = static_cast<AtomTreeView*>(current_tab);
  // TODO(bryce): Need to show a clear error if we fail to save, to avoid
  // losing work.
  current_tree_view->SaveAtoms();
}

// Begin drag and drop handling.

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void MainWindow::dropEvent(QDropEvent* event) {
  QList<QUrl> url_list = event->mimeData()->urls();
  for (const QUrl& url : url_list) {
    QString const file_name = url.toLocalFile();
    OpenFile(file_name);
  }
}

// End drag and drop handling.

}  // namespace mp4_manipulator
