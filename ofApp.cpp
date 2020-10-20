#include <stdlib.h>
#include "ofApp.h"
#include "GameField.h"

int col_deck = 4;
int col_ship = 1;
bool play = false;
bool game_over = false;
bool cpu_turn = false;
bool showMenu = true;
bool helpOn = false;
string helpText;
GameField playerField, computerField;

//--------------------------------------------------------------
string ToUnicode(const wstring& str) {
	string result;
	result.resize(str.size());
	for (int i = 0; i < str.length(); ++i) {
		ofUTF8Append(result, str[i]);
	}
	return result;
}

typedef void(*OnClickType)();
//**************Menu
struct MenuItem {
	static ofTrueTypeFont verdana14;
	static int menuItemWidth;
	static int menuItemHeight;
	static bool firstTime;

	OnClickType OnClick;
	ofRectangle button;
	string Name;
	MenuItem() :OnClick(NULL){}
	MenuItem(wstring name, int menu_x, int menu_y, OnClickType on_click);
	void InitializeFont();
	void draw();
	bool includes(int x, int y);
};
int MenuItem::menuItemWidth = 100;
int MenuItem::menuItemHeight = 50;
bool MenuItem::firstTime = true;
ofTrueTypeFont MenuItem::verdana14;

bool MenuItem::includes(int x, int y) {
	return button.inside(x, y);
}

void MenuItem::InitializeFont() {
	if (!firstTime) return;
	firstTime = false;
	const string filename = "verdana.ttf";
	const int fontSize = 14;
	const bool antialiased = true;
	const bool fullCharacterSet = true;
	ofTrueTypeFontSettings settings(filename, fontSize);
	settings.antialiased = antialiased;
	settings.ranges = {
		ofUnicode::Latin1Supplement,
		ofUnicode::Cyrillic
	};
	verdana14.load(settings);
	assert(verdana14.isLoaded());
	verdana14.setLineHeight(18.0f);
	verdana14.setLetterSpacing(1.037);
}

MenuItem::MenuItem(wstring name, int menu_x, int menu_y, OnClickType on_click)
	: Name(ToUnicode(name)){
	InitializeFont();
	button = ofRectangle(menu_x, menu_y, menuItemWidth, menuItemHeight);
	OnClick = on_click;
}

void MenuItem::draw() {
	ofSetColor(ofColor::royalBlue);
	ofDrawRectangle(button);
	ofSetColor(ofColor::black);
	verdana14.drawString(Name, button.getLeft(),
		button.getCenter().y);
	ofSetColor(ofColor::white);

	if (helpOn) { //!!!
		verdana14.drawString(helpText, 10, 10);
	}
}

const size_t menuSize = 3;
MenuItem menu[menuSize];
//****************

int target_CPU, target_player;

int offset = 0;

ofImage img;


ofSoundPlayer musicPlayer;
float musicVolumeLevel = 1.0;

ofSoundPlayer effectPlayer;

//--------------------------------------------------------------
void FinishGame(int n){
	game_over = true;
	if (n == 2) {
		ofSystemAlertDialog("Вы победили!");
	}
	if (n == 1) {
		game_over = true;
		ofSystemAlertDialog("Вы проиграли!");
	}
}
//--------------------------------------------------------------
bool IsShot(int x, int y, int n) {
	if (n == 1)
		return playerField.IsShot(x, y);
	if (n == 2)
		return computerField.IsShot(x, y);
}

//--------------------------------------------------------------
int PlayerTurn(int x, int y) {
	if (computerField.IsShot(x, y)) {
		ofSystemAlertDialog("Еще раз! Туда уже стреляли!");
		return 0;
	}
	if (!computerField.Scan(x, y)){
		ofSystemAlertDialog("Попал! Еще раз!");
		target_player++;
		return -1;
	}
	effectPlayer.play();
	ofSystemAlertDialog("Мимо! Ход оппонента.");
	effectPlayer.stop();
	return 1;
}
//--------------------------------------------------------------
int ComputerTurn(int x, int y) {
	if (playerField.IsShot(x, y)){
		return 0;
	}
	if (!playerField.Scan(x, y)){
		target_CPU++;
		return -1;
	}
	return 1;
}
//--------------------------------------------------------------
int MakeTurn(int x, int y, int n){
	if (!play) return 0;

	if (n == 1){
		return ComputerTurn(x, y);
	}
	return PlayerTurn(x, y);
}
//--------------------------------------------------------------
bool FindDirection(int x, int y, int& dx, int& dy) {
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (abs(i) == abs(j))
				continue;
			if (!IsShot(x + i * 20, y + j * 20, 1)) {
				dx = i;
				dy = j;
				return true;
			}
		}
	}
	dx = dy = 0;
	return false;
}
//--------------------------------------------------------------

void DrawX(int x, int y) {
	ofPushStyle();
	ofSetLineWidth(3);
	ofSetColor(ofColor::black);
	ofDrawLine(x, y, x + 20, y + 20);
	ofDrawLine(x, y + 20, x + 20, y);
	ofPopStyle();
}
//--------------------------------------------------------------
void player_DrawArea(const Ship& sh) {//!!!	
	unsigned int x1, y1, x2, y2;

	x1 = min(sh.desp_of_part[0].x, sh.desp_of_part[sh.col_deck - 1].x);
	y1 = min(sh.desp_of_part[0].y, sh.desp_of_part[sh.col_deck - 1].y);

	x2 = max(sh.desp_of_part[0].x, sh.desp_of_part[sh.col_deck - 1].x) + 1;
	y2 = max(sh.desp_of_part[0].y, sh.desp_of_part[sh.col_deck - 1].y) + 1;

	unsigned int w = x2 - x1,
		h = y2 - y1;

	x1 = max<unsigned int>(x1 - 1, 0);
	y1 = max<unsigned int>(y1 - 1, 0);
	x2 = min<unsigned int>(x2, 9);
	y2 = min<unsigned int>(y2, 9);

	for (int i = x1; i <= x2; ++i)
		for (int j = y1; j <= y2; ++j) {
			DrawX(20 * i, 20 * j);
		}
}
//--------------------------------------------------------------
void CPU_DrawArea(const Ship& sh) { //!!!
	unsigned int x1, y1, x2, y2;

	x1 = min(sh.desp_of_part[0].x, sh.desp_of_part[sh.col_deck - 1].x);
	y1 = min(sh.desp_of_part[0].y, sh.desp_of_part[sh.col_deck - 1].y);

	x2 = max(sh.desp_of_part[0].x, sh.desp_of_part[sh.col_deck - 1].x) + 1;
	y2 = max(sh.desp_of_part[0].y, sh.desp_of_part[sh.col_deck - 1].y) + 1;

	unsigned int w = x2 - x1,
		h = y2 - y1;
	//x1 - 1, y1 - 1, x2 + 1, y2 + 1

	x1 = max<unsigned int>(x1 - 1, 0);
	y1 = max<unsigned int>(y1 - 1, 0);
	x2 = min<unsigned int>(x2, 9);
	y2 = min<unsigned int>(y2, 9);

	for (int i = x1; i <= x2; ++i)
		for (int j = y1; j <= y2; ++j) {
			DrawX(20 * i + offset, 20 * j);
		}
}
//-------------------------------------------------------------------------------
void OnClick_play() {
	showMenu = false;
}
//-------------------------------------------------------------------------------
void OnClick_exit() {
	ofExit();
}
//--------------------------------------------------------------
void OnClick_help() {
	if (helpOn) {
		helpOn = false;
		return;
	}
	helpOn = true;
	helpText.clear();
	ifstream InputFile("data/help.txt");
	assert(InputFile.is_open());
	while (InputFile) {
		string utf8;
		std::getline(InputFile, utf8);

		const int textWidth = 35;

		size_t i = 0;
		for (auto c : ofUTF8Iterator(utf8)) {
			if (i % textWidth == 0) {
				ofUTF8Append(helpText, '\n');
			}
			ofUTF8Append(helpText, c);
			i++;
		}
		ofUTF8Append(helpText, '\n');
	}
}
//--------------------------------------------------------------
void ofApp::setup() {
	//setlocale(LC_ALL, "RU");

	//*********MENU********** 
	wstring names[] = { L"Играть", L"Помощь", L"Выход" };
	OnClickType on_click[] = {&OnClick_play, &OnClick_help, &OnClick_exit};

	for (int i = 0; i < menuSize; ++i) {
		int menu_x = ofGetWindowWidth() / 2 - MenuItem::menuItemWidth / 2;
		int menu_y = ofGetWindowHeight() / 2 - i * MenuItem::menuItemHeight;
		menu[i] = MenuItem(names[i], menu_x, menu_y, on_click[i]);
	}

	//musicPlayer.load("main_menu_music.mp3");
	//assert(musicPlayer.isLoaded());
	//musicPlayer.setLoop(true);
	//musicPlayer.setMultiPlay(true);
	//musicPlayer.play();

	effectPlayer.load("miss.mp3");
	effectPlayer.setMultiPlay(true);
	assert(effectPlayer.isLoaded());

	bool isLoaded = img.load("net.png");
	assert(isLoaded);

	computerField.Initialize();

	
	
	col_ship = 0; //!!!
	col_deck = 4;
	
	target_CPU = 0;
	target_player = 0;

	offset = ofGetWindowWidth() / 2; //???
}
//--------------------------------------------------------------
void ofApp::update() {
	ofSoundUpdate();
	if (game_over) return;
	playerField.Notify();
	computerField.Notify();
	if (cpu_turn) {
		static char state = 'A';
		static int first_x = -1; //место первого попадания во вражеский корабль
		static int first_y = -1;
		static int nx = 0; //место следующего выстрела
		static int ny = 0;
		static int dx = 0; //направление дальнейшей стрельбы
		static int dy = 0;
		bool repeat = true;

		int shoot_result = 0;
		int x, y;
		switch (state){
			case 'A':
				x = rand() % 200;
				y = rand() % 200;
				//x = 50;
				//y = 10;
				shoot_result = MakeTurn(x, y, 1);
				if (shoot_result > 0)
					state = 'B';
				if (shoot_result < 0) {
					first_x = x;
					first_y = y;
					state = 'C';
				}
				break;
			case 'B':
				cpu_turn = false;
				repeat = false;
				state = 'A';
				break;
			case 'C':
				cpu_turn = true;
				if (playerField.FleetIsDestroyed() || target_CPU > 19)
					state = 'D';
				else {
					if (!FindDirection(first_x, first_y, dx, dy)) {
						dx = dy = 0;
					}
					state = 'E';
				}
				ofSleepMillis(2000);
				break;
			case 'D':
				FinishGame(1);
				return;
			case 'E':
				cpu_turn = true;
				if (playerField.GetShip(first_x, first_y).IsDestroyed()) {
					state = 'A';
					break;
				}

				shoot_result = MakeTurn(first_x + 20 * dx, first_y + 20 * dy, 1);

				if (shoot_result > 0) {
					dx = 0;
					dy = 0;
					repeat = false;
					cpu_turn = false;
					state = 'C';
				}
				if (shoot_result < 0) {
					cpu_turn = true;
					nx = first_x + 20 * dx;
					ny = first_y + 20 * dy;
					state = 'F';
				}
				ofSleepMillis(2000);
				break;
			case 'F':
				if (playerField.GetShip(nx, ny).IsDestroyed()) {
					state = 'A';
					break;
				}
				nx = nx + 20 * dx;
				ny = ny + 20 * dy;
				shoot_result = MakeTurn(nx, ny, 1);
				if (0 == shoot_result) {
					cpu_turn = true;
					dx = -dx;
					dy = -dy;
					state = 'E';
				}
				if (shoot_result > 0) {
					cpu_turn = false;
					dx = -dx;
					dy = -dy;
					repeat = false;
					state = 'E';
				}
				if (shoot_result < 0) {
					cpu_turn = true;
					state = 'F';
				}
				ofSleepMillis(2000);
				break;
		}
	}
}
//--------------------------------------------------------------
void DrawDestroyedShips(vector<Ship> ship, int offset) {
	for(int s = 0; s < ship.size(); ++s){
		CPU_DrawArea(ship[s]); //!!!
		int nDecks = ship[s].col_deck;

		for (int d = 0; d < nDecks; ++d) {
			ofSetColor(ofColor::black);
			ofDrawRectangle(
				offset + ship[s].desp_of_part[d].x * 20,
				ship[s].desp_of_part[d].y * 20, 20, 20);
			ofSetColor(ofColor::white);
		}
	}
}
//--------------------------------------------------------------
void DrawInjuredDecks(vector<ofPoint> deck, int offset) {
	for (int d = 0; d < deck.size(); ++d) {
		ofSetColor(ofColor::yellow);
		ofDrawRectangle(
			offset + deck[d].x * 20,
			deck[d].y * 20, 20, 20);
		ofSetColor(ofColor::white);
	}
}
//--------------------------------------------------------------
void DrawComputerField() {
	img.draw(offset, 0);

	vector<Ship> destroyedShips = computerField.GetDestroyedShips();
	DrawDestroyedShips(destroyedShips, offset);

	vector<ofPoint> injuredDecks = computerField.GetInjuredDecks();
	DrawInjuredDecks(injuredDecks, offset);

	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j) {
			if (computerField.cells[i][j] == Aim::Miss) {
				int x = i * 20 + offset;
				int y = j * 20;
				ofSetColor(ofColor::red);
				ofDrawEllipse(x + 10, y + 10, 20, 20);
				ofSetColor(ofColor::white);
				ofDrawEllipse(x + 10, y + 10, 10, 10);
			}
		}
}
//--------------------------------------------------------------
void DrawDecks(vector<ofPoint> deck) {
	ofSetColor(ofColor::darkRed);
	for (int d = 0; d < deck.size(); ++d) {

		ofFill();
		ofDrawRectangle(
			deck[d].x * 20,
			deck[d].y * 20, 20, 20);
		ofNoFill();
		//Contour
		ofSetColor(ofColor::red);
		ofDrawRectangle(
			deck[d].x * 20,
			deck[d].y * 20, 20, 20);
		ofSetColor(ofColor::white);
		ofFill();
	}
}
//--------------------------------------------------------------
void DrawPlayerField() {
	img.draw(0, 0);

	vector<Ship> destroyedShips = playerField.GetDestroyedShips();
	DrawDestroyedShips(destroyedShips, 0);
	vector<ofPoint> injuredDecks = playerField.GetInjuredDecks();
	DrawInjuredDecks(injuredDecks, 0);

	vector<ofPoint> decks = playerField.GetDecks();
	DrawDecks(decks);
	
	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j) {
			if (playerField.cells[i][j] == Aim::Miss) {
				int x = i * 20;
				int y = j * 20;
				ofSetColor(ofColor::blue);
				ofDrawEllipse(x + 10, y + 10, 20, 20);
				ofSetColor(ofColor::white);
				ofDrawEllipse(x + 10, y + 10, 10, 10);
			}
		}
}
//--------------------------------------------------------------
void ofApp::draw(){
	if (showMenu) {

		for (int i = 0; i < menuSize; ++i) {
			menu[i].draw();
		}
		return;
	}


	ofDrawBitmapString("Volume: " + ofToString(musicVolumeLevel), 250, 250);
	
	DrawComputerField();
	DrawPlayerField();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
	if (key == '-') {
		musicVolumeLevel = max<float>(0, musicVolumeLevel - 0.1);
		musicPlayer.setVolume(musicVolumeLevel);
	}

	if (key == '+') {
		musicVolumeLevel = min<float>(1.0, musicVolumeLevel + 0.1);
		musicPlayer.setVolume(musicVolumeLevel);
	}
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}
//--------------------------------------------------------------
void Shoot(int x, int y){
	if (game_over) return;
	
	int shoot_result = MakeTurn(x, y, 2);
	if (shoot_result > 0) {
		cpu_turn = true;
		return;
	}
	if (shoot_result < 0) {
		if (computerField.FleetIsDestroyed() || target_player > 19) {
			FinishGame(2);
		}
	}
}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (showMenu) {
		for (int i = 0; i < menuSize; ++i) {
			if (menu[i].includes(x, y)) {
				menu[i].OnClick();
				return;
			}
		}
	}

	if (cpu_turn) return;
	
	if (play) {
		Shoot(x, y);
		return;
	}
	//x /= 20; y /= 20;
	Ship newShip = Ship::Create(col_deck, x, y, button==0);

	if (!playerField.CanPlaceShip(col_ship, newShip)){
		ofSystemAlertDialog("Невозможно выполнить действие!");
		return;
	}
	player->ship[col_ship] = newShip;
	/*player->ship[col_ship].col_deck = col_deck;
	if (button == 0)
		for (int i = 0; i < col_deck; i++){
			player->ship[col_ship].decks[i] = true;
			player->ship[col_ship].desp_of_part[i].x = x + i;
			player->ship[col_ship].desp_of_part[i].y = y;
		}
	else
		for (int i = 0; i < col_deck; i++){
			player->ship[col_ship].decks[i] = true;
			player->ship[col_ship].desp_of_part[i].x = x;
			player->ship[col_ship].desp_of_part[i].y = y + i;
		}   */
	
	col_ship++;//!!!
	if (col_ship == 1) col_deck--;
	if (col_ship == 3) col_deck--;
	if (col_ship == 6) col_deck--;
	if (col_ship == 10) {
		play = true;
		ofSystemAlertDialog("Поехали!");
		return;
	}
}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){}
//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){}