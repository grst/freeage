#include "FreeAge/server/building.hpp"

#include "FreeAge/common/logging.hpp"
#include "FreeAge/server/game.hpp"

ServerBuilding::ServerBuilding(int playerIndex, BuildingType type, const QPoint& baseTile)
    : ServerObject(ObjectType::Building, playerIndex),
      productionPercentage(0),
      type(type),
      baseTile(baseTile) {}

bool ServerBuilding::CanProduce(UnitType unitType, PlayerInGame* /*player*/) {
  // Can the building produce this unit in principle?
  if (type == BuildingType::TownCenter) {
    if (unitType != UnitType::FemaleVillager &&
        unitType != UnitType::MaleVillager) {
      return false;
    }
  } else {
    return false;
  }
  
  // Does the player have the necessary civilization / technologies to produce this unit?
  // TODO
  
  return true;
}

bool ServerBuilding::IsUnitQueued(UnitType* type) {
  if (!productionQueue.empty()) {
    *type = productionQueue.front();
  }
  return !productionQueue.empty();
}

void ServerBuilding::RemoveCurrentItemFromQueue() {
  if (productionQueue.empty()) {
    LOG(ERROR) << "RemoveCurrentItemFromQueue() called on an empty queue";
    return;
  }
  productionQueue.erase(productionQueue.begin());
}