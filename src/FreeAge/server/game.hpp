#pragma once

#include <memory>
#include <vector>

#include <QByteArray>
#include <QString>
#include <QTcpSocket>

#include "FreeAge/common/free_age.hpp"
#include "FreeAge/common/resources.hpp"
#include "FreeAge/server/map.hpp"
#include "FreeAge/server/settings.hpp"

class ServerBuilding;
class ServerUnit;

/// Represents a player in a game.
struct PlayerInGame {
  /// The player's index in the playersInGame vector.
  int index;
  
  /// Socket that can be used to send and receive data to/from the player.
  QTcpSocket* socket;
  
  /// Buffer for bytes that have been received from the client, but could not
  /// be parsed yet (because only a partial message was received so far).
  QByteArray unparsedBuffer;
  
  /// The player name as provided by the client.
  QString name;
  
  /// The player color index.
  int playerColorIndex;
  
  /// The last point in time at which a ping was received from this player.
  TimePoint lastPingTime;
  
  /// Whether the player finished loading the game resources.
  bool finishedLoading = false;
  
  /// The current game resources of the player (wood, food, gold, stone).
  ResourceAmount resources;
  
  /// The resources of the player at the end of the last game simulation step.
  /// Only if this differs from the current resource amount, then an update message
  /// needs to be sent to the client.
  ResourceAmount lastResources;
};

class Game {
 public:
  Game(ServerSettings* settings);
  
  void RunGameLoop(std::vector<std::shared_ptr<PlayerInGame>>* playersInGame);
  
 private:
  enum class ParseMessagesResult {
    NoAction = 0,
    PlayerLeftOrShouldBeDisconnected
  };
  
  void HandleLoadingProgress(const QByteArray& msg, PlayerInGame* player, const std::vector<std::shared_ptr<PlayerInGame>>& players);
  void SendChatBroadcast(u16 sendingPlayerIndex, const QString& text, const std::vector<std::shared_ptr<PlayerInGame>>& players);
  void HandleChat(const QByteArray& msg, PlayerInGame* player, u32 len, const std::vector<std::shared_ptr<PlayerInGame>>& players);
  void HandlePing(const QByteArray& msg, PlayerInGame* player);
  void HandleMoveToMapCoordMessage(const QByteArray& msg, PlayerInGame* player, u32 len);
  void HandleProduceUnitMessage(const QByteArray& msg, PlayerInGame* player);
  ParseMessagesResult TryParseClientMessages(PlayerInGame* player, const std::vector<std::shared_ptr<PlayerInGame>>& players);
  
  // TODO: Right now, this creates a message containing the whole map content.
  //       Later, only the areas seen by a client (and a small border around them)
  //       should be sent to the client.
  QByteArray CreateMapUncoverMessage();
  QByteArray CreateAddObjectMessage(u32 objectId, ServerObject* object);
  
  inline double GetCurrentServerTime() { return SecondsDuration(Clock::now() - settings->serverStartTime).count(); }
  
  void StartGame();
  void SimulateGameStep(double gameStepServerTime, float stepLengthInSeconds);
  void SimulateGameStepForUnit(u32 unitId, ServerUnit* unit, float stepLengthInSeconds, std::vector<QByteArray>* accumulatedMessages);
  void SimulateGameStepForBuilding(u32 buildingId, ServerBuilding* building, float stepLengthInSeconds, std::vector<QByteArray>* accumulatedMessages);
  
  void ProduceUnit(ServerBuilding* building, UnitType unitInProduction, std::vector<QByteArray>* accumulatedMessages);
  
  
  std::shared_ptr<ServerMap> map;
  
  /// The server time in seconds at which the actual game begins (after all clients
  /// finished loading).
  double gameBeginServerTime;
  
  /// The server time at which the last game iteration was simulated.
  /// The next iteration is due 1 / FPS seconds after this time.
  double lastSimulationTime;
  
  std::vector<std::shared_ptr<PlayerInGame>>* playersInGame;
  ServerSettings* settings;  // not owned
};