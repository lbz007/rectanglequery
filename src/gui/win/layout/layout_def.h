#ifndef EDI_GUI_LAYERZ_H_
#define EDI_GUI_LAYERZ_H_

namespace open_edi {
namespace gui {

using ScaleFactor = double;

enum class LayerZ {
    kBase = 0,
    kInstanceItemZ,
    kRouting,
    kBaseMax = 1024 + kRouting,
    kHighlightItemZ,
    kSelectionItemZ
};

constexpr const char* kLiInstanceName  = "Instance";
constexpr const char* kLiPinName       = "Pin Shape";
constexpr const char* kLiNetName       = "Net";
constexpr const char* kLiDieAreaName   = "Die area";
constexpr const char* kLiHighlightName = "Highlight";
constexpr const char* kLiWireName      = "Wire";


constexpr int kSplitNum = 64;
//kSplitNum = kSplitPart * kSplitPart
constexpr int kSplitPart = 8;
} // namespace gui
} // namespace open_edi

#endif