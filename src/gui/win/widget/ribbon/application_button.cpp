#include "application_button.h"

namespace open_edi {
namespace gui {

ApplicationButton::ApplicationButton(QString text, QWidget* parent)
    : QPushButton (parent)
{
    setObjectName("ApplicationButton");
    setFocusPolicy(Qt::NoFocus);
    setFlat(true);
    setText(text);
    setAutoFillBackground(true);
}

}
}
