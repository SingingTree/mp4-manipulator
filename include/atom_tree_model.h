#ifndef MP4_MANIPULATOR_ATOM_TREE_MODEL_H_
#define MP4_MANIPULATOR_ATOM_TREE_MODEL_H_

#include <QAbstractItemModel>

#include "atom.h"

namespace mp4_manipulator {

// TODO(bryce): this could be subclassed so that the atom and field versions
// avoid having redundant members.
struct ModelItem {
  enum class Type { kUnset = -1, kAtom = 0, kDescriptor = 1, kField = 2 };

  QString name;
  Type type{Type::kUnset};

  // Atom specific members
  std::optional<uint32_t> header_size{std::nullopt};
  std::optional<uint32_t> size{std::nullopt};
  std::optional<uint32_t> position{std::nullopt};
  // End atom specific members

  // Field specific members
  std::optional<QString> value{std::nullopt};
  // End field specific members

  ModelItem* parent{nullptr};
  std::vector<std::unique_ptr<ModelItem>> children;

  AtomOrDescriptorBase const* underlying_item = nullptr;
};

class AtomTreeModel : public QAbstractItemModel {
  // Q_OBJECT

 public:
  explicit AtomTreeModel(QObject* parent = nullptr);
  ~AtomTreeModel() override = default;

  // QAbstractItemModel overrides.
  QVariant data(QModelIndex const& index, int role) const override;
  Qt::ItemFlags flags(QModelIndex const& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column,
                    QModelIndex const& parent = QModelIndex()) const override;
  QModelIndex parent(QModelIndex const& index) const override;
  int rowCount(QModelIndex const& parent = QModelIndex()) const override;
  int columnCount(QModelIndex const& parent = QModelIndex()) const override;
  // End QAbstractItemModel overrides.

  void SetAtoms(
      std::vector<std::unique_ptr<AtomOrDescriptorBase>>&& top_level_atoms);

 private:
  // Update the model item based on the current state of the atoms.
  void UpdateModelItems();

  //   void setupModelData(QStringList const &lines, TreeItem *parent);
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> top_level_atoms_;

  // We store model items instead of directly deriving the data from the atoms.
  // This simplifies handling the different data types involved, i.e. we can
  // just reduce all atoms, descriptors, and fields to ModelItems.
  //
  // This is the (dummy) root of our tree, all the root atoms will be children
  // of this item.
  std::unique_ptr<ModelItem> model_root_;
};

}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_ATOM_TREE_MODEL_H_