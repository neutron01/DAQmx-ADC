/*********************************************************************
*
* ANSI C Example program:
*    Acq-IntClk-DigRef.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a finite amount of data
*    using a digital reference trigger.
*
* Instructions for Running:
*    1. Select the Physical Channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage Ranges.
*    Note: For better accuracy try to match the Input Ranges to the
*          expected voltage level of the measured signal.
*    3. Select how many Samples to Acquire on Each Channel.
*    4. Set the Rate of the Acquisiton.
*    Note: The Rate should be AT LEAST twice as fast as the maximum
*          frequency component of the signal being acquired.
*    5. Select the Source and Edge of the Digital Reference Trigger
*       for the acquisition.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Define the parameters for an Internal Clock Source.
*       Additionally, define the sample mode to be Finite.
*    4. Define the parameters for a Digital Edge Reference Trigger.
*    5. Call the Start function to begin the acquisition.
*    6. Use the Read function to retrieve the waveform. Set a timeout
*       so an error is returned if the samples are not returned in
*       the specified time limit.
*    7. Call the Clear Task function to clear the Task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O Control. Also, make sure your digital trigger
*    terminal matches the Trigger Source Control. For further
*    connection information, refer to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	double		sps = 63072.24;		// 100k samples per second
	int32		length = 266;		// 14s irradiation/count cycles repeated 10 times, skipping 1st irradiation, 19 * 14s = 266
	int32		acqlength = (int32)(sps * length) + 1;
	//int32       read;
	//float64     data[1000];
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"/Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",sps,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,acqlength));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(taskHandle,"/Dev1/PFI0",DAQmx_Val_Rising));

	/*********************************************/
	// DAQmx TDMS Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxConfigureLogging(taskHandle, "C:\\Users\\neutr\\Documents\\example.tdms", DAQmx_Val_Log, "GroupName", DAQmx_Val_OpenOrCreate));

	printf("Waiting for trigger on PFIO\n");

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));
	DAQmxErrChk (DAQmxWaitUntilTaskDone(taskHandle, -1.0));
	DAQmxErrChk(DAQmxClearTask(taskHandle));

	printf("Acquisition complete of ai0\n");
	printf("Wrote %d samples\n", acqlength);

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	// DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,1000,-1.0,DAQmx_Val_GroupByChannel,data,1000,&read,NULL));

	// printf("Acquired %d points\n",(int)read);

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
