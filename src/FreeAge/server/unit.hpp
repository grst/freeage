#pragma once

#include <QPointF>

#include "FreeAge/common/unit_types.hpp"
#include "FreeAge/server/object.hpp"

/// Represents a unit on the server.
class ServerUnit : public ServerObject {
 public:
  ServerUnit(int playerIndex, UnitType type, const QPointF& mapCoord);
  
  inline UnitType GetUnitType() const { return type; }
  
  inline const QPointF& GetMapCoord() const { return mapCoord; }
  inline void SetMapCoord(const QPointF& mapCoord) { this->mapCoord = mapCoord; }
  
  inline UnitAction GetCurrentAction() const { return currentAction; }
  inline void SetCurrentAction(UnitAction newAction) { currentAction = newAction; }
  
  /// Attempts to command the unit to interact with the given target object.
  /// If the unit cannot actually interact with that object, this call does nothing.
  void SetTarget(u32 targetObjectId, ServerObject* targetObject);
  void RemoveTarget();
  inline u32 GetTargetObjectId() const { return targetObjectId; }
  
  /// Commands the unit to move to the given mapCoord.
  void SetMoveToTarget(const QPointF& mapCoord);
  inline bool HasMoveToTarget() const { return hasMoveToTarget; }
  inline const QPointF& GetMoveToTargetMapCoord() const { return moveToTarget; }
  
  // TODO: Accept more complex paths (rather than just a single target).
  inline bool HasPath() const { return hasPath; }
  inline void SetPath(const QPointF& pathTarget) { hasPath = true; this->pathTarget = pathTarget; }
  inline void StopMovement() { currentAction = UnitAction::Idle; hasMoveToTarget = false; hasPath = false; currentMovementDirection = QPointF(0, 0); }
  inline const QPointF& GetPathTarget() const { return pathTarget; }
  
  inline const QPointF& GetMovementDirection() const { return currentMovementDirection; }
  inline void SetMovementDirection(const QPointF& direction) { currentMovementDirection = direction; }
  
  // TODO: Load this from some database for each unit type
  inline float GetMoveSpeed() const { return (type == UnitType::Scout) ? 2.f : 1.f; }
  
 private:
  void SetTargetInternal(u32 targetObjectId, ServerObject* targetObject);
  
  
  UnitType type;
  QPointF mapCoord;
  
  UnitAction currentAction;
  
  /// The unit's target object (if any). Set to kInvalidObjectId if the unit does not have a target.
  u32 targetObjectId = kInvalidObjectId;
  
  /// The unit's map coord target (if any).
  bool hasMoveToTarget = false;
  QPointF moveToTarget;
  
  /// The currenly planned path to the unit's target.
  bool hasPath = false;
  QPointF pathTarget;
  
  /// The current movement direction of the unit for the current linear segment of its planned path.
  /// This is in general the only movement-related piece of information that the clients know about.
  /// If this changes, the clients that see the unit need to be notified.
  QPointF currentMovementDirection = QPointF(0, 0);
};
