#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <mcimage.h>
#include <mccodimage.h>

//Bibliotheque perso
#include <libcrop.h>


uint32_t read_size_on_disk(FILE *fd, uint64_t *rs, uint64_t *cs, uint64_t *d)
{
	char buffer[BUFFERSIZE];
	uint32_t type_image,c;

	/* Ouverture du fichier */
	rewind(fd);

	/* Lecture des 2 premiers octets P7 ou P5*/
	fgets(buffer, 3, fd); /* P7: raw byte 3d  ; P4: ascii 3d  ==  extensions MC */

	if (buffer[0] != 'P')
    {
		fprintf(stderr,"read_size_on_disk: invalid image format\n");
		return(0);
    }

	if (buffer[1]=='5')type_image = 5 ;
	else if (buffer[1]=='7')type_image = 7;
	else if(buffer[1]=='4')type_image = 4;
	else if(buffer[1]=='8')type_image = 8;
	else fprintf(stderr,"read_size_on_disk: invalid image format (2)\n");


	/*On passe tous les commentaires (en espérant qu'il n'y ait pas de chiffres dans les commentaires
	Cette partie doit être améliorée*/
	fgets(buffer, 2, fd);
	fgets(buffer, 2, fd);
	while(buffer[0] == '#')
	{
		while(buffer[0] != '\n')
			fgets(buffer, 2, fd);
		fgets(buffer, 2, fd);
    }
	fgets(&buffer[1], 100, fd);
	/* Sauvegarde des longueurs (rs,cs,depth) de l'image */
	c=sscanf(buffer, "%lld %lld %lld", rs, cs, d);
	if (c == 2) *d = 1;
	else if (c != 3)
    {
    	fprintf(stderr,"read_size_on_disk: invalid image format (3)\n");
    	return(0);
    }

	return(1);
}



//Original author : Damien Frébet
//Adapted by J.Chaussard in 2008
//Handles P4, P5, P7, P8 images
uint32_t crop_on_disk(FILE *fd, struct xvimage** image_decoupee, uint64_t cx, uint64_t cy, uint64_t cz, uint64_t width, uint64_t height, uint64_t dpth)
{
	char *buffer;
	//uint32_t bufferlong[BUFFERSIZE];
	uint64_t c, largeur_max, hauteur_max, profondeur_max, type_image;
	uint64_t rs, cs, d;
	uint64_t supx, supy, supz;
	uint64_t jump_ligne, jump_plan, premierpoint;
	uint64_t hauteur,largeur,profondeur, cpt_tableau, ndgmax, N;

	cpt_tableau=0;
	hauteur=0; largeur=0; profondeur=0;

	buffer=(char*)calloc(BUFFERSIZE, sizeof(char));
	if(buffer==NULL)
	{
		fprintf(stderr, "crop_on_disk: Error when allocating space for buffer.\n");
		return(0);
	}

	/* Ouverture du fichier */
	rewind(fd);

	/* Lecture des 2 premiers octets P7 ou P5*/
	fgets(buffer, 3, fd); /* P7: raw byte 3d  ; P4: ascii 3d  ==  extensions MC */

	if (buffer[0] != 'P')
    {
		fprintf(stderr,"crop_on_disk: invalid image format\n");
		free(buffer);
		return(0);
    }

	if (buffer[1]=='5')type_image = 5 ;
	else if (buffer[1]=='7')type_image = 7;
	else if(buffer[1]=='4')type_image = 4;
	else if(buffer[1]=='8')type_image = 8;
	else
	{
		fprintf(stderr,"crop_on_disk: invalid image format (2)\n");
		free(buffer);
		return(0);
	}


	/*On passe tous les commentaires*/
	fgets(buffer, 2, fd);
	fgets(buffer, 2, fd);
	while(buffer[0] == '#')
	{
		while(buffer[0] != '\n')
			fgets(buffer, 2, fd);
		fgets(buffer, 2, fd);
    }
	fgets(&buffer[1], 100, fd);
	/* Sauvegarde des longueurs (rs,cs,depth) de l'image */
	c = sscanf(buffer, "%lld %lld %lld", &rs, &cs, &d);
	if (c == 2) d = 1;
	else if (c != 3)
    {
    	fprintf(stderr,"crop_on_disk : invalid image format (3)\n");
    	free(buffer);
    	return(0);
    }

	/* On lit le nombre maximum que peut prendre un voxel (255 pour une image en niveau de gris)*/
	fgets(buffer,100, fd);
	sscanf(buffer, "%lld", &ndgmax);

	/* Nombre total de voxel (ou pixel) dans l'image */
	N = rs * cs * d;

	/* Initialisation des variables hauteur,largeur et profondeur en cas de debordement de l'image source */
	largeur_max = width;
	hauteur_max = height;
	profondeur_max = dpth;

	supx = cx + width;
	supy = cy + height;
	supz = cz + dpth;

	if(supx >= rs)
    {
    	//Too much on rowsize asked... we have an overflow.... better is to make smaller
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too large zone. It was reduced.\n");
		largeur_max = rs-cx;
    }

	if(supy >= cs)
    {
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too high zone. It was reduced.\n");
		hauteur_max = cs-cy;
	}

	if(supz >= d)
    {
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too deep zone. It was reduced.\n");
		profondeur_max = d-cz;
	}


	//check the type of the image
	if( (type_image!=8 && datatype(*image_decoupee)!=VFF_TYP_1_BYTE) || (type_image==8 && datatype(*image_decoupee)!=VFF_TYP_4_BYTE))
	{
		fprintf(stderr, "crop_on_disk: Output image has wrong type.\n");
		free(buffer);
		return(0);
	}

	//and its size
	if(rowsize(*image_decoupee)<width || colsize(*image_decoupee)<height || depth(*image_decoupee)<dpth)
	{
		fprintf(stderr, "crop_on_disk: Output image is too small.\n");
		free(buffer);
		return(0);
	}


	/* Si c'est une image P5 ou P7 */
	if(type_image == 5 || type_image == 7)
	{
		if(largeur_max+1>BUFFERSIZE)
		{
			free(buffer);
			buffer=(char*)calloc(largeur_max+1, sizeof(char));
			if(buffer==NULL)
			{
				fprintf(stderr, "crop_on_disk: Error when allocating space for buffer (2).\n");
				return(0);
			}
		}

		if((largeur_max<=0)||(hauteur_max<=0)||(profondeur_max<=0))
		{
			fprintf(stderr, "crop_on_disk: Required to crop on a negative size or out of image.\n");
			free(buffer);
			return (0);
		}


		jump_ligne = (rs-largeur_max);
		jump_plan = rs*(cs-hauteur_max+1)-largeur_max;
		premierpoint = rs*cs*cz + rs*cy + cx;

		/* Remplissage du tableau imagedata de la valeur de tous les voxel du cube a extraire  */
		/* On se place au debut de l'image (pointeur pointe sur le premier voxel du cube a extraire*/
		/* Puis on parcours les lignes. Au bout d'un ligne on met le pointeur au debut de la prochaine ligne */
		/* Quand on a finis un plan on met le pointeur au debut de la premier ligne du plan suivant */

		fseek(fd,premierpoint,SEEK_CUR);


		for (profondeur=0;profondeur<profondeur_max;profondeur++)
		{
			for (hauteur=0;hauteur<hauteur_max;hauteur++)
			{
				fgets(buffer,largeur_max+1,fd);

				for (largeur=0;largeur<largeur_max;largeur++)
				{
					UCHARDATA(*image_decoupee)[cpt_tableau]=(u_int8_t)buffer[largeur];
					cpt_tableau++;
				}

				if(hauteur<hauteur_max-1)
				{
					fseek(fd,jump_ligne,SEEK_CUR);
				}

				//fin ligne
			}

			if(profondeur<profondeur_max-1)
			{
				fseek(fd,jump_plan,SEEK_CUR);
			}

			//fin plan

		}
	}

	else
	{
		fprintf(stderr,"crop_on_disk: Error, unknown format.\n");
		freeimage(*image_decoupee);
		free(buffer);
		return(0);
	}

	free(buffer);

	return(1);
}


//Original author : Damien Frébet
//Adapted by J.Chaussard in 2008
//Handles RAW 1 byte images
uint32_t crop_raw_on_disk(FILE *fd, struct xvimage* image_decoupee, uint64_t cx, uint64_t cy, uint64_t cz, uint64_t width, uint64_t height, uint64_t dpth, uint64_t rs, uint64_t cs, uint64_t d, uint64_t header_size, uint64_t size_voxel, char compteur)
{
	char *buffer;
	//uint32_t bufferlong[BUFFERSIZE];
	uint64_t largeur_max, hauteur_max, profondeur_max;
	uint64_t supx, supy, supz;
	uint64_t jump_ligne, jump_plan, premierpoint;
	uint64_t hauteur,profondeur, cpt_tableau;

	/* Initialisation des variables hauteur,largeur et profondeur en cas de debordement de l'image source */
	largeur_max = width;
	hauteur_max = height;
	profondeur_max = dpth;

	supx = cx + width;
	supy = cy + height;
	supz = cz + dpth;

	if(supx >= rs)
    {
    	//Too much on rowsize asked... we have an overflow.... better is to make smaller
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too large zone. It was reduced.\n");
		largeur_max = rs-cx;
    }

	if(supy >= cs)
    {
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too high zone. It was reduced.\n");
		hauteur_max = cs-cy;
	}

	if(supz >= d)
    {
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too deep zone. It was reduced.\n");
		profondeur_max = d-cz;
	}


	//If the exit image was allocated, please check the type of the image
	if(	(datatype(image_decoupee)==VFF_TYP_1_BYTE && size_voxel!=sizeof(uint8_t)) ||
		(datatype(image_decoupee)==VFF_TYP_2_BYTE && size_voxel!=sizeof(uint16_t)) ||
		(datatype(image_decoupee)==VFF_TYP_4_BYTE && size_voxel!=sizeof(uint32_t)) ||
		(datatype(image_decoupee)==VFF_TYP_FLOAT && size_voxel!=sizeof(float)) ||
		(datatype(image_decoupee)==VFF_TYP_DOUBLE && size_voxel!=sizeof(double)) )
	{
		fprintf(stderr, "crop_raw_on_disk: Output image and voxel size do not match.\n");
		free(buffer);
		return(0);
	}


	buffer=(char*)calloc(largeur_max+1, sizeof(char)*size_voxel);
	if(buffer==NULL)
	{
		fprintf(stderr, "crop_raw_on_disk: Error when allocating space for buffer (2).\n");
		return(0);
	}


	if((largeur_max<=0)||(hauteur_max<=0)||(profondeur_max<=0))
	{
		fprintf(stderr, "crop_raw_on_disk: Required to crop on a negative size or out of image.\n");
		free(buffer);
		return (0);
	}


	jump_ligne = (rs-largeur_max)*size_voxel;
	jump_plan = (rs*(cs-hauteur_max+1)-largeur_max)*size_voxel;
	premierpoint = (rs*cs*cz + rs*cy + cx)*size_voxel;

	if(compteur!=0)
	{
		fprintf(stdout, "Processing image...     ");
		fflush(stdout);
	}

	/* Remplissage du tableau imagedata de la valeur de tous les voxel du cube a extraire  */
	/* On se place au debut de l'image (pointeur pointe sur le premier voxel du cube a extraire*/
	/* Puis on parcours les lignes. Au bout d'un ligne on met le pointeur au debut de la prochaine ligne */
	/* Quand on a finis un plan on met le pointeur au debut de la premier ligne du plan suivant */

	fseeko(fd, (off_t)(header_size+premierpoint), SEEK_SET);

	for (profondeur=0;profondeur<profondeur_max;profondeur++)
	{
		cpt_tableau=profondeur*width*height;

		for (hauteur=0;hauteur<hauteur_max;hauteur++)
		{
			if(fread(buffer, size_voxel, largeur_max, fd)!=largeur_max)
			{
				fprintf(stderr, "crop_raw_on_disk: Could not read info in raw image: does it have good size ?\n");
				return(0);
			}

			memcpy(&(UCHARDATA(image_decoupee)[cpt_tableau*size_voxel]), buffer, largeur_max*size_voxel);
			cpt_tableau+=width;

			if(hauteur<hauteur_max-1)
			{
				fseeko(fd, jump_ligne, SEEK_CUR);
			}

			//fin ligne
		}

		if(compteur!=0)
		{
			fprintf(stdout, "\b\b\b\b");
			fprintf(stdout, "%3u%c", (u_int32_t)((double)profondeur/(double)(profondeur_max-1)*100.0), '%');
			fflush(stdout);
		}

		if(profondeur<profondeur_max-1)
		{
			fseeko(fd, jump_plan, SEEK_CUR);
		}

		//fin plan

	}

	if(compteur!=0)
	{
		fprintf(stdout, "\b\b\b\b");
		fprintf(stdout, "done\n");
		fflush(stdout);
	}


	free(buffer);

	return(1);
}


uint32_t crop_raw_on_disk_to_disk(FILE *fd, FILE *image_decoupee, uint64_t cx, uint64_t cy, uint64_t cz, uint64_t width, uint64_t height, uint64_t dpth, uint64_t rs, uint64_t cs, uint64_t d, uint64_t header_size, uint64_t size_voxel, char compteur)
{
	char *buffer;
	//uint32_t bufferlong[BUFFERSIZE];
	uint64_t largeur_max, hauteur_max, profondeur_max;
	uint64_t supx, supy, supz;
	uint64_t jump_ligne, jump_plan, premierpoint;
	uint64_t hauteur,largeur,profondeur, cpt_tableau;

	cpt_tableau=0;
	hauteur=0; largeur=0; profondeur=0;


	/* Initialisation des variables hauteur,largeur et profondeur en cas de debordement de l'image source */
	largeur_max = width;
	hauteur_max = height;
	profondeur_max = dpth;

	supx = cx + width;
	supy = cy + height;
	supz = cz + dpth;

	if(supx >= rs)
    {
    	//Too much on rowsize asked... we have an overflow.... better is to make smaller
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too large zone. It was reduced.\n");
		largeur_max = rs-cx;
    }

	if(supy >= cs)
    {
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too high zone. It was reduced.\n");
		hauteur_max = cs-cy;
	}

	if(supz >= d)
    {
    	//fprintf(stdout, "crop_on_disk: Warning, you asked to crop a too deep zone. It was reduced.\n");
		profondeur_max = d-cz;
	}


	buffer=(char*)calloc(largeur_max+1, sizeof(char)*size_voxel);
	if(buffer==NULL)
	{
		fprintf(stderr, "crop_raw_on_disk_to_disk: Error when allocating space for buffer (2).\n");
		return(0);
	}


	if((largeur_max<=0)||(hauteur_max<=0)||(profondeur_max<=0))
	{
		fprintf(stderr, "crop_raw_on_disk_to_disk: Required to crop on a negative size or out of image.\n");
		free(buffer);
		return (0);
	}


	jump_ligne = (rs-largeur_max)*size_voxel;
	jump_plan = (rs*(cs-hauteur_max+1)-largeur_max)*size_voxel;
	premierpoint = (rs*cs*cz + rs*cy + cx)*size_voxel;

	if(compteur!=0)
	{
		fprintf(stdout, "Processing image...     \n");
		fflush(stdout);
	}

	/* Remplissage du tableau imagedata de la valeur de tous les voxel du cube a extraire  */
	/* On se place au debut de l'image (pointeur pointe sur le premier voxel du cube a extraire*/
	/* Puis on parcours les lignes. Au bout d'un ligne on met le pointeur au debut de la prochaine ligne */
	/* Quand on a finis un plan on met le pointeur au debut de la premier ligne du plan suivant */

	fseeko(fd, (off_t)(header_size+premierpoint), SEEK_SET);

	for (profondeur=0;profondeur<profondeur_max;profondeur++)
	{
		for (hauteur=0;hauteur<hauteur_max;hauteur++)
		{
			if(fread(buffer, size_voxel, largeur_max, fd)!=largeur_max)
			{
				fprintf(stderr, "crop_raw_on_disk_to_disk: Could not read info in raw image: does it have good size ?\n");
				return(0);
			}

			if(fwrite(&buffer[0], size_voxel, largeur_max, image_decoupee)!=largeur_max)
			{
				fprintf(stderr, "crop_raw_on_disk_to_disk: Could not write in output raw image.\n");
				return(0);
			}

			if(hauteur<hauteur_max-1)
			{
				fseeko(fd, jump_ligne, SEEK_CUR);
			}

			//fin ligne
		}

		if(compteur!=0)
		{
			fprintf(stdout, "\b\b\b\b");
			fprintf(stdout, "%3u%c", (u_int32_t)((double)profondeur/(double)(profondeur_max-1)*100.0), '%');
			fflush(stdout);
		}


		if(profondeur<profondeur_max-1)
		{
			fseeko(fd, jump_plan, SEEK_CUR);
		}

		//fin plan

	}

	if(compteur!=0)
	{
		fprintf(stdout, "\b\b\b\b");
		fprintf(stdout, "done\n");
		fflush(stdout);
	}


	free(buffer);

	return(1);
}
