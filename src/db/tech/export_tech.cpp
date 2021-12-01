/* @file  export_tech.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/tech/export_tech.h"

#include <unistd.h>

#include "db/core/db.h"
#include "db/tech/tech.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;

ExportTechLef::ExportTechLef(const char* tech_name) {
    if (!tech_name) return;

    std::string old_tech_name = tech_name;
    old_tech_name += ".old";
    std::ifstream ifs(tech_name, std::ios::in);
    if (ifs.is_open()) {
        ifs.close();
        ifs.open(old_tech_name, std::ios::in);
        if (ifs.good()) {
            ifs.close();
            unlink(old_tech_name.c_str());
        } else {
            ifs.close();
        }
        rename(tech_name, old_tech_name.c_str());
    }
    tech_lib_ = getTechLib();
    io_manager_.open(tech_name, "w");
}

ExportTechLef::~ExportTechLef() {
    io_manager_.close();
}

UInt32 ExportTechLef::getTechPrecision() {
    if (tech_precision_ != 0) {
        return tech_precision_;
    }
    UInt32 tech_dbu = tech_lib_->getUnits()->getLengthFactor();
    while (tech_dbu) {
        if (tech_dbu > 1) ++tech_precision_;
        tech_dbu = tech_dbu / 10;
    }
    if (tech_precision_ == 0) tech_precision_ = 1;

    return tech_precision_;
}

UInt32 ExportTechLef::getCurPrecision() { return cur_precision_; }

UInt32 ExportTechLef::setPrecision(UInt32 prec) {
    if (prec > 0) {
        cur_precision_ = prec;
        ofs_.setf(std::ios::fixed, std::ios::floatfield);
        return ofs_.precision(prec);
    }
    return default_precision_;
}

UInt32 ExportTechLef::resetPrecision() {
    ofs_.unsetf(std::ios::fixed | std::ios::floatfield);
    return ofs_.precision(default_precision_);
}

void ExportTechLef::exportAll() {
    exportVersion();
    exportNameCaseSensitive();
    exportBusBit();
    exportDividerChar();
    exportUseMinSpacing();
    exportClearanceMeasure();

    exportUnits();
    exportMGrid();

    exportAlias();
    exportPropertyDefinitions();
    exportFixedMask();
    exportMaxViaStack();
    exportSite();
    exportAllLayers();
    exportViaMaster();
    exportViaRule();
    exportNDR();
    exportCells();
    exportExtensions();
}

void ExportTechLef::exportVersion() {
    if (tech_lib_->getVersion() != 0.0) {
        io_manager_.write("VERSION %g ;\n", tech_lib_->getVersion());
    }
}

void ExportTechLef::exportNameCaseSensitive() {
    if (tech_lib_->getCaseSensitive()) {
        io_manager_.write("NAMESCASESENSITIVE ON ;\n");
    }
}

void ExportTechLef::exportBusBit() {
    const char* str_value = tech_lib_->getBusBitsName();
    if (strcmp(str_value, "")) {
        io_manager_.write("BUSBITCHARS \"%s\" ;\n", str_value);
    }
}

void ExportTechLef::exportDividerChar() {
    const char* str_value = tech_lib_->getDividerName();
    if (strcmp(str_value, "")) {
        io_manager_.write("DIVIDERCHAR \"%s\" ;\n", str_value);
    }
}

void ExportTechLef::exportUnits() {
    io_manager_.write("\n");
    Units* units = tech_lib_->getUnits();
    if (!units) return;
    units->printLEF(io_manager_);
}

void ExportTechLef::exportMGrid() {
    double dbl_value = tech_lib_->dbuToMicrons(tech_lib_->getManuGrids());
    io_manager_.write("MANUFACTURINGGRID %g ;\n\n", dbl_value);
}

void ExportTechLef::exportUseMinSpacing() {
    std::string str_value = "OFF";
    if (tech_lib_->getHasOBSUseMinSpacing()) {
        if (tech_lib_->getOBSUseMinSpacing()) {
            str_value = "ON";
        }
        io_manager_.write("USEMINSPACING OBS %s ;\n", str_value.c_str());
    }
    if (tech_lib_->getHasPinUseMinSpacing()) {
        if (tech_lib_->getPinUseMinSpacing()) {
            str_value = "ON";
        }
        io_manager_.write("USEMINSPACING PIN %s ;\n", str_value.c_str());
    }
}

void ExportTechLef::exportClearanceMeasure() {
    if (!tech_lib_->getHasClearanceMeasure()) {
        return;
    }
    std::string str_value = "EUCLIDEAN";
    if (tech_lib_->getClearanceMeasureUseMaxxy()) {
        str_value = "MAXXY";
    }
    io_manager_.write("CLEARANCEMEASURE %s ;\n", str_value.c_str());
}

void ExportTechLef::exportAlias() {}

void ExportTechLef::exportPropertyDefinitions() {
    io_manager_.write("PROPERTYDEFINITIONS\n");
    ObjectId vobj_id =
        tech_lib_->getPropertyDefinitionVectorId();
    if (vobj_id == 0) return;
    IdArray* arr_ptr = Object::addr<IdArray>(vobj_id);
    if (!arr_ptr) return;
    for (IdArray::iterator iter = arr_ptr->begin(); 
            iter != arr_ptr->end(); iter++) {
        ObjectId obj_id = (*iter);
        if (!obj_id) return;
        PropertyDefinition* obj_data =
            Object::addr<PropertyDefinition>(obj_id);
        if (!obj_data) continue;
        if (obj_data->getPropOriginType() == kPropDef) continue;
        obj_data->printLEF(io_manager_);
    }
    io_manager_.write("END PROPERTYDEFINITIONS\n\n");
}

void ExportTechLef::exportFixedMask() {}

void ExportTechLef::exportMaxViaStack() {}

void ExportTechLef::exportSite() {
    ObjectId vobj_id = tech_lib_->getSiteVectorId();
    if (vobj_id == 0) return;
    IdArray *arr_ptr = Object::addr<IdArray>(vobj_id);
    if (!arr_ptr) return;

    for (IdArray::iterator iter = arr_ptr->begin(); iter != arr_ptr->end(); ++iter) {
        ObjectId obj_id = (*iter);
        if (!obj_id) continue;
        Site* obj_data = Object::addr<Site>(obj_id);
        if (!obj_data) continue;
        obj_data->printLEF(io_manager_);
    }
}

void ExportTechLef::exportImplantLayer(Layer* layer) {
    ImplantLayerRule* rule = layer->getImplantLayerRule();
    if (!rule) return;
    io_manager_.write("LAYER %s\n", layer->getName());
    io_manager_.write("    TYPE  IMPLANT ;\n");
    if (layer->getWidth()) {
        io_manager_.write("    WIDTH %g ;\n",
            tech_lib_->dbuToMicrons(layer->getWidth()));
    }
    ArrayObject<ObjectId> *array = rule->getSpacingLists();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        ImplantSpacing* sp = Object::addr<ImplantSpacing>(*iter);
            io_manager_.write("    SPACING %g ",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()));
            if (sp->getLayer2Id() > 0) {
                io_manager_.write("LAYER %s ",
                    tech_lib_->getLayer(sp->getLayer2Id())->getName());
            }
            io_manager_.write(";\n");
        }
    exportLayerProperty(layer);
    io_manager_.write("END %s\n\n", layer->getName());
}

void ExportTechLef::exportMinArea(const Layer* layer) {
    Layer::minAreaIter iter(layer->getMinAreaRuleId());
    MinArea* area;
    for (; area = iter.getNext();) {
        io_manager_.write("    AREA %g ",
            tech_lib_->areaDBUToMicrons(area->getArea()));
        if (area->getMaskNum()) {
            io_manager_.write("MASK %d ", area->getMaskNum());
        }
        if (area->isExceptMinWidth()) {
            io_manager_.write("    EXCEPTMINWIDTH %g \n",
                tech_lib_->dbuToMicrons(area->getMinWidth()));
        } else if (area->isExceptEdgeLength()) {
            io_manager_.write("    EXCEPTEDGELENGTH ");
            if (area->isExceptMinMaxEdgeLength()) {
                io_manager_.write("%g %g",
                    tech_lib_->dbuToMicrons(area->getExceptMinEdgeLength()),
                    tech_lib_->dbuToMicrons(area->getExceptMaxEdgeLength()));
            } else {
                io_manager_.write("%g ",
                    tech_lib_->dbuToMicrons(area->getExceptEdgeLength()));
            }
            io_manager_.write("\n");
            if (area->isExceptMinSize()) {
                io_manager_.write("    EXCEPTMINSIZE ");
                UInt32 size = area->getExceptMinSizeSize();
                for (UInt32 ii = 0; ii < size; ++ii) {
                    auto width_pair = area->getExceptMinSizePair(ii);
                    io_manager_.write("%g %g ",
                        tech_lib_->dbuToMicrons(width_pair->data_first),
                        tech_lib_->dbuToMicrons(width_pair->data_second));
                }
                io_manager_.write("\n");
            }
            if (area->isExceptStep()) {
                io_manager_.write("    EXCEPTSTEP %g %g\n",
                    tech_lib_->dbuToMicrons(area->getExceptStepLength1()),
                    tech_lib_->dbuToMicrons(area->getExceptStepLength2()));
            }
        } else if (area->isRectWidth()) {
            io_manager_.write("    RECTWIDTH %g\n",
                tech_lib_->dbuToMicrons(area->getRectWidth()));
        } else if (area->isExceptRectangle()) {
            io_manager_.write("    EXCEPTRECTANGLE  ;\n");
        } else if (area->isLayer()) {
            Layer* trim = area->getLayer();
            UInt32 overlap = area->getOverlap();
            io_manager_.write("    LAYER %s ", trim->getName());
            if (overlap) {
                io_manager_.write("OVERLAP %d ;\n", overlap);
            }
        }
        io_manager_.write(" ;\n");
    }
}

void ExportTechLef::exportMinSize(const Layer* layer) {
    RoutingLayerRule* r = layer->getRoutingLayerRule();
    if (r) {
        ArrayObject<ObjectId> *array = r->getMinSizes();
        if (array == nullptr)
            return;
        for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter)
        {
            MinSize *m = Object::addr<MinSize>(*iter);
            if (!m)
                return;
            io_manager_.write("   MINSIZE ");
            if (m->isRectOnly())
                io_manager_.write("RECTONLY ");
            UInt32 num = m->getMinSizeNum();
            for (UInt32 ii = 0; ii < num; ++ii)
            {
                io_manager_.write("%g %g ",
                    tech_lib_->dbuToMicrons(m->getWidth(ii)),
                    tech_lib_->dbuToMicrons(m->getLength(ii)));
            }
            io_manager_.write(";\n");
        }
    }
}

void ExportTechLef::exportMinEnclosedArea(const RoutingLayerRule* layer) {
    ArrayObject<ObjectId> *array = layer->getMinEnclAreas();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        MinEnclArea* mea = Object::addr<MinEnclArea>(*iter);
        io_manager_.write("    MINENCLOSEDAREA %g ",
            tech_lib_->areaDBUToMicrons(mea->getArea()));
        if (mea->getWidth()) {
            io_manager_.write("WIDTH %g ",
                tech_lib_->dbuToMicrons(mea->getWidth()));
        }
        io_manager_.write(";\n");
    }
}

void ExportTechLef::exportMinStep(const RoutingLayerRule* layer) {
    ArrayObject<ObjectId> *array = layer->getMinSteps();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        MinStep* ms = Object::addr<MinStep>(*iter);
        io_manager_.write("    MINSTEP %g ",
            tech_lib_->dbuToMicrons(ms->getMinStepLength()));
        if (ms->isMaxEdges()) {
            io_manager_.write("MAXEDGES %d ", ms->getMaxEdges());
            // TODO:
            io_manager_.write(";\n");
        } else {
            if (ms->isInsideCorner())
                io_manager_.write("INSIDECORNER ");
            else if (ms->isOutsideCorner())
                io_manager_.write("OUTSIDECORNER ");
            else if (ms->isStep())
                io_manager_.write("STEP ");
            if (ms->getMaxLength()) {
                io_manager_.write("LENGTHSUM %g ",
                    tech_lib_->dbuToMicrons(ms->getMaxLength()));
            }
            io_manager_.write(";\n");
        }
    }
}

void ExportTechLef::exportProtrusionWidth(const RoutingLayerRule* layer) {
    ArrayObject<ObjectId> *array = layer->getProtrusionRules();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        ProtrusionRule* rule = Object::addr<ProtrusionRule>(*iter);
        if (rule->isLength()) {
            io_manager_.write("    PROTRUSIONWIDTH %g LENGTH %g WIDTH %g ;\n",
                tech_lib_->dbuToMicrons(rule->getWidth1()),
                tech_lib_->dbuToMicrons(rule->getLength()),
                tech_lib_->dbuToMicrons(rule->getWidth2()));
        } else if (rule->isWidth()) {
        }
    }
}

void ExportTechLef::exportRoutingSpacing(RoutingLayerRule* r) {
    ArrayObject<ObjectId> *array = r->getSpacings();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        RoutingSpacing* sp = Object::addr<RoutingSpacing>(*iter);
        if (!sp) return;
        if (sp->isRange()) {
            io_manager_.write("    SPACING %g RANGE %g %g ",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                tech_lib_->dbuToMicrons(sp->getRangeMinWidth()),
                tech_lib_->dbuToMicrons(sp->getRangeMaxWidth()));
            if (sp->isRangeUseLengthThres()) {
                io_manager_.write("USELENGTHTHRESHOLD ");
            } else if (sp->isRangeInfluence()) {
                io_manager_.write("INFLUENCE %g ",
                    tech_lib_->dbuToMicrons(sp->getRangeInfluenceValue()));
                if (sp->isRangeInfluenceRange()) {
                    io_manager_.write("RANGE %g %g ",
                        tech_lib_->dbuToMicrons(sp->getRangeStubMinWidth()),
                        tech_lib_->dbuToMicrons(sp->getRangeStubMaxWidth()));
                }
            } else if (sp->isRangeRange()) {
                io_manager_.write("RANGE %g %g ",
                    tech_lib_->dbuToMicrons(sp->getRangeRangeMinWidth()),
                    tech_lib_->dbuToMicrons(sp->getRangeRangeMaxWidth()));
            }
            io_manager_.write(" ;\n");
        } else if (sp->isLengthThreshold()) {
            io_manager_.write("    SPACING %g LENGTHTHRESHOLD %g ",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                tech_lib_->dbuToMicrons(sp->getLenThresMaxLength()));
            if (sp->isLengthThresholdRange()) {
                io_manager_.write("RANGE %g %g ",
                    tech_lib_->dbuToMicrons(sp->getLenThresMinWidth()),
                    tech_lib_->dbuToMicrons(sp->getLenThresMaxWidth()));
            }
            io_manager_.write(";\n");
        } else if (sp->isEndOfLine()) {
            io_manager_.write("    SPACING %g ENDOFLINE %g WITHIN %g ",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                tech_lib_->dbuToMicrons(sp->getEOLWidth()),
                tech_lib_->dbuToMicrons(sp->getEOLWithin()));
            if (sp->getEOLPRLEdgeSpace()) {
                io_manager_.write("PARALLELEDGE %g WITHIN %g %s",
                    tech_lib_->dbuToMicrons(sp->getEOLPRLEdgeSpace()),
                    tech_lib_->dbuToMicrons(sp->getEOLPRLEdgeWithin()),
                    (sp->isEOLTwoEdges() ? " TWOEDGES " : " "));
            }
            io_manager_.write(";\n");
        } else if (sp->isSameNet()) {
            io_manager_.write("    SPACING %g SAMENET %s;\n",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                (sp->isSameNetPGOnly() ? "PGONLY " : " "));
        } else if (sp->isNotchLength()) {
            if (sp->isLef58NotchLength()) {
                io_manager_.write("    PROPERTY LEF58_SPACING \"SPACING %g NOTCHLENGTH %g ",
                    tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                    tech_lib_->dbuToMicrons(sp->getNotchLengthMinNotchLength()));
                if (sp->getNotchLengthLowExclSpacing() ||
                    sp->getNotchLengthHighExclSpacing()) {
                    io_manager_.write("EXCEPWITHIN %g %g ",
                        tech_lib_->dbuToMicrons(sp->getNotchLengthLowExclSpacing()),
                        tech_lib_->dbuToMicrons(sp->getNotchLengthHighExclSpacing()));
                }
                if (sp->isNotchLengthWithin()) {
                    io_manager_.write("WITHIN %g SPANLENGTH %g ",
                        tech_lib_->dbuToMicrons(sp->getNotchLengthWithin()),
                        tech_lib_->dbuToMicrons(sp->getNotchLengthSpanLength()));
                }
                if (sp->isNotchLengthWidth() ||
                    sp->isNotchLengthConcaveEnds()) {
                    io_manager_.write("%s%g ",
                        (sp->isNotchLengthWidth() ? "WIDTH " : "CONCAVEENDS "),
                        tech_lib_->dbuToMicrons(sp->getNotchLengthSideNotchWidth()));
                }
                if (sp->isNotchLengthNotchWidth()) {
                    io_manager_.write("NOTCHWIDTH %g ",
                        tech_lib_->dbuToMicrons(sp->getNotchLengthNotchWidth()));
                }
                io_manager_.write(";\" ;\n");
            } else {
                io_manager_.write("    SPACING %g NOTCHLENGTH %g ;\n",
                    tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                    tech_lib_->dbuToMicrons(sp->getNotchLengthMinNotchLength()));
            }
        } else if (sp->isEndOfNotchWidth()) {
            io_manager_.write("    SPACING %g ENDOFNOTCHWIDTH %g "
                                "NOTCHSPACING %g NOTCHLENGTH %g ;\n",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                tech_lib_->dbuToMicrons(sp->getEONWidth()),
                tech_lib_->dbuToMicrons(sp->getEONSpacing()),
                tech_lib_->dbuToMicrons(sp->getEONLength()));
        } else if (sp->isEOLPerp()) {
            io_manager_.write("    PROPERTY LEF58_SPACING \"SPACING %g "
                                "EOLPERPENDICULAR %g %g ;\" ;\n",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                tech_lib_->dbuToMicrons(sp->getEOLPerpEolWidth()),
                tech_lib_->dbuToMicrons(sp->getEOLPerpPerpWidth()));
        } else if (sp->isArea()) {
            io_manager_.write("    PROPERTY LEF58_SPACING \"SPACING"
                                " %g AREA %g ;\" ;\n",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                tech_lib_->areaDBUToMicrons(sp->getArea()));
        } else if (sp->isTrimLayerSpacing()) {
        } else if (sp->isSameMask()) {
            io_manager_.write("    PROPERTY LEF58_SPACING \"SPACING"
                                " %g SAMEMASK ;\" ;\n",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()));
        } else if (sp->isWrongDir()) {
            io_manager_.write("    PROPERTY LEF58_SPACING \"SPACING"
                                " %g WRONGDIRECTION ",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()));
            if (sp->getWrongDirEolWidth()) {
                io_manager_.write("NOEOL %g ",
                    tech_lib_->dbuToMicrons(sp->getWrongDirEolWidth()));
            }
            if (sp->getWrongDirPRL()) {
                io_manager_.write("PRL %g ",
                    tech_lib_->dbuToMicrons(sp->getWrongDirPRL()));
            }
            if (sp->getWrongDirLength()) {
                io_manager_.write("LENGTH %g ",
                    tech_lib_->dbuToMicrons(sp->getWrongDirLength()));
            }
            io_manager_.write(";\" ;\n");
        } else if (sp->isNotchSpan()) {
            io_manager_.write("    PROPERTY LEF58_SPACING \"SPACING %g NOTCHSPAN"
                            " %g NOTCHSPACING %g EXCEPTNOTCHLENGTH %g ;\" ;\n",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()),
                tech_lib_->dbuToMicrons(sp->getNotchSpanSpan()),
                tech_lib_->dbuToMicrons(sp->getNotchSpanSpacing()),
                tech_lib_->dbuToMicrons(sp->getNotchSpanLength()));
        } else if (sp->isConvexCorners()) {
        } else {
            io_manager_.write("    SPACING %g ;\n",
                tech_lib_->dbuToMicrons(sp->getMinSpacing()));
        }

    }
}

/**
SPACINGTABLE
PARALELLRUNLENGTH AND INFLUENCE
*/
void ExportTechLef::exportSpacingTable(const RoutingLayerRule* r) {
    std::string two_space = "  ";
    std::string four_space = "    ";
    std::string end = " ; \n";
    std::string line = " \n";
    UInt32 i = 0;
    UInt32 j = 0;

    // PARALLELRUNLENGTH
    ArrayObject<ObjectId> *array = r->getWidthSpTbls();
    if (array == nullptr) {
        return;
    } else {
        for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
            WidthSpTbl* edi_sp = Object::addr<WidthSpTbl>(*iter);
            UInt32 prl_dim = edi_sp->getPRLDim();
            io_manager_.write(four_space + "SPACINGTABLE" + line);
            if (edi_sp->isPRLWidth()) {  // PARALLELRUNLENGTH
                io_manager_.write(four_space + two_space + " PARALLELRUNLENGTH ");
                for (i = 0; i < prl_dim; i++) {
                    io_manager_.write("%g ", tech_lib_->dbuToMicrons(edi_sp->getPRL(i)));
                }
            } else if (edi_sp->isTwoWidths()) {  // TWOWIDTHS
                io_manager_.write(four_space + two_space + " TWOWIDTHS ");
            }

            io_manager_.write(line);

            UInt32 width_dim = edi_sp->getWidthDim();
            for (i = 0; i < width_dim; i++) {
                io_manager_.write(four_space + four_space + " WIDTH ");
                io_manager_.write("%g",
                    tech_lib_->dbuToMicrons(edi_sp->getWidth(i)));

                if (edi_sp->isTwoWidths()) {  // TWOWIDTHS
                    if (edi_sp->hasWidthPRL(i))
                        io_manager_.write(" PRL %g",
                            tech_lib_->dbuToMicrons(edi_sp->getPRL(i)));
                    else
                        io_manager_.write(four_space + four_space);
                }

                io_manager_.write(four_space);

                for (j = 0; j < prl_dim; j++) {
                    io_manager_.write("%g ",
                        tech_lib_->dbuToMicrons(edi_sp->getSpacing(i, j)));
                }

                if (i != width_dim - 1)
                    io_manager_.write(line);
                else
                    io_manager_.write(end);
            }

        }
    }

    // INFLUENCE
    array = r->getInfluenceSpTbls();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        InfluenceSpTbl* edi_inf = Object::addr<InfluenceSpTbl>(*iter);
        io_manager_.write(four_space + "SPACINGTABLE" + line);
        io_manager_.write(four_space + two_space + "INFLUENCE" + line);

        UInt32 row_num = edi_inf->getRowNum();

        for (i = 0; i < row_num; i++) {
            UInt32 width = edi_inf->getWidth(i);
            io_manager_.write(four_space + four_space + " WIDTH ");
            io_manager_.write("%g WITHIN %g SPACING %g",
                tech_lib_->dbuToMicrons(edi_inf->getWidth(i)),
                tech_lib_->dbuToMicrons(edi_inf->getWithin(i)),
                tech_lib_->dbuToMicrons(edi_inf->getSpacing(i)));
            if (i != row_num - 1)
                io_manager_.write(line);
            else
                io_manager_.write(end);
        }

    }
}

void ExportTechLef::exportMinCut(const RoutingLayerRule* r) {
    std::string four_space = "    ";
    std::string end = " ; \n";

    ArrayObject<ObjectId> *array = r->getMinCuts();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        MinCut* min_cut = Object::addr<MinCut>(*iter);
        io_manager_.write("%sMINIMUMCUT %d",
            four_space.c_str(), min_cut->getNumCuts());
        io_manager_.write(" WIDTH %g",
            tech_lib_->dbuToMicrons(min_cut->getWidth()));

        if (min_cut->getCutWithin() > 0) {
            io_manager_.write(" WITHIN %g",
                tech_lib_->dbuToMicrons(min_cut->getCutWithin()));
        }

        if (min_cut->isFromAbove()) {
            io_manager_.write(" FROMABOVE ");
        } else if (min_cut->isFromBelow()) {
            io_manager_.write(" FROMBELOW ");
        }

        if (min_cut->getLength() > 0) {
            io_manager_.write(" LENGTH %g",
                tech_lib_->dbuToMicrons(min_cut->getLength()));
        }

        if (min_cut->getLengthWithin() > 0) {
            io_manager_.write(" WITHIN %g",
                tech_lib_->dbuToMicrons(min_cut->getLengthWithin()));
        }

        io_manager_.write(end);
    }
}

void ExportTechLef::exportLayerAntennaModel(const Layer* layer) {
    int model_num = kMaxOxideNum;
    int epsilon = 0.00001;
    for (UInt32 ii = 1; ii < model_num; ++ii) {
        const AntennaModel* model = layer->getAntennaModel(ii);
        if (!model || !model->isSet()) continue;
        io_manager_.write("    ANTENNAMODEL OXIDE%d ;\n", ii);
        if (model->getAreaRatio() > epsilon) {
            io_manager_.write("    ANTENNAAREARATIO %g ;\n",
                model->getAreaRatio());
        }
        if (model->isDiffAreaRatioPWL()) {
            io_manager_.write("    ANTENNADIFFAREARATIO PWL (");
            for (int kk = 0; kk < model->getDiffAreaRatioPWLSize(); ++kk) {
                auto pwl = model->getDiffAreaRatioPWL(kk);
                io_manager_.write(" ( %g %g ) ",
                    pwl->data_first, pwl->data_second);
            }
            io_manager_.write(") ;\n");
        } else if (model->getDiffAreaRatio() > epsilon) {
            io_manager_.write("    ANTENNADIFFAREARATIO %g ;\n",
                model->getDiffAreaRatio());
        }
        if (!model->isDefaultAreaFactor() && model->getAreaFactor() > epsilon) {
            io_manager_.write("    ANTENNAAREAFACTOR %g ",
                model->getAreaFactor());
            if (model->isDiffUseOnly()) {
                io_manager_.write("DIFFUSEONLY ");
            }
            io_manager_.write(";\n");
        }
        if (model->getCumAreaRatio() > epsilon) {
            io_manager_.write("    ANTENNACUMAREARATIO %g ;\n",
                model->getCumAreaRatio());
        }
        if (model->isCumDiffAreaRatioPWL()) {
            io_manager_.write("    ANTENNACUMDIFFAREARATIO PWL (");
            for (int kk = 0; kk < model->getCumDiffAreaRatioPWLSize(); ++kk) {
                auto pwl =
                    model->getCumDiffAreaRatioPWL(kk);
                io_manager_.write(" ( %g %g ) ",
                    pwl->data_first, pwl->data_second);
            }
            io_manager_.write(") ;\n");
        } else if (model->getCumDiffAreaRatio() > epsilon) {
            io_manager_.write("    ANTENNACUMDIFFAREARATIO %g ;\n",
                model->getCumDiffAreaRatio());
        }
        if (model->isCumRoutingPlusCut()) {
            io_manager_.write("    ANTENNACUMROUTINGPLUSCUT ;\n");
        }
        if (model->getGatePlusDiffId()) {
            io_manager_.write("    PROPERTY LEF58_ANTENNAGATEPLUSDIFF "
                "\"ANTENNAGATEPLUSDIFF OXIDE%d ", model->getGatePlusDiffId());
            if (model->getGatePlusDiffPWLSize()) {
                io_manager_.write("PWL (");
                for (int kk = 0; kk < model->getGatePlusDiffPWLSize(); ++kk) {
                    auto pwl =
                        model->getGatePlusDiffPWL(kk);
                    io_manager_.write("( %g %g ) ", pwl->data_first, pwl->data_second);
                }
                io_manager_.write(")\" ;");
            } else {
                io_manager_.write("%g ;\" ;\n", model->getGatePlusDiff());
            }
        } else if (model->getGatePlusDiff() > epsilon) {
            io_manager_.write("    ANTENNAGATEPLUSDIFF %g ;\n",
                model->getGatePlusDiff());
        }
        if (model->getAreaMinusDiff() > epsilon) {
            io_manager_.write("    ANTENNAAREAMINUSDIFF %g ;\n",
                model->getAreaMinusDiff());
        }
        if (model->getAreaDiffReducePWLSize()) {
            io_manager_.write("    ANTENNAAREADIFFREDUCEPWL (");
            for (int kk = 0; kk < model->getAreaDiffReducePWLSize(); ++kk) {
                auto pwl = model->getAreaDiffReducePWL(kk);
                io_manager_.write(" ( %g %g ) ",
                    pwl->data_first, pwl->data_second);
            }
            io_manager_.write(") ;\n");
        }
        if (model->getDiffGatePWLSize()) {
            io_manager_.write("    PROPERTY LEF58_ANTENNADIFFGATEPWL "
                "\"ANTENNADIFFGATEPWL OXIDE%d (", model->getDiffGatePWLId());
            for (int kk = 0; kk < model->getDiffGatePWLSize(); ++kk) {
                auto pwl = model->getDiffGatePWL(kk);
                io_manager_.write("( %g %g ) ", pwl->data_first, pwl->data_second);
            }
            io_manager_.write(")\" ;");
        }
        if (model->getGatePWLSize()) {
            io_manager_.write("    PROPERTY LEF58_ANTENNAGATEPWL"
                    " \"ANTENNAGATEPWL OXIDE%d (",
                model->getDiffGatePWLId());
            for (int kk = 0; kk < model->getGatePWLSize(); ++kk) {
                auto pwl = model->getGatePWL(kk);
                io_manager_.write("( %g %g ) ", pwl->data_first, pwl->data_second);
            }
            io_manager_.write(")\" ;");
        }
        if (layer->isRouting()) {
            if (model->getSideAreaRatio() > epsilon) {
                io_manager_.write("    ANTENNASIDEAREARATIO %g ;\n",
                    model->getSideAreaRatio());
            }
            if (model->getDiffSideAreaRatioPWLSize()) {
                io_manager_.write("    ANTENNADIFFSIDEAREARATIO PWL (");
                for (int kk = 0; kk < model->getDiffSideAreaRatioPWLSize();
                     ++kk) {
                    auto pwl =
                        model->getDiffSideAreaRatioPWL(kk);
                    io_manager_.write(" ( %g %g ) ",
                        pwl->data_first, pwl->data_second);
                }
                io_manager_.write(") ;\n");
            } else if (model->getDiffSideAreaRatio() > epsilon) {
                io_manager_.write("    ANTENNADIFFSIDEAREARATIO %g ;\n",
                    model->getDiffSideAreaRatio());
            }
            if (model->getCumSideAreaRatio() > epsilon) {
                io_manager_.write("    ANTENNACUMSIDEAREARATIO %g ;\n",
                    model->getCumSideAreaRatio());
            }
            if (model->getCumDiffSideAreaRatioPWLSize()) {
                io_manager_.write("    ANTENNACUMDIFFSIDEAREARATIO PWL (");
                for (int kk = 0; kk < model->getCumDiffSideAreaRatioPWLSize();
                     ++kk) {
                    auto pwl =
                        model->getCumDiffSideAreaRatioPWL(kk);
                    io_manager_.write(" ( %g %g ) ",
                        pwl->data_first, pwl->data_second);
                }
                io_manager_.write(") ;\n");
            } else if (model->getCumDiffSideAreaRatio() > epsilon) {
                io_manager_.write("    ANTENNACUMDIFFSIDEAREARATIO %g ;\n",
                    model->getCumDiffSideAreaRatio());
            }
            if (model->getSideAreaFactor() > epsilon) {
                io_manager_.write("    ANTENNASIDEAREAFACTOR %g ",
                    model->getSideAreaFactor());
                if (model->isSideDiffUseOnly()) {
                    io_manager_.write("DIFFUSEONLY ");
                }
                io_manager_.write(";\n");
            }
        }
    }
}

void ExportTechLef::exportLayerCurrentDensityACItem(const Layer* layer,
                                                    CurrentDen* den) {
    if (!den) return;
    io_manager_.write("    ACCURRENTDENSITY ");
    if (den->isPeak())
        io_manager_.write("PEAK ");
    else if (den->isAverage())
        io_manager_.write("AVERAGE ");
    else if (den->isRMS())
        io_manager_.write("RMS ");
    bool is_simple_value = false;
    if (den->getFrequenciesNum()) {
        is_simple_value = true;
        io_manager_.write("\n        FREQUENCY ");
        for (int kk = 0; kk < den->getFrequenciesNum(); ++kk) {
            io_manager_.write("%g ", den->getFrequency(kk));
        }
        io_manager_.write(";\n");
    }

    if (den->getWidthsNum()) {
        is_simple_value = true;
        if (layer->isRouting())
            io_manager_.write("        WIDTH ");
        else
            io_manager_.write("        CUTAREA ");
        for (int kk = 0; kk < den->getWidthsNum(); ++kk) {
            if (layer->isRouting()) {
                io_manager_.write("%g ",
                    tech_lib_->dbuToMicrons(den->getWidth(kk)));
                    // width & cut area use same storage
            } else {
                io_manager_.write("%g ",
                    tech_lib_->areaDBUToMicrons(den->getCutArea(kk)));
                    // width & cut area use same storage
            }
        }
        io_manager_.write(";\n");
    }
    if (den->hasTableEntries()) {
        is_simple_value = true;
        io_manager_.write("        TABLEENTRIES");
        UInt32 row_num = den->getFrequenciesNum();
        UInt32 col_num = den->getWidthsNum();
        if (col_num == 0) {  // FREQUENCY without WIDTH or CUTAREA
            col_num = row_num;
            row_num = 1;
        }
        if (row_num > 1) {
            io_manager_.write("\n            ");
        } else {
            io_manager_.write(" ");
        }
        for (int row_idx = 0; row_idx < row_num; ++row_idx) {
            for (int col_idx = 0; col_idx < col_num; ++col_idx) {
                io_manager_.write("%g ",
                    den->getTableElem(row_idx, col_idx));
            }
            if (row_idx == (row_num - 1))
                io_manager_.write(";\n");
            else
                io_manager_.write("\n          ");
        }
    }
    if (!is_simple_value) {
        io_manager_.write("g ;\n", den->getCurrentDen());
    }
}

void ExportTechLef::exportLayerCurrentDensityDCItem(const Layer* layer,
                                                    CurrentDen* den) {
    if (den) {
        io_manager_.write("    DCCURRENTDENSITY AVERAGE ");
        UInt32 col_num = den->getWidthsNum();
        if (col_num) {
            if (layer->isRouting())
                io_manager_.write("\n        WIDTH ");
            else
                io_manager_.write("\n        CUTAREA ");
            for (int col_idx = 0; col_idx < col_num; ++col_idx) {
                if (layer->isRouting()) {
                    io_manager_.write("%g ",
                        tech_lib_->dbuToMicrons(den->getWidth(col_idx)));
                } else {
                    io_manager_.write("%g ",
                        tech_lib_->areaDBUToMicrons(den->getCutArea(col_idx)));
                }
            }
            io_manager_.write(";\n");
            if (den->hasTableEntries()) {
                io_manager_.write("        TABLEENTRIES ");
                for (int col_idx = 0; col_idx < col_num; ++col_idx) {
                    io_manager_.write("%g ", den->getTableElem(0, col_idx));
                }
                io_manager_.write(";\n");
            }
        } else {
            io_manager_.write("%g ;\n", den->getCurrentDen());
        }
    }
}

void ExportTechLef::exportLayerCurrentDensity(const Layer* layer) {
    // ACCURRENTDENSITY
    CurrentDenContainer* ac = layer->getACCurrentDenContainer();
    if (ac) {
        exportLayerCurrentDensityACItem(layer, ac->getACPeak());
        exportLayerCurrentDensityACItem(layer, ac->getACAverage());
        exportLayerCurrentDensityACItem(layer, ac->getACRMS());
    }

    // DCCURRENTDENSITY
    CurrentDenContainer* dc = layer->getDCCurrentDenContainer();
    if (dc) {
        exportLayerCurrentDensityDCItem(layer, dc->getDCAverage());
    }
}

void ExportTechLef::exportRoutingLayer(Layer* layer) {
    RoutingLayerRule* r = layer->getRoutingLayerRule();
    float epsilon = 0.00001;
    io_manager_.write("LAYER %s\n", layer->getName());
    io_manager_.write("    TYPE  %s ;\n",
        (layer->isRouting() ? "ROUTING" : "ERROR"));
    if (layer->getNumMask() > 1)
        io_manager_.write("    MASK %d ;\n", layer->getNumMask());
    io_manager_.write("    DIRECTION ");
    if (r->isHoriz())
        io_manager_.write("HORIZONTAL ;\n");
    else if (r->isVert())
        io_manager_.write("VERTICAL ;\n");
    else if (r->isDiag45())
        io_manager_.write("DIAG45 ;\n");
    else if (r->isDiag135())
        io_manager_.write("DIAG135 ;\n");
    else
        io_manager_.write("ERROR ;\n");
    if (r->isXYPitch()) {
        io_manager_.write("    PITCH %g %g ;\n",
            tech_lib_->dbuToMicrons(r->getPitchX()),
            tech_lib_->dbuToMicrons(r->getPitchY()));
    } else if (r->getPitchX()) {
        io_manager_.write("    PITCH %g ;\n",
            tech_lib_->dbuToMicrons(r->getPitchX()));
    }
    if (r->isXYDiagPitch()) {
        io_manager_.write("    DIAGPITCH %g %g ;\n",
            tech_lib_->dbuToMicrons(r->getDiagPitch45()),
            tech_lib_->dbuToMicrons(r->getDiagPitch135()));
    } else if (r->getDiagPitch()) {
        io_manager_.write("    DIAGPITCH %g ;\n",
            tech_lib_->dbuToMicrons(r->getDiagPitch()));
    }
    if (layer->getWidth()) {
        io_manager_.write("    WIDTH %g ; \n",
            tech_lib_->dbuToMicrons(layer->getWidth()));
    }
    if (r->isXYOffset()) {
        io_manager_.write("    OFFSET %g %g ;\n",
            tech_lib_->dbuToMicrons(r->getOffsetX()),
            tech_lib_->dbuToMicrons(r->getOffsetY()));
    } else {
        io_manager_.write("    OFFSET %g ;\n",
            tech_lib_->dbuToMicrons(r->getOffset()));
    }
    if (r->getDiagWidth()) {
        io_manager_.write("    DIAGWIDTH %g ;\n",
            tech_lib_->dbuToMicrons(r->getDiagWidth()));
    }
    if (r->getDiagSpacing()) {
        io_manager_.write("    DIAGSPACING %g ;\n",
            tech_lib_->dbuToMicrons(r->getDiagSpacing()));
    }
    if (r->getDiagMinEdgeLength()) {
        io_manager_.write("    DIAGMINEDGELENGTH %g ;\n",
            tech_lib_->dbuToMicrons(r->getDiagMinEdgeLength()));
    }
    if (r->getMinWidth()) {
        io_manager_.write("    MINWIDTH %g ;\n",
            tech_lib_->dbuToMicrons(r->getMinWidth()));
    }
    if (r->getMaxWidth()) {
        io_manager_.write("    MAXWIDTH %g ;\n",
            tech_lib_->dbuToMicrons(r->getMaxWidth()));
    }
    if (r->getWireExt()) {
        io_manager_.write("    WIREEXTENSION %g ;\n",
            tech_lib_->dbuToMicrons(r->getWireExt()));
    }

    exportMinArea(layer);
    exportMinSize(layer);
    exportRoutingSpacing(r);
    exportSpacingTable(r);
    exportMinCut(r);
    exportMinEnclosedArea(r);
    exportMinStep(r);
    exportProtrusionWidth(r);

    if (r->getResPerSq() > epsilon) {
        io_manager_.write("    RESISTANCE RPERSQ %g ;\n",
            r->getResPerSq());
    }
    if (r->getCapPerSq() > epsilon) {
        io_manager_.write("    CAPACITANCE CPERSQDIST %g ;\n",
            r->getCapPerSq());
    }
    if (r->getHeight()) {
        io_manager_.write("    HEIGHT %g ;\n",
            tech_lib_->dbuToMicrons(r->getHeight()));
    }
    if (r->getThickness()) {
        io_manager_.write("    THICKNESS %g ;\n",
            tech_lib_->dbuToMicrons(r->getThickness()));
    }
    if (r->getShrinkage()) {
        io_manager_.write("    SHRINKAGE %g ;\n",
            tech_lib_->dbuToMicrons(r->getShrinkage()));
    }
    if (r->getCapMul() > epsilon) {
        io_manager_.write("    CAPMULTIPLIER %g ;\n",
            r->getCapMul());
    }
    if (r->getEdgeCap() > epsilon) {
        io_manager_.write("    EDGECAPACITANCE %g ;\n",
            r->getEdgeCap());
    }
    if (r->getFillMinDensity() > epsilon) {
        io_manager_.write("    MINIMUMDENSITY %g ;\n",
            r->getFillMinDensity());
    }
    if (r->getFillMaxDensity() > epsilon) {
        io_manager_.write("    MAXIMUMDENSITY %g ;\n",
            r->getFillMaxDensity());
    }
    if (r->getFillCheckWinLength() || r->getFillCheckWinWidth()) {
        io_manager_.write("    DENSITYCHECKWINDOW %g %g ;\n",
            tech_lib_->dbuToMicrons(r->getFillCheckWinLength()),
            tech_lib_->dbuToMicrons(r->getFillCheckWinWidth()));
    }
    if (r->getFillCheckWinStep()) {
        io_manager_.write("    DENSITYCHECKSTEP %g ;\n",
            tech_lib_->dbuToMicrons(r->getFillCheckWinStep()));
    }
    if (r->getFillActiveSpacing()) {
        io_manager_.write("    FILLACTIVESPACING %g ;\n",
            tech_lib_->dbuToMicrons(r->getFillActiveSpacing()));
    }

    exportLayerProperty(layer);
    exportLayerAntennaModel(layer);
    exportLayerCurrentDensity(layer);

    io_manager_.write("END %s\n\n", layer->getName());
}

void ExportTechLef::exportCutLayer(Layer* layer) {
    io_manager_.write("LAYER %s\n", layer->getName());
    io_manager_.write("    TYPE  %s ;\n",
        (layer->isCut() ? "CUT" : "ERROR"));

    exportLayerProperty(layer);
    CutLayerRule* cut_rule = layer->getCutLayerRule();
    if (cut_rule) {
        exportCutSpacing(cut_rule);
        exportEnclosure(cut_rule);
        exportPreferEnclosure(cut_rule);
        exportArraySpacing(cut_rule);
        exportSpTblOrtho(cut_rule);
    }
    exportLayerAntennaModel(layer);
    exportLayerCurrentDensity(layer);

    io_manager_.write("END %s\n\n", layer->getName());
}

void ExportTechLef::exportCutSpacing(const CutLayerRule* cut_rule) {
    ArrayObject<ObjectId> *array = cut_rule->getCutSpacingArray();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        CutSpacing* cut_spacing = Object::addr<CutSpacing>(*iter);
        if (!cut_spacing)
            return;
        io_manager_.write("    SPACING  %g",
            tech_lib_->dbuToMicrons(cut_spacing->getSpacing()));
        if (cut_spacing->getIsC2C()) {
            io_manager_.write(" CENTERTOCENTER ");
        }
        if (cut_spacing->getIsSameNet()) {
            io_manager_.write(" SAMENET ");
        }
        if (cut_spacing->getIsAdjCuts()) {
            AdjacentCuts* adj_cuts = cut_spacing->getAdjCuts();
            if (adj_cuts) {
                io_manager_.write("  ADJACENTCUTS  %d  WITHIN  %g",
                    adj_cuts->getCutNum(),
                    tech_lib_->dbuToMicrons(adj_cuts->getCutWithin()));
                if (adj_cuts->getIsExceptSamePGNet()) {
                    io_manager_.write("  EXCEPTSAMEPGNET ");
                }
            }
        } else if (cut_spacing->getIsParallelOverlap()) {
            CutSpacingPrlOvlp* prl_ovlp = cut_spacing->getParallelOverlap();
            if (prl_ovlp) {
                if (prl_ovlp->getIsParallelOverlap()) {
                    io_manager_.write("  PARALLELOVERLAP  ");
                }
            }
        } else if (cut_spacing->getIsSecondLayer()) {
            SecondLayer* sec_layer = cut_spacing->getSecondLayer();
            if (sec_layer) {
                Layer* layer = getTopCell()->getLayerByLayerId(
                    sec_layer->getSecondLayerId());
                if (layer) {
                    io_manager_.write("  LAYER %s", layer->getName());
                    if (sec_layer->getIsStack()) {
                        io_manager_.write("  STACK ");
                    }
                }
            }
        } else if (cut_spacing->getIsCutArea()) {
            io_manager_.write("  AREA %g",
                tech_lib_->areaDBUToMicrons(cut_spacing->getCutArea()));
        }
        io_manager_.write(" ;\n");
    }
}

void ExportTechLef::exportEnclosure(const CutLayerRule* cut_rule) {
    ArrayObject<ObjectId> *array = cut_rule->getEnclosureArray();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        Enclosure* enclosure = Object::addr<Enclosure>(*iter);
        if (!enclosure)
            return;
        io_manager_.write("    ENCLOSURE  ");
        if (enclosure->getIsAbove()) {
            io_manager_.write("ABOVE  ");
        } else if (enclosure->getIsBelow()) {
            io_manager_.write("BELOW ");
        }
        if (enclosure->getIsOverhang()) {
            EnclosureOverhang* enc_overhang = enclosure->getOverhang();
            if (enc_overhang) {
                io_manager_.write("%g %g ",
                    tech_lib_->dbuToMicrons(enc_overhang->getOverhang1()),
                    tech_lib_->dbuToMicrons(enc_overhang->getOverhang2()));
                if (enc_overhang->isWidth()) {
                    io_manager_.write("WIDTH %g ",
                        tech_lib_->dbuToMicrons(enc_overhang->getMinWidth()));
                    if (enc_overhang->isExceptExtraCut()) {
                        io_manager_.write("EXCEPTEXTRACUT %g ",
                            tech_lib_->dbuToMicrons(enc_overhang->getExceptExtraCutWithin()));
                    }
                } else if (enc_overhang->isLength()) {
                    io_manager_.write("LENGTH %g ",
                        tech_lib_->dbuToMicrons(enc_overhang->getMinLength()));
                }
                io_manager_.write("; \n");
            }
        } else if (enclosure->getIsEol()) {
        }
    }
}

void ExportTechLef::exportPreferEnclosure(const CutLayerRule* cut_rule) {
    ArrayObject<ObjectId> *array = cut_rule->getPreferEnclosureArray();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        Enclosure* prefer_enclosure = Object::addr<Enclosure>(*iter);
        if (!prefer_enclosure)
            return;
        io_manager_.write("    PREFERENCLOSURE  ");
        if (prefer_enclosure->getIsAbove()) {
            io_manager_.write("ABOVE  ");
        } else if (prefer_enclosure->getIsBelow()) {
            io_manager_.write("BELOW ");
        }
        if (prefer_enclosure->getIsOverhang()) {
            EnclosureOverhang* enc_overhang = prefer_enclosure->getOverhang();
            if (enc_overhang) {
                io_manager_.write("%g %g ",
                    tech_lib_->dbuToMicrons(enc_overhang->getOverhang1()),
                    tech_lib_->dbuToMicrons(enc_overhang->getOverhang2()));
                if (enc_overhang->isWidth()) {
                    io_manager_.write("WIDTH %g ",
                        tech_lib_->dbuToMicrons(enc_overhang->getMinWidth()));
                }
                io_manager_.write("; \n");
            }
        }
    }
}

void ExportTechLef::exportArraySpacing(const CutLayerRule* cut_rule) {
    ArrayObject<ObjectId> *array = cut_rule->getArraySpacingArray();
    if (array == nullptr)
        return;
    for (ArrayObject<ObjectId>::iterator iter = array->begin(); iter != array->end(); ++iter) {
        ArraySpacing* array_spacing = Object::addr<ArraySpacing>(*iter);
        if (!array_spacing)
            return;
        io_manager_.write("    ARRAYSPACING  ");
        if (array_spacing->getIsLongArray()) {
            io_manager_.write("LONGARRAY  ");
        }
        if (array_spacing->hasViaWidth()) {
            io_manager_.write("WIDTH  %g  ",
                tech_lib_->dbuToMicrons(array_spacing->getViaWidth()));
        }
        io_manager_.write("CUTSPACING  %g  ",
            tech_lib_->dbuToMicrons(array_spacing->getCutSpacing()));
        for (int i = 0; i < array_spacing->getNumArrayCuts(); i++) {
            io_manager_.write("ARRAYCUTS  %d ",
                array_spacing->getArrayCuts(i));
            io_manager_.write("SPACING  %g  ",
                tech_lib_->dbuToMicrons(array_spacing->getArraySpacing(i)));
        }
        io_manager_.write(" ;\n");
    }
}

void ExportTechLef::exportSpTblOrtho(const CutLayerRule* cut_rule) {
    SpacingTableOrthogonal *sp_tbl_ortho = cut_rule->getSpTblOrth();
    if (sp_tbl_ortho) {
        io_manager_.write("    SPACINGTABLE ORTHOGONAL\n");
        for (int i = 0; i < sp_tbl_ortho->getNumOrtho(); i++) {
            io_manager_.write("        WITHIN %g ",
                tech_lib_->dbuToMicrons(sp_tbl_ortho->getCutWithin(i)));
            io_manager_.write("        SPACING  %g  \n",
                tech_lib_->dbuToMicrons(sp_tbl_ortho->getOrthoSpacing(i)));
        }
        io_manager_.write("    ;\n");
    }
}

void ExportTechLef::exportMastersliceLayer(Layer* layer) {
    io_manager_.write("LAYER %s\n", layer->getName());
    io_manager_.write("    TYPE  MASTERSLICE ;\n");
    exportLayerProperty(layer);
    io_manager_.write("END %s\n\n", layer->getName());
}

void ExportTechLef::exportOverlapLayer(Layer* layer) {
    io_manager_.write("LAYER %s\n", layer->getName());
    io_manager_.write("    TYPE  OVERLAP ;\n");
    exportLayerProperty(layer);
    io_manager_.write("END %s\n\n", layer->getName());
}

void ExportTechLef::exportLayerProperty( Layer* layer) {
    Cell *top_cell = getTopCell();
    if (!top_cell) return;
    PropertyManager* table = top_cell->getPropertyManager(); 
    std::vector<std::string> properties;
    table->getSparsePropertyList<Layer>(layer, properties);
    if (properties.size() > 0) {
        for(int i = 0; i < properties.size(); i++) {
            Property* prop = table->getSparseProperty<Layer>(layer, properties[i].c_str());
            prop->printLEF(io_manager_);
        }
    }
}

void ExportTechLef::exportLayer(Layer* layer) {
    if (!layer) return;
    if (layer->isRouting()) {
        exportRoutingLayer(layer);
    } else if (layer->isCut()) {
        exportCutLayer(layer);
    } else if (layer->isImplant()) {
        exportImplantLayer(layer);
    } else if (layer->isMS()) {
        exportMastersliceLayer(layer);
    } else if (layer->isOverlap()) {
        exportOverlapLayer(layer);
    }
}

void ExportTechLef::exportAllLayers() {
    UInt32 num_layers = tech_lib_->getNumLayers();
    for (UInt32 ii = 0; ii < num_layers; ++ii) {
        exportLayer(tech_lib_->getLayer(ii));
    }
}

void ExportTechLef::exportViaMaster() {
    ObjectId vobj_id = tech_lib_->getViaMasterVectorId();
    if (vobj_id == 0) return;
    IdArray* arr_ptr = Object::addr<IdArray>(vobj_id);
    if (!arr_ptr) return;

    for (IdArray::iterator iter = arr_ptr->begin(); iter != arr_ptr->end();
         ++iter) {
        ObjectId obj_id = (*iter);
        if (!obj_id) continue;
        ViaMaster* obj_data = Object::addr<ViaMaster>(obj_id);
        if (!obj_data) continue;
        if (obj_data->isFromDEF()) continue;
        if (obj_data->isFromNDR()) continue;
        obj_data->printLEF(io_manager_);
    }
}

void ExportTechLef::exportViaRule() {
    ObjectId vobj_id = tech_lib_->getViaRuleVectorId();
    if (vobj_id == 0) return;
    IdArray* arr_ptr= Object::addr<IdArray>(vobj_id);
    if (!arr_ptr) return;

    for (IdArray::iterator iter = arr_ptr->begin(); iter != arr_ptr->end();
         ++iter) {
        ObjectId obj_id = (*iter);
        if (!obj_id) continue;
        ViaRule* obj_data = Object::addr<ViaRule>(obj_id);
        if (!obj_data) continue;

        obj_data->printLEF(io_manager_);
    }
}

void ExportTechLef::exportNDR() {
    ObjectId vobj_id = tech_lib_->getNonDefaultRuleVectorId();
    if (vobj_id == 0) return;
    IdArray* arr_ptr = Object::addr<IdArray>(vobj_id);
    if (!arr_ptr) return;

    for (IdArray::iterator iter = arr_ptr->begin(); iter != arr_ptr->end();
         ++iter) {
        ObjectId obj_id = (*iter);
        if (!obj_id) continue;
        NonDefaultRule* obj_data =
            Object::addr<NonDefaultRule>(obj_id);
        if (!obj_data) continue;
        if (obj_data->getFromDEF()) continue;
        obj_data->printLEF(io_manager_);
    }
}

void ExportTechLef::exportCells() {
    Tech* tech_lib = getTechLib();
    if (!tech_lib) return;
    for (uint64_t i = 0; i < tech_lib->getNumOfCells(); i++) {
        Cell* cell = tech_lib->getCell(i);
        if (cell) {
            // message->info("print out %dth macro to topcell\n", test_counttt);
            cell->printLEF(io_manager_);
            // cell->print();
        }
    }
}

// this section also display end-of-library
void ExportTechLef::exportExtensions() {
    const char* str_value = tech_lib_->getExtensionsName();
    if (strcmp(str_value, "")) {
        io_manager_.write(str_value);
    }
    io_manager_.write("\nEND LIBRARY\n");
}

}  // namespace db
}  // namespace open_edi
