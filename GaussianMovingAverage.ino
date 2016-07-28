const uint8_t analogDataPin = 0; // connect to a pot (simulates the analog signal)
const uint8_t variancePin = 1; // connect to a pot (controls the variance)
const uint8_t numSamplesPin = 2; // connect to a pot (controls the number of sample points that are averaged.  The width of the gaussian curve spans the number of points averaged)
const uint8_t cutoffPin = 3; // connect to a pot (controls how near the edges of a fixed gaussian curve we use for weights.  Doesn't control the variance or standard deviation)
const uint8_t changeNumSamplesThreshold = 1; // set to a value between minNumSamples (most resolution) and the value of maxNumSamples (least resolution)
const uint8_t minNumSamples = 7;
const uint8_t maxNumSamples = 255; // you can increase this (max of 255) if you want to be able to average more data points
const float maxVariance = 100.0;
const float maxCutoff = 12.0;
// This is the normal distribution table with z values from 1.5 to 3.0 in 0.1 increments (it is only used to calculate cutoffCorrection). Once you find a value that works, you can hard-code it:
const float normalTable[16] = {0.93319279, 0.94520070, 0.95543453, 0.96406968, 0.9712834, 0.97724986, 0.98213557, 0.98609655, 0.9892758, 0.99180246, 0.99379033, 0.99533881, 0.99653302, 0.9974448, 0.99813418, 0.99865010};

float gaussianCurveCutoff; // only include 2.5 standard deviations to either side of the center
float cutoffCorrection;
uint8_t numSamples, currentSampleNumber = 0;
uint16_t analogData[maxNumSamples];
uint16_t currentSampleData;
bool hasNumSamplesChanged = false; // TODO: make these static and move them inside loop

void setup() {
	Serial.begin(9600);
	pinMode(analogDataPin, INPUT);
	pinMode(variancePin, INPUT);
	pinMode(numSamplesPin, INPUT);
	pinMode(cutoffPin, INPUT);
	numSamples = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
}

void loop() {
	float variance = mapf(analogRead(variancePin), 0, 1023, 0, maxVariance);

	// TODO: optimize this like numSamples, but put mapf inside if statement, as well
	gaussianCurveCutoff = mapf(analogRead(cutoffPin), 0, 1023, 1, maxCutoff);
	//								array index = ((2.5 - 1.5)*10) = 15
	cutoffCorrection = 1.0 / normalTable[round((gaussianCurveCutoff*10) - 15)]; // for a gaussianCurveCutoff of 2.5, index should be 15 and cutoffCorrection should equal 1.01256 

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
				// Serial.println(analogData[i]);
				analogData[i] = analogData[i + 1]; // shift all values in the array left by one ...
			}
			analogData[numSamples-1] = currentSampleData; // ... and add the current sample to the end
			currentSampleNumber = 0; // reset currentSampleNumber to beginning of array
		}
		else {
			analogData[currentSampleNumber] = currentSampleData;

			if(currentSampleNumber == numSamples - 1) { // if we are at the last sample
				// eventually, call and print getGaussianAverage here.

				// Serial.println();
				// Serial.println();
				// Serial.println(getGaussianAverage(variance, analogData, numSamples), 3); // print gaussian weighted average of array
				// Serial.println();
				// delay(100);
			}
			currentSampleNumber++;
		}
	}
	// Serial.print(analogData[1]);
	// Serial.print("\t");
	// Serial.print(variance);
	// Serial.print("\t");
	// Serial.print(numSamples);
	// Serial.print("\t");
	// Serial.println(gaussianCurveCutoff);
}

float getGaussianAverage(uint16_t analogData[], uint8_t numberOfSamples, float gaussianCurveCutoff) {
	static const float inv_sqrt_2pi = 0.3989422804014327;
	// float sigma = sqrt(variance);
	float sigma = 1;
	// float sigma = numberOfSamples + numberOfSamples*gaussianCurveCutoff; // help here
	uint32_t gaussianSum = 0;
	float gaussianAverage;
	// uint32_t mean = 0;
	// for(uint8_t i = 0; i < numberOfSamples; i++) { // calculate the mean
	// 	mean += analogData[i];
	// }
	// mean /= numberOfSamples;

	// calculate the gaussian weighted average of the data points in analogData:
	// for(uint8_t i = 0; i < numberOfSamples; i++) {
	// 	float a = (analogData[i] - mean) / sigma;
	// 	analogData[i] = inv_sqrt_2pi / sigma * exp(-0.5f * a * a);
	// }

	// print values for a gaussian distribution:
	// for(int8_t i = -numberOfSamples/2; i < numberOfSamples/2; i++) {
	// 	float a = (i*gaussianCurveCutoff*2/numberOfSamples - gaussianCurveCutoff) / sigma;
	// 	Serial.println(10*(inv_sqrt_2pi / sigma * exp(-0.5f * a * a)), 3);
	// }

	// float sigma = numberOfSamples;

	// print values for a gaussian distribution:
	for(uint8_t i = 0; i < numberOfSamples; i++) {
		// float a = (i*gaussianCurveCutoff*2/numberOfSamples - gaussianCurveCutoff) / sigma;
		// float a = (i - numberOfSamples/2) / sigma;
		float a = (i*gaussianCurveCutoff/numberOfSamples - gaussianCurveCutoff/2) / sigma;
		float gaussianWeight = inv_sqrt_2pi / sigma * exp(-0.5f * a * a);
		Serial.print(10*gaussianWeight, 3);
		Serial.print(" ");
		Serial.print(numSamples);
		Serial.print(" ");
		Serial.print(gaussianCurveCutoff);
		Serial.println();
		gaussianSum += (analogData[i]*gaussianWeight);
	}
	return (gaussianSum / numberOfSamples)*cutoffCorrection;
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}