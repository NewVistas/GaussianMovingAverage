const uint8_t analogDataPin = 0; // connect to a potentiometer
const uint8_t numSamplesPin = 1; // connect to another potentiometer
const uint8_t changeNumSamplesThreshold = 2; // set to a value between minNumSamples (most resolution) and the value of maxNumSamples (least resolution)
const uint8_t minNumSamples = 1;
const uint8_t maxNumSamples = 50; // you can increase this (max of 255) if you want to be able to average more data points

uint8_t numSamples, currentSampleNumber = 0;
uint16_t analogData[maxNumSamples];
uint16_t currentSampleData;
float mean;
bool hasNumSamplesChanged = false; // TODO: make these static and move them inside loop

void setup() {
	Serial.begin(9600);
	pinMode(analogDataPin, INPUT);
	pinMode(numSamplesPin, INPUT);
	numSamples = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
	mean = analogRead(analogDataPin);
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
			for(int i = 0; i < numSamples; i++) { // print samples in array
				Serial.print(analogData[i]);
				Serial.print("\t");
				analogData[i] = analogData[i + 1]; // shift all values in the array left by one ...
			}
			Serial.println();
			analogData[numSamples-1] = currentSampleData; // ... and add the current sample to the end
			for(int i = 0; i < numSamples; i++) {
				Serial.print(analogData[i]);
				Serial.print("\t");
			}
			Serial.println();
			Serial.println();

			// Serial.println();
			// Serial.println(getGaussianAverage(mean, 2147483600, analogData, numSamples)); // print gaussian weighted average of array
			// Serial.println();
			// Serial.println();
			currentSampleNumber = 0; // reset currentSampleNumber to beginning of array
		}
		else {
			analogData[currentSampleNumber] = currentSampleData;
			currentSampleNumber++;
		}
	}
}

// float getGaussianAverage(float mean, float variance, int16_t analogData[], uint8_t numberOfSamples) {
// 	static const float inv_sqrt_2pi = 0.3989422804014327;
// 	float sigma = sqrt(variance);
// 	float a = (x - mean) / sigma;

// 	return inv_sqrt_2pi / sigma * exp(-0.5f * a * a); // TODO: optimize the divide by multiplying and right shifting
// }