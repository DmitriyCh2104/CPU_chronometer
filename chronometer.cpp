#include <windows.h>
#include <string>
#include <iostream>

// globals *****
unsigned long long cpuHz; // frequancy
unsigned long long startCycles;
unsigned long long endCycles;
bool isRunning;
HWND time_label;
HINSTANCE hInst;

// get cycles 
unsigned long long getcycles(){
unsigned int lo,hi;
__asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
return ((unsigned long long)hi<<32)|lo; // joining
}

// get cpu frequancy with windows registry
unsigned long long getCpuFreq(){
DWORD dwMHz=0;
DWORD BufSize=sizeof(DWORD);
HKEY hKey;
long lError=RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                          "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                          0,KEY_READ,&hKey);
if(lError!=ERROR_SUCCESS){
std::cerr<<"Failed to read CPU frequency from registry"<<std::endl;
return 0; // error
}
lError=RegQueryValueExA(hKey,"~MHz",NULL,NULL,(LPBYTE)&dwMHz,&BufSize);
RegCloseKey(hKey);
if(lError!=ERROR_SUCCESS){
std::cerr<<"Failed to read MHz value from registry"<<std::endl;
return 0; // error
}
return (unsigned long long)dwMHz*1000000; // to Hz
}

// timer 
void start(){
if(isRunning){
} else {
if(endCycles>startCycles){
startCycles=getcycles()-(endCycles-startCycles); // keep old time if restarted
} else {
startCycles=getcycles(); 
}
isRunning=true; 
}}

void stop(){
if(!isRunning){
} else {
endCycles=getcycles();
isRunning=false; 
}}

void reset(){
startCycles=0;
endCycles=0;
isRunning=false; 
}

std::string gettime(){
if(isRunning) return"Running";
if(endCycles<=startCycles) return"Not started";
unsigned long long cycles=endCycles-startCycles;
double seconds=cycles/(double)cpuHz; // easy calc
return std::to_string(seconds)+" seconds";
}

// window commands*****
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
switch(msg){
case WM_COMMAND:
if(LOWORD(wParam)==100)start();
if(LOWORD(wParam)==200)stop();
if(LOWORD(wParam)==400)reset();
if(LOWORD(wParam)==500)DestroyWindow(hwnd);
InvalidateRect(hwnd,NULL,TRUE);
break;
case WM_PAINT:
{
PAINTSTRUCT ps;
HDC hdc=BeginPaint(hwnd,&ps);
std::string text=gettime();
SetWindowTextA(time_label,text.c_str()); // show time
EndPaint(hwnd,&ps);
}
break;
case WM_DESTROY:
PostQuitMessage(0); // close
break;
default:
return DefWindowProc(hwnd,msg,wParam,lParam);
}
return 0;
}

// main program *****
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev,LPSTR lpCmdLine,int nCmdShow){
hInst=hInstance;
cpuHz=getCpuFreq(); // set frequancy
if(cpuHz==0)return 1; // quit if failed
WNDCLASSA wc={0};
wc.lpfnWndProc=WndProc;
wc.hInstance=hInstance;
wc.lpszClassName="ChronometerClass";
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
RegisterClassA(&wc);
HWND hwnd=CreateWindowA("ChronometerClass","Chronometer",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,800,600,NULL,NULL,hInstance,NULL);
if(!hwnd)return 1;
time_label=CreateWindowA("STATIC","Not started",WS_VISIBLE|WS_CHILD,50,50,200,20,hwnd,NULL,hInstance,NULL);
CreateWindowA("BUTTON","Start",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,150,50,80,25,hwnd,(HMENU)100,hInstance,NULL);
CreateWindowA("BUTTON","Stop",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,240,50,80,25,hwnd,(HMENU)200,hInstance,NULL);
CreateWindowA("BUTTON","Reset",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,330,50,80,25,hwnd,(HMENU)400,hInstance,NULL);
CreateWindowA("BUTTON","Exit",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,50,500,80,25,hwnd,(HMENU)500,hInstance,NULL);
ShowWindow(hwnd,nCmdShow);
UpdateWindow(hwnd);
MSG msg;
while(GetMessage(&msg,NULL,0,0)){
TranslateMessage(&msg);
DispatchMessage(&msg);
}
return (int)msg.wParam;
}