#pragma once

#include <QByteArray>

#include "FreeAge/free_age.h"

static constexpr int hostTokenLength = 6;

/// Types of messages sent by clients to the server.
enum class ClientToServerMessage {
  /// Initial message sent by the host to the server.
  HostConnect = 0,
  
  /// Initial message sent by a non-host to the server.
  Connect,
  
  /// A message that contains the latest game settings set by the host.
  SettingsUpdate,
  
  /// Sent when the ready state of the client changes.
  ReadyUp,
  
  /// A chat message.
  Chat,
  
  /// A regularly sent message to check whether the client is still
  /// connected and to measure the current ping.
  Ping,
  
  /// Sent by the player upon leaving the match.
  Leave,
};

QByteArray CreateHostConnectMessage(const QByteArray& hostToken, const QString& playerName);

QByteArray CreateConnectMessage(const QString& playerName);

QByteArray CreateSettingsUpdateMessage(bool allowMorePlayersToJoin, u16 mapSize, bool isBroadcast);

QByteArray CreateReadyUpMessage(bool clientIsReady);

QByteArray CreateChatMessage(const QString& text);

QByteArray CreatePingMessage(u64 number);

QByteArray CreateLeaveMessage();


/// Types of messages sent by the server to clients.
enum class ServerToClientMessage {
  /// A response to the ClientToServerMessage::HostConnect and
  /// ClientToServerMessage::Connect messages.
  Welcome = 0,
  
  /// A message that the server sends to all non-host clients after the host changed a setting.
  SettingsUpdateBroadcast,
  
  /// The game has been aborted because the host left.
  GameAborted,
  
  /// An update to the player list.
  PlayerList,
  
  /// A chat message.
  ChatBroadcast,
  
  /// A response to a ping message, containing the value of the server time at the time
  /// the response message was sent.
  PingResponse,
};

QByteArray CreateWelcomeMessage();

QByteArray CreateGameAbortedMessage();

QByteArray CreateChatBroadcastMessage(u16 sendingPlayerIndex, const QString& text);

QByteArray CreatePingResponseMessage(u64 number, double serverTimeSeconds);