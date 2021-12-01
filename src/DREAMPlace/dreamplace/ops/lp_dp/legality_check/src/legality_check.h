/*************************************************************************
    > File Name: legality_check.h
    > Author: Fei
    > Mail:
    > Created Time: Fri 25 Sep 2020 03:20:14 PM CDT
 ************************************************************************/

#ifndef _DREAMPLACE_OEM_LEGALITY_CHECK_H
#define _DREAMPLACE_OEM_LEGALITY_CHECK_H

#include <vector>
#include <limits>
#include <math.h>
#include "utility/src/Msg.h"
#include "flow/src/common_place_DB.h"

DREAMPLACE_BEGIN_NAMESPACE
bool legalityCheckRun(CommonPlaceDB* db);

/// @brief a wrapper class of required data for edi db
class LegalityCheck
{
  public:
    LegalityCheck()
    {
      db_ = CommonPlaceDB::getPlaceDBInstance();
    }
    ~LegalityCheck()                               = default;
    LegalityCheck(const LegalityCheck&)            = delete;  // no copy
    LegalityCheck &operator=(const LegalityCheck&) = delete;  // no copy

    // common DB interface : get
    CommonPlaceDB*       getDB()                    const { return db_;                             }
    bool                 isCommonDBReady()          const { return db_ && db_->isCommonDBReady();   }
    const Coord*         getInitX()                 const { return db_->getInitX();                 }
    const Coord*         getInitY()                 const { return db_->getInitX();                 }
    const Coord*         getNodeSizeX()             const { return db_->getNodeSizeX();             }
    const Coord*         getNodeSizeY()             const { return db_->getNodeSizeY();             }
    const Coord*         getFlatRegionBoxes()       const { return db_->getFlatRegionBoxes();       }
    const int*           getFlatRegionBoxesStart()  const { return db_->getFlatRegionBoxesStart();  }
    const int*           getNode2RegionMap()        const { return db_->getNode2RegionMap();        }
    const Coord*         getFlatFenceBoxes()        const { return db_->getFlatFenceBoxes();        }
    const int*           getFlatFenceBoxesStart()   const { return db_->getFlatFenceBoxesStart();   }
    const int*           getNode2FenceMap()         const { return db_->getNode2FenceMap();         }
    const int*           getFlatNet2PinMap()        const { return db_->getFlatNet2PinMap();        } 
    const int*           getFlatNet2PinStartMap()   const { return db_->getFlatNet2PinStartMap();   } 
    const int*           getPin2NetMap()            const { return db_->getPin2NetMap();            } 
    const int*           getFlatNode2PinMap()       const { return db_->getFlatNode2PinMap();       } 
    const int*           getFlatNode2PinStartMap()  const { return db_->getFlatNode2PinStartMap();  } 
    const int*           getFlatPin2NodeMap()       const { return db_->getFlatPin2NodeMap();       } 
    const Coord*         getPinOffsetX()            const { return db_->getPinOffsetX();            } 
    const Coord*         getPinOffsetY()            const { return db_->getPinOffsetY();            } 
    const unsigned char* getNetMask()               const { return db_->getNetMask();               } 
    int                  getNumNodes()              const { return db_->getNumNodes();              }
    int                  getNumNets()               const { return db_->getNumNets();               }
    int                  getNumPins()               const { return db_->getNumPins();               }
    int                  getNumRegions()            const { return db_->getNumRegions();            }
    int                  getNumFences()             const { return db_->getNumFences();             }

    // internal DB interface: get
    double         getNodeWeight(int id)            const { return db_->getNodeWeight(id);     }
    const Coord*   getCurX()                        const { return db_->getCurX();             } 
    const Coord*   getCurY()                        const { return db_->getCurY();             }
    Coord          getAreaLLX()                     const { return db_->getAreaLLX();          }
    Coord          getAreaLLY()                     const { return db_->getAreaLLY();          }
    Coord          getAreaURX()                     const { return db_->getAreaURX();          }
    Coord          getAreaURY()                     const { return db_->getAreaURY();          }
    Coord          getSiteWidth()                   const { return db_->getSiteWidth();        }
    Coord          getRowHight()                    const { return db_->getRowHight();         }
    Coord          getBinSizeX()                    const { return db_->getBinSizeX();         }
    Coord          getBinSizeY()                    const { return db_->getBinSizeY();         }
    int            getNumBinsX()                    const { return db_->getNumBinsX();         }
    int            getNumBinsY()                    const { return db_->getNumBinsY();         }
    int            getNumSitesX()                   const { return db_->getNumSitesX();        }
    int            getNumSitesY()                   const { return db_->getNumSitesY();        }
    int            getNumMoveableNodes()            const { return db_->getNumMoveableNodes(); }
  
    // interface 
    int run();
  private:
    CommonPlaceDB* db_ = nullptr;
};

DREAMPLACE_END_NAMESPACE

#endif