#include "import_dlg.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

namespace open_edi {
namespace gui {

ImportDlg::ImportDlg(const QString& title, QWidget* parent)
    : DialogBase (parent) {
    init();
    setDialogTitle(title);
    setObjectName("ImportDialog");
}

ImportDlg::~ImportDlg() {
    delete lib_edit_;
    delete net_edit_;
}

void ImportDlg::init() {

    QWidget* central = new QWidget(this);
    QVBoxLayout* main_layout = new QVBoxLayout;
    central->setLayout(main_layout);
    setCentralWidget(central);

    QGroupBox* lib_group = new QGroupBox(this);
    QGroupBox* net_group = new QGroupBox(this);
    main_layout->addWidget(lib_group);
    main_layout->addWidget(net_group);

    QDialogButtonBox* button_box = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
    main_layout->addWidget(button_box);

    lib_group->setTitle(tr("Technology/Physical Libraries"));
    lib_edit_ = new QLineEdit(this);
    QPushButton* lib_broswer = new QPushButton(tr("Broswer"), this);
    lib_broswer->setObjectName("LibBroswer");
    connect(lib_broswer, &QAbstractButton::clicked, this, &ImportDlg::slotBroswer);

    QHBoxLayout* h_layout_2 = new QHBoxLayout;
    h_layout_2->addWidget(new QLabel(tr("Lef Files: ")));
    h_layout_2->addWidget(lib_edit_);
    h_layout_2->addWidget(lib_broswer);
    lib_group->setLayout(h_layout_2);

    net_group->setTitle(tr("Netlist"));
    net_edit_ = new QLineEdit(this);
    QPushButton* net_broswer = new QPushButton(tr("Broswer"), this);
    net_broswer->setObjectName("NetBroswer");
    connect(net_broswer, &QAbstractButton::clicked, this, &ImportDlg::slotBroswer);

    QHBoxLayout* h_layout = new QHBoxLayout;
    h_layout->addWidget(new QLabel(tr("Def Files: ")));
    h_layout->addWidget(net_edit_);
    h_layout->addWidget(net_broswer);
    net_group->setLayout(h_layout);
}

QString ImportDlg::getLefPath() {
    return lib_edit_->text();
}

QString ImportDlg::getDefPath() {
    return net_edit_->text();
}

void ImportDlg::slotBroswer() {
    QObject* obj = qobject_cast<QObject*>(sender());

    QString file_filter = "";
    if (obj->objectName() == "LibBroswer") {
        file_filter = "*.lef";
    } else if (obj->objectName() == "NetBroswer") {
        file_filter = "*.def";
    }

    QFileDialog  file_dialog(this);
    file_dialog.setOption(QFileDialog::DontUseNativeDialog);
    file_dialog.setNameFilter(file_filter);
    if(file_dialog.exec() == QDialog::Accepted){
        auto file_name = file_dialog.selectedFiles();
        if (!file_name.isEmpty()) {
            if (obj->objectName() == "LibBroswer") {
                lib_edit_->setText(file_name.at(0));
            } else if (obj->objectName() == "NetBroswer") {
                net_edit_->setText(file_name.at(0));
            }
        }
    }
}

} // namespace gui
} // namespace open_edi
