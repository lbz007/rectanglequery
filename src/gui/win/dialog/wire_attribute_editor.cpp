
#include "wire_attribute_editor.h"

#include <QVBoxLayout>

namespace open_edi {
namespace gui {

WirePropertyDlg::WirePropertyDlg(open_edi::db::Object* wire, QWidget* parent) : AttributeEditorBase(wire, parent) {

    setWindowTitle("Wire");
    wire_ = static_cast<open_edi::db::Wire*>(wire);

    setMinimumWidth(500);

    int index = 0;

    QString str;

    auto inst_grid_layout_ = new QGridLayout;

    //Name
    // editor_inst_name_->setText(QString(inst_->getName().c_str()));
    // inst_grid_layout_->addWidget(new QLabel("Name:", this), index, kName);
    // inst_grid_layout_->addWidget(editor_inst_name_, index, kValue);

    // index++;

    //Location
    auto lib = open_edi::db::getTechLib();

    auto box = wire_->getBBox();
    auto llx = lib->dbuToMicrons(box.getLLX());
    auto lly = lib->dbuToMicrons(box.getLLY());
    auto urx = lib->dbuToMicrons(box.getURX());
    auto ury = lib->dbuToMicrons(box.getURY());

    str = QString::number(llx)
          + "," + QString::number(lly)
          + "," + QString::number(urx)
          + "," + QString::number(ury);

    editor_location_->setText(str);
    inst_grid_layout_->addWidget(new QLabel("Location:", this), index, kName);
    inst_grid_layout_->addWidget(editor_location_, index, kValue);
    index++;

    editor_layere_->setText(wire_->getLayer()->getName());
    inst_grid_layout_->addWidget(new QLabel("Layer:", this), index, kName);
    inst_grid_layout_->addWidget(editor_layere_, index, kValue);
    index++;

    auto net = wire_->getNet();
    auto net_name = net->getName();
    net_name_->setText(net_name.c_str());
    inst_grid_layout_->addWidget(new QLabel("Net Name:", this), index, kName);
    inst_grid_layout_->addWidget(net_name_, index, kValue);
    index++;

    inst_grid_layout_->setContentsMargins(10, 20, 10, 5);

    setLayout(inst_grid_layout_);
}

} // namespace gui
} // namespace open_edi