const uint8_t analogDataPin = 0; // connect to a pot (simulates the analog signal)
const uint8_t numSamplesPin = 2; // connect to a pot (controls the number of sample points that are averaged.  The width of the gaussian curve spans the number of points averaged)
const uint8_t cutoffPin = 3; // connect to a pot (controls how near the edges of a fixed gaussian curve we use for weights.  Doesn't control the variance or standard deviation)
const uint8_t minNumSamples = 5;
const uint8_t maxNumSamples = 255; // you can increase this (max of 255) if you want to be able to average more data points
const float minCutoff = 3.0;
const float maxCutoff = 8.0;

uint16_t analogData[maxNumSamples];
uint16_t currentSampleData;
uint16_t middleSample;

enum Mode {
	plotCurve,
	plotData,
	printWeights
} mode;

void setup() {
	mode = plotCurve; // change this to "plotCurve" to plot the shape of the gaussian curve used for filtering.  Once values are tweaked to your liking, change to "printWeights", copy the weights and hard-code them.

	Serial.begin(115200);
	pinMode(analogDataPin, INPUT);
	pinMode(numSamplesPin, INPUT);
	pinMode(cutoffPin, INPUT);
}

void loop() {
	// TODO: optimize this like numSamples, but put mapf inside if statement, as well
	float gaussianCurveCutoff = mapf(analogRead(cutoffPin), 0, 1023, minCutoff, maxCutoff); // how wide the gaussian curve is (number of standard deviations to either side of the center)
	uint8_t numSamples = map(analogRead(numSamplesPin), 0, 1023, minNumSamples, maxNumSamples);

	if(mode == printWeights) {
		printGaussianWeights(numSamples, gaussianCurveCutoff);
		while(1);
	}

	if(mode == plotData) {
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
	else if(mode == plotCurve) {
		for(uint8_t i = 0; i < numSamples; i++) {
			float gaussianWeight = calculateGaussian(i, numSamples, gaussianCurveCutoff);
			Serial.println(100*gaussianWeight, 3);
		}
	}
}

float getGaussianAverage(uint16_t analogData[], uint8_t numSamples, float gaussianCurveCutoff) {
	float gaussianSum = 0;
	float sum = 0;

	for(uint8_t i = 0; i < numSamples; i++) {
		float gaussianWeight = calculateGaussian(i, numSamples, gaussianCurveCutoff);
		gaussianSum += (analogData[i]*gaussianWeight);
		sum += gaussianWeight;
	}
	return gaussianSum / sum; // normalize
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float calculateGaussian(uint8_t currentPoint, uint8_t numSamples, float gaussianCurveCutoff) {
	float a = (currentPoint*gaussianCurveCutoff/(numSamples-1) - gaussianCurveCutoff/2);
	return exp(-0.5f * a * a);
}

void printGaussianWeights(uint8_t numSamples, float gaussianCurveCutoff) {
	float gaussianWeights[numSamples];
	float sum = 0;

	for(uint8_t i = 0; i < numSamples; i++) {
		gaussianWeights[i] = calculateGaussian(i, numSamples, gaussianCurveCutoff);
		sum += gaussianWeights[i];
	}

	Serial.println();
	Serial.println();
	for(uint8_t i = 0; i < numSamples; i++) {
		Serial.println(gaussianWeights[i] / sum, 7);
	}

	Serial.println();
	Serial.print("Number of samples: ");
	Serial.print(numSamples);
	Serial.print("\t");
	Serial.print("Gaussian curve cutoff: ");
	Serial.println(gaussianCurveCutoff);
	Serial.println();
}