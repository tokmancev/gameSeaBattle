#pragma once
#include "ofPoint.h"

struct Ship {
	unsigned int col_deck;
	bool* decks;
	ofPoint* desp_of_part; // ??? ofPoint2d

	Ship() {
		col_deck = 0;
		decks = NULL;
		desp_of_part = NULL;
	}
	Ship(unsigned int n) {
		col_deck = n;
		decks = new bool[col_deck];
		for (int i = 0; i < col_deck; ++i) {
			decks[i] = true;
		}
		desp_of_part = new ofPoint[col_deck];
	}
	//--------------------------------------------------------------
	static Ship Create(int col_deck, int x, int y, bool horizontal) {
		Ship newShip(col_deck);

		if (horizontal)
			for (int i = 0; i < col_deck; i++) {
				newShip.desp_of_part[i].x = x + i;
				newShip.desp_of_part[i].y = y;
			}
		else
			for (int i = 0; i < col_deck; i++) {
				newShip.desp_of_part[i].x = x;
				newShip.desp_of_part[i].y = y + i;
			}
		return newShip;
	}
	//--------------------------------------------------------------
	~Ship() {
		delete[] decks;
		delete[] desp_of_part;
		decks = NULL;
		desp_of_part = NULL;
	}

	bool IsDestroyed() const{
		for (int i = 0; i < col_deck; i++)
			if (decks[i]) {
				return false;
			}
		return true;
	}
};
