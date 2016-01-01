/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsSystem.cpp
 * @brief    Class WindowsSystem
 **/
#include "WindowsSystem.h"
#include "CcStringWin.h"
#include "CcKernel.h"
#include "WindowsFilesystem.h"
#include "WindowsSocket.h"
#include "WindowsGlobals.h"
#include "WindowsPipeIn.h"
#include "WindowsPipeOut.h"
#include <io.h>
#include <fcntl.h>

extern int main(int argc, char **argv);
int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
  ){
  main(__argc, __argv);
  CC_UNUSED(hInstance);
  CC_UNUSED(hPrevInstance);
  CC_UNUSED(lpCmdLine);
  CC_UNUSED(nCmdShow);
}


WindowsSystem::WindowsSystem() :
  m_GuiInitialized(false)
{
  m_Display       = 0;
  m_Timer         = 0;
}

WindowsSystem::~WindowsSystem() {
  delete m_Display;
}

void WindowsSystem::init(void){
  initSystem();
  initFilesystem();
}

bool WindowsSystem::initGUI(void){
  initDisplay();
  m_GuiInitialized = true;
  return true; // YES we have a gui
}


bool WindowsSystem::initCLI(void){
  if ( _isatty( _fileno(stdout))) {
    /* this is a terminal */
    AllocConsole(); 
    AttachConsole(GetCurrentProcessId());
    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;
  }
  return true; // YES we have a cli
}

void WindowsSystem::initFilesystem(){
  m_Filesystem = new WindowsFilesystem();
}

bool WindowsSystem::start( void ){
  m_SystemState = true; // We are done
  Kernel.systemReady();
  BOOL fGotMessage;
  MSG msg;
  while ( m_SystemState == true)
  {
    if (m_GuiInitialized)
    {
      while ( (fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 &&
              fGotMessage != -1)
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      return false;
    }
    else{
      Sleep(100);
    }
  }
  return false;
}

void WindowsSystem::stop(void)
{
  m_SystemState = false;
  delete m_Timer;
}

/**
 * @brief Function to start the ThreadObject
 * @param Param: Param containing pointer to ThreadObject
 * @return alway returns 0, TODO: get success of threads as return value;
 */
DWORD WINAPI threadFunction(void *Param){
  CcThreadObject *pThreadObject = static_cast<CcThreadObject *>(Param);
  pThreadObject->enterState(CCTHREAD_RUNNING);
  pThreadObject->run();
  pThreadObject->enterState(CCTHREAD_STOPPED);
  return 0;
}

bool WindowsSystem::createThread(CcThreadObject* threadObj){
  DWORD threadId;
  if (NULL == CreateThread(0, 0, threadFunction, (void*)threadObj, 0, &threadId))
    return false;
  else
    return true;
}

int WindowsSystem::createProcess(CcProcess &processToStart){
  CcStringWin commandline(processToStart.m_Name + " " + processToStart.m_Arguments.collapseList(" "));
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  WindowsPipeIn  *pipeIn = 0;
  WindowsPipeOut *pipeOut = 0;
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(STARTUPINFO);
  si.dwFlags |= STARTF_USESTDHANDLES;
  if (processToStart.m_Input != 0){
    pipeIn = new WindowsPipeIn(processToStart.m_Input);
    si.hStdInput = pipeIn->m_Handle;
    createThread(pipeIn);
  }
  if (processToStart.m_Output != 0){
    pipeOut = new WindowsPipeOut(processToStart.m_Output);
    si.hStdOutput = pipeOut->m_Handle;
    si.hStdError = pipeOut->m_Handle;
    createThread(pipeOut);
  }
  // Start the child process. 
  if (!CreateProcess( NULL,   // No module name (use command line)
                      commandline.getCharString(),        // Command line
                      NULL,           // Process handle not inheritable
                      NULL,           // Thread handle not inheritable
                      TRUE,          // Set handle inheritance to FALSE
                      CREATE_NO_WINDOW,              // No creation flags
                      NULL,           // Use parent's environment block
                      NULL,           // Use parent's starting directory 
                      &si,            // Pointer to STARTUPINFO structure
                      &pi)           // Pointer to PROCESS_INFORMATION structure
    )
  {
    printf("CreateProcess failed (%d).\n", GetLastError());
  }
  else{
    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  // Close process and thread handles.
  if (pipeIn != 0)
    delete pipeIn;
  if (pipeOut != 0)
    delete pipeOut;
  return 0;
}

typedef bool(*KernelEntry)(CcKernel*);

void WindowsSystem::loadModule(CcString &Path){
  HINSTANCE hinstLib = LoadLibrary(TEXT("mod_test.dll"));
  KernelEntry ProcAdd;
  BOOL fFreeResult, fRunTimeLinkSuccess = FALSE;
  if (hinstLib != NULL)
  {
    ProcAdd = (KernelEntry)GetProcAddress(hinstLib, "KernelEntry");

    // If the function address is valid, call the function.

    if (NULL != ProcAdd)
    {
      fRunTimeLinkSuccess = TRUE;
      bool testit = (ProcAdd)(&Kernel);
    }
    else{
      printf("%d", GetLastError());
    }
    // Free the DLL module.

    fFreeResult = FreeLibrary(hinstLib);
  }
  // If unable to call the DLL function, use an alternative.
  if (!fRunTimeLinkSuccess)
    printf("Message printed from executable\n");
}

time_t WindowsSystem::getTime(void){
  SYSTEMTIME SystemTime;
  GetSystemTime(&SystemTime);
  return SystemTime.wMilliseconds;
}

void WindowsSystem::sleep(time_t timeoutMs){
  DWORD dwTemp =  timeoutMs;
  Sleep(dwTemp);
}

CcFileSystem* WindowsSystem::getFileSystem(){
  return m_Filesystem;
}

CcSocket* WindowsSystem::getSocket(eSocketType type){
  CcSocket* newSocket = new WindowsSocket(type);
  return newSocket;
}

void WindowsSystem::initSystem(void){
  initTimer();
}

void WindowsSystem::initTimer( void ){
  m_Timer = new WindowsTimer();
  Kernel.setDevice(m_Timer, eTimer);
}

void WindowsSystem::initDisplay( void ){
  m_Display = new WindowsDisplay();
  Kernel.setDevice(m_Display, eDisplay);
  m_Display->open();
}

void WindowsSystem::systemTick( void ){
  Kernel.systemTick();
}
