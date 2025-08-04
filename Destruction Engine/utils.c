#include "utils.h"
int drawCircle(SDL_Renderer* gRenderer, Vector2 center, float radius) {
	int x = radius, y = 0;
    
    // Printing the initial point on the axes 
    // after translation
    printf("(%d, %d) ", x + center.x, y + center.y);
	SDL_RenderPoint(gRenderer, x+center.x, y+center.y);

    
    // When radius is zero only a single
    // point will be printed
    if (radius <= 0)
    {
        printf("(%d, %d) ", x + center.x, -y + center.y);
        printf("(%d, %d) ", y + center.x, x + center.y);
        printf("(%d, %d)\n", -y + center.x, x + center.y);

		SDL_RenderPoint(gRenderer, x+center.x, -y+center.y);
		SDL_RenderPoint(gRenderer, y + center.x, x + center.y);
		SDL_RenderPoint(gRenderer, -y + center.x, x + center.y);
		return 0;
    }
    
    // Initialising the value of P
    int P = 1 - radius;
    while (x > y)
    { 
        y++;
        
        // Mid-point is inside or on the perimeter
        if (P <= 0)
            P = P + 2*y + 1;
            
        // Mid-point is outside the perimeter
        else
        {
            x--;
            P = P + 2*y - 2*x + 1;
        }
        
        // All the perimeter points have already been printed
        if (x < y)
            break;
        
        // Printing the generated point and its reflection
        // in the other octants after translation
        printf("(%d, %d) ", x + center.x, y + center.y);
        printf("(%d, %d) ", -x + center.x, y + center.y);
        printf("(%d, %d) ", x + center.x, -y + center.y);
        printf("(%d, %d)\n", -x + center.x, -y + center.y);

		SDL_RenderPoint(gRenderer, x + center.x, y + center.y);
		SDL_RenderPoint(gRenderer, -x + center.x, y + center.y);
		SDL_RenderPoint(gRenderer, x + center.x, -y + center.y);
		SDL_RenderPoint(gRenderer, -x + center.x, -y + center.y);
        
        // If the generated point is on the line x = y then 
        // the perimeter points have already been printed
        if (x != y)
        {
            printf("(%d, %d) ", y + center.x, x + center.y);
            printf("(%d, %d) ", -y + center.x, x + center.y);
            printf("(%d, %d) ", y + center.x, -x + center.y);
            printf("(%d, %d)\n", -y + center.x, -x + center.y);

			SDL_RenderPoint(gRenderer, y + center.x, x + center.y);
			SDL_RenderPoint(gRenderer, -y + center.x, x + center.y);
			SDL_RenderPoint(gRenderer, y + center.x, -x + center.y);
			SDL_RenderPoint(gRenderer, -y + center.x, -x + center.y);
        }
    }
	return 0;
}