/*Acq-IntClk-DigRef.c
* Acquires 100 k sps on AI0 after waiting for trigger high on PF0 and writes compressed TDMS files 
* adaption of Acq-IntClk-DigRef.c which has analog acqusition and triggering
* modified Raul Ocampo
* moddate 8/22/2016, 8/23/2016
*/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	double      sps = 63072.24;		// 100k samples per second
	int32       length = 266;		// 14s irradiation/count cycles repeated 10 times, skipping 1st irradiation, 19 * 14s = 266
	int32       acqlength = (int32)(sps * length) + 1;
	char        errBuff[2048]={'\0'};

	// DAQmx task, timing, trigger configure
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"/Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",sps,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,acqlength));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(taskHandle,"/Dev1/PFI0",DAQmx_Val_Rising));
	printf("Waiting for trigger on PFIO\n");

	// DAQmx logging to TDMS
	DAQmxErrChk (DAQmxConfigureLogging(taskHandle, "C:\\Users\\neutr\\Documents\\example.tdms", DAQmx_Val_Log, "GroupName", DAQmx_Val_OpenOrCreate));

	// DAQmx Start Code
	DAQmxErrChk (DAQmxStartTask(taskHandle));
	DAQmxErrChk (DAQmxWaitUntilTaskDone(taskHandle, -1.0));
	DAQmxErrChk(DAQmxClearTask(taskHandle));

	printf("Acquisition complete of ai0\n");
	printf("Wrote %d samples\n", acqlength);

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ){
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}
