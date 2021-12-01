#ifndef EDI_GUI_APPLICATION_BUTTON_H_
#define EDI_GUI_APPLICATION_BUTTON_H_

#include <QPushButton>

namespace open_edi {
namespace gui {

class ApplicationButton : public QPushButton
{
    Q_OBJECT
public:
    ApplicationButton(QString text, QWidget* parent);
};


}
}


#endif // APPLICATION_BUTTON_H
