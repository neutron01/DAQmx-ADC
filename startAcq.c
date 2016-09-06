/*startAcq.c
* Acquires 100 k sps on AI0 after waiting for trigger high on PF0 and writes compressed TDMS files,
* then waits for next trigger up to number specified
* adaption of Acq-IntClk-DigRef.c which has analog acqusition and triggering and
* TDMS-Acq-Int Clk which has TDMS logging
* modified Raul Ocampo
* moddate 8/22/2016, 8/23/2016, 9/2/2016
* 
* Usage
* startAcq (seconds recording time, int) (cycles to capture, int) (filename, char[])
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(int argc, char *argv[])
{
	// called correctly?
	if (argc != 4) {
		printf("usage: startAcq 14 10 U235_100ng_14s_100ksps_\n");
		return 1;
	}
	int32       error = 0;
	TaskHandle  taskHandle = 0;
	double		sps = 100000;				// for ADC clock, samples per second
	int32		length = atoi(argv[1]);		// 14s count per irradiation + 2s transit/dead time
	int32		acqlength = (int32)(sps * length);
	int32		cycles = atoi(argv[2]);		// triggers to wait for
	char        errBuff[2048]={'\0'};
	char		istr[2];

	for (int i = 0; i < cycles; i++) {
		char		str[80];
		/*********************************************/
		// DAQmx Configure Code
		/*********************************************/
		DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
		// http://zone.ni.com/reference/en-XX/help/370471W-01/daqmxcfunc/daqmxcreateaivoltagechan/
		DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, "/Dev1/ai0", "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", sps, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, acqlength));
		DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandle, "/Dev1/PFI0", DAQmx_Val_Rising));
		strcpy_s(str, 80, argv[3]);		// assemble filename
		_itoa_s(i, istr, 80, 10);
		strcat_s(str, 80, istr);
		strcat_s(str, 80, ".tdms");
		printf("%s filename\n", str);
		DAQmxErrChk(DAQmxConfigureLogging(taskHandle, str, DAQmx_Val_Log, "GroupName", DAQmx_Val_OpenOrCreate));

		printf("Waiting for trigger on PFIO\n");

		/*********************************************/
		// DAQmx Start Code
		/*********************************************/
		DAQmxErrChk(DAQmxStartTask(taskHandle));
		// http://zone.ni.com/reference/en-XX/help/370471AE-01/daqmxcfunc/daqmxwaituntiltaskdone/
		DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, -1.0));
		DAQmxErrChk(DAQmxClearTask(taskHandle));

		printf("Acquisition %d complete of ai0\n", i);
	}

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
