// DemoTemplateOGL.cpp : Defines the entry point for the application.
//
#include "WinAPIHeaders/framework.h"
#include "WinAPIHeaders/DemoTemplateOGL.h"
#include "Base/Utilities.h"
#include "InputDevices/KeyboardInput.h"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Base/glext.h"
#include "Base/wglext.h"
#include "Base/model.h"
#include "Base/Scene.h"
#include "Scenario.h"
//imgui
#include "imgui.h"
#include "imgui_impl_win32.h"

//combat system
#include "CombatSystem.h"

// *** IMPORTANTE: Decirle a ImGui que ya tenemos GLAD cargado ***
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui_impl_opengl3.h"

#define MAX_LOADSTRING 100
#ifdef _WIN32 
#include "InputDevices/GamePadRR.h"
HINSTANCE hInst;                                // current instance
HWND hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HDC dc;
HGLRC rc;
GamePadRR* gamPad;                  // Manejador de gamepad
// Funciones para activar OpenGL version > 2.0
int prepareRenderWindow(HINSTANCE hInstance, int nCmdShow);
bool SetUpPixelFormat(HDC hDC, PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB, PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Callback principal de la ventana en WINAPI
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
#else
#include <GLFW/glfw3.h>
#define ZeroMemory(x,y) memset(x,0,y)
#define wcscpy_s(x,y,z) wcscpy(x,z)
#define wcscat_s(x,y,z) wcscat(x,z)
GLFWwindow* window;
void window_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
#endif

#define Timer1 100
bool renderiza;                     // Variable para controlar el render
bool checkInput(GameActions* actions, Scene* scene);
void mouseActions();
int isProgramRunning(void *ptr);
void swapGLBuffers();
int finishProgram(void *ptr);
int gamePadEvents(GameActions *actions);
void updatePosCords(Texto* coordenadas);
void updateFPS(Texto *fps, int totFrames);
int startGameEngine(void* ptrMsg);

// Propiedades de la ventana
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
glm::vec2 windowSize;
bool showHitbox = false;
bool showStats = false;
bool newContext = false; // Bandera para identificar si OpenGL 2.0 > esta activa
struct GameTime gameTime;
Camera* Camera::cameraInstance = NULL;

//CONTADOR MONEDAS
int contadorMonedas = 0;

//Destruir enemigo
bool enemigoDerrotado = false;

//ENUM PARA MOSTRAR DIFERENTES DIALOGOS SEGUN CONTEXTO
enum DialogoID {
    DIALOGO_NONE = 0,
    DIALOGO_INTRO,
    DIALOGO_MONEDAS0,
    DIALOGO_ENEMIGO,
    DIALOGO_BUSCA_VENDEDOR
    // agrega más diálogos aquí
};

enum MisionID {
    MISION_BUSCAR_MONEDAS,
    MISION_IR_TIENDA,
    MISION_BUSCAR_VENDEDOR,
    MISION_DERROTA_AL_VENDEDOR
};

static DialogoID dialogoActual = DIALOGO_INTRO;
static MisionID misionActual = MISION_BUSCAR_MONEDAS;
static CombatSystem* combatSystem = nullptr;

void MostrarDialogo(const char* texto, DialogoID& dialogoActual);
void MostrarCombate(CombatSystem* combat);
void MostrarResultadoCombate(CombatSystem* combat, DialogoID& dialogoActual, CombatSystem*& combatSystem);
void DestruirEnemigo();

// Objecto de escena y render
Scene *OGLobj;

#ifdef _WIN32 
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DEMOTEMPLATEOGL, szWindowClass, MAX_LOADSTRING);
    // Si no logra activar OpenGL 2 o superior termina el programa
    if (prepareRenderWindow(hInstance, nCmdShow))
        return 1;
    LOGGER::LOGS::getLOGGER().setWindow(&hWnd);
    // game loop
    gamPad = new GamePadRR(1); // Obtenemos el primer gamepad conectado
    MSG msg = { 0 };
    void *ptrMsg = (void*)&msg;
#else
int main(int argc, char** argv){
    if (!glfwInit()){
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT, "DemoTemplateOGL", NULL, NULL);
    windowSize = glm::vec2(SCR_WIDTH, SCR_HEIGHT);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        ERRORL("No opengl load", "OPENGL");
        glfwTerminate();
        return -1;
    }
    void *ptrMsg = NULL;
#endif
    return startGameEngine(ptrMsg);
}


int startGameEngine(void *ptrMsg){
    //ESTO CARGA EL MODELO DEL PROTAGONISTA
    // Main character with it's camera
    glm::vec3 translate, scale, v(0, 0, -1);
    translate = glm::vec3(5.0f, 10.0f, -5.0f);
    //5, ye - 1,-5
    Camera* camera = Camera::getInstance();
    Model* model = new Model("models/Calaca/CalacaChida.fbx", translate, camera);
    model->setTranslate(&translate);
    camera->setFront(v);
    camera->setCharacterHeight(5.0);
    scale = glm::vec3(0.02f, 0.02f, 0.02f);
    model->setScale(&scale);
    model->setTranslate(&translate);

    // ✅ CREAR HITBOX PERSONALIZADO
    Node nodoCalaca = model->AABBsize;

    // Ajustar tamaño del hitbox (puedes modificar estos valores)
    nodoCalaca.m_halfWidth = 1.0f;   // Ancho del hitbox
    nodoCalaca.m_halfHeight = 1.0f;  // Alto del hitbox
    nodoCalaca.m_halfDepth = 1.0f;   // Profundidad del hitbox
    nodoCalaca.m_center = glm::vec4(0, 1.5f, 0,1.0f); // Centro relativo al modelo

    // Eliminar hitbox automático si existe
    if (model->getModelAttributes()->size() > 0 &&
        model->getModelAttributes()->at(0).hitbox != NULL) {
        Model* oldAABB = (Model*)model->getModelAttributes()->at(0).hitbox;
        delete oldAABB;
        model->getModelAttributes()->at(0).hitbox = NULL;
    }

    // Generar nuevo hitbox personalizado
    Model* hitboxCalaca = CollitionBox::GenerateAABB(translate, nodoCalaca, camera);

    // Asignar el hitbox al modelo
    if (model->getModelAttributes()->size() == 0) {
        ModelAttributes attr;
        attr.hitbox = hitboxCalaca;
        attr.setTranslate(&translate);
        attr.setNextTranslate(&translate);
        model->getModelAttributes()->push_back(attr);
    }
    else {
        model->getModelAttributes()->at(0).hitbox = hitboxCalaca;
    }

    // ✅ CARGAR ANIMACIONES (DESPUÉS de configurar el hitbox)
    try {
        std::vector<Animation> animations = Animation::loadAllAnimations(
            "models/Calaca/CalacaChida.fbx",
            model->GetBoneInfoMap(),
            model->getBonesInfo(),
            model->GetBoneCount()
        );
        for (Animation animation : animations)
            model->setAnimator(Animator(animation));
        model->setAnimation(1);
    }
    catch (...) {
        ERRORL("Could not load animation!", "ANIMACION");
    }
    

    OGLobj = new Scenario(model); // Creamos nuestra escena con esa posicion de inicio
    translate = glm::vec3(5.0f, OGLobj->getTerreno()->Superficie(5.0, -5.0), -5.0f);
    model->setTranslate(&translate);
    model->setNextTranslate(&translate);
    renderiza = false;

    int running = 1;
    Texto *fps = new Texto((WCHAR*)L"0 fps", 20, 0, 0, 22, 0, model);
    fps->name = "FPSCounter";
    OGLobj->getLoadedText()->emplace_back(fps);
    Texto *coordenadas = new Texto((WCHAR*)L"0", 20, 0, 0, 0, 0, model);;
	coordenadas->name = "Coordenadas";
    OGLobj->getLoadedText()->emplace_back(coordenadas);
    updatePosCords(coordenadas);

    

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    // *** INICIALIZAR IMGUI ***
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Estilo de ImGui (puedes cambiar a ImGui::StyleColorsClassic() o ImGui::StyleColorsLight())
    ImGui::StyleColorsDark();

    // Inicializar backends
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplOpenGL3_Init("#version 330");

    gameTime.lastTick = get_nanos() / 1000000.0; // ms
    int totFrames = 0;
    double deltasCount = 0;
    double jump = 0;
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (isProgramRunning(ptrMsg)) {
        deltasCount += gameTime.deltaTime;
        totFrames++;
        // *** NUEVO FRAME DE IMGUI ***
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (deltasCount >= 1000.0f){
            updateFPS(fps, totFrames);
            deltasCount -= 1000.0f;
            totFrames = 1;
        }
        updatePosCords(coordenadas);
        GameActions actions;
        actions.jump = &jump;


     
        // Mostrar diálogo según estado
        switch (dialogoActual)
        {
        case DIALOGO_INTRO:
            MostrarDialogo("Tienes hambre y necesitas monedas para comprar comida...", dialogoActual);
            break;

        case DIALOGO_MONEDAS0:
            MostrarDialogo("Ve a la tienda por algo de comer. Estas en los huesos xDxDXDxxdxdxdx", dialogoActual);
            break;

        case DIALOGO_ENEMIGO:
            MostrarDialogo("Oye tu, te vi robando de la tienda! Preparate para morir!!!!!!111!!?", dialogoActual);
            break;

        case DIALOGO_BUSCA_VENDEDOR:
            MostrarDialogo("Como siempre no hay nadie atendiendo, debiste ir a un seven...\n PERO EN TU RANCHO NO HAY SEVEN XD\n Busca al vendedor.", dialogoActual);
            break;

        default:
            break;
        }

        if (combatSystem && combatSystem->isCombatActive() && dialogoActual == DIALOGO_NONE) {
			combatSystem->update(gameTime.deltaTime);
            MostrarCombate(combatSystem);
        }

        else if (combatSystem && !combatSystem->isCombatActive() && dialogoActual== DIALOGO_NONE) {
            MostrarResultadoCombate(combatSystem, dialogoActual, combatSystem);
        }
       


        //Menu dinamico con contadores, vida, etc
        
        {
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 250, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSizeConstraints(ImVec2(240, 0), ImVec2(240, FLT_MAX));

            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoCollapse;



            ImGui::Begin("Misiones", nullptr, flags);

            //ImGui::Text("FPS: %d", totFrames);
            //ImGui::Separator();

            ImGui::Text("Objetivo:");
            switch (misionActual) {
            case MISION_BUSCAR_MONEDAS:
                ImGui::Text("Recolecta 3 monedas");
                ImGui::Text("Monedas: %d", contadorMonedas);
                break;

            case MISION_IR_TIENDA:
                ImGui::Text("Ve a la tienda");
                break;
                
            case MISION_BUSCAR_VENDEDOR:
                ImGui::Text("Busca al vendedor");
                break;

            case MISION_DERROTA_AL_VENDEDOR:
                ImGui::Text("Derrota al vendedor");
                break;

            default:
                ImGui::Text("Sin misión activa");
                break;
            }

            if (ImGui::CollapsingHeader("Opciones Debug")) {
                ImGui::Checkbox("Show Hitboxes", &showHitbox);
                ImGui::Checkbox("Show Stats", &showStats);
            }

            //ImGui::Text("Player Position:");
            //glm::vec3* pos = OGLobj->getMainModel()->getTranslate();
            //ImGui::Text("  X: %.2f", pos->x);
            //ImGui::Text("  Y: %.2f", pos->y);
            //ImGui::Text("  Z: %.2f", pos->z);
            //ImGui::Separator();


            ImGui::End();
        }


        // render
        // ------
        bool checkCollition = checkInput(&actions, OGLobj);
        int cambio = OGLobj->update();
        
		//COMPRUEBA SI HAY COLISION CON MONEDA
        if (cambio == 1) { // Código especial para moneda recogida
            contadorMonedas++;

            //CAMBIO DE DIALOGO y MISION
            if (contadorMonedas == 3 && dialogoActual!= DIALOGO_MONEDAS0) {
                dialogoActual = DIALOGO_MONEDAS0;
                misionActual = MISION_IR_TIENDA;
            }
        }

        //COMPROBAR ZONA Y MONEDAS PARAA DIALOGO
        if (contadorMonedas == 3 && dialogoActual != DIALOGO_BUSCA_VENDEDOR) {
            float targetX = 115.0f;
            float targetZ = 35.0f;
            float activationRadius = 5.0f;

            float distanceX = model->getTranslate()->x - targetX;
            float distanceZ = model->getTranslate()->z - targetZ;
            float distance = sqrt(distanceX * distanceX + distanceZ * distanceZ);

            if (distance <= activationRadius) {
                dialogoActual = DIALOGO_BUSCA_VENDEDOR;
                misionActual = MISION_BUSCAR_VENDEDOR;
            }
        }

        //DIALOGO ENEMIGO
        if(contadorMonedas==3 && dialogoActual!=DIALOGO_ENEMIGO && cambio==2){
            dialogoActual = DIALOGO_ENEMIGO;
            misionActual = MISION_DERROTA_AL_VENDEDOR;

            if (combatSystem == nullptr) {
                combatSystem = new CombatSystem();
            }
            combatSystem->setPlayerModel(model);

			//find enemy model in scene
			Model* enemyModel = nullptr;
            std::vector<Model*>* loaded = OGLobj->getLoadedModels();
            for (Model* m : *loaded) {
                if (m && m->getModelType() == "Enemigo") {
                    enemyModel = m;
                    break;
                }
            }

            if (enemyModel != nullptr) {
				combatSystem->setEnemyModel(enemyModel);
            }

            combatSystem->startCombat();
        }

        Scene* escena = OGLobj->Render();
        if (escena != OGLobj) {
            delete OGLobj;
            OGLobj = escena;
            OGLobj->getLoadedText()->emplace_back(fps);
            OGLobj->getLoadedText()->emplace_back(coordenadas);
        }

        // *** RENDERIZAR IMGUI ***
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        swapGLBuffers();
    }
    model = OGLobj->getMainModel();
    if (OGLobj != NULL) delete OGLobj;
    if (camera != NULL) delete camera;
    if (model != NULL) delete model;
    if (fps != NULL) delete fps;
    if (coordenadas != NULL) delete coordenadas;

    // *** CLEANUP IMGUI ***
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    font_atlas::clearInstance();
    return finishProgram(ptrMsg);
}

bool checkInput(GameActions* actions, Scene* scene) {
    bool changeAnimation = false;
    if (gamePadEvents(actions)) {
    }
    else {
        mouseActions();
        KeysEvents(actions);
    }
    Model* OGLobj = scene->getMainModel();

    // ✅ **DETECTAR SI EL PERSONAJE SE ESTÁ MOVIENDO**
    bool isMoving = (actions->advance != 0 || actions->hAdvance != 0);

    if (actions->displayHitboxStats) {
        showHitbox = !showHitbox;
        showStats = !showStats;
    }
    if (actions->firstPerson) {
        OGLobj->cameraDetails->setFirstPerson(!OGLobj->cameraDetails->getFirstPerson());
    }
    if (actions->sideAdvance != 0) {
        OGLobj->setNextRotY(OGLobj->getNextRotY() + ((6 * gameTime.deltaTime / 100) * actions->sideAdvance));
    }
    if (actions->hAdvance != 0) {
        glm::vec3 pos = *OGLobj->getTranslate();
        pos.x += actions->hAdvance * (3 * gameTime.deltaTime / 100) * glm::cos(glm::radians(OGLobj->getRotY()));
        pos.z += actions->hAdvance * (3 * gameTime.deltaTime / 100) * glm::sin(glm::radians(OGLobj->getRotY()));
        OGLobj->setNextTranslate(&pos);
    }
    if (actions->advance != 0) {
        glm::vec3 pos = *OGLobj->getTranslate();
        pos.x += actions->advance * (3 * gameTime.deltaTime / 100) * glm::sin(glm::radians(OGLobj->getRotY()));
        pos.z += actions->advance * (3 * gameTime.deltaTime / 100) * glm::cos(glm::radians(OGLobj->getRotY()));
        OGLobj->setNextTranslate(&pos);
    }
    if (*actions->jump > 0) {
        glm::vec3 pos = *OGLobj->getNextTranslate();
        double del = (*actions->jump) * gameTime.deltaTime / 100;
        pos.y += del;
        (*actions->jump) -= del;
        if (*actions->jump < 0.01f)
            *actions->jump = 0.0f;
        OGLobj->setNextTranslate(&pos);
    }
    if (actions->getAngle() != NULL) {
        OGLobj->cameraDetails->calculateAngleAroundPlayer((*actions->getAngle()) * (6 * gameTime.deltaTime / 100));
    }
    if (actions->getPitch() != NULL) {
        OGLobj->cameraDetails->setPitch(OGLobj->cameraDetails->getPitch() + (*actions->getPitch()) * (6 * gameTime.deltaTime / 100));
    }
    if (actions->getZoom() != NULL) {
        OGLobj->cameraDetails->setZoom(OGLobj->cameraDetails->getZoom() + *actions->getZoom() * (6 * gameTime.deltaTime / 100));
    }
    if (actions->getPlayerZoom() != NULL) {
        OGLobj->cameraDetails->calculateZoomPlayer(*actions->getPlayerZoom() * (6 * gameTime.deltaTime / 100));
    }

    

    if (!(combatSystem && combatSystem->isCombatActive())) {
        if (isMoving) {
            OGLobj->setAnimation(2); // caminar
        }
        else {
            OGLobj->setAnimation(1); // idle normal
        }
    }

    return true; // siempre buscar colision
}

#ifdef _WIN32
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	// ImGui event handler
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

    switch (message) {
        case WM_CREATE: {
            RECT rect;
            if (GetClientRect(hWnd, &rect)) 
                windowSize = glm::vec2(rect.right - rect.left, rect.bottom - rect.top);
            else windowSize = glm::vec2(0);
        } break;
        case WM_COMMAND: {
            switch (wParam) {
                case 9999: memset(KEYS, 0, 256 * sizeof(bool));
                        break;
            }
        } break;
        case WM_MOUSEMOVE: {
            int value = lParam;
        }break;
        case WM_TIMER: {
        } break;
        case WM_PAINT: {
        }break;
        case WM_DESTROY: {
            if (newContext) {
                ReleaseDC(hWnd, dc);
                wglDeleteContext(rc);
                PostQuitMessage(0);
            }
        } break;
        case WM_SIZE: {
            if (newContext) {
                //esta opcion del switch se ejecuta una sola vez al arrancar y si se
                //afecta el tama�o de la misma se dispara de nuevo
                int height = HIWORD(lParam),
                    width = LOWORD(lParam);
                if (height == 0)
                    width = 1;
                SCR_HEIGHT = height;
                SCR_WIDTH = width;
                glViewport(0, 0, width, height);
                RECT rect;
                if (GetClientRect(hWnd, &rect)) 
                    windowSize = glm::vec2(rect.right - rect.left, rect.bottom - rect.top);
                else windowSize = glm::vec2(0);
            }
        } break;
        case WM_LBUTTONDOWN: {
            cDelta.setLbtn(true);
        }break;
        case WM_LBUTTONUP: {
            cDelta.setLbtn(false);
        }break;
        case WM_RBUTTONDOWN: {
            cDelta.setRbtn(true);
        }break;
        case WM_RBUTTONUP: {
            cDelta.setRbtn(false);
        }break;
        case WM_MOUSEWHEEL: {
            char delta = HIWORD(wParam);
            cDelta.setMouseWheel(delta);
        }break;
        case WM_KEYDOWN: {
            KEYS[wParam] = true;
        } break;
        case WM_KEYUP: {
//            if (wParam == KEYB_CAMERA || wParam == KEYB_HMOVEMENT)
                KEYS[wParam] = false;
        } break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool SetUpPixelFormat(HDC hDC, PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB, PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB) {
    const int pixelAttribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
//        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
//        WGL_SAMPLES_ARB, 4,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_ALPHA_BITS_ARB, 8,
        0 
    };
    int pixelFormatID;
    UINT numFormats;
    bool status = wglChoosePixelFormatARB(hDC, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);
    if (status == false || numFormats == 0)
        return true;
    PIXELFORMATDESCRIPTOR PFD;
    DescribePixelFormat(hDC, pixelFormatID, sizeof(PFD), &PFD);
    SetPixelFormat(hDC, pixelFormatID, &PFD);
    return false;
}

// Funciones de inicializacion para ventana compatible con OpenGL
int prepareRenderWindow(HINSTANCE hInstance, int nCmdShow) {
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = szWindowClass;
    RegisterClassEx(&wc);
    hInst = hInstance;
    HWND fakeWND = CreateWindow(
        szWindowClass, L"Fake Window",      // window class, title
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
        0, 0,                       // position x, y
        1, 1,                       // width, height
        NULL, NULL,                 // parent window, menu
        hInstance, NULL);           // instance, param
    HDC fakeDC = GetDC(fakeWND);        // Device Context
    PIXELFORMATDESCRIPTOR fakePFD;
    ZeroMemory(&fakePFD, sizeof(fakePFD));
    fakePFD.nSize = sizeof(fakePFD);
    fakePFD.nVersion = 1;
    fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    fakePFD.iPixelType = PFD_TYPE_RGBA;
    fakePFD.cColorBits = 32;
    fakePFD.cAlphaBits = 8;
    fakePFD.cDepthBits = 24;
    int fakePFDID = ChoosePixelFormat(fakeDC, &fakePFD);
    if (fakePFDID == 0) {
        MessageBox(fakeWND, L"ChoosePixelFormat() failed.", L"", 0);
        return 1;
    }
    if (SetPixelFormat(fakeDC, fakePFDID, &fakePFD) == false) {
        MessageBox(fakeWND, L"SetPixelFormat() failed.", L"", 0);
        return 1;
    }
    HGLRC fakeRC = wglCreateContext(fakeDC);    // Rendering Contex
    if (fakeRC == 0) {
        MessageBox(fakeWND, L"wglCreateContext() failed.", L"", 0);
        return 1;
    }
    if (wglMakeCurrent(fakeDC, fakeRC) == false) {
        MessageBox(fakeWND, L"wglMakeCurrent() failed.", L"", 0);
        return 1;
    }
    wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
    if (wglChoosePixelFormatARB == nullptr) {
        MessageBox(fakeWND, L"wglGetProcAddress() failed.", L"", 0);
        return 1;
    }
    wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
    if (wglCreateContextAttribsARB == nullptr) {
        MessageBox(fakeWND, L"wglGetProcAddress() failed.", L"", 0);
        return 1;
    }
    gladLoadGL();
    // create window
    RECT wr = { 0, 0, SCR_WIDTH, SCR_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindow(szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100,
        wr.right - wr.left, //SCR_WIDTH, 
        wr.bottom - wr.top, //SCR_HEIGHT,
        NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) {
        return 1;
    }
    dc = NULL;
    dc = GetDC(hWnd);
    if (SetUpPixelFormat(dc, wglChoosePixelFormatARB, wglCreateContextAttribsARB)) {
        MessageBox(hWnd, L"wglChoosePixelFormatARB() failed.", L"", 0);
        return 1;
    }
    const int major_min = 3, minor_min = 3;
    int  contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    rc = NULL;
//    rc = wglCreateContext(dc);
    rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
    if (rc == NULL) {
        MessageBox(hWnd, L"wglCreateContextAttribsARB() failed.", L"", 0);
        return 1;
    }
//    wglMakeCurrent(dc, rc);
    wglDeleteContext(fakeRC);
    ReleaseDC(fakeWND, fakeDC);
    DestroyWindow(fakeWND);
    if (!wglMakeCurrent(dc, rc)) {
        MessageBox(hWnd, L"wglMakeCurrent() failed.", L"", 0);
        return 1;
    }
    gladLoadGL();
    newContext = true;
    return 0;
}
#else
void window_size_callback(GLFWwindow* window, int width, int height){
    if (height == 0)
        width = 1;
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
    glViewport(0, 0, width, height);
    windowSize = glm::vec2(SCR_WIDTH, SCR_HEIGHT);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    char k = (key == GLFW_KEY_LEFT_SHIFT)? input.Shift : key;
    if (action == GLFW_PRESS || action == GLFW_REPEAT || action == GLFW_RELEASE){
        if (k > 5 && k < 10)
            switch(k){
                case 6: KEYS[input.Right] = GLFW_RELEASE == action ? false : true;
                    break;
                case 7: KEYS[input.Left] = GLFW_RELEASE == action ? false : true;
                    break;
                case 8: KEYS[input.Down] = GLFW_RELEASE == action ? false : true;
                    break;
                case 9: KEYS[input.Up] = GLFW_RELEASE == action ? false : true;
                    break;
            }
        else
            KEYS[k] = GLFW_RELEASE == action ? false : true;
    }else
        if (k == KEYB_CAMERA || k == KEYB_HMOVEMENT)
            KEYS[k] = false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        cDelta.setRbtn(action == GLFW_PRESS);
    else if (button == GLFW_MOUSE_BUTTON_LEFT)
        cDelta.setLbtn(action == GLFW_PRESS);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    char delta = yoffset;
    cDelta.setMouseWheel(delta);
}
#endif

void mouseActions() {
    double x, y;
#ifdef _WIN32 
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(hWnd, &p);
    x = p.x;
    y = p.y;
#else
    glfwGetCursorPos(window, &x, &y);
#endif
    glm::vec2 scale = glm::vec2(x, y) / windowSize;
    OGLobj->getMainModel()->cameraDetails->setPitch(scale.y * 70.0f - 30.f);
    scale = cDelta.setPosition(x, y, cDelta.getLbtn() || cDelta.getRbtn());
/*    scale = cDelta.setPosition(x, y, true);
    if (scale.x != 0)
        OGLobj->getMainModel()->cameraDetails->calculateAngleAroundPlayer((scale.x / abs(scale.x)) * -3.0);*/
}

int isProgramRunning(void *ptr){
    double currentTime = get_nanos() / 1000000.0;
    gameTime.deltaTime =  currentTime - gameTime.lastTick; // ms
    gameTime.lastTick = currentTime;  // ms
    int flag = 1;
#ifndef _WIN32
    if (!renderiza){
        glfwSetKeyCallback(window, key_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        renderiza = true;
    }
    flag = !glfwWindowShouldClose(window);
    if (flag)
        glfwPollEvents();
#else
    if (!renderiza)
        renderiza = true;
    MSG &msg = *(MSG*)ptr;
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        flag = msg.message == WM_QUIT? 0 : 1;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif
    return flag;
}

void swapGLBuffers(){
#ifdef _WIN32
    SwapBuffers(dc);
#else
    glfwSwapBuffers(window);
#endif

}

int finishProgram(void *ptr){
#ifdef _WIN32
    MSG &msg = *(MSG*)ptr;
    return (int)msg.wParam;
#else
    glfwTerminate();
    return 0;
#endif
}

int gamePadEvents(GameActions *actions){
#ifdef _WIN32 
    if (gamPad->IsConnected()) {
        //convierto a flotante el valor analogico de tipo entero
        double grados = (float)gamPad->GetState().Gamepad.sThumbLX / 32767.0;
        //debido a que los controles se aguadean con el uso entonces ya no dan el cero
        //en el centro, por eso lo comparo con una ventana de aguadencia de mi control
        if (grados > 0.19 || grados < -0.19)
            //model->CamaraGiraY(grados * 3.0);
            actions->setAngle(grados * 3.0);
        float velocidad = (float)gamPad->GetState().Gamepad.sThumbLY / 32767;
        if (velocidad > 0.19 || velocidad < -0.19) {
            //model->movePosition(velocidad);
            actions->advance = velocidad;
        }
        return 1;
    } else
        return 0;
#else
    if (false)
        ERRORL("This should be the gamepad code", "GAMEPAD");
    return 0;
#endif
}

void updatePosCords(Texto* coordenadas) {
    wchar_t wCoordenadas[350] = { 0 };
	wchar_t componente[100] = { 0 };
	wcscpy_s(wCoordenadas, 350, L"X: ");
	swprintf(componente, 100, L"%f", OGLobj->getMainModel()->getTranslate()->x);
	wcscat_s(wCoordenadas, 350, componente);
	wcscat_s(wCoordenadas, 350, L" Y: ");
	swprintf(componente, 100, L"%f", OGLobj->getMainModel()->getTranslate()->y);
	wcscat_s(wCoordenadas, 350, componente);
	wcscat_s(wCoordenadas, 350, L" Z: ");
	swprintf(componente, 100, L"%f", OGLobj->getMainModel()->getTranslate()->z);
	wcscat_s(wCoordenadas, 350, componente);
	coordenadas->initTexto((WCHAR*)wCoordenadas);
}

void updateFPS(Texto *fps, int totFrames){
    WCHAR conv[50] = { 0 };
    swprintf((wchar_t*)conv, 50, L"%d", totFrames);
    wcscat_s((wchar_t*)conv, 50, L" FPS");
    fps->initTexto(conv);
}

void MostrarCombate(CombatSystem* combat) {
    if (!combat || !combat->isCombatActive()) return;

    ImGuiIO& io = ImGui::GetIO();

    
    float width = 720.0f;  
    float posX = (io.DisplaySize.x - width) * 0.5f;  
    float posY = io.DisplaySize.y - 230;  

    ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 0), ImGuiCond_Always); 

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize |           
        ImGuiWindowFlags_AlwaysAutoResize |   
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Combate", nullptr, flags);

    auto playerStats = combat->getPlayerStats();
    auto enemyStats = combat->getEnemyStats();

    // --- Información de vida ---
    ImGui::Text("TU VIDA: %d/%d", playerStats.health, playerStats.maxHealth);
    ImGui::ProgressBar(playerStats.health / (float)playerStats.maxHealth, ImVec2(-1, 0));

    ImGui::Spacing();

    ImGui::Text("ENEMIGO: %d/%d", enemyStats.health, enemyStats.maxHealth);
    ImGui::ProgressBar(enemyStats.health / (float)enemyStats.maxHealth, ImVec2(-1, 0));

    ImGui::Separator();
    ImGui::TextWrapped("%s", combat->getLastActionLog().c_str());
    ImGui::Separator();

    // --- Botones del jugador ---
    if (combat->getIsPlayerTurn()) {
        if (ImGui::Button("Atacar", ImVec2(140, 40))) combat->executePlayerAction(CombatAction::ATTACK);
        ImGui::SameLine();
        if (ImGui::Button("Defender", ImVec2(140, 40))) combat->executePlayerAction(CombatAction::DEFEND);
        ImGui::SameLine();
        if (ImGui::Button("Burla", ImVec2(140, 40))) combat->executePlayerAction(CombatAction::TAUNT);
        ImGui::SameLine();
        if (ImGui::Button("Esquivar", ImVec2(140, 40))) combat->executePlayerAction(CombatAction::DODGE);
    }
    else {
        ImGui::Text("Turno del enemigo...");
    }

    ImGui::End();
}




void MostrarResultadoCombate(CombatSystem* combat, DialogoID& dialogoActual, CombatSystem*& combatSystem) {
    if (!combat) return;

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2 - 250, io.DisplaySize.y / 2 - 150), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Always);

    ImGui::Begin("Resultado del Combate", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse);

    ImGui::Spacing();
    ImGui::Spacing();

    if (combat->isPlayerAlive()) {
        // VICTORIA
        if (!enemigoDerrotado) {
            enemigoDerrotado = true;
            DestruirEnemigo();
        }

        ImVec2 textSize = ImGui::CalcTextSize("¡¡¡VICTORIA!!!");
        ImGui::SetCursorPosX((500 - textSize.x) / 2.0f);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "¡¡¡VICTORIA!!!");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("Le diste en su madre al cajero! Ya puedes salir con tu comida.");

        

        ImGui::Spacing();
        ImGui::Spacing();

        float buttonWidth = 200.0f;
        ImGui::SetCursorPosX((500 - buttonWidth) / 2.0f);
        if (ImGui::Button("Continuar", ImVec2(buttonWidth, 50))) {
            dialogoActual = DIALOGO_NONE;
            delete combatSystem;
            combatSystem = nullptr;
        }
    }
    else {
        // DERROTA
        ImVec2 textSize = ImGui::CalcTextSize("DERROTA");
        ImGui::SetCursorPosX((500 - textSize.x) / 2.0f);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DERROTA");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("No pues, ni pedo XD. El cajero te dio en tu madre.");


        ImGui::Spacing();
        ImGui::Spacing();

        float buttonWidth = 200.0f;
        ImGui::SetCursorPosX((500 - buttonWidth) / 2.0f);
        if (ImGui::Button("Reintentar", ImVec2(buttonWidth, 50))) {
            combat->startCombat();
        }
    }

    ImGui::End();
}


void MostrarDialogo(const char* texto, DialogoID& dialogoActual)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(20, io.DisplaySize.y - 150), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 40, 130), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Dialogo", nullptr, flags);

    ImGui::TextWrapped(texto);

    ImGui::Spacing();
    if (ImGui::Button("Cerrar")) {
        dialogoActual = DIALOGO_NONE; // ✅ Simplemente cierra el diálogo
    }

    ImGui::End();
}

void DestruirEnemigo() {
    if (!OGLobj) return;

    std::vector<Model*>* models = OGLobj->getLoadedModels();

    // Buscar y eliminar el enemigo
    for (auto it = models->begin(); it != models->end(); ) {
        if ((*it)->getModelType() == "Enemigo") {
            Model* enemigo = *it;

            // Eliminar el modelo
            delete enemigo;

            // Removerlo del vector
            it = models->erase(it);
            break;
        }
        else {
            ++it;
        }
    }
}
