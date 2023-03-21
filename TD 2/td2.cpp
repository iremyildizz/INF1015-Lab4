#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp"      // Structures de données pour la collection de films en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>

#include "cppitertools/range.hpp"
#include "gsl/span"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;
using namespace gsl;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}
ListeFilms::ListeFilms() {
	capacite_ = 1;
	nElements_ = 0;
	elements_ = new Film* [capacite_];
}

span<Film*> ListeFilms::enSpan() const{
	return span(elements_, nElements_);
}

int ListeFilms::getNElements() const {
	return nElements_;
}

Film** ListeFilms::getElements() {
	return elements_;
}


//TODO:done Une fonction pour ajouter un Film à une ListeFilms, le film existant déjà; on veut uniquement ajouter le pointeur vers le film existant.  Cette fonction doit doubler la taille du tableau alloué, avec au minimum un élément, dans le cas où la capacité est insuffisante pour ajouter l'élément.  Il faut alors allouer un nouveau tableau plus grand, copier ce qu'il y avait dans l'ancien, et éliminer l'ancien trop petit.  Cette fonction ne doit copier aucun Film ni Acteur, elle doit copier uniquement des pointeurs.
void ListeFilms::ajouterFilm(Film* film) {
	if (capacite_ == nElements_) {
		capacite_ *= 2;
		Film** nouvelleListe = new Film* [capacite_];
		
		for (int index : range(0, nElements_)) 
			nouvelleListe[index] = elements_[index];

		delete[] elements_;
		elements_ = nouvelleListe;
	}
	elements_[nElements_] = film;
	nElements_ += 1;
}

//TODO: done Une fonction pour enlever un Film d'une ListeFilms (enlever le pointeur) sans effacer le film; la fonction prenant en paramètre un pointeur vers le film à enlever.  L'ordre des films dans la liste n'a pas à être conservé.
void ListeFilms::enleverFilm(Film* film) {
	for (int index : range(0, nElements_))
		if (elements_[index] == film) { 
			nElements_ -= 1;
			elements_[index] = elements_[nElements_];
			elements_[nElements_] = nullptr;
		}
}

//TODO: done Une fonction pour trouver un Acteur par son nom dans une ListeFilms, qui retourne un pointeur vers l'acteur, ou nullptr si l'acteur n'est pas trouvé.  Devrait utiliser span.
Acteur* ListeFilms::trouverActeur(const string& nomActeur) {
	for (Film* film : enSpan()) 
		for (Acteur* acteur : span(film->acteurs.elements, film->acteurs.nElements)) 
			if (nomActeur == acteur->nom)
				return acteur;
	return nullptr;
}

//TODO: done Compléter les fonctions pour lire le fichier et créer/allouer une ListeFilms.  La ListeFilms devra être passée entre les fonctions, pour vérifier l'existence d'un Acteur avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).
Acteur* lireActeur(istream& fichier, ListeFilms& listeFilms) {
	Acteur acteur = {};
	acteur.nom = lireString(fichier);
	acteur.anneeNaissance = lireUint16(fichier);
	acteur.sexe = lireUint8(fichier);

	Acteur* acteurPtr = listeFilms.trouverActeur(acteur.nom);
	if (acteurPtr == nullptr) {
		acteurPtr = new Acteur(acteur);
		acteurPtr->joueDans = ListeFilms();
	}
	return acteurPtr; //TODO: done Retourner un pointeur soit vers un acteur existant ou un nouvel acteur ayant les bonnes informations, selon si l'acteur existait déjà.  Pour fins de débogage, affichez les noms des acteurs crées; vous ne devriez pas voir le même nom d'acteur affiché deux fois pour la création.
}

Film* lireFilm(istream& fichier, ListeFilms& listeFilms) {
	Film film = {};
	film.titre = lireString(fichier);
	film.realisateur = lireString(fichier);
	film.anneeSortie = lireUint16(fichier);
	film.recette = lireUint16(fichier);

	int nElements = lireUint8(fichier);
	film.acteurs.nElements = nElements;  //NOTE: Vous avez le droit d'allouer d'un coup le tableau pour les acteurs, sans faire de réallocation comme pour ListeFilms.  Vous pouvez aussi copier-coller les fonctions d'allocation de ListeFilms ci-dessus dans des nouvelles fonctions et faire un remplacement de Film par Acteur, pour réutiliser cette réallocation.
	film.acteurs.capacite = nElements;
	film.acteurs.elements = new Acteur* [nElements];

	Film* filmPtr = new Film(film);
//TODO: done Placer l'acteur au bon endroit dans les acteurs du film.
//TODO: done Ajouter le film à la liste des films dans lesquels l'acteur joue.
	for (int index : range(0, nElements)) {
		Acteur* newActeur = lireActeur(fichier, listeFilms);
		filmPtr->acteurs.elements[index] = newActeur;
		newActeur->joueDans.ajouterFilm(filmPtr);
	}
	return filmPtr; //TODO: done Retourner le pointeur vers le nouveau film.
}

ListeFilms creerListe(string nomFichier) {
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = lireUint16(fichier);
	
	//TODO: done Créer une liste de films vide.
	ListeFilms listeDeFilm = ListeFilms();

	for (int i : range(0, nElements))
		listeDeFilm.ajouterFilm(lireFilm(fichier, listeDeFilm)); //TODO: done Ajouter le film à la liste.

	return listeDeFilm; //TODO: done  Retourner la liste de films.
}

//TODO: Une fonction pour détruire un film (relâcher toute la mémoire associée à ce film, et les acteurs qui ne jouent plus dans aucun films de la collection).  Noter qu'il faut enleve le film détruit des films dans lesquels jouent les acteurs.  Pour fins de débogage, affichez les noms des acteurs lors de leur destruction.
void détruireFilm(Film* film) {
	for (Acteur* acteur : span(film->acteurs.elements, film->acteurs.nElements)) {
		for (Film* filmDActeur : acteur->joueDans.enSpan()) 
			if (film == filmDActeur) {
				acteur->joueDans.enleverFilm(film);
				break;
			}
		if (acteur->joueDans.getNElements() == 0) {
			delete [] acteur->joueDans.getElements();
			delete acteur;
		}
	}
	delete[] film->acteurs.elements;
	delete film;
}
//TODO: Une fonction pour détruire une ListeFilms et tous les films qu'elle contient.
void ListeFilms::detruireListeFilm() {
	for (Film* film : enSpan())
		détruireFilm(film);
	delete[] elements_;
}

void afficherActeur(const Acteur& acteur) {
	cout << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

//TODO: done Une fonction pour afficher un film avec tous ces acteurs (en utilisant la fonction afficherActeur ci-dessus).
void afficherFilm(const Film* film) {
	cout << " " << film->titre << ": " << film->recette << ", " << film->realisateur << ", " << film->anneeSortie << endl;
}

void afficherFilmAvecActeur(const Film* film) {
	afficherFilm(film);
	for (Acteur* acteur: span(film->acteurs.elements, film->acteurs.nElements)) 
		afficherActeur(*acteur);
}

void ListeFilms::afficherListeFilms() const {
	//TODO: done Utiliser des caractères Unicode pour définir la ligne de séparation (différente des autres lignes de séparations dans ce progamme).
	static const string ligneDeSeparation = "\033[34m------------------------\033[0m";
	cout << ligneDeSeparation << endl;
	//TODO: done Changer le for pour utiliser un span.
	for (Film* film : enSpan()) {
		//TODO: done Afficher le film.
		afficherFilmAvecActeur(film);
		cout << ligneDeSeparation << endl;
	}
}

void ListeFilms::afficherFilmographieActeur(const string& nomActeur) {
	//TODO: done Utiliser votre fonction pour trouver l'acteur (au lieu de le mettre à nullptr).
	const Acteur* acteur = trouverActeur(nomActeur);
	if (acteur == nullptr)
		cout << "Aucun acteur de ce nom" << endl;
	else
		acteur->joueDans.afficherListeFilms();
}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	//int* fuite = new int; //TODO: Enlever cette ligne après avoir vérifié qu'il y a bien un "Fuite detectee" de "4 octets" affiché à la fin de l'exécution, qui réfère à cette ligne du programme.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	//TODO: Chaque TODO dans cette fonction devrait se faire en 1 ou 2 lignes, en appelant les fonctions écrites.

	//TODO: done La ligne suivante devrait lire le fichier binaire en allouant la mémoire nécessaire.  Devrait afficher les noms de 20 acteurs sans doublons (par l'affichage pour fins de débogage dans votre fonction lireActeur).
	ListeFilms listeFilms = creerListe("films.bin");

	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
	//TODO: done Afficher le premier film de la liste.  Devrait être Alien.
	Film* alienPtr = listeFilms.getElements()[0];
	afficherFilm(alienPtr);

	cout << ligneDeSeparation << "Les films sont:" << endl;
	//TODO: done Afficher la liste des films.  Il devrait y en avoir 7.
	listeFilms.afficherListeFilms();

	//TODO: done Modifier l'année de naissance de Benedict Cumberbatch pour être 1976 (elle était 0 dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des films et des acteurs dans les listes, il faut y aller par son nom.
	listeFilms.trouverActeur("Benedict Cumberbatch")->anneeNaissance = 1976;

	//TODO: done Afficher la liste des films où Benedict Cumberbatch joue.  Il devrait y avoir Le Hobbit et Le jeu de l'imitation.
	cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;
	listeFilms.afficherFilmographieActeur("Benedict Cumberbatch");

	//TODO: done Détruire et enlever le premier film de la liste (Alien).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les acteurs Tom Skerritt et John Hurt, mais pas Sigourney Weaver puisqu'elle joue aussi dans Avatar.
	listeFilms.enleverFilm(alienPtr);
	détruireFilm(alienPtr);

	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
	//TODO: done Afficher la liste des films.
	listeFilms.afficherListeFilms();
	//TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.

	//TODO: Détruire tout avant de terminer le programme.  La bibliothèque de verification_allocation devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
	listeFilms.detruireListeFilm();
}
