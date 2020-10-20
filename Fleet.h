#pragma once
#include "Ship.h"

class Fleet {
public:
	Ship ship[10];
	Fleet() {
		int n = 1;
		for (int i = 0; i < 10; i++)
		{
			if (i < 6) n = 2;
			if (i < 3) n = 3;
			if (i == 0) n = 4;
			ship[i].col_deck = n;
			ship[i].decks = new bool[n];
			ship[i].desp_of_part = new ofPoint[n];
		}
	}
	vector<Ship> GetDestroyedShips() {
		vector<Ship> result;
		for (int i = 0; i < 10; ++i) {
			if (ship[i].IsDestroyed()) {
				result.push_back(ship[i]);
			}
		}
		return result;
	}
	const Ship& GetShip(int x, int y) {
		for (int i = 0; i < 10; i++)
			for (int j = 0; j < ship[i].col_deck; j++) {
				if (ship[i].desp_of_part[j].x == x &&
					ship[i].desp_of_part[j].y == y) {
					return ship[i];
				}
			}
		return Ship();
	}

	bool IsDestroyed() const{
		for (int i = 0; i < 10; i++) {
			if (!ship[i].IsDestroyed()) {
				return false;
			}
		}
		return true;
	}
};

