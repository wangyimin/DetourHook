Demo for how to use MS Detours  
https://blog.securehat.co.uk/process-injection/detecting-process-injection-using-microsoft-detour-hooks  
  
・DetourHook:  
&nbsp;&nbsp;(1) A hooker of MessageBoxW to show fixed content   
&nbsp;&nbsp;(2) A hooker of CreateFileW to redirect file location to my doucment folder of current user   
・Inject: Inject hooker into specified process  
・Tester: process for injection  
