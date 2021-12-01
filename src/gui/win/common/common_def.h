#ifndef DEFINE_H
#define DEFINE_H

namespace open_edi {
namespace gui {

class EDAGui {

  public:
    enum ActionType {

        kNone,

        kFileImportDesign,
        kFileSaveDesign,
        kFileSetPreference,
        kFileFindSelectObject,

        kEditUndo,
        kEditRedo,
        kEditHighLight,

        kViewZoomIn,
        kViewZoomOut,
        kViewZoomFit,
        kViewRefresh
    };
};

} // namespace gui

} // namespace open_edi

#endif
