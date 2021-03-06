#include <stdio.h>
#include <stdlib.h>
#include "labyrinthAPI.h"
#include <unistd.h>
#include <time.h>
#include <math.h> // pour la distance de Manhattan


extern int debug;/* hack to enable debug messages */

///////////////// STRUCTURE POUR GARDER EN MEMOIRE LES INFORMATIONS IMPORTANTES  //////////////////////
/*
 Prémices: Dans notre programme nous avons décidé de passer par les structures et l'allocation dynamique. Nous avons crée une structure dans un premier temps. Cette structure a été faite  afin de garder une trace des grandes variables du jeu c'est-à-dire le labyrinthe, la taille du labyrinthe, la position du joueur, la position du trésor et la position de l'adversaire.

*/

typedef struct labyrinthe
{
  char * lab;
  int sizeX;
  int sizeY;
  int position_x;
  int position_y;
  int position_tresor_x;
  int position_tresor_y;
  int position_adversaire_x;
  int position_adversaire_y;

}t_labyrinthe;

//////////////////////////// STRUCTURE POUR L'OPENLIST ET CLOSELIST ////////////////////////////

typedef struct cases
{
  int x,y;
  int xp,yp; //coordonnées des cases précédentes
  int cost;
  int heuristique;
  struct cases* next; // va permettre de passer au suivant
}Case;



///////////////////  ETAPE UNE: CREATION D'UNE FILE VIDE /////////////////////////

Case *List_empty()
{

  return NULL;

}

Case* annule()// fonction qui va donner des valeurs fictives à l'openlist sinon pb de NULL
{
  Case* c= (Case*)malloc(sizeof(Case)); // pr avoir de la mémoire -> place mettre infos
  c->x=-8;
  c->y=-10;
  c->cost=-100;
  c->heuristique=-4;
  c->next = NULL;
  return c;

}

////////////////////  ETAPE DEUX: AJOUTER UN ELEMENT A UNE FILE VIDE ////////


/////////////////////////  FONCTION POUR CALCULER LE CHEMIN VIA DISTANCE DE MANHATTAN ////////

int manhattan(int x1, int x2, int y1, int y2) // calcule la distance de Manhattan
{
  return abs(x1-x2) + abs(y1-y2);
}


void nouvelle_case (Case* c, int x, int y, int xp, int yp, int cost, int heuristique)
{
  c->x=x;
  c->y=y;
  c->xp=xp;
  c->yp=yp;
  c->cost=cost;
  c->heuristique=heuristique;
  c->next = NULL;


}

////////////////////////// TROISIEME ETAPE: RECUPERATION DU PLUS PETIT  /////////////////////////////////
/*Prémices: Cette fonction va nous permettre de comparer deux noeuds voisins. Un critère de séléction: l'heuristique. Plus l'heuristique est faible et plus le noeuds est "bon". Ce noeud sera ajouté à la closelist par la suite.

 */

Case* min (Case* Liste)
{
  if(Liste!=NULL)
    {
      Case* min=annule(); // on copie pour ne pas perdre de données
      Case* tmp=Liste->next; // on commence à l'élément suivant car il ne sert à rien de comparer le premier avec lui même
      min->x=Liste->x;
      min->y=Liste->y;
      min->xp=Liste->xp;
      min->yp=Liste->yp;
      min->cost=Liste->cost;
      min->heuristique=Liste->heuristique;
      min->next = NULL;
        
        
      while(tmp!=NULL)
        {
	  if(tmp->heuristique < min->heuristique) //heuristique ou cost à voir
            {
	      min->x=tmp->x;
	      min->y=tmp->y;
	      min->xp=tmp->xp;
	      min->yp=tmp->yp;
	      min->cost=tmp->cost;
	      min->heuristique=tmp->heuristique;
	      min->next = NULL;// le minimum devient l'élément en entrée
            }
            
	  tmp=tmp->next; // on passe à l'élément suivant
        }
      //min->next=NULL;
      return min; // retourne le plus petit élément
    }
  else
    return NULL;
    
}

////////////////// QUATRIEME ETAPE : AJOUT ET EXTRACTION D'ELEMENT (EX DE L'OPENLIST POUR LES AJOUTER DANS LA CLOSELIST) ///////

int list_find(Case* L,int x,int y,int xp, int yp)//cette fonction envoie 1 si le point (x,y) est déjà dans la liste L 0 sinon
{
  Case* Liste=L;
  while(Liste!=NULL)
    {
      if(Liste->x==x&&Liste->y==y && Liste->xp==xp && Liste->yp==yp)
	return 1;
      Liste=Liste->next;
    }
  return 0;
}

int list_find2(Case* L,int x,int y)//cette fonction envoie 1 si le point (x,y) est déjà dans la liste L 0 sinon
{
  Case* Liste=L;
  while(Liste!=NULL)
    {
      if(Liste->x==x&&Liste->y==y)
	return 1;
      Liste=Liste->next;
    }
  return 0;
}

int list_find_next(Case* L,int xp,int yp)//cette fonction envoie 1 si le point (x,y) est déjà dans la liste L 0 sinon
{
  Case* Liste=L;
  while(Liste!=NULL)
    {
      if(Liste->xp==xp&&Liste->yp==yp)
	return 1;
      Liste=Liste->next;
    }
  return 0;
}



Case* list_add(Case* L, Case* nouveau,int* elem)
{
    
  if(list_find(L,nouveau->x,nouveau->y, nouveau->xp, nouveau->yp)==0)
    {
      Case* Liste=(Case*)malloc(sizeof(Case));
      Liste=nouveau;
        
      if(L==NULL)
        {
	  L=nouveau;
	  return L;
        }else
        {
	  Liste->next=L;
	  L=Liste;
        }
      //printf("\nElem:%d\n",(*elem));
      (*elem)++;
    }
  return L;
    
}



Case* extraire (Case* element,int* x, int* y, int* cost, int* heuristique)
{


  if(element==NULL)
    return NULL;

  if(element->x==*x && element->y==*y)
    {
      Case* tmp;
      element=element->next;
      tmp=element;
      tmp->next=NULL; //si next nul on sait que c'est une case au lieu d'une liste
      return tmp;

    }

  element->next=extraire(element,x,y,cost,heuristique);
  return element;

}


void afficher(Case* element,char* tab) // permet d'afficher
{
  Case* copie=element;
    
  while(copie!=NULL)
    {
      printf("Case:%d x:%d y:%d xp:%d yp:%d cost:%d heuristique:%d\n",tab[copie->y*15+copie->x],copie->x,copie->y,copie->xp, copie->yp,copie->cost,copie->heuristique);
      copie=copie->next;
        
    }
    
}

Case* delete (Case* element,Case* element_supr)
{
    
  if(element==NULL)
    return NULL;
    
  if(element->x==element_supr->x&& element->y==element_supr->y)
    {
      Case* tmp;
      tmp=element->next;
      //free(element);
      //printf("\nDete\n");
      return tmp;
        
    }
    
  element->next=delete(element->next,element_supr);
  return element;
    
    
}



////////////// UTILISATION DE LA STRUCTURE POUR GARGER EN MEMOIRE LES INFORMATIONS SUR LE LABYRINTHE //////
/*
 Prémices: La première étape est de "former" le labyrinthe. La labyrinthe en lui même a déjà été crée. 3 fonctions fournies permettent de récupérer les données du labyrinthe. WaitForLabyrinth permet de récupérer le nom et la taille (ainsi que d'attenre une partie), getLabyrinth permet de récupérer les données initiales c'est-à-dire de remplir la structure avec des 0 si les cases sont vides et 1 si elles correspondent à des murs. Enfin la fonction printLabyrinth permet d'afficher le labyrinthe.
 Notre but ici va être de créer de la place en mémoire grâce à l'allocation dynamique pour un labyrinthe que l'on a appelé labData ainsi qu'utiliser la structure définie au préalable pour définir les positions initiales des joueurs et du trésor dans le labyrinthe. En réalité ceci était déjà fait mais utiliser cette structure nous donne la possibilité d'avoir les modifications à jours.

*/

int mylabyrinthe(t_labyrinthe* current)
{

  int sizeX,sizeY;
  char labName[50];
  waitForLabyrinth("DO_NOTHING timeout=100 rotation=False", labName, &sizeX, &sizeY);
  char* labData;
  labData = (char*) malloc( sizeX * sizeY );
  int player= getLabyrinth(labData);

  current->lab=labData;
  current->sizeX=sizeX;
  current->sizeY=sizeY;
  current->position_x=0;
  current->position_y=sizeY/2;
  current->position_tresor_x=sizeX/2;
  current->position_tresor_y=sizeY/2;
  current->position_adversaire_x=sizeX-1;
  current->position_adversaire_y=sizeY/2;
  return player;
}

Case* get_last(Case* Liste) // permet de donner le dernier élément de la closelist 
{
  if(Liste==NULL)
    return NULL;
    
  Case* last=Liste; // on copie
  Case* next=Liste->next; // on recupère l'élément suivant
    
  while(next!=NULL)
    {
      last=next;
      next=next->next;
        
        
    }
  return last;
    
}


int taille(Case* closelist)
{
  Case* tmp=closelist; 

  int x=0; 


  if(tmp==NULL)
    return 0; 

  tmp=closelist->next; 
  x++;
  while(tmp!=NULL)
    {x+=1; 
      tmp=tmp->next; 
    }

  tmp=NULL; 
  free(tmp); 
  return x; 

}

//fonction qui transforme une case en type move 

int transfo(Case* place, Case* unecase, int sizeX, int sizeY)
{

  int position_x=place->xp; 
  int position_y=place->yp; 
  int position_xn=unecase->xp; 
  int position_yn=unecase->yp; 

  if((position_x+1)%sizeX==position_xn && position_y==position_yn )
    return MOVE_LEFT; 

  if((position_x-1+sizeX)%sizeX==position_xn && position_y==position_yn)
    return MOVE_RIGHT; 

  if((position_y+1)%sizeY==position_yn && position_x==position_xn)
    return MOVE_UP; 

  if((position_y-1+sizeY)%sizeY==position_yn && position_x==position_xn)
    return MOVE_DOWN; 

  return -1; 

}


int* build_chemin(Case* closelist, int sizeX, int sizeY, int* sizeway)
{
  int i=0; //on crée un tableau avec tous les mouvements 
  int size=taille(closelist);
  Case* end=annule();
  Case* save=annule();
  nouvelle_case (end,closelist->x,closelist->y,closelist->x,closelist->y,closelist->cost,closelist->heuristique);

  if(closelist==NULL)
    {

      return NULL;
    } 

  Case* tmp=closelist;
  
  int *tab=calloc(size,sizeof(int)); 
  int move; 
     
  move=transfo(end,tmp,sizeX, sizeY);
  tab[i]=move; 
  i++;
  save=tmp;
  while(tmp->next!=NULL)
    {

      move=transfo(save,tmp->next,sizeX, sizeY);
      tab[i]=move; 
      i++; 
      tmp=tmp->next;
      if(move!=-1) save= tmp; 


    } 
  *sizeway=i; 

  for(i=0;i<size;i++)
    {
      if (tab[i]==MOVE_UP)
	printf("UP \n"); 

      if (tab[i]==MOVE_DOWN)
	printf("DOWN \n"); 

      if (tab[i]==MOVE_RIGHT)
	printf("RIGHT \n"); 

      if (tab[i]==MOVE_LEFT)
	printf("LEFT \n"); 

    }

  return tab; 
}


////////////////////////// FONCTION QUI PERMET DE PARCOURIR LE LABYRINTHE AFIN DE CREER L'OPENLIST ET CLOSELIST /////////
/*
 Cette fonction va avoir pour but de remplir les cases du labyrinthe avec +1 à chaque fois qu'on a une case voisine qui est vide
 Cela veut dire que le cost va valoir +1 à chaque fois qu'on avance et qu'il ne s'agit pas d'un mur. Ceci nous permettra plus tard de choisir le chemin avec le cost le plus faible.
*/
/*principe: On ajoute les éléments dans l'openlist si les cases sont vides. Concernant un groupement de cases voisines, si le cost d'une case est le plus faible alors nous ajouterons cette case dans la closelist



 */
//première étape: donner un cost aux cases et les explorer



int*  meilleur_chemin(t_labyrinthe* current, int* sizeway)
{
  Case* openlist= annule();  // on crée l'openlist vide qui va contenir les cases vides
  Case* closelist=List_empty(); // on crée la closelist qui va contenir le chemin menant au trésor
  int us_x=current->position_x;
  int us_y=current->position_y;
  int tresor_x=current->position_tresor_x;
  int tresor_y=current->position_tresor_y;
  int cost=0;
  int voisin=0,elem_closedlist=0;
  int heuristique;
  Case* copie=annule();
  Case* small;
 
  heuristique=manhattan(us_x,tresor_x,us_y,tresor_y); // Retourne la distance entre les deux qui correpond à l'heuristique
  nouvelle_case(openlist,us_x,us_y,-1,-1,0,heuristique); // on rajoute dans l'openlist les coordonnées du point de départ
  // list_add(openlist,copie);
  // création de l'openlist
    
  while(openlist!=NULL) // tant que l'openlist n'est pas vide si la case est vide et qu'on ne l'a pas encore exploré on l'ajoute à l'openlist
    {

      //1ère étape 
      small=min(openlist);

      closelist=list_add(closelist,small,&elem_closedlist); // va ajouter à closelist le minimum de openlist
      // quand on ajoute un element à la closelist on la supprime de l'openlist
      //afficher(openlist); 
      /*printf("Openlist before del\n");
afficher(openlist);
      */
      openlist=delete(openlist,small);
      
      /*count=count+1;
	printf("ok count=%d\n",count);
	if(count>15)
	  break;
      */
      //look at all reachable neighbours and add them to the openlist
      // Case* element_extrait=extraire(openlist,&us_x,&us_y,&cost,&heuristique); // on extrait le premier élément de la file puis les suivants


      //2nd étape 
      
      if(small->x==tresor_x && small->y==tresor_y)
	{
	  printf("startCloselit:\n");
	  int* chemin=build_chemin(closelist,current->sizeX,current->sizeY,sizeway); 
	  printf("Closelit:\n");
	  afficher(closelist,current->lab);
	  if(closelist==NULL)
	    printf("\nCloselist est vide");
	  printf("Chemin:\n");
	  return chemin; 

	}



      //3eme étape 
      us_x=small->x;
      us_y=small->y;

      if(list_find_next(closelist,us_x,us_y)==0 &&list_find2(closelist,us_x,(us_y-1+current->sizeX)%current->sizeX)==0 && current->lab[((us_y-1+current->sizeX)%current->sizeX)*current->sizeX + us_x]==0) // si la case du haut est vide
        {
	  int h = manhattan(us_x,tresor_x, (us_y-1+current->sizeX)%current->sizeX, tresor_y);
	  nouvelle_case(copie,us_x,(us_y-1+current->sizeX)%current->sizeX,us_x,us_y,++cost,h); // on rajoute dans l'openlist les coordonnées du point de départ

	  openlist=list_add(openlist,copie,&voisin);
	  cost--; 

        }
     
      copie=annule();

      if(list_find_next(closelist,us_x,us_y)==0 && list_find2(closelist,us_x,(us_y+1)%(current->sizeX) *current->sizeX )==0 && current->lab[(us_y+1)%(current->sizeX) *current->sizeX + us_x]==0) // si la case du bas
        {

	  int h = manhattan(us_x, tresor_x,(us_y+1)%(current->sizeX),  tresor_y);
	  nouvelle_case(copie,us_x,(us_y+1)%(current->sizeX),us_x,us_y,++cost,h); // on rajoute dans l'openlist les coordonnées du point de départ
          openlist=list_add(openlist,copie,&voisin);
	  cost--; 

        }
      //printf("Openlist add 2\n");
      //afficher(openlist);
      copie=annule();
      if(list_find_next(closelist,us_x,us_y)==0 &&list_find2(closelist,(us_x-1+current->sizeY)%current->sizeY,us_y)==0 && current->lab[(us_y)*current->sizeX + (us_x-1+current->sizeY)%current->sizeY]==0) // si la case de gauche
        {

	  int h = manhattan((us_x-1+current->sizeY)%current->sizeY,tresor_x, us_y,tresor_y);
	  nouvelle_case(copie,(us_x-1+current->sizeY)%current->sizeY,us_y,us_x,us_y,++cost,h); // on rajoute dans l'openlist les coordonnées du point de départ

	  openlist=list_add(openlist,copie,&voisin);
	  cost--;
             
        }
      //printf("Openlist add 3\n");
      //afficher(openlist);
      copie=annule();

      if(list_find_next(closelist,us_x,us_y)==0 &&list_find2(closelist,(us_x+1)%(current->sizeY),us_y)==0 && current->lab[(us_y) *current->sizeX + (us_x+1)%(current->sizeY)]==0) // droite
        {

	  int h = manhattan((us_x+1)%(current->sizeY),tresor_x, us_y,tresor_y);
	  nouvelle_case(copie,(us_x+1)%(current->sizeY),us_y,us_x,us_y,++cost,h); // on rajoute dans l'openlist les coordonnées du point de départ

	  openlist=list_add(openlist,copie,&voisin);
	  cost--;

        }
      //printf("\nCost before:%d\n",cost);
      if(voisin!=0) cost++;
      //printf("\nCost after:%d\n",cost);
      voisin=0;
      //printf("Openlist add 4\n");
      //afficher(openlist);
      copie=annule();
      //choose the best step to do by looking for the minimum of the F=cost + heuristics in the openlist (which does not include our own position)
        

    }
   

  return NULL;
}


// seconde étape: refaire le chemin à l'envers en utilisant le chemin avec le cost le plus faible pour aller au trésor


///////////////////// FONCTION QUI PERMET A NOTRE JOUEUR DE SE DEPLACER ///////////////////
/*
 Prémices: Une fois le labyrinthe défini, il nous est temps de passer au mouvement du joueur. C'est là où la structure crée va être particulièrement importante. Dans un premier temps, il nous faut créer des mouvements aléatoires. On décide de générer un nombre aléatoire entre 4 et 7. Ces nombres ont été choisis en concordance avec ce qui était défini par défaut. Une fois le nombre généré, si il s'agit de 4 et qu'il n'y a pas de mur le joueur se déplace vers le haut, de même si il s'agit d'un 5 et qu'il n'y a pas de mur il se déplace vers le bas, si c'est un 6 vers la gauche et si c'est un 7 vers la droite. On ajoute également une option due à la rotatibilité du labyrinthe qui est que si le joueur se retrouve à l'extrémité il lui est possible de passer de l'autre côté.

*/


/*
void make_move(t_labyrinthe* current, t_move *move, int* validite)
{

  int test=0;
  int deplacement=0; // permet de tester les conditions de positivités pour l'entrée dans la boucle
   

  while(test!=1)
    {

     //while(current->position_x!=current->position_tresor_x && current->position_y!=current->position_tresor_y) // tant qu'on est pas arrivés au trésor
     //   {

       if(current->position_y-1<0) // si on veut se déplacer vers des coordonnées négatives (ce qui n'est pas possible)
           deplacement=current->sizeY-1; // on se déplace d'une case quand même mais ceci entraine le passage de l'autre côté du labyrinthe

	   else
	       deplacement=current->position_y-1; // si le déplacement ce fait vers des coordonnées positives on peut y aller

	         // on essaie aller en haut, donc on verifie si la case [position_x -1, position_y] est valide
		   if(validite->y>current->position_y && current->lab[deplacement*current->sizeX + current->position_x]==0) // si la case est dans l'openlist et qu'il ne s'agit donc pas d'un mur
            {
	          if(current->lab[deplacement*current->sizeX + current->position_x]) // si c'est la plus petite valeur, on s'y déplace
                {
		  current->position_y=deplacement;
		    move->type = MOVE_UP;
		      test=1;
                }
            }

	      if(current->position_x-1<0)
	          deplacement=current->sizeX-1;
		  else
		      deplacement=current->position_x-1;


		        if(validite->x<current->position_x && current->lab[(current->position_y) *current->sizeX + deplacement]==0)
            {
	          current->position_x=deplacement;
		        move->type=MOVE_LEFT;
			      test=1;
            }



	      if(validite->x>current->position_x && current->lab[(current->position_y) *current->sizeX + (current->position_x+1)%(current->sizeX)]==0)
            {
	          current->position_x= (current->position_x+1)%(current->sizeX);
		        move->type=MOVE_RIGHT;
			      test=1;
            }


	      if(validite->y<current->position_y && current->lab[(current->position_y+1)%(current->sizeY) *current->sizeX + current->position_x]==0)
            {
	          current->position_y=(current->position_y+1)%(current->sizeY);

		        move->type=MOVE_DOWN;
			      test=1;
            }


     // }

      // printf("vide ou pas:%d %d %d",current->lab[(current->position_y*current->sizeX+current->position_x)], current->position_x, current->position_y);

    }
}
*/
 ///////////////////////////////////////////// MAIN FUNCTION ///////////////////////////////////////////////////////

void print_laby(char* tab,int sizeX,int sizeY)
{
  int i,j;
  for(i=0;i<sizeY;i++)
    {
      printf("\n");
      for(j=0;j<sizeX;j++)
	{
	  if(tab[i*sizeX+j]==2)printf("* ");
	  else if(i==sizeY/2 && j==sizeX/2)printf("@ ");
	  else printf("%d ",tab[i*sizeX+j]);
	}
    }
}

int main()
{
  srand(time(NULL));

  /* name of the labyrinth */
  debug=1;// par defaut donne les infos les plus importantes
  int player; /* data of the labyrinth */
  t_return_code ret = MOVE_OK;/* indicates the status of the previous move */
  t_move move;/* a move */
  t_labyrinthe* Laby=(t_labyrinthe*)malloc(sizeof(t_labyrinthe));

  /* connection to the server */
  connectToServer( "pc4023.polytech.upmc.fr", 1234,"Shab");


  /* wait for a game, and retrieve informations about it */

  player=mylabyrinthe(Laby);
  int sizeway;
  int* valide=meilleur_chemin(Laby, &sizeway); //On determine le meilleur chemin
  // A partir de ce meilleur chemin on determine les mouvements qui vont nous permettre d'aller au trésor 
  int comp=sizeway-1; // on va faire une boucle pour recuperer les elements car ils sont stockes à l'envers
  print_laby(Laby->lab, Laby->sizeX,Laby->sizeY);
  printLabyrinth();
        

  do {

    /* display the labyrinth */
    printLabyrinth();


    if (player==1)/* The opponent plays */
      {
	ret = getMove( &move);
	player=0;
      }

    else
      {
	//make_move(Laby,&move, valide);
	while(valide[comp]==-1&&comp>0) comp--;
	move.type=valide[comp]; //
	move.value=0; 
	printf("move %d %d %d \n",move.type, Laby->position_x,Laby->position_y);
	ret=sendMove(move);
	comp--;
	player=1; // afin que chacun joue à son tour
      }


  }while(ret==MOVE_OK);  // tant que la partie n'est pas finie on joue


  if ((player ==1 && ret == MOVE_WIN) || (player==0 && ret == MOVE_LOSE))
    printf("I lose the game\n");

  /* we do not forget to free the allocated array */
  free(Laby);


  /* end the connection, because we are polite */
  closeConnection();


  return EXIT_SUCCESS;
}

///////////////////////////////////////////// END OF THE PROGRAM ////////////////////////////////////
