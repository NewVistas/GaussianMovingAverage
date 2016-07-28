# Gaussian Moving Average Filter
##Summary
Arduino code for applying a gaussian filter to an analog signal for suppressing noise.

There are three modes that will help you find an appropriate gaussian filter for your signal.  The first two modes are used with the Serial Plotter and the third is used with the Serial Monitor.
- Plot curve mode shows the shape of the gaussian function that will be used as a weighted average.
- PlotData mode plots the raw data along with the gaussian weighted moving average of your data.  The raw signal plotted is delayed by n/2 points (where n is the number of past data samples to be averaged), so it can be compared with the gaussian weighted average, which will be centered n/2 points ago.

##Hardware connections
Connect a potentiometer to analog pins 2 and 3.  Connect your analog sensor to analog pin 0, or, if you have a digital sensor, use your interfacing code instead of this line:
```uint16_t currentSampleData = analogRead(analogDataPin);```

##Usage
1. To change the mode, change the first line in the setup function to on of the options of the Mode enum, in this case, ```mode = plotCurve;```.
2. In the Arduino IDE, open the Arduino Serial plotter (Tools menu -> Serial Plotter), and change the baudrate popup menu to 115200.
3. Turn the two pots to adjust parameters controlling the gaussian curve.  The pot on pin 2 adjusts the number of samples that will be weighted with the gaussian function.  The pot on pin 3 controls the width of the gaussian curve which is proportional to the number of standard deviations it covers.
4. When you have a curve that looks suitable (just a first guess), change the mode to plotCurve and continue tweaking the pots (most importantly, the pot on pin 2).  Again, to do this, change the first line in setup to ```mode = plotData;```.
5. You can check mode 1 at any time.  When you are satisfied with how the filtered signal is following your signal and rejecting unwanted noise, switch to printWeights mode by replacing the first line of setup with ```mode = printWeights```.
6. Open the serial monitor by selecting Tools -> Serial Monitor.  Copy the first line which contains the weights for the gaussian filter.  You can now use them to initialize an array for your own gaussian filter.
