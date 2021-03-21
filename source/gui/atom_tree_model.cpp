#include "gui/atom_tree_model.h"

#include <algorithm>  // std::find

namespace mp4_manipulator {

AtomTreeModel::AtomTreeModel(QObject* parent /*= nullptr */)
    : QAbstractItemModel{parent} {}

QVariant AtomTreeModel::data(QModelIndex const& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  ModelItem* item = static_cast<ModelItem*>(index.internalPointer());
  switch (index.column()) {
    case 0:  // Name
      return item->name;
    case 1:  // Value
      if (item->value.has_value()) {
        return item->value.value();
      }
      return QVariant{};
    case 2:  // Position
      if (item->position.has_value()) {
        return item->position.value();
      }
      return QVariant{};
    case 3:  // Size
      if (item->type == ModelItem::Type::kAtom) {
        // Report size for atoms.
        QString size_string;
        assert(item->size.has_value());
        assert(item->header_size.has_value());
        // We only report the total size. We can report a breakdown involving
        // header size in future if needed.
        QTextStream(&size_string) << item->size.value();
        return size_string;
      }
      return QVariant{};
    default:
      assert(false);
      return QVariant{};
  }
}

Qt::ItemFlags AtomTreeModel::flags(QModelIndex const& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  return QAbstractItemModel::flags(index);
}

QVariant AtomTreeModel::headerData(int section, Qt::Orientation orientation,
                                   int role /* = Qt::DisplayRole */) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case 0:
        return QVariant("Name");
      case 1:
        return QVariant("Value");
      case 2:
        return QVariant("Position");
      case 3:
        return QVariant("Size");
      default:
        return QVariant("Header section needs name!");
    }
  }
  return QVariant();
}

QModelIndex AtomTreeModel::index(
    int row, int column,
    QModelIndex const& parent /* = QModelIndex() */) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  ModelItem* parent_item =
      parent.isValid() ? static_cast<ModelItem*>(parent.internalPointer())
                       : model_root_.get();
  if (row > parent_item->children.size()) {
    assert(false);  // We shouldn't get rows > size.
    return QModelIndex();
  }
  return createIndex(row, column, parent_item->children.at(row).get());
}

QModelIndex AtomTreeModel::parent(QModelIndex const& index) const {
  if (!index.isValid()) {
    return QModelIndex();
  }

  ModelItem* child = static_cast<ModelItem*>(index.internalPointer());
  ModelItem* parent = child->parent;

  if (parent == model_root_.get()) {
    // We're one level below the root.
    return QModelIndex();
  }

  // if(parent->parent == nullptr) {
  //
  //  assert(parent == model_root_.get());
  //  return createIndex(0, 0, parent);
  //}

  // Get the vector containing our parent and it's siblings.
  std::vector<std::unique_ptr<ModelItem>> const& parent_and_siblings =
      parent->parent->children;

  // A predicate to check if our unique pointers match the raw pointer of
  // parent.
  auto predicate = [&parent](std::unique_ptr<ModelItem> const& element) {
    return parent == element.get();
  };
  // Find our parent in the vector.
  auto iterator = std::find_if(parent_and_siblings.begin(),
                               parent_and_siblings.end(), predicate);
  assert(iterator != parent_and_siblings.end());
  if (iterator == parent_and_siblings.end()) {
    // This shouldn't happen (per the assert), but gracefully handle just in
    // case.
    return QModelIndex();
  }
  // Get the parent's index in the vector.
  int const parent_index = iterator - parent_and_siblings.begin();
  return createIndex(parent_index, 0, parent);
}

int AtomTreeModel::rowCount(
    QModelIndex const& parent /* = QModelIndex() */) const {
  if (model_root_ == nullptr) {
    // The model doesn't have data yet.
    return 0;
  }

  if (parent.column() > 0) {
    return 0;
  }

  if (!parent.isValid()) {
    return model_root_->children.size();
  }
  ModelItem* parent_item = static_cast<ModelItem*>(parent.internalPointer());
  return parent_item->children.size();
}

int AtomTreeModel::columnCount(
    QModelIndex const& parent /* = QModelIndex() */) const {
  if (model_root_ == nullptr) {
    // The model doesn't have data yet.
    return 0;
  }

  Q_UNUSED(parent);
  return 4;
}

void AtomTreeModel::SetAtoms(
    std::vector<std::unique_ptr<AtomOrDescriptorBase>>&& top_level_atoms) {
  // The model is about to change layout from atoms being set, notify this.
  layoutAboutToBeChanged();

  top_level_atoms_ = std::move(top_level_atoms);
  UpdateModelItems();

  // Notify that the layout has changed as a result of the atoms being set.
  layoutChanged();
}

void AtomTreeModel::UpdateModelItems() {
  // Helper function that takes a ModelItem parent and a AtomOrDescriptorBase
  // and recursively constructs and adds to the parent the appropriate model
  // TODO(bryce): this would be more readable as a non-lambda in an anonymous
  // namespace
  std::function<void(ModelItem*, AtomOrDescriptorBase const*)>
      add_atom_or_descriptor =
          [&add_atom_or_descriptor](
              ModelItem* parent,
              AtomOrDescriptorBase const* atom_or_descriptor) -> void {
    std::unique_ptr<ModelItem> current_item = std::make_unique<ModelItem>();
    current_item->underlying_item = atom_or_descriptor;
    current_item->type =
        atom_or_descriptor->GetType() == AtomOrDescriptorBase::Type::kAtom
            ? ModelItem::Type::kAtom
            : ModelItem::Type::kDescriptor;
    current_item->name = atom_or_descriptor->GetName();
    current_item->header_size = atom_or_descriptor->GetHeaderSize();
    current_item->size = atom_or_descriptor->GetSize();
    current_item->position = atom_or_descriptor->GetPositionInFile();
    current_item->parent = parent;
    // Add the fields.
    for (Field const& field : atom_or_descriptor->GetFields()) {
      std::unique_ptr<ModelItem> model_field = std::make_unique<ModelItem>();
      model_field->type = ModelItem::Type::kField;
      model_field->name = field.name;
      model_field->value = field.data;
      model_field->parent = current_item.get();
      current_item->children.push_back(std::move(model_field));
    }
    // Handle child descriptors.
    std::vector<std::unique_ptr<AtomOrDescriptorBase>> const&
        child_descriptors = atom_or_descriptor->GetChildDescriptors();
    for (size_t i = 0; i < child_descriptors.size(); ++i) {
      AtomOrDescriptorBase* child = child_descriptors.at(i).get();
      add_atom_or_descriptor(current_item.get(), child);
    }
    // Handle child atoms.
    std::vector<std::unique_ptr<AtomOrDescriptorBase>> const& child_atoms =
        atom_or_descriptor->GetChildAtoms();
    for (size_t i = 0; i < child_atoms.size(); ++i) {
      AtomOrDescriptorBase* child = child_atoms.at(i).get();
      add_atom_or_descriptor(current_item.get(), child);
    }
    parent->children.push_back(std::move(current_item));
  };

  model_root_ = std::make_unique<ModelItem>();
  for (size_t i = 0; i < top_level_atoms_.size(); ++i) {
    add_atom_or_descriptor(model_root_.get(), top_level_atoms_.at(i).get());
  }
}

}  // namespace mp4_manipulator
