#include <SDL.h>        
#include <SDL2/SDL.h>
#include <SDL_image.h>        
#include <SDL_ttf.h>        
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char ** argv)
{

	// PROGRAMMATION EVENEMENTIELLE
	// Vous écrivez du code pour répondre à des événements
	//	Bouger la souris c'est un événement
	//	Redimensionner la fenêtre, c'est un événement
	//	cliquer sur le bouton quit (en haut de la fenêtre), c'est un événement

	// Il faut aller vite, 50 images / seconde
	//	while (1)
	//		1) regarder si il y a des événements						RECUPERER LES EVENTS
	//		2) si il y a des événements, il faut modifier l'état de l'application		MISE A JOUR DE l'ETAT
	//		3) redessiner la fenêtre graphique						REDRAW SUIVANT L'ETAT
	//	1,2,3, on revient en 1,2,3, on revient en 1,2,3 50 fois par seconde

	// double-buffer et renderer : un buffer qu'on affiche et un autre dans lequel on met à jour (REDRAW), on inverse les deux buffers
	// simple buffer graphique scintillement 
	// 
	int i,j;

    int quit = 0;
    SDL_Event event;
	int mx,my;

	int posMarteauX;
	int posMarteauY;
	int barrer;

    SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
 
    SDL_Window * window = SDL_CreateWindow("SDL2 EX1",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, 0);
 
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Surface *deck[2],*marteau;

	deck[0] = IMG_Load("SH13_0.png");
	deck[1] = IMG_Load("SH13_1.png");
	marteau = IMG_Load("marteau.png");

    SDL_SetColorKey(marteau, SDL_TRUE, SDL_MapRGB( marteau->format, 0, 0, 0 ) );

    SDL_Texture *texture_deck[2],*texture_marteau;

	for (i=0;i<2;i++)
		texture_deck[i] = SDL_CreateTextureFromSurface(renderer, deck[i]);

    texture_marteau = SDL_CreateTextureFromSurface(renderer, marteau);
    //SDL_SetTextureAlphaMod(texture_marteau,128);

    while (!quit)
    {
	/* Etape 1: RECUPERER LES EVENEMENTS */
	if (SDL_PollEvent(&event))
	{
		//printf("un event\n");
        	switch (event.type)
        	{
            		case SDL_QUIT:
                		quit = 1;
                		break;
			case  SDL_MOUSEBUTTONDOWN:
				SDL_GetMouseState( &mx, &my );
				//printf("mx=%d my=%d\n",mx,my);
				
				break;
			case  SDL_MOUSEMOTION:
				SDL_GetMouseState( &mx, &my );

				break;
        	}
	}

	/* Etape 2: METTRE A JOUR l'ETAT DE L'APPLICATION GRAPHIQUE */

	posMarteauX=mx;
	posMarteauY=my;
	if ((mx>=750) && (mx<=(750+250)) && (my>=0) && (my<=(0+165)))
	{
		barrer=1;
	}
	else
	{
		barrer=0;
	}
	//printf("barrer=%d\n",barrer);

	/* Etape 3: REDESSINER */

	SDL_SetRenderDrawColor(renderer, 255, 230, 230, 230);
	SDL_Rect rect = {0, 0, 1024, 768}; 
	SDL_RenderFillRect(renderer, &rect);

        	SDL_Rect dstrect0 = { 750, 0, 250, 165 };
        	SDL_RenderCopy(renderer, texture_deck[0], NULL, &dstrect0);

        	SDL_Rect dstrect1 = { 750, 200, 250, 165 };
        	SDL_RenderCopy(renderer, texture_deck[1], NULL, &dstrect1);

		if (barrer==1)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        		SDL_RenderDrawLine(renderer, 750,0,750+250,0+165);
		}

        	SDL_Rect dstrect_marteau = { posMarteauX, posMarteauY, 100, 82 };
        	SDL_RenderCopy(renderer, texture_marteau, NULL, &dstrect_marteau);

        SDL_RenderPresent(renderer);
    }
 
    SDL_DestroyTexture(texture_deck[0]);
    SDL_DestroyTexture(texture_deck[1]);
    SDL_FreeSurface(deck[0]);
    SDL_FreeSurface(deck[1]);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
 
    SDL_Quit();
 
    return 0;
}
