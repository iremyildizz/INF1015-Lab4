//*** Solutionnaire version 2, sans les //[ //] au bon endroit car le code est assez différent du code fourni.
#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp"      // Structures de données pour la collection de films en mémoire.

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.

#include <iostream>
#include <fstream>
#include<sstream>
#include <string>
#include <limits>
#include <algorithm>
#include "cppitertools/range.hpp"
#include "gsl/span"
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).
#include <iomanip>
#include <vector>
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

//TODO: Une fonction pour ajouter un Film à une ListeFilms, le film existant déjà; on veut uniquement ajouter le pointeur vers le film existant.  Cette fonction doit doubler la taille du tableau alloué, avec au minimum un élément, dans le cas où la capacité est insuffisante pour ajouter l'élément.  Il faut alors allouer un nouveau tableau plus grand, copier ce qu'il y avait dans l'ancien, et éliminer l'ancien trop petit.  Cette fonction ne doit copier aucun Film ni Acteur, elle doit copier uniquement des pointeurs.
//[
void ListeFilms::changeDimension(int nouvelleCapacite)
{
	Film** nouvelleListe = new Film * [nouvelleCapacite];

	if (elements != nullptr) {  // Noter que ce test n'est pas nécessaire puique nElements sera zéro si elements est nul, donc la boucle ne tentera pas de faire de copie, et on a le droit de faire delete sur un pointeur nul (ça ne fait rien).
		nElements = min(nouvelleCapacite, nElements);
		for (int i : range(nElements))
			nouvelleListe[i] = elements[i];
		delete[] elements;
	}

	elements = nouvelleListe;
	capacite = nouvelleCapacite;
}

void ListeFilms::ajouterFilm(Film* film)
{
	if (nElements == capacite)
		changeDimension(max(1, capacite * 2));
	elements[nElements++] = film;
}

//]

//TODO: Une fonction pour enlever un Film d'une ListeFilms (enlever le pointeur) sans effacer le film; la fonction prenant en paramètre un pointeur vers le film à enlever.  L'ordre des films dans la liste n'a pas à être conservé.
//[
// On a juste fait une version const qui retourne un span non const.  C'est valide puisque c'est la struct qui est const et non ce qu'elle pointe.  Ça ne va peut-être pas bien dans l'idée qu'on ne devrait pas pouvoir modifier une liste const, mais il y aurais alors plusieurs fonctions à écrire en version const et non-const pour que ça fonctionne bien, et ce n'est pas le but du TD (il n'a pas encore vraiment de manière propre en C++ de définir les deux d'un coup).
span<Film*> ListeFilms::enSpan() const { return span(elements, nElements); }

void ListeFilms::enleverFilm(const Film* film)
{
	for (Film*& element : enSpan()) {  // Doit être une référence au pointeur pour pouvoir le modifier.
		if (element == film) {
			if (nElements > 1)
				element = elements[nElements - 1];
			nElements--;
			return;
		}
	}
}
//]

//TODO: Une fonction pour trouver un Acteur par son nom dans une ListeFilms, qui retourne un pointeur vers l'acteur, ou nullptr si l'acteur n'est pas trouvé.  Devrait utiliser span.
//[
// Voir la NOTE ci-dessous pourquoi Acteur* n'est pas const.  Noter que c'est valide puisque c'est la struct uniquement qui est const dans le paramètre, et non ce qui est pointé par la struct.
span<shared_ptr<Acteur>> spanListeActeurs(const ListeActeurs& liste) { return span(liste.elements.get(), liste.nElements); }

//NOTE: Doit retourner un Acteur modifiable, sinon on ne peut pas l'utiliser pour modifier l'acteur tel que demandé dans le main, et on ne veut pas faire écrire deux versions.
shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const
{
	for (const Film* film : enSpan()) {
		for (shared_ptr<Acteur> acteur : spanListeActeurs(film->acteurs_)) {
			if (acteur->nom == nomActeur)
				return acteur;
		}
	}
	return nullptr;
}
//]

//TODO: Compléter les fonctions pour lire le fichier et créer/allouer une ListeFilms.  La ListeFilms devra être passée entre les fonctions, pour vérifier l'existence d'un Acteur avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).
shared_ptr<Acteur> lireActeur(istream& fichier//[
	, ListeFilms& listeFilms//]
)
{
	Acteur acteur = {};
	acteur.nom = lireString(fichier);
	acteur.anneeNaissance = lireUint16(fichier);
	acteur.sexe = lireUint8(fichier);
	//[
	shared_ptr<Acteur> acteurExistant = listeFilms.trouverActeur(acteur.nom);
	if (acteurExistant != nullptr)
		return acteurExistant;
	else {
		cout << "Création Acteur " << acteur.nom << endl;
		return make_shared<Acteur>(acteur);
	}
	//]
	return {}; //TODO: Retourner un pointeur soit vers un acteur existant ou un nouvel acteur ayant les bonnes informations, selon si l'acteur existait déjà.  Pour fins de débogage, affichez les noms des acteurs crées; vous ne devriez pas voir le même nom d'acteur affiché deux fois pour la création.
}

Film* lireFilm(istream& fichier//[
	, ListeFilms& listeFilms//]
)
{
	Film film{};
	film.titre_ = lireString(fichier);
	film.realisateur_ = lireString(fichier);
	film.annee_ = lireUint16(fichier);
	film.recette_ = lireUint16(fichier);
	film.acteurs_ = ListeActeurs(lireUint8(fichier));
	//NOTE: Vous avez le droit d'allouer d'un coup le tableau pour les acteurs, sans faire de réallocation comme pour ListeFilms.  Vous pouvez aussi copier-coller les fonctions d'allocation de ListeFilms ci-dessus dans des nouvelles fonctions et faire un remplacement de Film par Acteur, pour réutiliser cette réallocation.
  //[
   //NOTE: On aurait normalement fait le "new" au début de la fonction pour directement mettre les informations au bon endroit; on le fait ici pour que le code ci-dessus puisse être directement donné aux étudiants sans qu'ils aient le "new" déjà écrit.
  /*
  //]
  for (int i = 0; i < film.acteurs.nElements; i++) {
	  //[
  */
	for (shared_ptr<Acteur>& acteur : spanListeActeurs(film.acteurs_)) {
		acteur =
			//]
			lireActeur(fichier
				, listeFilms
			); //TODO: Placer l'acteur au bon endroit dans les acteurs du film.
		//TODO: Ajouter le film à la liste des films dans lesquels l'acteur joue.


	}
	Film* filmp = new Film(film);
	//[
	return filmp;
	//]
	return {}; //TODO: Retourner le pointeur vers le nouveau film.
}

ListeFilms::ListeFilms(const string& nomFichier) : possedeLesFilms_(true)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = lireUint16(fichier);

	//TODO: Créer une liste de films vide.
	//[
	/*
	//]
	for (int i = 0; i < nElements; i++) {
		//[
	*/
	for ([[maybe_unused]] int i : range(nElements)) { //NOTE: On ne peut pas faire un span simple avec spanListeFilms car la liste est vide et on ajoute des éléments à mesure.
		ajouterFilm(
			//]
			lireFilm(fichier//[
				, *this  //NOTE: L'utilisation explicite de this n'est pas dans la matière indiquée pour le TD2.
				//]
			)//[
		)
			//]
			; //TODO: Ajouter le film à la liste.
	}

	//[
	/*
	//]
	return {}; //TODO: Retourner la liste de films.
	//[
	*/
	//]
}

//TODO: Une fonction pour détruire un film (relâcher toute la mémoire associée à ce film, et les acteurs qui ne jouent plus dans aucun films de la collection).  Noter qu'il faut enleve le film détruit des films dans lesquels jouent les acteurs.  Pour fins de débogage, affichez les noms des acteurs lors de leur destruction.
//[
void detruireActeur(Acteur* acteur)
{
	cout << "Destruction Acteur " << acteur->nom << endl;
	delete acteur;
}
//bool joueEncore(const Acteur* acteur)
//{
//	return acteur->joueDans.size() != 0;
//}

void detruireFilm(Film* film)
{
	cout << "Destruction Film " << film->titre_ << endl;
	delete film;
}


//TODO: Une fonction pour détruire une ListeFilms et tous les films qu'elle contient.
//[
//NOTE: Attention que c'est difficile que ça fonctionne correctement avec le destructeur qui détruit la liste.  Mon ancienne implémentation utilisait une méthode au lieu d'un destructeur.  Le problème est que la matière pour le constructeur de copie/move n'est pas dans le TD2 mais le TD3, donc si on copie une liste (par exemple si on la retourne de la fonction creerListe) elle sera incorrectement copiée/détruite.  Ici, creerListe a été converti en constructeur, qui évite ce problème.
ListeFilms::~ListeFilms()
{
	if (possedeLesFilms_)
		for (Film* film : enSpan())
			detruireFilm(film);
	delete[] elements;
}
//]

ostream& afficherActeur(ostream& os, const Acteur& acteur)
{
	cout << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
	return os;
}

//TODO: Une fonction pour afficher un film avec tous ces acteurs (en utilisant la fonction afficherActeur ci-dessus).

ostream& operator<<(ostream& os, const Film& film)
{
	os << "Titre: " << film.titre_ << endl;
	os << "  Réalisateur: " << film.realisateur_ << "  Année :" << film.annee_ << endl;
	os << "  Recette: " << film.recette_ << "M$" << endl;

	os << "Acteurs:" << endl;
	for (const shared_ptr<Acteur> acteur : spanListeActeurs(film.acteurs_))
		afficherActeur(os, *acteur);

	return os;
}



void afficherListeItems(const vector<unique_ptr<Item>>& items)
{
}


int main()
{
#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
#endif
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	//TODO: Chaque TODO dans cette fonction devrait se faire en 1 ou 2 lignes, en appelant les fonctions écrites.

	//TODO: La ligne suivante devrait lire le fichier binaire en allouant la mémoire nécessaire.  Devrait afficher les noms de 20 acteurs sans doublons (par l'affichage pour fins de débogage dans votre fonction lireActeur).
	vector<unique_ptr<Item>> bibliotheque;
	ListeFilms listeFilms("films.bin");

	for (Film* film : listeFilms.enSpan())
	{
		bibliotheque.push_back(make_unique<Film>(*film));
	}

	ifstream listeLivres("livres.txt");
	if (listeLivres.is_open())
	{
		string ligne;
		while (getline(listeLivres, ligne))
		{
			stringstream ss(ligne);
			string titre, auteur;
			int annee, copieVendues, nbrePages;
			ss >> titre >> annee >> auteur >> copieVendues >> nbrePages;
			bibliotheque.emplace_back(make_unique<Livre>(titre, annee, auteur, copieVendues, nbrePages));
		}
		listeLivres.close();
	}

}