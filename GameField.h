#pragma once
#include "Fleet.h"
enum class Aim { Miss, Goal, Unshot };
//--------------------------------------------------------------
class GameField {
private:
	int originX;
	int originY;
	int cellWidth;
public:
	Aim cells[10][10];
	Fleet *fleet;
	//--------------------------------------------------------------
	GameField(int width = 20) :originX(0), originY(0), cellWidth(width) {
		for (int i = 0; i < 10; ++i) {
			for (int j = 0; j < 10; ++j) {
				cells[i][j] = Aim::Unshot;
			}
		}
		fleet = new Fleet();
	}
	//--------------------------------------------------------------
	void Notify() {}
	//--------------------------------------------------------------
	vector<Ship> GetDestroyedShips() {
		return fleet->GetDestroyedShips();
	}
	//--------------------------------------------------------------
	vector<ofPoint> GetInjuredDecks() {
		vector<ofPoint> result;
		for (int s = 0; s < 10; ++s) {
			int nDecks = fleet->ship[s].col_deck;
			for (int d = 0; d < nDecks; ++d) {
				if (!fleet->ship[s].decks[d]) {
					result.push_back(
						fleet->ship[s].desp_of_part[d]
					);
				}
			}
		}
		return result;
	}
	//--------------------------------------------------------------
	vector<ofPoint> GetDecks() {
		vector<ofPoint> result;
		for (int s = 0; s < 10; ++s) {
			int nDecks = fleet->ship[s].col_deck;
			for (int d = 0; d < nDecks; ++d) {
				if (fleet->ship[s].decks[d]) {
					result.push_back(
						fleet->ship[s].desp_of_part[d]
					);
				}
			}
		}
		return result;
	}
	//--------------------------------------------------------------
	bool FleetIsDestroyed() {
		return fleet->IsDestroyed();
	}
	//--------------------------------------------------------------
	const Ship& GetShip(int x, int y) {
		x -= originX;
		y -= originY;
		x /= 20; y /= 20; //!!! offset
		return fleet->GetShip(x, y);
	}
	//--------------------------------------------------------------
	ofRectangle BoundingBox(const Ship& sh) {
		unsigned int x1, y1,
			x2, y2;
		x1 = min(sh.desp_of_part[0].x, sh.desp_of_part[sh.col_deck - 1].x);
		y1 = min(sh.desp_of_part[0].y, sh.desp_of_part[sh.col_deck - 1].y);

		x2 = max(sh.desp_of_part[0].x, sh.desp_of_part[sh.col_deck - 1].x) + 1;
		y2 = max(sh.desp_of_part[0].y, sh.desp_of_part[sh.col_deck - 1].y) + 1;

		unsigned int w = x2 - x1,
			h = y2 - y1;
		return ofRectangle(x1 - 0.5, y1 - 0.5, w + 1, h + 1);
	}
	//--------------------------------------------------------------
	bool Intersect(const Ship& sh1, const Ship& sh2) {
		ofRectangle rect1 = BoundingBox(sh1),
			rect2 = BoundingBox(sh2);
		return rect1.intersects(rect2);
	}
	//--------------------------------------------------------------
	bool ShipInField(const Ship& sh) {
		unsigned int x2, y2;

		x2 = max(sh.desp_of_part[0].x, sh.desp_of_part[sh.col_deck - 1].x) + 1;
		y2 = max(sh.desp_of_part[0].y, sh.desp_of_part[sh.col_deck - 1].y) + 1;
		return x2 <= 10 && y2 <= 10;
	}
	//--------------------------------------------------------------
	bool CanPlaceShip(
		unsigned int col_ship,
		Ship ship
	) {
		//x /= 20; y /= 20;
		//Ship newShip = Ship::Create(col_deck, x, y, horizontal);

		for (int i = 0; i < col_ship; i++) {
			if (Intersect(ship, fleet->ship[i]))
				return false;
		}
		return ShipInField(ship);
	}
	//--------------------------------------------------------------
	void AddShipAt(int col_deck, int x, int y, bool horizontal) {
		x /= cellWidth; y /= cellWidth;
		Ship newShip = Ship::Create(col_deck, x, y, horizontal);
		fleet->ship[col_ship] = newShip;
	}
	//--------------------------------------------------------------
	void Generation(int col_ship, int col_deck) {
		unsigned int x, y;
		bool vertical = false;
		srand(time(NULL)); //!!!
		Ship newShip(col_deck);
		do {
			x = rand() % 10;
			y = rand() % 10;
			newShip = Ship::Create(col_deck, x, y, true);
			if (CanPlaceShip(col_ship, newShip)) {
				vertical = false;
				break;
			}
			newShip = Ship::Create(col_deck, x, y, false);
			if (CanPlaceShip(col_ship, newShip)) {
				vertical = true;
				break;
			}
		} while (true);

		fleet->ship[col_ship] = newShip;
		/*
		fleet->ship[col_ship].col_deck = col_deck;
		if (!vertical)
			for (int i = 0; i < col_deck; i++) {
				fleet->ship[col_ship].decks[i] = true;
				fleet->ship[col_ship].desp_of_part[i].x = x + i;
				fleet->ship[col_ship].desp_of_part[i].y = y;
			}
		else
			for (int i = 0; i < col_deck; i++) {
				fleet->ship[col_ship].decks[i] = true;
				fleet->ship[col_ship].desp_of_part[i].x = x;
				fleet->ship[col_ship].desp_of_part[i].y = y + i;
			}*/
	}
	//--------------------------------------------------------------
	void Initialize(){
		int col_ship = 0; 
		int col_deck = 4;

		for (int i = 0; i < 10; ++i) {
			Generation(col_ship, col_deck);
			col_ship++;
			if (col_ship == 1) col_deck--;
			if (col_ship == 3) col_deck--;
			if (col_ship == 6) col_deck--;
		}
	}
	//--------------------------------------------------------------
	int GetWidth() { return cellWidth; }
	//--------------------------------------------------------------
	bool IsShot(int x, int y) {
		x -= originX;
		y -= originY;
		x /= cellWidth; y /= cellWidth;
		return cells[x][y] != Aim::Unshot;
	}
	//--------------------------------------------------------------
	bool IsMissed(int x, int y) {
		x -= originX;
		y -= originY;
		x /= cellWidth; y /= cellWidth;
		return cells[x][y] == Aim::Miss;
	}
	//--------------------------------------------------------------
	int Scan(int x, int y) {
		x /= 20; y /= 20;
		for (int i = 0; i < 10; i++) //!!!
			for (int j = 0; j < fleet->ship[i].col_deck; j++)
				if (fleet->ship[i].desp_of_part[j].x == x &&
					fleet->ship[i].desp_of_part[j].y == y)
				{
					fleet->ship[i].decks[j] = false;
					return 0;
				}
		return 1;
	}
};

