#include<queue>
#include<algorithm>
#include "Outline.hpp"
#include "Maths.h"
#include<PolyPartition/polypartition.h>

#pragma region splitTexture
//Erases pixels in a texture in a circular radius determined by scale and marks the texture for alteration. 
//If all the pixels in the radius have been erased, then the texture is not marked for alteration.
void erasePixels(Texture* texture, SDL_Renderer* gRenderer, int scale, int x, int y) {
	Vector2 newOrigin = rotateAboutPoint(newVector2(x, y), texture->getCentre(), -texture->getAngle(), false);

	x = newOrigin.x - texture->getOrigin().x;
	y = newOrigin.y - texture->getOrigin().y;

	Uint32* pixels = texture->getPixels32();

	if (scale > 0) {
		for (int w = 0; w < scale * 2; w++)
		{
			for (int h = 0; h < scale * 2; h++)
			{
				int dx = scale - w; // horizontal offset
				int dy = scale - h; // vertical offset
				if ((dx * dx + dy * dy) < (scale * scale) && (x + dx < texture->getWidth()) && (x + dx > -1) && (y + dy < texture->getHeight()) && (y + dy > -1))
				{
					if (pixels[(y + dy) * texture->getWidth() + (x + dx)] == NO_PIXEL_COLOUR) continue;
					else {
						pixels[(y + dy) * texture->getWidth() + (x + dx)] = NO_PIXEL_COLOUR;
						if (!texture->isAltered()) texture->markAsAltered();
					}
				}
			}
		}
	}
	else {
		pixels[y * texture->getWidth() + x] = NO_PIXEL_COLOUR;
	}

	texture->loadFromPixels(gRenderer);
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

Texture* constructNewPixelBuffer(std::vector<int> indexes, Uint32* pixels, int arrayWidth, Texture* texture, SDL_Renderer* gRenderer) {
	Uint32* newPixelBuffer;
	Texture* newTexture;
	int width = 0;
	int height = (int)(indexes.back() / arrayWidth) - (int)(indexes.front() / arrayWidth) + 1; //Why is the height including the pixel buffer??? Surely that shouldn't come up

	printf("tHeight: %i, bHeight: %i\n", indexes.back(), indexes.front());

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

	printf("Height: %i\n", height);
	printf("Width: %i\n", width);
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
	float originX = ceilf(texture->getOrigin().x + (startLinePos))-1.0f; //Assume original textures also have transparent border
	float originY = ceilf(texture->getOrigin().y + ((int)floor(indexes[0] / arrayWidth)))-1.0f;

	printf("ORIGINAL OriginX: %f ORIGINAL OriginY: %f\n", originX, originY);

	//Have to manually calculate the center from the origin here.
	float centreX = originX + floorf((width)/2.0f); //floor instead of ceil because 0-indexed
	float centreY = originY + floorf((height) / 2.0f);

	printf("ORIGINAL CentreX: %f ORIGINAL CentreY: %f\n", centreX, centreY);

	//Set this as a pointer as otherwise this variable will be destroyed once this method finishes.
	newTexture = new Texture(centreX, centreY, width, height, newPixelBuffer, gRenderer, texture->getAngle());

	cleanup(pixels, indexes);
	return newTexture;

	//delete[] newPixelBuffer; //This needs to be commented out since we are actively using newPixelBuffer to create our texture.
}

std::vector<Texture*> splitTextureAtEdge(Texture* texture, SDL_Renderer* gRenderer) {
	if (!texture || !texture->isAltered()) return {};

	//Get the texture pixels
	Uint32* pixels = texture->getPixels32(); //This has the correct alpha values for the pixels (checked)
	//This is the transparent pixel colour
	//Uint32 noPixelColour = texture->mapRGBA(0xFF, 0xFF, 0xFF, 0x00);
	//A placement int that gets the length of the pixel 1D array
	int arrayLength = texture->getWidth() * texture->getHeight();
	//A bitmap that remembers if we visited a pixel before or not.
	int* visitedTracker = new int[arrayLength];
	//Initialising visitedTracker to all 0.
	memset(visitedTracker, 0, arrayLength * sizeof(int));
	//Pixel buffer vector
	std::vector<int> possibleStarts;
	//Vector for all the new textures that are being formed. This method will return them
	std::vector<Texture*> newTextures;

	//For loop to get all the split texture parts.
	for (int i = 0; i < arrayLength; i++) {
		if (pixels[i] != NO_PIXEL_COLOUR) {
			possibleStarts = bfs(i, texture->getWidth(), arrayLength, pixels, visitedTracker);
			//printf("bfs size: %i\n", possibleStarts.size());
			if (!possibleStarts.empty()) {
				newTextures.push_back(constructNewPixelBuffer(possibleStarts, pixels, texture->getWidth(), texture, gRenderer));
			}
		}
	}

	delete[] visitedTracker;
	return newTextures;
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
	std::vector<int> marchingSquares(Texture* texture) {
		Uint32* pixels = texture->getPixels32();
		int width = texture->getWidth();
		int length = texture->getHeight() * width;
		int totalPixels = width * length;

		std::vector<int> contourPoints;
		int startPoint = getStartingPixel(pixels, totalPixels);
		//printf("MS Starting Point: %d\n", startPoint);
		if (startPoint == -1) return contourPoints;
		//If the texture is filled on the LHS, we will end up with 15 as our first currentSquare. 
		//To avoid this, we simply offset startPoint one to the left, to get 12 as our currentSquare, 
		//and then marching squares handles the rest.
		if (getCurrentSquare(startPoint, width, length, pixels) == 15) {
			startPoint -= 1;
		}

		//printf("MS Starting Point: %d\n", startPoint);

		int stepX = 0, stepY = 0;
		int prevX = 0, prevY = 0;
		int currentPoint = startPoint;
		bool closedLoop = false;

		while (!closedLoop) {
			int currentSquare = getCurrentSquare(currentPoint, width, length, pixels);
			//printf("Current Square: %d\n", currentSquare);

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
			//printf("Current Point: %i\n", currentPoint);

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
			//printf("Next Index: %i\n", nextIndex);
			if (startIndex != nextIndex) {
				rdp(startIndex, nextIndex, epsilon, arrayWidth, allPoints, rdpPoints);
			}
			rdpPoints.push_back(allPoints[nextIndex]); 
			if (endIndex != nextIndex) {
				rdp(nextIndex, endIndex, epsilon, arrayWidth, allPoints, rdpPoints);
			}
		}
	}
#pragma endregion

#pragma region ColliderGeneration
	b2Vec2* getVec2Array(std::vector<int> rdpPoints, int arrayWidth) {
		b2Vec2* points = new b2Vec2[rdpPoints.size()];
		for (int i = 0; i < rdpPoints.size(); i++) {
			int* temp = convertIndexToCoords(rdpPoints[i], arrayWidth);
			points[i] = { (temp[0]) * pixelsToMetres, (temp[1]) * pixelsToMetres };
		}
		return points;
	}

	b2Vec2* convertToVec2(TPPLPoint* polyPoints, int numPoints) {
		b2Vec2* points = new b2Vec2[numPoints];
		for (int i = 0; i < numPoints; i++) {
			points[i].x = polyPoints[i].x;
			points[i].y = polyPoints[i].y;
		}
		return points;
	}

	void rotateTranslate(b2Vec2& vector, float angle) {
		b2Vec2 tmp;
		tmp.x = vector.x * cos(angle) - vector.y * sin(angle);
		tmp.y = vector.x * sin(angle) + vector.y * cos(angle);
		vector = tmp;
	}

	//Finds the center of a shape assuming that it has been partitioned into triangles
	TPPLPoint ComputeCompoundCentroid(TPPLPolyList &shapes) {
		TPPLPoint weightedCentroid = {0.0f, 0.0f};
		float totalArea = 0.0f;

		for (TPPLPolyList::iterator it = shapes.begin(); it != shapes.end(); ++it) {
			//Getting the vertices
			TPPLPoint A = it->GetPoint(0);
			TPPLPoint B = it->GetPoint(1);
			TPPLPoint C = it->GetPoint(2);

			//Getting the centroid
			TPPLPoint centroid = (A + B + C) / 3.0f;

			//Getting the area
			float area = 0.5f * fabs(A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));

			//Accumulate weighted centroid sum:
			weightedCentroid.x += centroid.x * area;
			weightedCentroid.y += centroid.y * area;
			totalArea += area;
		}

		if (totalArea != 0) {
			weightedCentroid.x /= totalArea;
			weightedCentroid.y /= totalArea;
		}

		return weightedCentroid;
	}

	void CenterCompundShape(TPPLPolyList &shapes, TPPLPoint centre) {
		//printf("Rotating shape\n");
		TPPLPoint compoundCentroid = ComputeCompoundCentroid(shapes);
		//printf("Centroid: (%f, %f)\n", compoundCentroid.x, compoundCentroid.y);

		for (TPPLPolyList::iterator it = shapes.begin(); it != shapes.end(); ++it) {
			for (int i = 0; i < it->GetNumPoints(); i++) {
				it->GetPoint(i).x -= centre.x;
				it->GetPoint(i).y -= centre.y;
			}
		}
	}

	//Creates a texture polygon by using pure triangulation, and then moves the origin so that it is in the centre of the 
	//shape rather than at the top-left corner.
	b2BodyId createTexturePolygon(std::vector<int> rdpPoints, int arrayWidth, b2WorldId worldId, Texture* texture) {
		//Getting points
		b2Vec2* points = getVec2Array(rdpPoints, arrayWidth);
		//printf("CentreX: %f, CentreY, %f\n", texture->getCentre().x, texture->getCentre().y);
		//printf("OriginX: %f, OriginY: %f\n", texture->getOrigin().x, texture->getOrigin().y);
		//Creating the b2Body
		b2BodyDef testbodyDef = b2DefaultBodyDef();
		testbodyDef.type = b2_dynamicBody;
		testbodyDef.position = { texture->getCentre().x * pixelsToMetres, texture->getCentre().y * pixelsToMetres };
		//testbodyDef.position = { static_cast<float>(centre.x) * pixelsToMetres, static_cast<float>(centre.y) * pixelsToMetres };
		//printf("ColliderPositionX: %f, ColliderPositionY: %f\n", testbodyDef.position.x * metresToPixels, testbodyDef.position.y * metresToPixels);
		testbodyDef.rotation = { (float)cos(texture->getAngle() * DEGREES_TO_RADIANS), (float)sin(texture->getAngle() * DEGREES_TO_RADIANS) };
		b2BodyId testId = b2CreateBody(worldId, &testbodyDef);

		//I am going to partition the polygon regardless of whether or not the number of vertices is less than 8, because
		//Box2D does some very aggressive oversimplification of the shape outline which I'm not a fan of.
		//It is better to just put in triangles so it can't mess things up. I am going to use triangulation instead of 
		//partitioning to make sure Box2D keeps all of the details, as in higher-vertex convex shapes there is a change
		//simplification could occur, which I want to avoid.
		 
		
		//Creating the polypartition polygon and copying all of the points over.
		TPPLPoly* poly = new TPPLPoly();
		poly->Init(rdpPoints.size());
		TPPLPolyList polyList;

		for (int i = 0; i < rdpPoints.size(); i++) {
			(*poly)[i].x = points[i].x;
			(*poly)[i].y = points[i].y;
		}

		//The problem, which can be checked by simply looking at the points outputted here, is that (0,0) comes up twice in the
		//list of points, so I think this confuses the partitioning algorithm. This may be a problem in how I am doing my rdp.
		//printf("Printing Poly points\n");
		for (int i = 0; i < rdpPoints.size(); i++) {
			//printf("X: %f, Y: %f\n", (*poly)[i].x, (*poly)[i].y);
		}

		//Need to set it to be oriented Counter-Clockwise otherwise the triangulation algorithm fails.
		poly->SetOrientation(TPPL_ORIENTATION_CCW); //This method does not actually check the order of each vertex. Need to change it so it sorts the points properly.
		TPPLPartition test = TPPLPartition();
		//int result = test.ConvexPartition_HM(poly, &polyList);	
		//int result = test.ConvexPartition_OPT(poly, &polyList);
		int result = test.Triangulate_OPT(poly, &polyList);
		//printf("Result: %i, Size: %i, ", result, polyList.size

		//Trying to center the polygon:
		CenterCompundShape(polyList, {static_cast<double>(texture->getWidth()/2)*pixelsToMetres, static_cast<double>(texture->getHeight() / 2)*pixelsToMetres});

		//Adding the polygons to the collider, or printing an error message if something goes wrong.
		for (TPPLPolyList::iterator it = polyList.begin(); it != polyList.end(); ++it) {
			//printf("Shape Coords\n");
			for (int i = 0; i < it->GetNumPoints(); i++) {
				//printf("%f, %f\n",it->GetPoint(i).x, it->GetPoint(i).y);
			}
			b2Hull hull = b2ComputeHull(convertToVec2(it->GetPoints(), it->GetNumPoints()), it->GetNumPoints());
			if (hull.count == 0) {
				//printf("Something odd has occured when generating a hull from a polyList\n");
			}
			else {
				b2Polygon testagon = b2MakePolygon(&hull, 0.0f);
				b2ShapeDef testshapeDef = b2DefaultShapeDef();
				testshapeDef.friction = 0.3f;
				b2CreatePolygonShape(testId, &testshapeDef, &testagon);
			}
		}
		//printf("Number of shapes on the body: %i\n", b2Body_GetShapeCount(testId));
		return testId;
	}
#pragma endregion