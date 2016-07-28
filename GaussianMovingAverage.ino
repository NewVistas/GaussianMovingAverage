const uint8_t analogDataPin = 0; // connect to a pot (simulates the analog signal)
const uint8_t numSamplesPin = 2; // connect to a pot (controls the number of sample points that are averaged.  The width of the gaussian curve spans the number of points averaged)
const uint8_t cutoffPin = 3; // connect to a pot (controls how near the edges of a fixed gaussian curve we use for weights.  Doesn't control the variance or standard deviation)
const uint8_t changeNumSamplesThreshold = 1; // set to a value between minNumSamples (most resolution) and the value of maxNumSamples (least resolution)
const uint8_t minNumSamples = 5;
const uint8_t maxNumSamples = 255; // you can increase this (max of 255) if you want to be able to average more data points
const float minCutoff = 3.0;
const float maxCutoff = 8.0;

float gaussianCurveCutoff; // how wide the gaussian curve is (number of standard deviations to either side of the center)
uint8_t numSamples;
uint16_t analogData[maxNumSamples];
uint16_t currentSampleData;
uint16_t middleSample;

void setup() {
	Serial.begin(115200);
	pinMode(analogDataPin, INPUT);
	pinMode(numSamplesPin, INPUT);
	pinMode(cutoffPin, INPUT);
	numSamples = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
}

void loop() {
	// analogRead(numSamplesPin);
	// analogRead(numSamplesPin);

	// TODO: optimize this like numSamples, but put mapf inside if statement, as well
	gaussianCurveCutoff = mapf(analogRead(cutoffPin), 0, 1023, minCutoff, maxCutoff);

	uint8_t numSamplesReading = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);
	if(abs(numSamplesReading - numSamples) >= changeNumSamplesThreshold) { // only change numSamples when the pot reading has changed more than changeNumSamplesThreshold
		numSamples = numSamplesReading;
	}

	uint16_t currentSampleData = analogRead(analogDataPin);
	for(uint8_t i = 0; i < numSamples; i++) {
		// shift the data to the left ...
		analogData[i] = analogData[i + 1]; // TODO: possibly use a linked list instead
	}
	if(numSamples % 2 == 0) { // numSamples is even
		middleSample = (analogData[numSamples/2+1] + analogData[numSamples/2-1])/2; // average the two middle points
	}
	else { // numSamples is odd
		middleSample = analogData[numSamples/2]; // example: numSamples = 7, numSamples/2 = 3 (truncates), middle sample is analogData[3].
	}
	analogData[numSamples-1] = currentSampleData; // ... and add the current sample to the end
	Serial.print(middleSample); // print current sample delayed by numSamples/2
	Serial.print(" ");
	Serial.println(getGaussianAverage(analogData, numSamples, gaussianCurveCutoff), 3); // this should be compared to a reading delayed by numSamples/2
}

float getGaussianAverage(uint16_t analogData[], uint8_t numberOfSamples, float gaussianCurveCutoff) {
	float gaussianSum = 0;
	float sum = 0;

	// print values for a gaussian distribution:
	for(uint8_t i = 0; i < numberOfSamples; i++) {
		float a = (i*gaussianCurveCutoff/(numberOfSamples-1) - gaussianCurveCutoff/2);
		float gaussianWeight = exp(-0.5f * a * a);
		// Serial.print(100*gaussianWeight, 3);
		// Serial.print(" ");
		// Serial.print(numSamples);
		// Serial.print(" ");
		// Serial.print(gaussianCurveCutoff);
		// Serial.println();
		gaussianSum += (analogData[i]*gaussianWeight);
		sum += gaussianWeight;
	}
	return gaussianSum / sum; // normalize
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}