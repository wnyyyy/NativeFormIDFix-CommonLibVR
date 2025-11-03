#pragma once

class GameEventHandler
{
private:
	GameEventHandler() {};
	GameEventHandler(GameEventHandler&) = delete;
	GameEventHandler& operator=(GameEventHandler&&) = delete;
	void operator=(GameEventHandler&) = delete;

public:
	[[nodiscard]] static GameEventHandler& getInstance()
	{
		static GameEventHandler instance;
		return instance;
	}

	void onLoad();
};
