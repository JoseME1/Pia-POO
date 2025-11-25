#include "Scenario.h"
#ifdef __linux__ 
#define ZeroMemory(x,y) memset(x,0,y)
#define wcscpy_s(x,y,z) wcscpy(x,z)
#define wcscat_s(x,y,z) wcscat(x,z)
#endif

Scenario::Scenario(Camera *cam) {
    glm::vec3 translate;
	glm::vec3 scale;
    Model* model = new Model("models/Cube/Cube.obj", cam);
	translate = glm::vec3(0.0f, 0.0f, 3.0f);
	scale = glm::vec3(0.25f, 0.25f, 0.25f);	// it's a bit too big for our scene, so scale it down
	model->setScale(&scale);
	model->setTranslate(&translate);
	model->setNextTranslate(&translate);
	InitGraph(model);
}
Scenario::Scenario(Model *camIni) {
    InitGraph(camIni);
}
void Scenario::InitGraph(Model *main) {
    float matAmbient[] = { 1,1,1,1 };
	float matDiff[] = { 1,1,1,1 };
	//Defino las escalas de cada eje por separado
	float escalaY;
	float escalaX;
	float escalaZ;
	//Defino las posiciones de cada eje por separado
	float posX;
	float posY;
	float posZ;
	angulo = 0;
	camara = main;
	//creamos el objeto skydome
	sky = new SkyDome(32, 32, 20, (WCHAR*)L"skydome/earth3.png", main->cameraDetails);
	//creamos el terreno
	terreno = new Terreno((WCHAR*)L"skydome/terreno2.jpg", (WCHAR*)L"skydome/texterr.jpg", 400, 400, main->cameraDetails);
	water = new Water((WCHAR*)L"textures/terreno.bmp", (WCHAR*)L"textures/water.bmp", 5, 5, camara->cameraDetails);
	glm::vec3 translate;
	glm::vec3 scale;
	glm::vec3 rotation;
	translate = glm::vec3(80.0f, 1.0f, 40.0f);
	water->setTranslate(&translate);
	scale = glm::vec3(1.0f, 1.0f, 1.0f);
	water->setScale(&scale);
	// load models
	// -----------
	ourModel.emplace_back(main);
	Model* model;

	/*ESTO CARGA AL PEZ Y LO CLONA
	Model *pez = new Model("models/pez/pez.obj", main->cameraDetails);
	translate = glm::vec3(0.0f, terreno->Superficie(0.0f, 50.0f), 50.0f);
	pez->setNextTranslate(&translate);
	pez->setTranslate(&translate);
	ourModel.emplace_back(pez);
	ModelAttributes m;
	m.setTranslate(&translate);
	m.setNextTranslate(&translate);
	m.translate.x = 5;
	model = CollitionBox::GenerateAABB(m.translate, pez->AABBsize, main->cameraDetails);
	model->setTranslate(&m.translate);
	model->setNextTranslate(&m.translate);
	m.hitbox = model;
	pez->getModelAttributes()->push_back(m);
	m.setTranslate(&translate);
	m.setNextTranslate(&translate);
	m.translate.x = 10;
	model = CollitionBox::GenerateAABB(m.translate, pez->AABBsize, main->cameraDetails);
	model->setTranslate(&m.translate);
	model->setNextTranslate(&m.translate);
	m.hitbox = model; // Le decimos al ultimo ModelAttribute que tiene un hitbox asignado
	pez->getModelAttributes()->push_back(m);*/

	//CARGAMOS LA MONEDA (Esto tiene corregido el tema de aparecer en la superficie del terreno)
	Model *moneda = new Model("models/Moneda/Moneda.fbx", main->cameraDetails);
	
	escalaY=escalaX=escalaZ = 4.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	float alturaMoneda = 1.0f * escalaY; // Altura real del modelo en Blender es 1.0, si fuera 2.0 se pondria 2.0, etc
	posX = 50.0f;
	posZ = -10.0f;
	//IMPORTANTE ESTABLECER EL VALOR DE POSX Y POSZ ANTES DE CALCULAR POSY
	//Si quiero evitar hacer lo de abajo, podria cambiar el origen del modelo en Blender
	posY = terreno->Superficie(posX, posZ)+(alturaMoneda/2.0f); //Dividimos entre 2 la altura porque el origen del modelo esta en el centro
	/*Aqui se hacen unos malabares con la escala xd*/
	translate = glm::vec3(posX, posY, posZ);
	moneda->setTranslate(&translate);
	moneda->setNextTranslate(&translate);
	moneda->setScale(&scale);
	moneda->setModelType("Moneda"); //ACTUA COMO LOS TAGS DE UNITY
	ourModel.emplace_back(moneda);
	//PRIMER CLON DE MONEDA
	ModelAttributes m;
	m.setScale(&scale);
	m.setTranslate(&translate);
	m.setNextTranslate(&translate);
	m.translate.x = 55;
	model = CollitionBox::GenerateAABB(m.translate, moneda->AABBsize, main->cameraDetails);
	model->setTranslate(&m.translate);
	model->setNextTranslate(&m.translate);
	m.hitbox = model;
	moneda->getModelAttributes()->push_back(m);
	//SEGUNDO CLON DE MONEDA
	m.translate.x = 60;
	model = CollitionBox::GenerateAABB(m.translate, moneda->AABBsize, main->cameraDetails);
	model->setTranslate(&m.translate);
	model->setNextTranslate(&m.translate);
	m.hitbox = model; // Le decimos al ultimo ModelAttribute que tiene un hitbox asignado
	moneda->getModelAttributes()->push_back(m);
	

	//CARGAMOS EL VOCHO
	Model* vocho = new Model("models/Vocho/vocho.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 0.5f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 30.0f;
	posZ = -10.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	vocho->setTranslate(&translate);
	vocho->setNextTranslate(&translate);
	vocho->setScale(&scale);
	ourModel.emplace_back(vocho);
	
	//CARGAMOS EL CARRO
	Model* carro = new Model("models/Carro/carro.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 3.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 20.0f;
	posZ = -10.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	carro->setTranslate(&translate);
	carro->setNextTranslate(&translate);
	carro->setScale(&scale);
	ourModel.emplace_back(carro);

	//CARGAMOS EL PERRO
	Model* perro = new Model("models/Perro/perroIdle.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 0.04f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 10.0f;
	posZ = -10.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	perro->setTranslate(&translate);
	perro->setNextTranslate(&translate);
	perro->setScale(&scale);
	ourModel.emplace_back(perro);
	try {
		std::vector<Animation> animations = Animation::loadAllAnimations("models/Perro/perroIdle.fbx", perro->GetBoneInfoMap(), perro->getBonesInfo(), perro->GetBoneCount());
		for (Animation animation : animations)
			perro->setAnimator(Animator(animation));
		perro->setAnimation(0);
	}
	catch (...) {
		ERRORL("Could not load animation!", "ANIMACION");
	}
	

	//CARGAMOS EL BOTE DE BASURA
	Model* basura = new Model("models/Basura/basura.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 2.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 0.0f;
	posZ = -10.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	basura->setTranslate(&translate);
	basura->setNextTranslate(&translate);
	basura->setScale(&scale);
	ourModel.emplace_back(basura);

	//CARGAMOS LA CASA1
	Model* casa1 = new Model("models/casa1/casa1.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 15.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = -20.0f;
	posZ = -10.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	casa1->setTranslate(&translate);
	casa1->setNextTranslate(&translate);
	casa1->setScale(&scale);
	casa1->setNextRotX(-90);
	ourModel.emplace_back(casa1);

	//CARGAMOS LA CASA2
	Model* casa2 = new Model("models/casa2/casa2.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 10.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = -50.0f;
	posZ = -10.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	casa2->setScale(&scale);
	casa2->setNextRotX(180);
	casa2->setNextRotY(90);
	casa2->setTranslate(&translate);
	casa2->setNextTranslate(&translate);
	ourModel.emplace_back(casa2);

	//CARGAMOS LA CASA3
	Model* casa3 = new Model("models/casa3/casa3.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 10.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = -50.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	casa3->setScale(&scale);
	casa3->setNextRotX(-90);
	casa3->setNextRotZ(90);
	casa3->setTranslate(&translate);
	casa3->setNextTranslate(&translate);
	ourModel.emplace_back(casa3);

	//CARGAMOS EL OXXO
	Model* oxxo = new Model("models/FachadaOxxo/fachadaOxxo.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 2.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = -10.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	oxxo->setScale(&scale);
	oxxo->setNextRotX(-90);
	oxxo->setNextRotZ(180);
	oxxo->setTranslate(&translate);
	oxxo->setNextTranslate(&translate);
	ourModel.emplace_back(oxxo);

	//CARGAMOS EL LETRERO DEL OXXO
	Model* letreroOxxo = new Model("models/LetreroOxxo/letreroOxxo.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 2.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 20.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	letreroOxxo->setScale(&scale);
	letreroOxxo->setNextRotX(-90);
	letreroOxxo->setNextRotZ(180);
	letreroOxxo->setTranslate(&translate);
	letreroOxxo->setNextTranslate(&translate);
	ourModel.emplace_back(letreroOxxo);

	//CARGAMOS LOS TACOS
	Model* tacos = new Model("models/Tacos/tacos.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 2.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 40.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	tacos->setScale(&scale);
	tacos->setNextRotX(-90);
	tacos->setTranslate(&translate);
	tacos->setNextTranslate(&translate);
	ourModel.emplace_back(tacos);

	//CARGAMOS EL ENEMIGOIDLE
	Model* enemigo = new Model("models/Enemigo/enemigoanimado.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 0.02f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 60.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	enemigo->setScale(&scale);
	enemigo->setNextRotY(180);
	enemigo->setTranslate(&translate);
	enemigo->setNextTranslate(&translate);
	enemigo->setModelType("Enemigo"); 
	ourModel.emplace_back(enemigo);
	try {
		std::vector<Animation> animations = Animation::loadAllAnimations("models/Enemigo/enemigoanimado.fbx", enemigo->GetBoneInfoMap(), enemigo->getBonesInfo(), enemigo->GetBoneCount());
		for (Animation animation : animations)
			enemigo->setAnimator(Animator(animation));
		enemigo->setAnimation(0);
	}
	catch (...) {
		ERRORL("Could not load animation!", "ANIMACION");
	}

	//CARGAMOS EL TELEFONO PUBLICO
	Model* telefono = new Model("models/Telefono/telefono.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 2.0f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 70.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	telefono->setScale(&scale);
	telefono->setNextRotY(180);
	telefono->setNextRotX(90);
	telefono->setTranslate(&translate);
	telefono->setNextTranslate(&translate);
	ourModel.emplace_back(telefono);

	//CARGAMOS LA ALBERCA
	Model* alberca = new Model("models/Alberca/alberca.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 1.5f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 80.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	alberca->setScale(&scale);
	alberca->setNextRotY(180);
	alberca->setNextRotX(90);
	alberca->setTranslate(&translate);
	alberca->setNextTranslate(&translate);
	ourModel.emplace_back(alberca);

	//CARGAMOS EL OXXO (ACCESIBLE)
	Model* oxxoAcc = new Model("models/OxxoInterior/OxxoInterior3.fbx", main->cameraDetails);
	escalaX = escalaY = escalaZ = 0.5f;
	scale = glm::vec3(escalaX, escalaY, escalaZ);
	posX = 150.0f;
	posZ = 40.0f;
	posY = terreno->Superficie(posX, posZ);
	translate = glm::vec3(posX, posY, posZ);
	oxxoAcc->setScale(&scale);
	oxxoAcc->setNextRotY(180);
	oxxoAcc->setNextRotX(90);
	oxxoAcc->setTranslate(&translate);
	oxxoAcc->setNextTranslate(&translate);
	ourModel.emplace_back(oxxoAcc);
	//ELIMINAR HITBOX
	if (oxxoAcc->getModelAttributes()->at(0).hitbox != NULL) {
		Model* AABB = (Model*)oxxoAcc->getModelAttributes()->at(0).hitbox;
		delete AABB;
		oxxoAcc->getModelAttributes()->at(0).hitbox = NULL;
	}
	
	Node nodoWall = oxxoAcc->AABBsize;
	//pared 1
	nodoWall.m_center.x = 18;
	nodoWall.m_center.y = 1;
	nodoWall.m_center.z = 1;
	nodoWall.m_halfWidth = 1;
	nodoWall.m_halfHeight = 10;
	nodoWall.m_halfDepth = 10;
	model = CollitionBox::GenerateAABB(translate, nodoWall, main->cameraDetails);
	m.hitbox = model;
	m.active = false;
	oxxoAcc->getModelAttributes()->push_back(m);
	
	//pared2
	nodoWall.m_center.x = -20;
	nodoWall.m_halfDepth = 15;
	model = CollitionBox::GenerateAABB(translate, nodoWall, main->cameraDetails);
	m.hitbox = model;
	m.active = false;
	oxxoAcc->getModelAttributes()->push_back(m);

	//pared 3
	nodoWall.m_center.x = 1;
	nodoWall.m_center.z = 17;
	nodoWall.m_halfDepth = 1;
	nodoWall.m_halfWidth = 20;
	nodoWall.m_halfHeight = 10;
	model = CollitionBox::GenerateAABB(translate, nodoWall, main->cameraDetails);
	m.hitbox = model;
	m.active = false;
	oxxoAcc->getModelAttributes()->push_back(m);
	

	/*ModelAttributes m;
	m.setScale(&scale);
	m.setTranslate(&translate);
	m.setNextTranslate(&translate);
	m.translate.x = 55;
	model = CollitionBox::GenerateAABB(m.translate, moneda->AABBsize, main->cameraDetails);
	model->setTranslate(&m.translate);
	model->setNextTranslate(&m.translate);
	m.hitbox = model;
	moneda->getModelAttributes()->push_back(m);*/
	
	

	// CARGA BILLBOARDS
	//inicializaBillboards();
	//std::wstring prueba(L"Recoge 3 monedas");
	//ourText.emplace_back(new Texto(prueba, 20, 0, 1000, 20, 0, camara));
	//billBoard2D.emplace_back(new Billboard2D((WCHAR*)L"billboards/awesomeface.png", 6, 6, 100, 200, 0, camara->cameraDetails));
	//scale = glm::vec3(100.0f, 100.0f, 0.0f);	// it's a bit too big for our scene, so scale it down
	//billBoard2D.back()->setScale(&scale);
	}

/*void Scenario::inicializaBillboards() {
	float ye = terreno->Superficie(0, 0);
	billBoard.emplace_back(new Billboard((WCHAR*)L"billboards/Arbol.png", 6, 6, 0, ye - 1, 0, camara->cameraDetails));

	ye = terreno->Superficie(-9, -15);
	billBoard.emplace_back(new Billboard((WCHAR*)L"billboards/Arbol3.png", 8, 8, -9, ye - 1, -15, camara->cameraDetails));

	BillboardAnimation *billBoardAnimated = new BillboardAnimation();
	ye = terreno->Superficie(5, -5);
	for (int frameArbol = 1; frameArbol < 4; frameArbol++){
		wchar_t textura[50] = {L"billboards/Arbol"};
		if (frameArbol != 1){
			wchar_t convert[25];
			swprintf(convert, 25, L"%d", frameArbol);
			wcscat_s(textura, 50, convert);
		}
		wcscat_s(textura, 50, L".png");
		billBoardAnimated->pushFrame(new Billboard((WCHAR*)textura, 6, 6, 5, ye - 1, -5, camara->cameraDetails));		
	}
	billBoardAnim.emplace_back(billBoardAnimated);
}*/

	//el metodo render toma el dispositivo sobre el cual va a dibujar
	//y hace su tarea ya conocida
Scene* Scenario::Render() {
	//borramos el biffer de color y el z para el control de profundidad a la 
	//hora del render a nivel pixel.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
//	glClearColor(255.0f, 255.0f, 255.0f, 255.0f);

	if (this->animacion > 10) { // Timer se ejecuta cada 1000/30 = 33.333 ms
		for (BillboardAnimation *b : billBoardAnim){
			b->nextAnimation();
		}
		this->animacion = 0;
	} else {
		animacion = animacion + (1 * gameTime.deltaTime/100);
	}
	// Decimos que dibuje la media esfera
	sky->Draw();
	// Ahora el terreno
	terreno->Draw();
	water->Draw();
	// Dibujamos cada billboard que este cargado en el arreglo de billboards.
	for (int i = 0; i < billBoard.size(); i++)
		billBoard[i]->Draw();
	for (int i = 0; i < billBoardAnim.size(); i++)
		billBoardAnim[i]->Draw();
	for (int i = 0; i < billBoard2D.size(); i++)
		billBoard2D[i]->Draw();
	// Dibujamos cada modelo que este cargado en nuestro arreglo de modelos
	for (int i = 0; i < ourModel.size(); i++) {
			ourModel[i]->Draw();
	}
	for (int i = 0; i < ourText.size(); i++) {
		ourText[i]->Draw();
	}
		// Le decimos a winapi que haga el update en la ventana
	return this;
}
	
std::vector<Model*> *Scenario::getLoadedModels() {
	return &ourModel;
}
std::vector<Billboard*> *Scenario::getLoadedBillboards() {
	return &billBoard;
}
std::vector<Billboard2D*> *Scenario::getLoadedBillboards2D(){
	return &billBoard2D;
}
std::vector<Texto*> *Scenario::getLoadedText(){
	return &ourText;
}
std::vector<BillboardAnimation*> *Scenario::getLoadedBillboardsAnimation(){
	return &billBoardAnim;
}

Model* Scenario::getMainModel() {
	return this->camara;
}
void Scenario::setMainModel(Model* mainModel){
	this->camara = mainModel;
}
float Scenario::getAngulo() {
	return this->angulo;
}
void Scenario::setAngulo(float angulo) {
	this->angulo = angulo;
}
SkyDome* Scenario::getSky() {
	return sky;
}
Terreno* Scenario::getTerreno() {
	return terreno;
}

Scenario::~Scenario() {
	if (this->sky != NULL) {
		delete this->sky;
		this->sky = NULL;
	}
	if (this->terreno != NULL) {
		delete this->terreno;
		this->terreno = NULL;
	}
	if (billBoard.size() > 0)
		for (int i = 0; i < billBoard.size(); i++)
			delete billBoard[i];
	if (billBoardAnim.size() > 0)
		for (int i = 0; i < billBoardAnim.size(); i++)
			delete billBoardAnim[i];
	if (billBoard2D.size() > 0)
		for (int i = 0; i < billBoard2D.size(); i++)
			delete billBoard2D[i];
	this->billBoard.clear();
	if (ourText.size() > 0)
		for (int i = 0; i < ourText.size(); i++)
			if (!(ourText[i]->name.compare("FPSCounter") || ourText[i]->name.compare("Coordenadas")))
				delete ourText[i];
	this->ourText.clear();
	if (ourModel.size() > 0)
		for (int i = 0; i < ourModel.size(); i++)
			if (ourModel[i] != camara)
			delete ourModel[i];
	this->ourModel.clear();
}
