const uint8_t analogDataPin = 0; // connect to a pot (simulates the analog signal)
const uint8_t numSamplesPin = 2; // connect to a pot (controls the number of sample points that are averaged.  The width of the gaussian curve spans the number of points averaged)
const uint8_t cutoffPin = 3; // connect to a pot (controls how near the edges of a fixed gaussian curve we use for weights.  Doesn't control the variance or standard deviation)
const uint8_t changeNumSamplesThreshold = 1; // set to a value between minNumSamples (most resolution) and the value of maxNumSamples (least resolution)
const uint8_t minNumSamples = 7;
const uint8_t maxNumSamples = 255; // you can increase this (max of 255) if you want to be able to average more data points
const float minCutoff = 1.0;
const float maxCutoff = 12.0;

float gaussianCurveCutoff; // how wide the gaussian curve is (number of standard deviations to either side of the center)
uint8_t numSamples, currentSampleNumber = 0;
uint16_t analogData[maxNumSamples];
uint16_t currentSampleData;
bool hasNumSamplesChanged = false; // TODO: make these static and move them inside loop

void setup() {
	Serial.begin(9600);
	pinMode(analogDataPin, INPUT);
	pinMode(numSamplesPin, INPUT);
	pinMode(cutoffPin, INPUT);
	numSamples = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
}

void loop() {
	// TODO: optimize this like numSamples, but put mapf inside if statement, as well
	gaussianCurveCutoff = mapf(analogRead(cutoffPin), 0, 1023, minCutoff, maxCutoff);

	uint8_t numSamplesReading = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
	if(abs(numSamplesReading - numSamples) >= changeNumSamplesThreshold) { // only change numSamples when the pot reading has changed more than changeNumSamplesThreshold
		numSamples = numSamplesReading;
		hasNumSamplesChanged = true;
	}
	getGaussianAverage(analogData, numSamples, gaussianCurveCutoff);
	delay(100);

	if(hasNumSamplesChanged == true) {
		hasNumSamplesChanged = false;
		currentSampleNumber = 0;
	}
	else {
		uint16_t currentSampleData = analogRead(analogDataPin);

		if(currentSampleNumber == numSamples) { // if we are one past the last sample
			for(uint8_t i = 0; i < numSamples; i++) { // print samples in array
				// if(i >= numSamples/2 && i < numSamples) { // TODO: verify this code:
				// 	Serial.print(analogData[i - numSamples/2]);
				// }
				// else if(i >= 0) {
				// 	Serial.print(analogData[numSamples - (i + 1)]);
				// }
				analogData[i] = analogData[i + 1]; // TODO: possibly use a linked list instead. shift all values in the array left by one ...
			}
			analogData[numSamples-1] = currentSampleData; // ... and add the current sample to the end
			currentSampleNumber = 0; // reset currentSampleNumber to beginning of array
		}
		else {
			analogData[currentSampleNumber] = currentSampleData;

			if(currentSampleNumber == numSamples - 1) { // if we are at the last sample
				// Serial.print(" ");
				// Serial.println(getGaussianAverage(variance, analogData, numSamples), 3); // print gaussian weighted average of array
			}
			currentSampleNumber++;
		}
	}
}

float getGaussianAverage(uint16_t analogData[], uint8_t numberOfSamples, float gaussianCurveCutoff) {
	float gaussianSum = 0;
	float sum = 0;

	// print values for a gaussian distribution:
	for(uint8_t i = 0; i < numberOfSamples; i++) {
		float a = (i*gaussianCurveCutoff/(numberOfSamples-1) - gaussianCurveCutoff/2);
		float gaussianWeight = exp(-0.5f * a * a);
		Serial.print(10*gaussianWeight, 3);
		Serial.print(" ");
		Serial.print(numSamples);
		Serial.print(" ");
		Serial.print(gaussianCurveCutoff);
		Serial.println();
		gaussianSum += (analogData[i]*gaussianWeight);
		sum += gaussianWeight;
	}
	return gaussianSum / sum; // normalize
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}