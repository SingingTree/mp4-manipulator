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
      open_file_action_{new QAction{"&Open file", this}} {
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
  // When removing widgets, Qt doesn't handle deletion, so we manually delete.
  delete removed_widget;
}

void MainWindow::SetupMenuBar() {
  file_menu_->addAction(open_file_action_);
  [[maybe_unused]] bool ok = connect(open_file_action_, &QAction::triggered,
                                     this, &MainWindow::OpenFileUsingDialog);
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

void MainWindow::SetupNewTab(
    QString const& file_name,
    std::vector<std::unique_ptr<AtomOrDescriptorBase>>&&
        top_level_inspected_atoms,
    std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms) {
  AtomTreeView* atom_tree_view = new AtomTreeView(
      std::move(top_level_inspected_atoms), std::move(top_level_ap4_atoms));

  tabbed_widget_->addTab(atom_tree_view, file_name);
}

void MainWindow::OpenFile(QString const& file_name) {
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

  SetupNewTab(file_name, std::move(holder.top_level_inspected_atoms),
              std::move(holder.top_level_ap4_atoms));
}

void MainWindow::OpenFileUsingDialog() {
  QString const file_name = QFileDialog::getOpenFileName(this);
  OpenFile(file_name);
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
