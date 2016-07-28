const uint8_t analogDataPin = 0; // connect to a potentiometer
const uint8_t numSamplesPin = 1; // connect to another potentiometer
const uint8_t changeNumSamplesThreshold = 2; // set to a value between minNumSamples (most resolution) and the value of maxNumSamples (least resolution)
const uint8_t minNumSamples = 1;
const uint8_t maxNumSamples = 50; // you can increase this (max of 255) if you want to be able to average more data points
const uint8_t gaussianCurveCutoff = 2.5; // only include 2.5 standard deviations to either side of the center
// This is the normal distribution table with z values from 1 to 3.5 in 0.1 increments (it is only used to calculate cutoffCorrection). Once you find a value that works, you can hard-code it:
const normalTable[26] = {0.84134474, 0.86433393, 0.8849303, 0.90319951, 0.91924334, 0.93319279, 0.94520070, 0.95543453, 0.96406968, 0.9712834, 0.97724986, 0.98213557, 0.98609655, 0.9892758, 0.99180246, 0.99379033, 0.99533881, 0.99653302, 0.9974448, 0.99813418, 0.99865010, 0.99903239, 0.99931286, 0.99951657, 0.99966307, 0.99976737};

if(gaussianCurveCutoff >= 1.0 && gaussianCurveCutoff <= 3.5) { // make sure it's within bounds
	// 						array index = ((2.5 * 10) -10) = 15
	const float cutoffCorrection = 1.0 / normalTable[gaussianCurveCutoff * 10 - 10]; // index should be 15 and cutoffCorrection should equal 1.01256 
}

uint8_t numSamples, currentSampleNumber = 0;
uint16_t analogData[maxNumSamples];
uint16_t currentSampleData;
bool hasNumSamplesChanged = false; // TODO: make these static and move them inside loop

void setup() {
	Serial.begin(9600);
	pinMode(analogDataPin, INPUT);
	pinMode(numSamplesPin, INPUT);
	numSamples = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
}

void loop() {
	uint8_t numSamplesReading = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
	if(abs(numSamplesReading - numSamples) >= changeNumSamplesThreshold) { // only change numSamples when the pot reading has changed more than changeNumSamplesThreshold
		numSamples = numSamplesReading;
		hasNumSamplesChanged = true;
	}
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
				// Serial.println();
				// Serial.println();
				Serial.println(getGaussianAverage(map(analogRead(analogDataPin), 0, 1023, 1, 20), analogData, numSamples), 3); // print gaussian weighted average of array
				// Serial.println();
				delay(100);
			}
			currentSampleNumber++;
		}
	}
}

float getGaussianAverage(float variance, uint16_t analogData[]) {
	static const float inv_sqrt_2pi = 0.3989422804014327;
	float sigma = sqrt(variance);
	uint32_t gaussianSum = 0, 
	float gaussianAverage;
	// uint32_t mean = 0;
	// for(uint8_t i = 0; i < numSamples; i++) { // calculate the mean
	// 	mean += analogData[i];
	// }
	// mean /= numSamples;

	// calculate the gaussian weighted average of the data points in analogData:
	// for(uint8_t i = 0; i < numSamples; i++) {
	// 	float a = (analogData[i] - mean) / sigma;
	// 	analogData[i] = inv_sqrt_2pi / sigma * exp(-0.5f * a * a);
	// }

	// print values for a gaussian distribution:
	// for(int8_t i = -numSamples/2; i < numSamples/2; i++) {
	// 	float a = (i*gaussianCurveCutoff*2/numSamples - 0) / sigma;
	// 	Serial.println(inv_sqrt_2pi / sigma * exp(-0.5f * a * a), 3);
	// }
	
	// print values for a gaussian distribution:
	for(uint8_t i = 0; i < numSamples; i++) {
		float a = (i*gaussianCurveCutoff*2/numSamples - gaussianCurveCutoff) / sigma;
		float gaussianWeight = inv_sqrt_2pi / sigma * exp(-0.5f * a * a);
		Serial.print(analogData[i]);
		Serial.print("\t");
		Serial.println(gaussianWeight, 3);
		gaussianSum += (analogData[i]*gaussianWeight);
	}
	return (gaussianSum / numSamples)*cutoffCorrection;
}
