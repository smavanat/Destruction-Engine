#include<queue>
#include<algorithm>
#include "Outline.h"
#include "Maths.h"

#pragma region splitTexture
//Erases pixels in a texture in a circular radius determined by scale and marks the texture for alteration. 
//If all the pixels in the radius have been erased, then the texture is not marked for alteration.
void erasePixels(Sprite &s, Transform& t, SDL_Renderer* gRenderer, int scale, int x, int y) {
	Vector2 temp = {static_cast<float>(x), static_cast<float>(y)};
	Vector2 newOrigin = rotateAboutPoint(&temp, &t.position, -t.rotation, false);

	x = newOrigin.x - getOrigin(s, t).x;
	y = newOrigin.y - getOrigin(s, t).y;
	int width = s.surfacePixels->w;
	int height = s.surfacePixels->h;

	Uint32* pixels = getPixels32(s);

	if (scale > 0) {
		for (int w = 0; w < scale * 2; w++)
		{
			for (int h = 0; h < scale * 2; h++)
			{
				int dx = scale - w; // horizontal offset
				int dy = scale - h; // vertical offset
				if ((dx * dx + dy * dy) < (scale * scale) && (x + dx < width) && (x + dx > -1) && (y + dy < height) && (y + dy > -1))
				{
					if (pixels[(y + dy) * width + (x + dx)] == NO_PIXEL_COLOUR) continue;
					else {
						pixels[(y + dy) * width + (x + dx)] = NO_PIXEL_COLOUR;
						if (!s.needsSplitting) s.needsSplitting = true;
					}
				}
			}
		}
	}
	else {
		pixels[y * width + x] = NO_PIXEL_COLOUR;
	}

	loadFromPixels(s, gRenderer);
}

void erasePixelsRectangle(Sprite &s, Transform& t, SDL_Renderer* gRenderer, int halfX, int halfY, float rot, int x, int y) {
	Vector2 temp = {static_cast<float>(x), static_cast<float>(y)};
	Vector2 newOrigin = rotateAboutPoint(&temp, &t.position, -t.rotation, false);

	x = newOrigin.x - getOrigin(s, t).x;
	y = newOrigin.y - getOrigin(s, t).y;
	int width = s.surfacePixels->w;
	int height = s.surfacePixels->h;

	Uint32* pixels = getPixels32(s);

    //Precompute cos/sin
    float cosA = cos(rot * DEGREES_TO_RADIANS);
    float sinA = sin(rot * DEGREES_TO_RADIANS);

    // Compute the AABB of the rotated rectangle to limit pixel checks
	int maxRadius = static_cast<int>(ceil(sqrt(halfX * halfX + halfY * halfY)));
	int startX = std::max(0, x - maxRadius);
	int endX = std::min(width - 1, x + maxRadius);
	int startY = std::max(0, y - maxRadius);
	int endY = std::min(height - 1, y + maxRadius);

    for (int px = startX; px <= endX; px++) {
		for (int py = startY; py <= endY; py++) {
			// Translate pixel into rectangle local space (centered at x, y)
			float dx = px - x;
			float dy = py - y;

			// Rotate into local unrotated space
			float localX = dx * cosA + dy * sinA;
			float localY = -dx * sinA + dy * cosA;

			// Now check if within rectangle bounds
			if (abs(localX) <= halfX && abs(localY) <= halfY) {
				if (pixels[py * width + px] != NO_PIXEL_COLOUR) {
					pixels[py * width + px] = NO_PIXEL_COLOUR;
					if (!s.needsSplitting) s.needsSplitting = true;
				}
			}
		}
	}
	loadFromPixels(s, gRenderer);
}

bool isAtTopEdge(int pixelPosition, int arrayWidth) {
	if (pixelPosition < arrayWidth) {
		return true;
	}
	return false;
}
bool isAtBottomEdge(int pixelPosition, int arrayWidth, int arrayLength) {
	if (pixelPosition >= arrayLength - arrayWidth) {
		return true;
	}
	return false;
}
bool isAtLeftEdge(int pixelPosition, int arrayWidth) {
	if (pixelPosition % arrayWidth == 0) {
		return true;
	}
	return false;
}
bool isAtRightEdge(int pixelPosition, int arrayWidth) {
	if (pixelPosition % arrayWidth == arrayWidth - 1) {
		return true;
	}
	return false;
}

int* getNeighbours(int pixelPosition, int arrayWidth, int arrayLength) {
	int* neighbourArr = new int[8];
	int index = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			if (j == 0 && i == 0) {
				continue;
			}
			else {
				neighbourArr[index] = pixelPosition + (i * arrayWidth) + j;
				index++;
			}
		}
	}

	//If we are at an edge, set the direction associated with that edge to -1, as there can be no neighbour there
	if (isAtTopEdge(pixelPosition, arrayWidth)) {
		neighbourArr[0] = -1;
		neighbourArr[1] = -1;
		neighbourArr[2] = -1;
	}
	if (isAtRightEdge(pixelPosition, arrayWidth)) {
		neighbourArr[2] = -1;
		neighbourArr[4] = -1;
		neighbourArr[7] = -1;
	}
	if (isAtBottomEdge(pixelPosition, arrayWidth, arrayLength)) {
		neighbourArr[5] = -1;
		neighbourArr[6] = -1;
		neighbourArr[7] = -1;
	}
	if (isAtLeftEdge(pixelPosition, arrayWidth)) {
		neighbourArr[0] = -1;
		neighbourArr[3] = -1;
		neighbourArr[5] = -1;
	}
	return neighbourArr;
}

void cleanup(Uint32* pixels, std::vector<int> indexes) {
	for (int i = 0; i < indexes.size(); i++) {
		pixels[indexes[i]] = NO_PIXEL_COLOUR;
	}
}

//Crappy augmented flood-fill algorithm implementation taken from here: https://www.geeksforgeeks.org/flood-fill-algorithm/
std::vector<int> bfs(int index, int arrayWidth, int arrayLength, Uint32* pixels, int* visitedTracker) {
	std::vector<int> indexes;
	std::queue<int> q;

	indexes.push_back(index);
	q.push(index);
	//Need to use the visited tracker otherwise the program doesn't know if we have visited a pixel or not
	//so it keeps looping infinitely
	visitedTracker[index] = 1;

	while (!q.empty()) {
		int currentIndex = q.front();
		q.pop();
		if (!isAtTopEdge(currentIndex, arrayWidth) && pixels[currentIndex - arrayWidth] != NO_PIXEL_COLOUR && visitedTracker[currentIndex - arrayWidth] == 0) {
			indexes.push_back(currentIndex - arrayWidth);
			q.push(currentIndex - arrayWidth);
			visitedTracker[currentIndex - arrayWidth] = 1;
		}
		if (!isAtLeftEdge(currentIndex, arrayWidth) && pixels[currentIndex - 1] != NO_PIXEL_COLOUR && visitedTracker[currentIndex - 1] == 0) {
			indexes.push_back(currentIndex - 1);
			q.push(currentIndex - 1);
			visitedTracker[currentIndex - 1] = 1;
		}
		if (!isAtBottomEdge(currentIndex, arrayWidth, arrayLength) && pixels[currentIndex + arrayWidth] != NO_PIXEL_COLOUR && visitedTracker[currentIndex + arrayWidth] == 0) {
			indexes.push_back(currentIndex + arrayWidth);
			q.push(currentIndex + arrayWidth);
			visitedTracker[currentIndex + arrayWidth] = 1;
		}
		if (!isAtRightEdge(currentIndex, arrayWidth) && pixels[currentIndex + 1] != NO_PIXEL_COLOUR && visitedTracker[currentIndex + 1] == 0) {
			indexes.push_back(currentIndex + 1);
			q.push(currentIndex + 1);
			visitedTracker[currentIndex + 1] = 1;
		}
	}

	std::sort(indexes.begin(), indexes.end());

	return indexes;
}

std::pair<Sprite, Transform> constructNewPixelBuffer(std::vector<int> indexes, Uint32* pixels, int arrayWidth, Sprite& s, Transform& t, SDL_Renderer* gRenderer) {
	Uint32* newPixelBuffer;
	Sprite newSprite;
	int width = 0;
	int height = (int)(indexes.back() / arrayWidth) - (int)(indexes.front() / arrayWidth) + 1; //Why is the height including the pixel buffer??? Surely that shouldn't come up

	int startLinePos = indexes[0] % arrayWidth;
	int endLinePos = indexes[0] % arrayWidth;

	for (int i = 1; i < indexes.size() - 1; i++) {
		//THE SMALLEST startLinePos AND BIGGEST endLinePos DO NOT HAVE TO BE ON THE SAME ROW
		//if the pixel ahead of the current one is on the same row but the one behind is on a different row 
		//we have a startrow. But we only want to update the value if it's % is smaller than the current one
		//as this indicates it is further to the left.
		if (startLinePos > indexes[i] % arrayWidth) {
			startLinePos = indexes[i] % arrayWidth;
		}
		//If the pixel behind the current one is on the same row but the one ahead is on a new row, 
		//we have an endrow. But we only want to update the value if its % is bigger than the current one 
		//as this indicates it is further to the right.
		if (endLinePos < indexes[i] % arrayWidth) {
			endLinePos = indexes[i] % arrayWidth;
		}
	}
	width = endLinePos - startLinePos;
	width = width + 1;

	//Essentially, in order for marching squares to work, there has to be a one-pixel wide colourless perimeter around
	//the texture. This is so that there is an actual "border" for marching squares to trace around, and I think this is 
	//the least code and computationally expenseive method of implementing this, as the other way would be to have
	//marching squares "imagine" such a border, which would require a lot of checking of assumptions and pixel positions
	//ie, more code and work.
	//To implement this, we need to increase height and width by 2 (1 pixel for each side). We can increase height 
	//temporarily when we call it, but we need to increase width permanently, as otherwise it gets messed up in 
	//multiplication calls.
	width += 2;
	height += 2;

	//Creating the pixel buffer for the new texture
	newPixelBuffer = new Uint32[(width) * (height)];
	//The memset here is actually making all the pixels have an alpha of 255 for some reason, even though noPixelColour has an alpha of 0.
	//memset(newPixelBuffer, noPixelColour, width * height * sizeof(Uint32));//Filling it with transparent pixels
	//Using a for loop instead of memset fixes the alpha problem here.
	for (int i = 0; i < ((width) * (height)); i++) {
		newPixelBuffer[i] = NO_PIXEL_COLOUR;
	}

	//Populating the new pixel buffer with data
	//startLinePos acts as an offset to figure out how far left the texture has moved. 
	//Current heigh measures the current row of the new pixel buffer we are on
	int currentHeight = 1; //Since the first row will be the blank pixel perimeter.
	for (int i = 0; i < indexes.size(); i++) {
		//If the pixels are on different rows, increment the current height by their difference.
		if (i != 0 && (floor(indexes[i] / arrayWidth) > floor(indexes[i - 1] / arrayWidth))) {
			currentHeight += floor(indexes[i] / arrayWidth) - floor(indexes[i - 1] / arrayWidth);
		}
		//Add 1 here as an offset to the LHS perimeter. The RHS and BHS perimeters will be automatically accounted for
		//as the code will never reach them, so no need to worry about that.
		newPixelBuffer[(currentHeight * (width)) + ((indexes[i] % arrayWidth) - startLinePos) + 1] = pixels[indexes[i]];
	}

	//Edge case of 1x1 textures. Need to do newPixelBuffer[5], since the fifth pixel would be the middle one in a 3x3 grid.
	if (indexes.size() == 1) {
		newPixelBuffer[5] = pixels[indexes[0]];
	}

	//This works now. Cieling it gives the correct value. However I now get a memory error somewhere
	float originX = ceilf(getOrigin(s, t).x + (startLinePos)) - 1.0f; //Assume original textures also have transparent border
	float originY = ceilf(getOrigin(s, t).y + ((int)floor(indexes[0] / arrayWidth))) - 1.0f;

	//Have to manually calculate the center from the origin here.
	float centreX = originX + floorf((width) / 2.0f); //floor instead of ceil because 0-indexed
	float centreY = originY + floorf((height) / 2.0f);

	//Set this as a pointer as otherwise this variable will be destroyed once this method finishes.
	newSprite = createSprite(width, height, newPixelBuffer, gRenderer);
	Transform newTransform = Transform((Vector2){centreX, centreY}, t.rotation);

	cleanup(pixels, indexes);
	return std::make_pair(newSprite, newTransform);

	//delete[] newPixelBuffer; //This needs to be commented out since we are actively using newPixelBuffer to create our texture.
}

std::vector<std::pair<Sprite, Transform>> splitTextureAtEdge(Sprite& s, Transform& t, SDL_Renderer* gRenderer) {
	if (!s.needsSplitting) return {};
	int width = s.surfacePixels->w;
	int height = s.surfacePixels->h;

	//Get the texture pixels
	Uint32* pixels = getPixels32(s); //This has the correct alpha values for the pixels (checked)
	//A placement int that gets the length of the pixel 1D array
	int arrayLength = width * height;
	//A bitmap that remembers if we visited a pixel before or not.
	int* visitedTracker = new int[arrayLength];
	//Initialising visitedTracker to all 0.
	memset(visitedTracker, 0, arrayLength * sizeof(int));
	//Pixel buffer vector
	std::vector<int> possibleStarts;
	//Vector for all the new textures that are being formed. This method will return them
	std::vector<std::pair<Sprite, Transform>> retArr;

	//For loop to get all the split texture parts.
	for (int i = 0; i < arrayLength; i++) {
		if (pixels[i] != NO_PIXEL_COLOUR) {
			possibleStarts = bfs(i, width, arrayLength, pixels, visitedTracker);
			if (!possibleStarts.empty()) {
				retArr.push_back(constructNewPixelBuffer(possibleStarts, pixels, width, s, t, gRenderer));
			}
		}
	}

	delete[] visitedTracker;
	return retArr;
}
#pragma endregion

#pragma region marchingSquares 
	//Marching squares: First we need to get the starting pixel. This is just done by iterating over the array until 
	//					we find a non-transparent pixel
	//					Then we need to find the square value of it and the four pixels surrounding it
	//					Then based on that square value (and in the special saddle cases also on the previous square value)
	//					we choose a new direction to move the "analysis" and add the currently analysed pixel to a vector;
	//					Good source code and ideas from here: https://emanueleferonato.com/2013/03/01/using-marching-squares-algorithm-to-trace-the-contour-of-an-image/
	//					And here: https://barradeau.com/blog/?p=391

	int getStartingPixel(Uint32* pixels, int arrayLength) {
		for (int i = 0; i < arrayLength; i++) {
			if (pixels[i] != NO_PIXEL_COLOUR) {
				return i;
			}
		}
		return -1;
	}

	//ChatGPT version: (This is actually far less retarded than my original code and actually quite nice. Doesn't stop the fact that the code overall still doesn't work though)
	int getCurrentSquare(int startIndex, int textureWidth, int textureLength, const Uint32* pixels) {
		int result = 0;

		// Calculate row and column of startIndex
		int row = startIndex / textureWidth;
		int col = startIndex % textureWidth;

		// Top-left pixel
		if (pixels[startIndex] != NO_PIXEL_COLOUR) result += 1;

		// Top-right pixel
		if (pixels[startIndex + 1] != NO_PIXEL_COLOUR) result += 2;

		// Bottom-left pixel
		if (pixels[startIndex + textureWidth] != NO_PIXEL_COLOUR) result += 4;

		// Bottom-right pixel
		if (pixels[startIndex + textureWidth + 1] != NO_PIXEL_COLOUR) result += 8;

		return result;
	}

	//Actual marching squares method. Requires that every texture has a one-pixel transparent border so that
	//it does not get confused by the lack of empty textueres.

	std::vector<int> marchingSquares(Sprite s) {
		Uint32* pixels = getPixels32(s);
		int width = s.surfacePixels->w;
		int length = s.surfacePixels->h * width;
		int totalPixels = width * length;

		std::vector<int> contourPoints;
		int startPoint = getStartingPixel(pixels, totalPixels);
		if (startPoint == -1) return contourPoints;
		//If the texture is filled on the LHS, we will end up with 15 as our first currentSquare. 
		//To avoid this, we simply offset startPoint one to the left, to get 12 as our currentSquare, 
		//and then marching squares handles the rest.
		if (getCurrentSquare(startPoint, width, length, pixels) == 15) {
			startPoint -= 1;
		}

		int stepX = 0, stepY = 0;
		int prevX = 0, prevY = 0;
		int currentPoint = startPoint;
		bool closedLoop = false;

		while (!closedLoop) {
			int currentSquare = getCurrentSquare(currentPoint, width, length, pixels);

			// Movement lookup based on currentSquare value
			switch (currentSquare) {
			case 1: case 13:
				stepX = 0; stepY = -1;
				break;
			case 8: case 10: case 11:
				stepX = 0; stepY = 1;
				break;
			case 4: case 12: case 14:
				stepX = -1; stepY = 0;
				break;
			case 2: case 3: case 7:
				stepX = 1; stepY = 0;
				break;
			case 5:
				stepX = 0; stepY = -1;
				break;
			case 6:
				stepX = (prevY == -1) ? -1 : 1;
				stepY = 0;
				break;
			case 9:
				stepX = 0;
				stepY = (prevX == 1) ? -1 : 1;
				break;
			default:
				printf("Unhandled or empty square encountered at index: %d\n", currentPoint);
				return contourPoints;
			}

			currentPoint += stepY * width + stepX;

			// Boundary checks. Should not happen but here just in case.
			if (currentPoint < 0 || currentPoint >= totalPixels) {
				printf("Out-of-bounds detected at index: %d\n", currentPoint);
				return contourPoints;
			}

			contourPoints.push_back(currentPoint);
			prevX = stepX;
			prevY = stepY;

			if (currentPoint == startPoint) closedLoop = true;
		}

		return contourPoints;
	}
#pragma endregion

#pragma region rdp
	//Code source: https://editor.p5js.org/codingtrain/sketches/SQjSugKn6
	int* convertIndexToCoords(int index, int arrayWidth) {
		return new int[2] { index% arrayWidth, (int)floor(index / arrayWidth) };
	}

	float lineDist(int point, int startPoint, int endPoint, int arrayWidth) {
		int* pointCoords = convertIndexToCoords(point, arrayWidth);
		int* startPointCoords = convertIndexToCoords(startPoint, arrayWidth);
		int* endPointCoords = convertIndexToCoords(endPoint, arrayWidth);

		//The source for this very cursed single line of code can be found here : https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
		float distance = abs(((endPointCoords[1] - startPointCoords[1]) * pointCoords[0]) -
			((endPointCoords[0] - startPointCoords[0]) * pointCoords[1]) +
			(endPointCoords[0] * startPointCoords[1]) - (endPointCoords[1] * startPointCoords[0])) /
			sqrt(pow((endPointCoords[1] - startPointCoords[1]), 2) + pow((endPointCoords[0] - startPointCoords[0]), 2));
		return distance;
	}

	int findFurthest(std::vector<int> allPoints, int a, int b, int epsilon, int arrayWidth) {
		float recordDistance = -1;
		int furthestIndex = -1;
		int start = allPoints[a];
		int end = allPoints[b];
		for (int i = a + 1; i < b; i++) {
			float d = lineDist(allPoints[i], start, end, arrayWidth);
			if (d > recordDistance) {
				recordDistance = d;
				furthestIndex = i;
			}
		}
		if (recordDistance > epsilon) return furthestIndex;
		else return -1;
	}

	//This method would be used for lines that do not join up
	void rdp(int startIndex, int endIndex, int epsilon, int arrayWidth, std::vector<int> allPoints, std::vector<int>& rdpPoints) {
		int nextIndex = findFurthest(allPoints, startIndex, endIndex, epsilon, arrayWidth);
		if (nextIndex > 0) {
			if (startIndex != nextIndex) {
				rdp(startIndex, nextIndex, epsilon, arrayWidth, allPoints, rdpPoints);
			}
			rdpPoints.push_back(allPoints[nextIndex]); 
			if (endIndex != nextIndex) {
				rdp(nextIndex, endIndex, epsilon, arrayWidth, allPoints, rdpPoints);
			}
		}
	}

	float lineDist(Vector2 point, Vector2 startPoint, Vector2 endPoint) {

		//The source for this very cursed single line of code can be found here : https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
		float distance = abs(((endPoint.y - startPoint.y) * point.x) -
			((endPoint.x - startPoint.x) * point.y) +
			(endPoint.x * startPoint.y) - (endPoint.y * startPoint.x)) /
			sqrt(pow((endPoint.y - startPoint.y), 2) + pow((endPoint.x - startPoint.x), 2));
		return distance;
	}

	int findFurthest(std::vector<Vector2>& allPoints, int a, int b, float epsilon) {
		float recordDistance = -1;
		int furthestIndex = -1;
		Vector2 start = allPoints[a];
        Vector2 end = allPoints[b];
		for (int i = a + 1; i < b; i++) {
			float d = lineDist(allPoints[i], start, end);
			if (d > recordDistance) {
				recordDistance = d;
				furthestIndex = i;
			}
		}
		if (recordDistance > epsilon) return furthestIndex;
		else return -1;
	}

	//This method would be used for lines that do not join up
	void rdp(int startIndex, int endIndex, float epsilon, std::vector<Vector2>& allPoints, std::vector<Vector2>& rdpPoints) {
		int nextIndex = findFurthest(allPoints, startIndex, endIndex, epsilon);
		if (nextIndex > 0) {
			if (startIndex != nextIndex) {
				rdp(startIndex, nextIndex, epsilon, allPoints, rdpPoints);
			}
			rdpPoints.push_back(allPoints[nextIndex]); 
			if (endIndex != nextIndex) {
				rdp(nextIndex, endIndex, epsilon, allPoints, rdpPoints);
			}
		}
	}
#pragma endregion

#pragma region ColliderGeneration
	b2BodyId createTexturePolygon(std::vector<int> rdpPoints, int arrayWidth, b2WorldId worldId, b2BodyType type, Transform& t) {
		//Getting points
		Vector2* points = new Vector2[rdpPoints.size()];
		for (int i = 0; i < rdpPoints.size(); i++) {
			int* temp = convertIndexToCoords(rdpPoints[i], arrayWidth);
			points[i] = { (temp[0]) * pixelsToMetres, (temp[1]) * pixelsToMetres };
		}

		return createPolygonCollider(points, rdpPoints.size(), t.position, t.rotation, worldId, type);
	}
#pragma endregion
