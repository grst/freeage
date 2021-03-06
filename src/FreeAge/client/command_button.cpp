// Copyright 2020 The FreeAge authors
// This file is part of FreeAge, licensed under the new BSD license.
// See the COPYING file in the project root for the license text.

#include "FreeAge/client/command_button.hpp"

#include "FreeAge/client/game_controller.hpp"
#include "FreeAge/client/unit.hpp"

void CommandButton::InitializePointBuffers() {
  QOpenGLFunctions_3_2_Core* f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
  
  int elementSizeInBytes = 3 * sizeof(float);
  
  f->glGenBuffers(1, &iconPointBuffer);
  f->glBindBuffer(GL_ARRAY_BUFFER, iconPointBuffer);
  f->glBufferData(GL_ARRAY_BUFFER, 1 * elementSizeInBytes, nullptr, GL_STREAM_DRAW);
  
  f->glGenBuffers(1, &overlayPointBuffer);
  f->glBindBuffer(GL_ARRAY_BUFFER, overlayPointBuffer);
  f->glBufferData(GL_ARRAY_BUFFER, 1 * elementSizeInBytes, nullptr, GL_STREAM_DRAW);
}

void CommandButton::UnloadPointBuffers() {
  QOpenGLFunctions_3_2_Core* f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
  f->glDeleteBuffers(1, &iconPointBuffer);
  f->glDeleteBuffers(1, &overlayPointBuffer);
}

void CommandButton::SetInvisible() {
  type = Type::Invisible;
  this->hotkey = Qt::Key_unknown;
}

void CommandButton::SetBuilding(BuildingType type, Qt::Key hotkey) {
  this->type = Type::ConstructBuilding;
  this->hotkey = hotkey;
  
  buildingConstructionType = type;
}

void CommandButton::SetProduceUnit(UnitType type, Qt::Key hotkey) {
  this->type = Type::ProduceUnit;
  this->hotkey = hotkey;
  
  unitProductionType = type;
}

void CommandButton::SetAction(ActionType actionType, const Texture* texture, Qt::Key hotkey) {
  this->type = Type::Action;
  this->hotkey = hotkey;
  
  this->actionType = actionType;
  this->texture = texture;
}

CommandButton::State CommandButton::GetState(GameController* gameController) const {

  // NOTE: Checks must follow an order of importance. For example the CannotAfford is less
  // important than MaxLimitReached because if the max limit has been reached, if it can be
  // affored or not is irrelevant.

  if (GetType() == CommandButton::Type::ProduceUnit) {
    if (!gameController->GetLatestKnownResourceAmount().CanAfford(
            GetUnitCost(GetUnitProductionType()))) {
      return State::CannotAfford;
    }

    // TODO: return State::Locked, if the unit is not unlocked yet

  } else if (GetType() == CommandButton::Type::ConstructBuilding) {
    BuildingType buildingType = GetBuildingConstructionType();

    // Check for the max limit
    int max = GetBuildingMaxInstances(buildingType);
    if (max != -1 && gameController->GetBuildingTypeCount(buildingType) >= max) {
      return State::MaxLimitReached;
    }

    // TODO: return State::Locked, if the unit is not unlocked yet

    // Can afford must be the last check (less important than all the other checks)
    if (!gameController->GetLatestKnownResourceAmount().CanAfford(GetBuildingCost(buildingType))) {
      return State::CannotAfford;
    }
  }

  return State::Valid;
}

void CommandButton::Render(
    float x, float y, float size, float iconInset,
    const Texture& iconOverlayNormalTexture,
    UIShader* uiShader, int widgetWidth, int widgetHeight, QOpenGLFunctions_3_2_Core* f) {
  if (type == Type::Invisible) {
    return;
  } else if (type == Type::ConstructBuilding ||
             type == Type::ProduceUnit ||
             type == Type::Action) {
    const Texture* texture;
    if (type == Type::ConstructBuilding) {
      texture = ClientBuildingType::GetBuildingTypes()[static_cast<int>(buildingConstructionType)].GetIconTexture();
    } else if (type == Type::ProduceUnit) {
      texture = ClientUnitType::GetUnitTypes()[static_cast<int>(unitProductionType)].GetIconTexture();
    } else {  // if (type == Type::Action) {
      texture = this->texture;
    }
    
    RenderUIGraphic(
        x + iconInset,
        y + iconInset,
        size - 2 * iconInset,
        size - 2 * iconInset,
        qRgba(255, 255, 255, 255), iconPointBuffer,
        *texture,
        uiShader, widgetWidth, widgetHeight, f);
    RenderUIGraphic(
        x,
        y,
        size,
        size,
        qRgba(255, 255, 255, 255), overlayPointBuffer,
        iconOverlayNormalTexture,
        uiShader, widgetWidth, widgetHeight, f);
  }
  
  buttonRect = QRectF(x, y, size, size);
}

void CommandButton::Pressed(const std::vector<u32>& selection, GameController* gameController, bool shift) {
  // called only if state is Valid
  
  if (type == Type::Invisible) {
    LOG(ERROR) << "An invisible button has been pressed.";
    return;
  } else if (type == Type::ProduceUnit) {
    int count = shift ? 5 : 1;
    gameController->ProduceUnit(selection, unitProductionType, count);
  }
}
