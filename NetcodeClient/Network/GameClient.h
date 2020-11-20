#pragma once

#include "NetwUtil.h"
#include <Netcode/Network/ClientSession.h>

class GameClient {
	Ref<nn::ClientSession> clientSession;
	Ref<Connection> playerConnection;
	Ref<AnimationSet> remoteAvatarAnimationSet;
	std::vector<ServerReconciliation> reconciliations;
	std::vector<GameObject *> remoteObjects;
	std::vector<ReplData> replicationData;
	Netcode::GameClock * clock;
	uint32_t processedTick;

	void FetchUpdate();

	void ProcessUpdate();

	void ProcessCommand(const ServerReconciliation & sr);

	void ProcessResult(const ServerReconciliation & sr);

	void RemoveRemoteObjectsByOwner(int32_t ownerId);
	GameObject* ClientCreateRemoteAvatar(int32_t playerId, uint32_t objId);
	
public:
	void SetClock(Netcode::GameClock* gameClock) {
		clock = gameClock;
	}
	
	bool IsConnected() const;
	
	bool IncludeNetworkTick();
	
	void Stop() {
		
	}

	void Receive();

	void Send();

	void SendAction(ClientAction ca);

	void SendDebug();
	
	void Start(Netcode::Module::INetworkModule * network, Netcode::GameClock * gameClock);
};