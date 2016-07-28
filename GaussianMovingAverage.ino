const uint8_t analogDataPin = 0; // connect to a potentiometer
const uint8_t numSamplesPin = 1; // connect to another potentiometer
const uint8_t changeNumSamplesThreshold = 2; // set to a value between minNumSamples (most resolution) and the value of maxNumSamples (least resolution)
const uint8_t minNumSamples = 1;
const uint8_t maxNumSamples = 50; // you can increase this (max of 255) if you want to be able to average more data points

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
				getGaussianAverage(map(analogRead(analogDataPin), 0, 1023, 1, 20), analogData, numSamples); // print gaussian weighted average of array
				// Serial.println();
			}
			currentSampleNumber++;
		}
	}
}

void getGaussianAverage(float variance, uint16_t analogData[], uint8_t numberOfSamples) {
	static const float inv_sqrt_2pi = 0.3989422804014327;
	float sigma = sqrt(variance);
	uint32_t mean = 0;
	for(uint8_t i = 0; i < numberOfSamples; i++) { // calculate the mean
		mean += analogData[i];
	}
	mean /= numberOfSamples;

	// calculate the gaussian weighted average of the data points in analogData:
	for(uint8_t i = 0; i < numberOfSamples; i++) {
		float a = (analogData[i] - mean) / sigma;
		analogData[i] = inv_sqrt_2pi / sigma * exp(-0.5f * a * a);
	}

	// print values for a gaussian distribution:
	for(int8_t i = -numberOfSamples/2; i < numberOfSamples/2; i++) {
		float a = (i - 0) / sigma;
		Serial.println(inv_sqrt_2pi / sigma * exp(-0.5f * a * a), 3);
	}
	delay(100);
}