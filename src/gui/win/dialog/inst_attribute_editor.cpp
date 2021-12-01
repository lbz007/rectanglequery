
#include "inst_attribute_editor.h"

#include <QVBoxLayout>

namespace open_edi {
namespace gui {

InstPropertyDlg::InstPropertyDlg(open_edi::db::Object* inst, QWidget* parent) : AttributeEditorBase(inst, parent) {

    setWindowTitle("Instance");
    inst_ = static_cast<open_edi::db::Inst*>(inst);

    setMinimumWidth(500);

    int index = 0;

    QString str;

    auto inst_grid_layout_ = new QGridLayout;

    //Name
    editor_inst_name_->setText(QString(inst_->getName().c_str()));
    inst_grid_layout_->addWidget(new QLabel("Name:", this), index, kName);
    inst_grid_layout_->addWidget(editor_inst_name_, index, kValue);

    index++;

    //Location

    auto lib = open_edi::db::getTechLib();

    auto box = inst_->getBox();
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
    // inst_grid_layout_->addWidget(new QLabel("Double", this), index, kType);
    index++;

    //number of pins
    str = QString::number(inst_->getMaster()->getNumOfTerms());
    editor_num_pins_->setText(str);
    inst_grid_layout_->addWidget(new QLabel("Num of Pins:", this), index, kName);
    inst_grid_layout_->addWidget(editor_num_pins_, index, kValue);
    // inst_grid_layout_->addWidget(new QLabel("Int", this), index, kType);
    index++;

    //Orient
    auto orient = inst_->getOrient();
    switch (orient) {

    case open_edi::util::Orient::kN /* North, R0 in OpenAccess */:
        editor_orient_->setText(QString("North, R0 in OpenAccess"));
        break;
    case open_edi::util::Orient::kFW /* Flipped West, MX90 in OpenAccess */:
        editor_orient_->setText(QString("Flipped West, MX90 in OpenAccess"));
        break;
    case open_edi::util::Orient::kS /* South, R180 in OpenAcces */:
        editor_orient_->setText(QString("South, R180 in OpenAcces"));
        break;
    case open_edi::util::Orient::kFE /* Flipped East, MY90 in OpenAccess */:
        editor_orient_->setText(QString("Flipped East, MY90 in OpenAccess"));
        break;
    case open_edi::util::Orient::kW /* East, R270 in OpenAccess */:
        editor_orient_->setText(QString("East, R270 in OpenAccess"));
        break;
    case open_edi::util::Orient::kFS /* Flipped South, MX in OpenAccess */:
        editor_orient_->setText(QString("Flipped South, MX in OpenAccess"));
        break;
    case open_edi::util::Orient::kE /* West, R90 in OpenAccess */:
        editor_orient_->setText(QString("West, R90 in OpenAccess"));
        break;
    case open_edi::util::Orient::kFN /* Flipped north, MY in OpenAccess */:
        editor_orient_->setText(QString("Flipped north, MY in OpenAccess"));
        break;
    default:
        printf("error: instance orient unknow\n");
        break;
    }

    inst_grid_layout_->addWidget(new QLabel("Orient:", this), index, kName);
    inst_grid_layout_->addWidget(editor_orient_, index, kValue);
    // inst_grid_layout_->addWidget(new QLabel("Orient", this), index, kType);
    index++;

    inst_grid_layout_->setContentsMargins(10, 20, 10, 5);

    setLayout(inst_grid_layout_);
}

} // namespace gui
} // namespace open_edi