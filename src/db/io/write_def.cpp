/* @file  write_def.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/io/write_def.h"

#include <stdio.h>
#include <time.h>
#include <string>
#include <vector>

#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/util/property_definition.h"
#include "db/util/array.h"
#include "db/util/transform.h"
#include "util/util.h"
#include "infra/command_manager.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;
using namespace open_edi::infra;

#define OK (0)
#define ERROR (1)

static Cell *top_cell;

static bool writeFileHead(IOManager &io_manager);
static bool writeVersion(IOManager &io_manager);
static bool writeDividerChar(IOManager &io_manager);
static bool writeBusbitChars(IOManager &io_manager);
static bool writeDesign(IOManager &io_manager);
static bool writeTechnology(IOManager &io_manager);
static bool writeUnits(IOManager &io_manager);
static bool writeHistory(IOManager &io_manager);
static bool writePropertyDefinitions(IOManager &io_manager);
static bool writeDieArea(IOManager &io_manager);
static bool writeRows(IOManager &io_manager);
static bool writeTracks(IOManager &io_manager);
static bool writeGcellGrid(IOManager &io_manager);
static bool writeVias(IOManager &io_manager);
static bool writeStyles(IOManager &io_manager);
static bool writeNonDefaultRules(IOManager &io_manager);
static bool writeRegions(IOManager &io_manager);
static bool writeComponentMaskShift(IOManager &io_manager);
static bool writeComponents(IOManager &io_manager);
static bool writePins(IOManager &io_manager);
static bool writePinProperties(IOManager &io_manager);
static bool writeBlockages(IOManager &io_manager);
static bool writeSlots(IOManager &io_manager);
static bool writeFills(IOManager &io_manager);
static bool writeSpecialNets(IOManager &io_manager);
static bool writeNets(IOManager &io_manager);
static bool writeScanChains(IOManager &io_manager);
static bool writeGroups(IOManager &io_manager);
static bool writeExtension(IOManager &io_manager);
static bool writeEndDesign(IOManager &io_manager);

int cmdWriteDef(Command* cmd) {
    if (cmd == nullptr) return 0;
    char *def_file_name = nullptr;
    if (cmd->isOptionSet("file"))  {
        std::string file;
        bool res = cmd->getOptionValue("file", file);
        def_file_name = const_cast<char *>(file.c_str());
        return writeDef(def_file_name);
    }
    return TCL_ERROR;
}

int writeDef(char *def_file_name) {
    int num_out_file = 0;
    //char *def_file_name = nullptr;
    bool debug_mode = false;

    top_cell = getTopCell();
    if (!top_cell) {
        message->issueMsg("DBIO", 44, kError, "top cell");
        return ERROR;
    }

    std::string old_file_name = def_file_name;
    old_file_name += ".old";
    std::ifstream ifs(def_file_name, std::ios::in);
    if (ifs.is_open()) {
        ifs.close();
        ifs.open(old_file_name, std::ios::in);
        if (ifs.good()) {
            ifs.close();
            unlink(old_file_name.c_str());
        } else {
            ifs.close();
        }
        rename(def_file_name, old_file_name.c_str());
    }

    IOManager io_manager;
    if (!io_manager.open(def_file_name, "w")) {
        message->issueMsg("DBIO", 45, kError, def_file_name);
        return ERROR;
    }
    message->info("\nWriting DEF\n");
    fflush(stdout);
    if (!writeFileHead(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Header");
        io_manager.close();
        return ERROR;
    }
    if (!writeVersion(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Version");
        io_manager.close();
        return ERROR;
    }
    if (!writeDividerChar(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Divider");
        io_manager.close();
        return ERROR;
    }
    if (!writeBusbitChars(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Busbit char");
        io_manager.close();
        return ERROR;
    }
    if (!writeDesign(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Design");
        io_manager.close();
        return ERROR;
    }
    if (!writeTechnology(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Technology");
        io_manager.close();
        return ERROR;
    }
    if (!writeUnits(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Unit");
        io_manager.close();
        return ERROR;
    }
    if (!writeHistory(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "History");
        io_manager.close();
        return ERROR;
    }
    if (!writePropertyDefinitions(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Property definitions");
        io_manager.close();
        return ERROR;
    }
    if (!writeDieArea(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Die area");
        io_manager.close();
        return ERROR;
    }
    if (!writeRows(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Rows");
        io_manager.close();
        return ERROR;
    }
    if (!writeTracks(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Tracks");
        io_manager.close();
        return ERROR;
    }
    if (!writeGcellGrid(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Gcell grids");
        io_manager.close();
        return ERROR;
    }
    if (!writeVias(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Vias");
        io_manager.close();
        return ERROR;
    }
    if (!writeStyles(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Styles");
        io_manager.close();
        return ERROR;
    }
    if (!writeNonDefaultRules(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Nondefault rules");
        io_manager.close();
        return ERROR;
    }
    if (!writeRegions(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Regions");
        io_manager.close();
        return ERROR;
    }
    if (!writeComponentMaskShift(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Component mask shift");
        io_manager.close();
        return ERROR;
    }
    if (!writeComponents(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Components");
        io_manager.close();
        return ERROR;
    }
    if (!writePins(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Pins");
        io_manager.close();
        return ERROR;
    }
    if (!writePinProperties(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Pin properties");
        io_manager.close();
        return ERROR;
    }
    if (!writeBlockages(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Blockages");
        io_manager.close();
        return ERROR;
    }
    if (!writeSlots(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Slots");
        io_manager.close();
        return ERROR;
    }
    if (!writeFills(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Fills");
        io_manager.close();
        return ERROR;
    }
    if (!writeSpecialNets(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Special nets");
        io_manager.close();
        return ERROR;
    }
    if (!writeNets(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Nets");
        io_manager.close();
        return ERROR;
    }
    if (!writeScanChains(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Scan chains");
        io_manager.close();
        return ERROR;
    }
    if (!writeGroups(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Groups");
        io_manager.close();
        return ERROR;
    }
    if (!writeExtension(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "Extensions");
        io_manager.close();
        return ERROR;
    }
    if (!writeEndDesign(io_manager)) {
        message->issueMsg("DBIO", 46, kError, "End design");
        io_manager.close();
        return ERROR;
    }

    io_manager.close();

    message->info("\nWrite DEF successfully.\n");
    return OK;
}

static bool writeFileHead(IOManager &io_manager) {
    time_t timep;
    time(&timep);
    char time_str[256];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
             localtime(&timep));

    io_manager.write("##################################"
        "#############################################\n"
        "# DEF file generated by NIICEDA Tool\n# %s\n"
        "########################################"
        "#######################################\n", time_str);
    return true;
}
static bool writeVersion(IOManager &io_manager) {
    io_manager.write("VERSION 5.8 ;\n");
    return true;
}
static bool writeDividerChar(IOManager &io_manager) {
    io_manager.write("DIVIDERCHAR \"/\" ;\n");
    return true;
}
static bool writeBusbitChars(IOManager &io_manager) {
    io_manager.write("BUSBITCHARS \"[]\" ;\n");
    return true;
}
static bool writeDesign(IOManager &io_manager) {
    io_manager.write("DESIGN %s ;\n", top_cell->getName().c_str());
    return true;
}
static bool writeTechnology(IOManager &io_manager) {
    return true;
}
static bool writeUnits(IOManager &io_manager) {
    Tech *tech_lib = top_cell->getTechLib();
    if (!tech_lib) {
        message->issueMsg("DBIO", 44, kError, "tech lib");
        return false;
    }
    Units *units = tech_lib->getUnits();
    if (!units) {
        message->issueMsg("DBIO", 44, kError, "units");
        return false;
    }
    io_manager.write("UNITS DISTANCE MICRONS %d ;\n",
                units->getLengthFactor());
    return true;
}
static bool writeHistory(IOManager &io_manager) {
    return true;
}
static bool writePropertyDefinitions(IOManager &io_manager) {
    Tech *tech_lib = top_cell->getTechLib();
    if (!tech_lib) {
        message->issueMsg("DBIO", 44, kError, "tech lib");
        return false;
    }
    io_manager.write("\n\n################################"
        "###############################################\n"
        "# PROPERTYDEFINITIONS\n"
        "#    objectType propName propType [ RANGE min max ] [ value ] ;"
        " ... \n# END PROPERTYDEFINITIONS\n"
        "#######################################"
        "########################################\n");

    int count = 0;
    //for (int i = 0; i < toInteger(PropType::kUnknown); ++i) {
    ObjectId vobj_id =
        tech_lib->getPropertyDefinitionVectorId();
    if (vobj_id != 0) {
        ArrayObject<ObjectId> *arr_ptr = Object::addr<ArrayObject<ObjectId>>(vobj_id);
        if (arr_ptr) {
            for (ArrayObject<ObjectId>::iterator iter = arr_ptr->begin(); iter != arr_ptr->end();
                 ++iter) {
                ObjectId obj_id = (*iter);
                if (!obj_id) continue;
                PropertyDefinition *obj_data =
                    Object::addr<PropertyDefinition>(obj_id);
                if (!obj_data) continue;
                if (obj_data->getPropOriginType() != kPropDef) continue;
                if (count == 0) {
                    io_manager.write("PROPERTYDEFINITIONS\n");
                }
                obj_data->printDEF(io_manager);
                ++count;
            }
        }
    }

    //}
    if (count > 0) {
        io_manager.write("END PROPERTYDEFINITIONS\n");
    }
    return true;
}

static bool writeDieArea(IOManager &io_manager) {
    Polygon *polygon = nullptr;
    io_manager.write("\n\n#################################"
        "##############################################\n"
        "# [DIEAREA pt pt [pt] ... ;]\n"
        "######################################"
        "#########################################\n");

    Floorplan *floorplan = top_cell->getFloorplan();
    if (!floorplan) {
        message->issueMsg("DBIO", 44, kError, "floorplan");
        return false;
    }
    if (floorplan) polygon = floorplan->getDieAreaPolygon();
    if (!polygon) {
        message->issueMsg("DBIO", 44, kError, "die area polygon");
        return false;
    }
    io_manager.write("DIEAREA");

    for (int i = 0; i < polygon->getNumPoints(); ++i) {
        io_manager.write(" ( %d %d )", polygon->getPoint(i).getX(),
            polygon->getPoint(i).getY());
    }
    io_manager.write(" ;\n");

    return true;
}
static bool writeRows(IOManager &io_manager) {
    io_manager.write("\n\n#####################################"
        "##########################################\n"
        "# [ROW rowName siteName origX origY siteOrient\n"
        "#     [DO numX BY numY [STEP stepX stepY]]\n"
        "#     [+ PROPERTY {propName propVal} ...] ... ;] ...\n"
        "#############################################"
        "##################################\n");

    Floorplan *floorplan = top_cell->getFloorplan();
    if (!floorplan) {
        message->issueMsg("DBIO", 44, kError, "floorplan");
        return false;
    }

    ObjectId rows = floorplan->getRows();
    if (0 == rows) {
        return true;
    }
    IdArray *obj_vector = Object::addr< IdArray >(rows);
    if (!obj_vector) {
        message->issueMsg("DBIO", 44, kError, "rows vector");
        return false;
    }
    for (int i = 0; i < obj_vector->getSize(); ++i) {
        Row *row = Object::addr<Row>((*obj_vector)[i]);
        if (!row) {
            message->issueMsg("DBIO", 47, kError, "row", i);
            continue;
        }
        row->print(io_manager);
    }
    return true;
}
static bool writeTracks(IOManager &io_manager) {
    io_manager.write("\n\n###################################"
        "############################################\n"
        "# [TRACKS\n#     [{X | Y} start DO numtracks STEP space\n"
        "#       [MASK maskNum [SAMEMASK]]\n"
        "#       [LAYER layerName ...]\n#     ;] ...] \n"
        "#######################################"
        "########################################\n");

    Floorplan *floorplan = top_cell->getFloorplan();
    if (!floorplan) {
        message->issueMsg("DBIO", 44, kError, "floorplan");
        return false;
    }

    ObjectId tracks = floorplan->getTracks();
    if (0 == tracks) {
        return true;
    }
    IdArray *obj_vector = Object::addr< IdArray >(tracks);
    if (!obj_vector) {
        message->issueMsg("DBIO", 44, kError, "tracks vector");
        return false;
    }

    for (int i = 0; i < obj_vector->getSize(); ++i) {
        Track *track = Object::addr<Track>((*obj_vector)[i]);
        if (!track) {
            message->issueMsg("DBIO", 47, kError, "track", i);
            continue;
        }
        track->print(io_manager);
    }
    return true;
}
static bool writeGcellGrid(IOManager &io_manager) {
    io_manager.write("\n\n###############################"
        "################################################\n"
        "# [GCELLGRID\n"
        "#     {X start DO numColumns+1 STEP space} ...\n"
        "#     {Y start DO numRows+1 STEP space} ...\n"
        "#######################################"
        "########################################\n");

    Floorplan *floorplan = top_cell->getFloorplan();
    if (!floorplan) {
        message->issueMsg("DBIO", 44, kError, "floorplan");
        return false;
    }

    ObjectId gcell_grids = floorplan->getGcellGrids();
    if (0 == gcell_grids) {
        return true;
    }
    IdArray *obj_vector =
        Object::addr< IdArray >(gcell_grids);
    if (!obj_vector) {
        message->issueMsg("DBIO", 44, kError, "gcell grid vector");
        return false;
    }

    for (int i = 0; i < obj_vector->getSize(); ++i) {
        Grid *gcell_grid = Object::addr<Grid>((*obj_vector)[i]);
        if (!gcell_grid) {
            message->issueMsg("DBIO", 47, kError, "gcell grid", i);
            continue;
        }
        gcell_grid->print(io_manager);
    }
    return true;
}
static bool writeVias(IOManager &io_manager) {
    Tech *lib = getTopCell()->getTechLib();
    ObjectId vias = lib->getViaMasterVectorId();
    if (vias == 0) return true;
    ArrayObject<ObjectId> *via_vector = Object::addr<ArrayObject<ObjectId>>(vias);
    int num_vias = 0;
    for (int i = 0; i < via_vector->getSize(); ++i) {
        ViaMaster *via = Object::addr<ViaMaster>((*via_vector)[i]);
        if (!via) {
            continue;
        }
        if (!via->isFromDEF()) continue;
        ++num_vias;
    }
    io_manager.write("\nVIAS %d ;\n", num_vias);

    for (int i = 0; i < via_vector->getSize(); ++i) {
        ViaMaster *via = Object::addr<ViaMaster>((*via_vector)[i]);
        if (!via) {
            message->issueMsg("DBIO", 47, kError, "via", i);
            continue;
        }
        if (!via->isFromDEF()) continue;
        via->printDEF(io_manager);
    }
    io_manager.write("END VIAS\n");

    return true;
}
static bool writeStyles(IOManager &io_manager) {
    return true;
}
static bool writeNonDefaultRules(IOManager &io_manager) {
    Tech *tech_lib = top_cell->getTechLib();
    if (!tech_lib) {
        message->issueMsg("DBIO", 44, kError, "tech lib");
        return false;
    }
    io_manager.write("\n\n################################"
        "###############################################\n"
        "# [NONDEFAULTRULES numNDRs ;\n"
        "#     {- STYLE styleNum pt pt ... ;} ...\n"
        "# END NONDEFAULTRULES]\n"
        "###############################################"
        "################################\n");

    ObjectId vobj_id = tech_lib->getNonDefaultRuleVectorId();
    if (vobj_id == 0) return true;
    ArrayObject<ObjectId> *arr_ptr = Object::addr<ArrayObject<ObjectId>>(vobj_id);
    if (!arr_ptr) return true;
    int defrule_count = 0;
    int nondefault_count = 0;

    for (ArrayObject<ObjectId>::iterator iter = arr_ptr->begin(); iter != arr_ptr->end();
         ++iter) {
        ObjectId obj_id = (*iter);
        if (!obj_id) continue;
        NonDefaultRule *obj_data =
            Object::addr<NonDefaultRule>(obj_id);
        if (!obj_data) continue;
        if (obj_data->getFromDEF()) {
            defrule_count++;
            std::string name = obj_data->getName();
            if (name.find("DEFAULT") != std::string::npos) {
                nondefault_count++;
            }
        }
    }

    if (defrule_count == 0) return true;

    io_manager.write("NONDEFAULTRULES %d ;\n", nondefault_count);


    for (ArrayObject<ObjectId>::iterator iter = arr_ptr->begin(); iter != arr_ptr->end();
         ++iter) {
        ObjectId obj_id = (*iter);
        if (!obj_id) continue;
        NonDefaultRule *obj_data =
            Object::addr<NonDefaultRule>(obj_id);
        if (!obj_data) continue;
        if (!obj_data->getFromDEF()) continue;
        obj_data->printDEF(io_manager);
    }

    io_manager.write("END NONDEFAULTRULES\n");

    return true;
}
static bool writeRegions(IOManager &io_manager) {
    Floorplan *floorplan = top_cell->getFloorplan();
    if (!floorplan) {
        message->issueMsg("DBIO", 44, kError, "floorplan");
        return false;
    }
    int region_num = floorplan->getNumOfRegions();
    if (0 == region_num) {
        return true;
    }
    io_manager.write("\n\n###############################"
        "################################################\n"
        "# [REGIONS numRegions ;\n"
        "# [- regionName {pt pt} ...\n"
        "#   [+ TYPE {FENCE | GUIDE}]\n"
        "#   [+ PROPERTY {propName propVal} ...] ...\n"
        "# ;] ...\n# END REGIONS]\n"
        "################################################"
        "###############################\nREGIONS %d ;\n", region_num);


    ObjectId regions_id = floorplan->getRegions();
    IdArray *obj_vector =
        Object::addr< IdArray >(regions_id);
    for (int i = 0; i < region_num; i++) {
        Constraint *region =
            Object::addr<Constraint>((*obj_vector)[i]);
        if (!region) {
            continue;
        }
        region->printRegion(io_manager);
    }
    io_manager.write("END REGIONS\n");

    return true;
}
static bool writeComponentMaskShift(IOManager &io_manager) {
    if (top_cell->getNumMaskShiftLayers() > 0) {
        io_manager.write("\n\n################################"
            "###############################################\n"
            "#[COMPONENTMASKSHIFT layer1 [layer2 ...] ;]\n"
            "#########################################"
            "######################################\n"
            "COMPONENTMASKSHIFT");

        for (int i = 0; i < top_cell->getNumMaskShiftLayers(); ++i) {
            ObjectId layer_id = top_cell->getMaskShiftLayer(i);
            Layer *layer = top_cell->getLayerByLayerId(layer_id);

            io_manager.write(" %s", layer->getName());
        }
        io_manager.write(" ;\n");
    }
    return true;
}
static bool writeComponents(IOManager &io_manager) {
    io_manager.write("\n\n###############################"
        "################################################\n"
        "# COMPONENTS numComps ;\n"
        "#     [– compName modelName\n"
        "#         [+ EEQMASTER macroName]\n"
        "#         [+ SOURCE {NETLIST | DIST | USER | TIMING}]\n"
        "#         [+ {FIXED pt orient | COVER pt orient | PLACED pt "
        "orient | UNPLACED} ]\n"
        "#         [+ MASKSHIFT shiftLayerMasks]\n"
        "#         [+ HALO [SOFT] left bottom right top]\n"
        "#         [+ ROUTEHALO haloDist minLayer maxLayer]\n"
        "#         [+ WEIGHT weight]\n"
        "#         [+ REGION regionName]\n"
        "#         [+ PROPERTY {propName propVal} ...]...\n"
        "#     ;] ...\n"
        "# END COMPONENTS\n"
        "########################################"
        "#######################################\n");

    uint64_t num_components = top_cell->getNumOfInsts();
    ObjectId components = top_cell->getInstances();
    ArrayObject<ObjectId> *component_vector =
        Object::addr< ArrayObject<ObjectId> >(components);
    if (!component_vector) {
        return true;
    }
    io_manager.write("COMPONENTS %d ;\n", num_components);

    for (auto iter = component_vector->begin(); iter != component_vector->end();
         ++iter) {
        Inst *instance = Object::addr<Inst>(*iter);
        if (!instance) {
            message->issueMsg("DBIO", 47, kError, "instance", (*iter));
            continue;
        }
        instance->print(io_manager);
    }
    io_manager.write("END COMPONENTS\n");

    return true;
}
static bool writePins(IOManager &io_manager) {
    uint64_t pin_num = top_cell->getNumOfIOPins();
    if (pin_num == 0) {
        return true;
    }

    const static int kIndentCharNum = 2;
    io_manager.write("PINS %d ;\n", pin_num);

    for (int i = 0; i < pin_num; i++) {
        Pin *pin = top_cell->getIOPin(i);
        if (!pin) {
            continue;
        }
        Term *term = pin->getTerm();
        if (!term->getIsWriteDef()) {
            continue;
        }
        io_manager.write("    - %s", pin->getName().c_str());

        Net *net = pin->getNet();
        io_manager.write(" + NET %s\n", (net ? net->getName().c_str() : "ERROR"));

        if (pin->getIsSpecial()) {
            io_manager.write("      + SPECIAL\n");
        }
        std::string str_value;
        if (term->hasDirection()) {
            str_value.assign(term->getDirectionStr());
            toUpper(str_value);
            io_manager.write("      + DIRECTION %s\n", str_value.c_str());
        }
        if (term->getNetExpr().size() != 0) {
            io_manager.write("      + NETEXPR \"%s\"\n", term->getNetExpr().c_str());
        }
        if (term->getSupplySensitivityIndex() != 0) {
            io_manager.write("      + SUPPLYSENSITIVITY %s\n",
                term->getSupplySensitivity().c_str());
        }
        if (term->getGroundSensitivityIndex() != 0) {
            io_manager.write("      + GROUNDSENSITIVITY %s\n",
                term->getGroundSensitivity().c_str());
        }
        if (term->hasUse()) {
            str_value.assign(term->getTypeStr());
            toUpper(str_value);
            io_manager.write("      + USE %s\n", str_value.c_str());
        }
        if (term->getAntennaPartialMetalAreaNum() > 0) {
            for (int i = 0; i < term->getAntennaPartialMetalAreaNum(); i++) {
                AntennaArea *a = term->getAntennaPartialMetalArea(i);

                io_manager.write("      + ANTENNAPINPARTIALMETALAREA %d",
                    a->getArea());
                Layer * layer = a->getLayer();
                if (layer != nullptr) {
                    io_manager.write(" LAYER %s\n", layer->getName());

                } else {
                    io_manager.write("\n");
                }
            }
        }
        if (term->getAntennaPartialMetalSideAreaNum() > 0) {
            for (int i = 0; i < term->getAntennaPartialMetalSideAreaNum();
                 i++) {
                AntennaArea *a = term->getAntennaPartialMetalSideArea(i);

                io_manager.write("      + ANTENNAPINPARTIALMETALSIDEAREA %d",
                    a->getArea());
                Layer * layer = a->getLayer();
                if (layer != nullptr) {
                    io_manager.write(" LAYER %s\n", layer->getName());
                } else {
                    io_manager.write("\n");
                }
            }
        }
        if (term->getAntennaPartialCutAreaNum() > 0) {
            for (int i = 0; i < term->getAntennaPartialCutAreaNum(); i++) {
                AntennaArea *a = term->getAntennaPartialCutArea(i);

                io_manager.write("      + ANTENNAPINPARTIALCUTAREA %d",
                    a->getArea());
                Layer *layer = a->getLayer();
                if (layer != nullptr) {
                    io_manager.write(" LAYER %s\n", layer->getName());
                } else {
                    io_manager.write("\n");
                }
            }
        }
        if (term->getAntennaDiffAreaNum() > 0) {
            for (int i = 0; i < term->getAntennaDiffAreaNum(); i++) {
                AntennaArea *a = term->getAntennaDiffArea(i);

                io_manager.write("      + ANTENNAPINDIFFAREA %d",
                    a->getArea());
                Layer * layer = a->getLayer();
                if (layer != nullptr) {
                    io_manager.write(" LAYER %s\n", layer->getName());
                } else {
                    io_manager.write("\n");
                }
            }
        }
        for (int i = 0; i < 6; i++) {
            const AntennaModelTerm *am = term->GetAntennaModelTerm(i);
            if (am->getValid()) {
                io_manager.write("      + ANTENNAMODEL OXIDE%d\n", i);

                if (am->getAntennaGateAreaNum() > 0) {
                    for (int i = 0; i < am->getAntennaGateAreaNum(); i++) {
                        AntennaArea *a = am->getAntennaGateArea(i);

                        io_manager.write("          + ANTENNAPINGATEAREA %d",
                            a->getArea());
                        Layer * layer = a->getLayer();
                        if (layer != nullptr) {
                            io_manager.write(" LAYER %s\n",
                                layer->getName());
                        } else {
                            io_manager.write("\n");
                        }
                    }
                }
                if (am->getAntennaMaxAreaCarNum() > 0) {
                    for (int i = 0; i < am->getAntennaMaxAreaCarNum(); i++) {
                        AntennaArea *a = am->getAntennaMaxAreaCar(i);

                        io_manager.write("          + ANTENNAPINMAXAREACAR %d",
                            a->getArea());
                        Layer * layer = a->getLayer();
                        if (layer != nullptr) {
                            io_manager.write(" LAYER %s\n",
                                layer->getName());
                        } else {
                            io_manager.write("\n");
                        }
                    }
                }
                if (am->getAntennaMaxSideAreaCarNum() > 0) {
                    for (int i = 0; i < am->getAntennaMaxSideAreaCarNum();
                         i++) {
                        AntennaArea *a = am->getAntennaMaxSideAreaCar(i);

                        io_manager.write("          + ANTENNAPINMAXSIDEAREACAR %d",
                            a->getArea());
                        Layer * layer = a->getLayer();
                        if (layer != nullptr) {
                            io_manager.write(" LAYER %s\n",
                                layer->getName());
                        } else {
                            io_manager.write("\n");
                        }
                    }
                }
                if (am->getAntennaMaxCutCarNum() > 0) {
                    for (int i = 0; i < am->getAntennaMaxCutCarNum(); i++) {
                        AntennaArea *a = am->getAntennaMaxCutCar(i);

                        io_manager.write("          + ANTENNAPINMAXCUTCAR %d",
                            a->getArea());
                        Layer * layer = a->getLayer();
                        if (layer != nullptr) {
                            io_manager.write(" LAYER %s\n",
                                layer->getName());
                        } else {
                            io_manager.write("\n");
                        }
                    }
                }
            }
        }
        if (term->getPortNum() > 0) {
            for (int i = 0; i < term->getPortNum(); i++) {
                Port *p = term->getPort(i);
                uint32_t space_count = kIndentCharNum * 3;
                if (p->getIsReal()) {
                    io_manager.write("      + PORT\n");
                    space_count += kIndentCharNum;
                }
                
                if (p->getLayerGeometryNum() > 0) {
                    for (int j = 0; j < p->getLayerGeometryNum(); j++) {
                        LayerGeometry *lg = p->getLayerGeometry(j);

                        if (!lg) continue;
                        if (p->getHasPlacement()) {
                            Transform transform(pin, p);
                            lg->printDEF(io_manager, space_count, transform);
                        } else {
                            lg->printDEF(io_manager, space_count);
                        }
                    }
                }
                if (p->getHasPlacement()) {
                    Point pt = p->getLocation();
                    if (p->getStatus() == PlaceStatus::kCover) {
                        if (p->getIsReal()) {
                            io_manager.write("        + COVER ( %d %d ) %s\n",
                                pt.getX(), pt.getY(),
                                toString(p->getOrient()).c_str());
                        } else {
                            io_manager.write("      + COVER ( %d %d ) %s\n",
                                pt.getX(), pt.getY(),
                                toString(p->getOrient()).c_str());
                        }
                    } else if (p->getStatus() == PlaceStatus::kFixed) {
                        if (p->getIsReal()) {
                            io_manager.write("        + FIXED ( %d %d ) %s\n",
                                pt.getX(), pt.getY(),
                                toString(p->getOrient()).c_str());
                        } else {
                            io_manager.write("        + FIXED ( %d %d ) %s\n",
                                pt.getX(), pt.getY(),
                                toString(p->getOrient()).c_str());
                        }
                    } else if (p->getStatus() == PlaceStatus::kPlaced) {
                        if (p->getIsReal()) {
                            io_manager.write("        + PLACED ( %d %d ) %s\n",
                                pt.getX(), pt.getY(),
                                toString(p->getOrient()).c_str());
                        } else {
                            io_manager.write("      + PLACED ( %d %d ) %s\n",
                                pt.getX(), pt.getY(),
                                toString(p->getOrient()).c_str());
                        }
                    }
                }
            }
        }
        io_manager.write("    ;\n");
    }
    io_manager.write("END PINS\n");

    return true;
}
static bool writePinProperties(IOManager &io_manager) {
    return true;
}
static bool writeBlockages(IOManager &io_manager) {
    Floorplan *floorplan = top_cell->getFloorplan();
    if (!floorplan) {
        message->issueMsg("DBIO", 44, kError, "floorplan");
        return false;
    }
    int sum_blockages = floorplan->getNumOfPlaceBlockages() +
                        floorplan->getNumOfRouteBlockages();
    if (0 == sum_blockages) {
        return true;
    }

    ObjectId route_blockages = floorplan->getRouteBlockages();
    IdArray *route_vector = nullptr;
    if (route_blockages > 0) {
        route_vector = Object::addr< IdArray >(route_blockages);
        if (!route_vector) {
            message->issueMsg("DBIO", 44, kError, "route blockage vector");
            return false;
        }
    }
    ObjectId place_blockages = floorplan->getPlaceBlockages();
    IdArray *place_vector = nullptr;
    if (place_blockages > 0) {
        place_vector = Object::addr< IdArray >(place_blockages);
        if (!place_vector) {
            message->issueMsg("DBIO", 44, kError, "place blockage vector");
            return false;
        }
    }

    io_manager.write("\n\n################################"
        "###############################################\n"
        "# [BLOCKAGES numBlockages ;\n"
        "#     [- LAYER layerName  [ + SLOTS | + FILLS] [ + PUSHDOWN]"
        " [ + EXCEPTPGNET]\n"
        "#       [ + COMPONENT compName]\n"
        "#       [ + SPACING minSpacing | + DESIGNRULEWIDTH effectiveWidth]\n"
        "#       [ + MASK maskNum]\n"
        "#            {RECT pt pt | POLYGON pt pt pt ...} ...\n"
        "#     ;] ...\n"
        "#     [- PLACEMENT [ + SOFT | + PARTIAL maxDensity] [ + PUSHDOWN] "
        "[ + COMPONENT compName]\n"
        "#            {RECT pt pt} ...\n"
        "#     ;] ...\n"
        "# END BLOCKAGES]\n"
        "########################################"
        "#######################################\n"
        "BLOCKAGES %d ;\n", sum_blockages);


    if (route_vector) {
        for (int i = 0; i < route_vector->getSize(); ++i) {
            Constraint *route_blockage =
                Object::addr<Constraint>((*route_vector)[i]);
            if (!route_blockage) {
                message->issueMsg("DBIO", 47, kError, "route blockage", i);
                continue;
            }
            route_blockage->printBlockage(io_manager);
        }
    }
    if (place_vector) {
        for (int i = 0; i < place_vector->getSize(); ++i) {
            Constraint *place_blockage =
                Object::addr<Constraint>((*place_vector)[i]);
            if (!place_blockage) {
                message->issueMsg("DBIO", 47, kError, "place blockage", i);
                continue;
            }
            place_blockage->printBlockage(io_manager);
        }
    }
    io_manager.write("END BLOCKAGES\n");

    return true;
}
static bool writeSlots(IOManager &io_manager) {
    return true;
}
static bool writeFills(IOManager &io_manager) {
    int fill_num = top_cell->getNumOfFills();
    if (fill_num == 0) {
        return true;
    }
    io_manager.write("FILLS %d ;\n", fill_num);

    for (int i = 0; i < fill_num; i++) {
        Fill *fill = top_cell->getFill(i);
        if (!fill) {
            continue;
        }
        if (fill->getIsLayer()) {
            Layer *layer = top_cell->getTechLib()->getLayer(fill->getLayerId());
            if (!layer) {
                continue;
            }
            io_manager.write("    - LAYER %s ", layer->getName());

            if (fill->getLayerMask()) {
                io_manager.write("+ MASK %d ",
                    fill->getLayerMask());
            }
            if (fill->getIsOpc()) {
                io_manager.write("+ OPC");
            }
            std::vector<std::vector<Point> > *points = fill->getPointsArray();
            for (unsigned int j = 0; j < points->size(); j++) {
                if (fill->getIsRect(j)) {
                    io_manager.write("\n        RECT ( %d %d ) ( %d %d )",
                        (*points)[j][0].getX(),
                        (*points)[j][0].getY(),
                        (*points)[j][1].getX(),
                        (*points)[j][1].getY());
                } else {
                    io_manager.write("\n        POLYGON");
                    for (int k = 0; k < (*points)[j].size(); k++) {
                        io_manager.write(" ( %d %d )",
                        (*points)[j][k].getX(),
                        (*points)[j][k].getY());
                    }
                }
            }
            io_manager.write(" ;\n");
        } else if (fill->getIsVia()) {
            ViaMaster *via = fill->getVia();
            if (!via) {
                continue;
            }
            io_manager.write("    - VIA %s ", via->getName().c_str());

            if (fill->getViaTopMask() || fill->getViaCutMask() ||
                fill->getViaBotMask()) {
                    io_manager.write("+ MASK ");
                if (fill->getViaTopMask()) {
                    io_manager.write("%d", fill->getViaTopMask());
                }
                if (fill->getViaCutMask()) {
                    io_manager.write("%d", fill->getViaCutMask());
                }
                if (fill->getViaBotMask()) {
                    io_manager.write("%d", fill->getViaBotMask());
                }
                io_manager.write(" ");
            }
            if (fill->getIsOpc()) {
                io_manager.write("+ OPC");
            }
            std::vector<std::vector<Point> > *points = fill->getPointsArray();
            for (unsigned int j = 0; j < points->size(); j++) {
                for (int k = 0; k < (*points)[j].size(); k++) {
                    io_manager.write(" ( %d %d )",
                        (*points)[j][k].getX(),
                        (*points)[j][k].getY());
                }
            }
            io_manager.write(" ;\n");
        }
    }
    io_manager.write("END FILLS\n");

    return true;
}
static bool writeSpecialNets(IOManager &io_manager) {
    int special_nets_num = top_cell->getNumOfSpecialNets();
    if (special_nets_num == 0) return true;
    ObjectId special_nets = top_cell->getSpecialNets();
    ArrayObject<ObjectId> *special_net_vector =
        Object::addr< ArrayObject<ObjectId> >(special_nets);
    io_manager.write("\nSPECIALNETS %d ;\n", special_nets_num);

    for (int i = 0; i < special_net_vector->getSize(); ++i) {
        SpecialNet *special_net =
            Object::addr<SpecialNet>((*special_net_vector)[i]);
        if (!special_net) {
            message->issueMsg("DBIO", 47, kError, "special net", i);
            continue;
        }
        special_net->printDEF(io_manager);
    }
    io_manager.write("END SPECIALNETS\n");

    return true;
}
static bool writeNets(IOManager &io_manager) {
    int nets_num = top_cell->getNumOfNets();
    if (nets_num == 0) return true;
    ObjectId nets = top_cell->getNets();
    ArrayObject<ObjectId> *net_vector =
                                   Object::addr< ArrayObject<ObjectId> >(nets);
    io_manager.write("\nNETS %d ;\n", nets_num);

    for (auto iter = net_vector->begin(); iter != net_vector->end(); ++iter) {
        Net *net = Object::addr<Net>(*iter);
        if (!net) {
            message->issueMsg("DBIO", 47, kError, "net", (*iter));
            continue;
        }
        if (net->getIsBusNet()) {
            continue;
        }
        net->printDEF(io_manager);
    }
    io_manager.write("END NETS\n");

    return true;
}
static void writeScanChainPoint(IOManager &io_manager, ScanChainPoint *point,
                                bool is_start) {
    if (is_start) {
        io_manager.write("        + START");
    } else {
        io_manager.write("        + STOP");
    }
    if (point->getIsIOPin()) {
        io_manager.write(" PIN");

        if (point->getHasPin()) {
            Pin *pin = top_cell->getIOPinById(point->getPinId());
            if (pin) {
                io_manager.write(" %s", pin->getName().c_str());
            }
        }
    } else {
        Inst *inst = top_cell->getInstance(point->getInstId());
        if (inst) {
            io_manager.write(" %s", inst->getName().c_str());

            Pin *pin = inst->getPinById(point->getPinId());
            if (pin) {
                io_manager.write(" %s", pin->getName().c_str());
            }
        }
    }
    io_manager.write("\n");
}
static void writeScanChainList(IOManager &io_manager, ScanChainList *list,
                               bool is_floating) {
    for (; list; list = list->getNext()) {
        if (is_floating) {
            io_manager.write("        + FLOATING\n");
        } else {
            io_manager.write("        + ORDERED\n");
        }
        ScanChainListItem *items = list->getItems();
        for (ScanChainListItem *item = items; item; item = item->getNext()) {
            Inst *inst = top_cell->getInstance(item->getInstId());
            if (inst) {
                io_manager.write("            %s", inst->getName().c_str());

                if (item->getHasInPin()) {
                    Pin *pin = inst->getPinById(item->getInPinId());
                    if (pin) {
                        io_manager.write(" ( IN %s )",
                            pin->getName().c_str());
                    }
                }
                if (item->getHasOutPin()) {
                    Pin *pin = inst->getPinById(item->getOutPinId());
                    if (pin) {
                        io_manager.write(" ( OUT %s )",
                            pin->getName().c_str());
                    }
                }
                if (item->getHasBits()) {
                    io_manager.write(" ( BITS %d )",
                        item->getNumBits());
                }
                io_manager.write("\n");
            }
        }
        io_manager.write("\n");
    }
}
static bool writeScanChains(IOManager &io_manager) {
    int scan_chain_num = top_cell->getNumOfScanChains();
    if (scan_chain_num == 0) {
        return true;
    }
    io_manager.write("SCANCHAINS %d", scan_chain_num);
    for (int i = 0; i < scan_chain_num; i++) {
        ScanChain *scan_chain = top_cell->getScanChain(i);
        if (!scan_chain) {
            continue;
        }
        io_manager.write("    - %s\n", scan_chain->getChainName());
        if (scan_chain->getHasPartition()) {
            io_manager.write("        + PARTITION %s ",
                scan_chain->getPartitionName());
            if (scan_chain->getHasMaxBits()) {
                io_manager.write("MAXBITS %d",
                    scan_chain->getMaxBits());
            }
            io_manager.write("\n");
        }
        if (scan_chain->getHasCommonScanPins()) {
            io_manager.write("        + COMMONSCANPINS");
            if (scan_chain->getHasCommonInPin()) {
                io_manager.write(" ( IN %s )",
                    scan_chain->getCommonInPin());
            }
            if (scan_chain->getHasCommonOutPin()) {
                io_manager.write(" ( OUT %s )",
                    scan_chain->getCommonOutPin());
            }
            io_manager.write("\n");
        }
        ScanChainPoint *start = scan_chain->getStart();
        if (start) {
            writeScanChainPoint(io_manager, start, true);
        }
        ScanChainList *floating = scan_chain->getFloating();
        if (floating) {
            writeScanChainList(io_manager, floating, true);
        }
        ScanChainList *ordered = scan_chain->getOrdered();
        if (ordered) {
            writeScanChainList(io_manager, ordered, false);
        }
        ScanChainPoint *stop = scan_chain->getStop();
        if (stop) {
            writeScanChainPoint(io_manager, stop, false);
        }
        io_manager.write("    ;\n");
    }
    io_manager.write("END SCANCHAINS\n");
    return true;
}
static bool writeGroups(IOManager &io_manager) {
    int groups_num = top_cell->getNumOfGroups();
    if (groups_num == 0) {
        return true;
    }
    ObjectId groups = top_cell->getGroups();
    ArrayObject<ObjectId> *group_vector =
        Object::addr< ArrayObject<ObjectId> >(groups);

    io_manager.write("\n\n################################"
        "###############################################\n"
        "# [GROUPS numGroups ;\n"
        "#      [– groupName [compNamePattern ... ]\n"
        "#         [+ REGION regionNam]\n"
        "#         [+ PROPERTY {propName propVal} ...] ...\n"
        "#      ;] ...\n"
        "# END GROUPS]\n"
        "########################################"
        "#######################################\n"
        "GROUPS %d ;\n", groups_num);

    for (int i = 0; i < group_vector->getSize(); ++i) {
        Group *group = Object::addr<Group>((*group_vector)[i]);
        if (!group) {
            message->issueMsg("DBIO", 47, kError, "group", i);
            continue;
        }
        group->print(io_manager);
    }
    io_manager.write("END GROUPS\n");

    return true;
}
static bool writeExtension(IOManager &io_manager) { return true; }
static bool writeEndDesign(IOManager &io_manager) {
    io_manager.write("END DESIGN\n");
    return true;
}

}  // namespace db
}  // namespace open_edi
