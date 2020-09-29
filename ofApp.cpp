#include <stdlib.h>
#include "ofApp.h"

int col_deck = 4;
int col_ship = 1;
bool play = false;
bool game_over = false;
bool cpu_turn = false;
bool showMenu = true;
bool helpOn = false;
string helpText;
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


enum Aim {Miss, Goal, Unshot};
Aim net1[10][10], net2[10][10];

ofSoundPlayer musicPlayer;
float musicVolumeLevel = 1.0;

ofSoundPlayer effectPlayer;
//--------------------------------------------------------------
struct Ship
{
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
	~Ship() {
		delete [] decks;
		delete [] desp_of_part;
		decks = NULL;
		desp_of_part = NULL; 
	}

	bool IsDestroyed() {
		for (int i = 0; i < col_deck; i++)
			if (decks[i]){
				return false;
			}
		return true;
	}
};
//--------------------------------------------------------------
class Flot
{
public:
	bool IsDestroyed;
	ofImage* Im;
	Ship ship[10];
	Flot(ofImage* I)
	{
		IsDestroyed = false;
		Im = I;
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

	Ship& GetShip(int x, int y) {
		x /= 20; y /= 20; //!!! offset
		for (int i = 0; i < 10; i++)
			for (int j = 0; j < ship[i].col_deck; j++) {
				if (ship[i].desp_of_part[j].x == x &&
					ship[i].desp_of_part[j].y == y) {
					return ship[i];
				}
			}
		return Ship();
	}

	int GetStatus()
	{
		IsDestroyed = true;
		for (int i = 0; i < 10; i++) {
			if (!ship[i].IsDestroyed()) {
				IsDestroyed = false;
				return 1;
			}
		}
		return 0;
	}
};
//--------------------------------------------------------------
Flot CPU(&img);
Flot* player;
//--------------------------------------------------------------
int EndRaund(int n){
	if (n == 2)
		ofSystemAlertDialog("Вы победили!");
	if (n == 1)
		ofSystemAlertDialog("Вы проиграли!");
	game_over = true;
	return 0;
}
//--------------------------------------------------------------
bool IsShot(Aim net[10][10], int x, int y) {
	x /= 20; y /= 20;
	return net[x][y] != Aim::Unshot;
}
//--------------------------------------------------------------
bool IsShot(int x, int y, int n) {
	if (n == 1)
		return IsShot(net1, x, y);
	if (n == 2)
		return IsShot(net2, x - offset, y);
}
//--------------------------------------------------------------
int Scan(Flot* fl, int x, int y) {
	x /= 20; y /= 20;
	for (int i = 0; i < 10; i++) //!!!
		for (int j = 0; j < fl->ship[i].col_deck; j++)
			if (fl->ship[i].desp_of_part[j].x == x &&
				fl->ship[i].desp_of_part[j].y == y)
			{
				fl->ship[i].decks[j] = false;
				return 0;
			}
	return 1;
}
//--------------------------------------------------------------
int GoScaning(int x, int y, int n){
	if (n == 1)
		if (!Scan(player, x, y))
			return 0;
	if (n == 2)
		if (!Scan(&CPU, x - offset, y))
			return 0;
	return 1;
}
//--------------------------------------------------------------
int Missed(int x, int y, int n){
	if (n == 1) {
		x /= 20; y /= 20;
		net1[x][y] = Aim::Miss;
	}
	if (n == 2) {
		x = (x - offset) / 20; y /= 20;
		net2[x][y] = Aim::Miss;
	}
	return 1;
}
//--------------------------------------------------------------
int Hit(int x, int y, int n)
{
	if (n == 1) {
		x /= 20; y /= 20;
		net1[x][y] = Aim::Goal;
	}
	if (n == 2) {
		x = (x - offset) / 20; y /= 20;
		net2[x][y] = Aim::Goal;
	}
	return 1;
}
//--------------------------------------------------------------
int ShootPlayer(int x, int y, int n){
	if (!play) return 0;
	if (IsShot(x, y, n))
	{
		if (n == 2) ofSystemAlertDialog("Еще раз! Туда уже стреляли!");
		return 0;
	}
	else if (!GoScaning(x, y, n))
	{
		Hit(x, y, n);
		if (n == 2)
		{
			ofSystemAlertDialog("Попал! Еще раз!");
			target_player++;
		}
		else target_CPU++;
		return -1;
	}
	Missed(x, y, n);
	effectPlayer.play();
	if (n == 2) ofSystemAlertDialog("Мимо! Ход оппонента.");
	effectPlayer.stop();
	return 1;
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
ofRectangle BoundingBox(const Ship &sh) {
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
bool Intersect(const Ship &sh1, const Ship &sh2) {
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
bool CanPlaceShip(Flot* fleet, 
	unsigned int col_ship, 
	unsigned int col_deck, 
	unsigned int x, 
	unsigned int y, 
	int button
){
	Ship newShip(col_deck);

	if (button == 0)
		for (int i = 0; i < col_deck; i++) {
			newShip.desp_of_part[i].x = x + i;
			newShip.desp_of_part[i].y = y;
		}
	else
		for (int i = 0; i < col_deck; i++) {
			newShip.desp_of_part[i].x = x;
			newShip.desp_of_part[i].y = y + i;
		}

	for (int i = 0; i < col_ship; i++)
	{
		if (Intersect(newShip, fleet->ship[i]))
			return false;
	}
	return ShipInField(newShip);
}
//--------------------------------------------------------------
int Generation (Flot* CPU, int col_ship, int col_deck){
	unsigned int x, y;
	bool vertical = false;
	srand(time(NULL)); //!!!
	do {
		x = rand() % 10;
		y = rand() % 10;
		if (CanPlaceShip(CPU, col_ship, col_deck, x, y, 0)) {
			vertical = false;
			break;
		}
		if (CanPlaceShip(CPU, col_ship, col_deck, x, y, 2)) {
			vertical = true;
			break;
		}
	} while (true);


	CPU->ship[col_ship].col_deck = col_deck;
	if (!vertical)
		for (int i = 0; i < col_deck; i++){
			CPU->ship[col_ship].decks[i] = true;
			CPU->ship[col_ship].desp_of_part[i].x = x + i;
			CPU->ship[col_ship].desp_of_part[i].y = y;
		}
	else
		for (int i = 0; i < col_deck; i++){
			CPU->ship[col_ship].decks[i] = true;
			CPU->ship[col_ship].desp_of_part[i].x = x;
			CPU->ship[col_ship].desp_of_part[i].y = y + i;
		}
	return 0;
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
	wifstream InputFile("data/help.txt");
	assert(InputFile.is_open());
	wstring line;
	InputFile >> line;
	helpText = ToUnicode(line);
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

	col_ship = 0; //!!!
	col_deck = 4;

	for(int i = 0; i < 10; ++i)
	{
		Generation(&CPU, col_ship, col_deck);
		col_ship++;//!!!
		if (col_ship == 1) col_deck--;
		if (col_ship == 3) col_deck--;
		if (col_ship == 6) col_deck--;
	}
	
	col_ship = 0; //!!!
	col_deck = 4;
	player = new Flot(&img);

	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
		{
			net1[i][j] = Aim::Unshot;
			net2[i][j] = Aim::Unshot;
		}
	target_CPU = 0;
	target_player = 0;

	offset = ofGetWindowWidth() / 2; //???
}
//--------------------------------------------------------------
void ofApp::update() {
	ofSoundUpdate();
	if (game_over) return;
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
				shoot_result = ShootPlayer(x, y, 1);
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
				player->GetStatus();
				if (player->IsDestroyed || target_CPU > 19)
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
				EndRaund(1);
				return;
			case 'E':
				cpu_turn = true;
				if (player->GetShip(first_x, first_y).IsDestroyed()) {
					state = 'A';
					break;
				}

				shoot_result = ShootPlayer(first_x + 20 * dx, first_y + 20 * dy, 1);

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
				if (player->GetShip(nx, ny).IsDestroyed()) {
					state = 'A';
					break;
				}
				nx = nx + 20 * dx;
				ny = ny + 20 * dy;
				shoot_result = ShootPlayer(nx, ny, 1);
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
void ofApp::draw(){
	if (showMenu) {

		for (int i = 0; i < menuSize; ++i) {
			menu[i].draw();
		}
		return;
	}


	ofDrawBitmapString("Volume: " + ofToString(musicVolumeLevel), 250, 250);
	img.draw(offset, 0);

	for (int s = 0; s < 10; ++s) {
		int nDecks = CPU.ship[s].col_deck;
		if (CPU.ship[s].IsDestroyed()) {

			CPU_DrawArea(CPU.ship[s]);

			for (int d = 0; d < nDecks; ++d) {
				ofSetColor(ofColor::black);
				ofDrawRectangle(
					offset + CPU.ship[s].desp_of_part[d].x * 20,
					CPU.ship[s].desp_of_part[d].y * 20, 20, 20);
				ofSetColor(ofColor::white);
			}
			continue;//!!!
		}
		for (int d = 0; d < nDecks; ++d) {
			if (!CPU.ship[s].decks[d]) {
				ofSetColor(ofColor::yellow);
				ofDrawRectangle(
					offset + CPU.ship[s].desp_of_part[d].x * 20,
					CPU.ship[s].desp_of_part[d].y * 20, 20, 20);
				ofSetColor(ofColor::white);
			}
			else {
				//ofSetColor(ofColor::blue);
			}
		}
	}
	
	img.draw(0, 0);

	for (int s = 0; s < col_ship; ++s) {
		int nDecks = player->ship[s].col_deck;
		if (player->ship[s].IsDestroyed()) {

			player_DrawArea(player->ship[s]);

			for (int d = 0; d < nDecks; ++d) {
				ofSetColor(ofColor::black);
				ofDrawRectangle(
					player->ship[s].desp_of_part[d].x * 20,
					player->ship[s].desp_of_part[d].y * 20, 20, 20);
				ofSetColor(ofColor::white);
			}
			continue;//!!!
		}

		for (int d = 0; d < nDecks; ++d) {
			if (player->ship[s].decks[d]) {
				ofSetColor(ofColor::darkRed);
			}
			else {
				ofSetColor(ofColor::yellow);
			}
			//Sheep deck
			ofFill();
			ofDrawRectangle(
				player->ship[s].desp_of_part[d].x * 20,
				player->ship[s].desp_of_part[d].y * 20, 20, 20);
			ofNoFill();
			//Contour
			ofSetColor(ofColor::red);
			ofDrawRectangle(
				player->ship[s].desp_of_part[d].x * 20,
				player->ship[s].desp_of_part[d].y * 20, 20, 20);
			ofSetColor(ofColor::white);
			ofFill();
		}
	}

	for(int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j){
			if (net2[i][j] == Aim::Miss){
				int x = i * 20 + offset; 
				int y = j * 20; 
				ofSetColor(ofColor::red);
				ofDrawEllipse(x+10, y+10, 20, 20);
				ofSetColor(ofColor::white);
				ofDrawEllipse(x + 10, y + 10, 10, 10);
			}
		}

	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j) {
			if (net1[i][j] == Aim::Miss) {
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
	
	int shoot_result = ShootPlayer(x, y, 2);
	if (shoot_result > 0) {
		cpu_turn = true;
		return;
	}
	if (shoot_result < 0)
		CPU.GetStatus();
	if (CPU.IsDestroyed || target_player > 19){
		EndRaund(2);
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

	x /= 20; y /= 20;
	
	if (!CanPlaceShip(player, col_ship, col_deck, x, y, button)){
		ofSystemAlertDialog("Невозможно выполнить действие!");
		return;
	}
	player->ship[col_ship].col_deck = col_deck;
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
		}   
	
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
void ofApp::mouseReleased(int x, int y, int button){

}
//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}