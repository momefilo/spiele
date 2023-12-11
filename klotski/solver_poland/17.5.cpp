/*
 *   Klotski Solver
 *   Copyright (C) 2009 Daniel Blazewicz
 *   e-mail : klajok@interia.pl
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <fstream>
#include <sstream>
#include <ctime>

#ifdef USE_MPI
  #include "mpi.h"
#endif

const char * version="1.7.5";

using namespace std;

typedef char bajt;

#if LONG_BIT == 64

  //most of 64-bit systems
  typedef long dlugi;
  typedef long Long;
  typedef unsigned long uLong;
  #define MPI_DLUGI (MPI_LONG)
  #define Long_MAX (LONG_MAX)
  #define order (64)
  uLong polynom    = 0x42F0E1EBA9EA3693UL; //CRC-64-ECMA-182 (based on http://en.wikipedia.org/wiki/Cyclic_redundancy_check)
  uLong crchighbit = 0x8000000000000000UL;
  uLong crctab[256];

#elif _WIN64

  //of course Windows must be...
  typedef long long dlugi;
  typedef long long Long;
  typedef unsigned long long uLong;
  #define MPI_DLUGI (MPI_LONG_LONG)
  #define Long_MAX (LONG_LONG_MAX)
  #define order (64)
  uLong polynom    = 0x42F0E1EBA9EA3693ULL; //CRC-64-ECMA-182
  uLong crchighbit = 0x8000000000000000ULL;
  uLong crctab[256];

#elif LONG_BIT == 32

  //32-bit systems
  typedef double dlugi;
  typedef long Long;
  typedef unsigned long uLong;
  #define MPI_DLUGI (MPI_DOUBLE)
  #define Long_MAX (LONG_MAX)
  #define order (32)
  uLong polynom    = 0x1EDC6F41UL; //CRC-32C(Castagnoli)
  uLong crchighbit = 0x80000000UL;
  uLong crctab[256];

#else

  Error: Not supported platform!

#endif

Long Rx, Ry, R;//rozmiar planszy do gry

Long * pl_konc;//jak wyglada podloze planszy (pl_konc planszy): obramowania i miejsce koncowe
Long * pl_pocz;//uklad klockow na planszy poczatkowej
bajt * konf_pocz;//konfiguracja poczatkowa (na podstawie planszy poczatkowej)
bool * wazniejszy;//czy klocek o danym numerze moze "wyjsc" poza plansze, chodzi o pola "-3"

Long N;//liczba klockow
Long M;//liczba grup klockow

Long * rozmiar;//tablica z rozmiarami poszczegolnych klockow pomniejszonymi o 1
Long ** ksztalt;//tablica z ksztaltami poszczegolnych klockow (zawsze bez pierwszego elementu klocka)

Long lwar;//liczba warunkow rozwiazania planszy
Long * war_nr;//tablica warunkow z numerami klockow
Long * war_pol;//talica warunkow z polozeniami klockow

Long * nr_grupy;//tablica z numerem grupy, do ktorej nalezy kazdy klocek
Long * pier_w_grupie;//tablica z numerem pierwszego klocka nalezacego do danej grupy
Long * ost_w_grupie;//tablica z numerem ostatniego klocka nalezacego do danej grupy
Long * licznosc_grupy;//tablica z liczba klockow nalezacych do danej grupy

Long * max_przesun;//tablica z maksymalnym polozeniem/przesunieciem danego klocka, tak by nie wychodzil poza plansze

Long max_K;//liczba konf. ktora sie zmiesci w tablicy konfiguracji
Long max_k1, max_k2, max_k3;//liczba konfiguracji, zaleznie, czy deklarowane sa inne tablice
Long nadmiar2, nadmiar3;//ile dac nadmiaru dla tablicy hashy

Long lkoszykow;//liczba koszykow, do ktorych wrzucac nowe konfiguracje


Long ile_long;//ile potrzeba liczb typu Long by zmiescila sie w nich konfiguracja

Long odl_rozw = 1000000000;//w ilu krokach mozna dojsc do rozwiazania ukladu

Long co_ile_zostaw;//co ktore pokolenia konfiguracji zostawiac na dysku

Long mytid = 0, nproc = 1;//MY Task ID, Number of PROCesses

union long_bajt
{
  Long longi[20];//mozna dopasowac te liczbe to wielkich tablic
  bajt bajty[20*sizeof(Long)/sizeof(bajt)];
};

Long wylicz_hash(const bajt * konf);
bool identyczne_konfiguracje(const bajt * pier, const bajt * drug);
void kopiuj_konf(const bajt * zrodlo, bajt * cel);
Long wybierz_koszyk(const bajt * konf);
void wyczysc_plik_koszyka(const Long tid, const Long ktory);
void wypisz_konf_literalnie(const bajt * konf);

inline void error_quit()
{
#ifdef USE_MPI
    MPI_Abort(MPI_COMM_WORLD, 1);
#else
    exit(1);
#endif
}

class miejsce
{
public:

  miejsce();
  ~miejsce();
  void inicjuj(const Long lkoszykow_, const double w_bajtach);
  void dodano_do_koszyka(const Long nr_koszyka, const double w_bajtach);
  void dodano_konf(const Long nr_koszyka, const double w_bajtach);
  double ile_nowych_konf(const Long nr_koszyka);
  double ile_w_koszyku(const Long nr_koszyka);
  void dodano_do_rozwiazan(const double w_bajtach);
  void zmiana_pokolen(const bool skasowano_dziadkow);
  void skasowano_koszyk(const Long ktory);
  void skasowano_rozwiazania();
  double wolne_miejsce();
  double do_znajdowania_potomkow();
#ifdef USE_MPI
  void synchronizuj_konf();
  void synchronizuj_koszyki();
#endif

private:

  void sprawdz_czy_inicjowano();

  bool inicjowano;
  //Long nazawsze;
  Long lkoszykow;
  double * dziadkowie;
  double * rodzice;
  double * dzieci;
  double * kosze;
  double rozwiazania;
  double dostepne_miejsce;
  double stare_dostepne_miejsce;

} Na_dysku;

class koszyki
{

public:

  koszyki(bajt * A, bajt * Z);
  ~koszyki();
  void dorzuc(const bajt * konf);
  void close_all();

private:

  Long rozmiar;
  bajt ** poczatek;
  bajt ** pozycja;
  Long * ile_w_koszyku;
  FILE ** plik;
};

class hashe
{
public:

  hashe(const Long max_K_, const Long nadmiar);
  ~hashe();
  void wyczysc();
  void dodaj_konf(bajt * konf);
  bool nowa_konfiguracja(bajt * konf);
  bajt * stara_konfiguracja(bajt * konf);
  double rzeczywisty_nadmiar();

private:

  Long max_K, max_H, wierzch;
  bajt ** H;//tablica (hashy) wskazujaca pozycje w tablicy K
  Long * H_nast;//tablica wskazujaca nastepna pozycje w tablicy H

};

class wczytujemy_koszyki
{

public:

  wczytujemy_koszyki(const Long tid, const Long koszyk);
  ~wczytujemy_koszyki();
  Long wczytaj_porcje(bajt * tablica, const Long rozmiar);

private:
  FILE * plik;

};

class wczytujemy_konfiguracje
{

public:

  wczytujemy_konfiguracje(const Long pokolenie, const Long koszyk);
  ~wczytujemy_konfiguracje();
  Long wczytaj_porcje(bajt * tablica, const Long rozmiar);

private:
  FILE * plik;

};

miejsce::miejsce()
{
  inicjowano = false;
}

miejsce::~miejsce()
{
  delete [] dziadkowie;
  delete [] rodzice;
  delete [] dzieci;
  delete [] kosze;
}

void miejsce::inicjuj(const Long lkoszykow_, const double w_bajtach)
{
  lkoszykow = lkoszykow_;
  dostepne_miejsce = w_bajtach;
  stare_dostepne_miejsce = w_bajtach;
  //nazawsze = 0;
  dziadkowie = new double[lkoszykow];
  rodzice = new double[lkoszykow];
  dzieci = new double[lkoszykow];
  kosze = new double[lkoszykow];
  for (Long i = 0; i < lkoszykow; ++i)
  {
    dziadkowie[i] = 0;
    rodzice[i] = 0;
    dzieci[i] = 0;
    kosze[i] = 0;
  }
  rozwiazania = 0;
  inicjowano = true;
}

void miejsce::dodano_do_koszyka(const Long nr_koszyka, const double w_bajtach)
{
  sprawdz_czy_inicjowano();
  kosze[nr_koszyka] += w_bajtach;
  dostepne_miejsce -= w_bajtach;
}

void miejsce::dodano_konf(const Long nr_koszyka, const double w_bajtach)
{
  sprawdz_czy_inicjowano();
  dzieci[nr_koszyka] += w_bajtach;
  dostepne_miejsce -= w_bajtach;
}
double miejsce::ile_nowych_konf(const Long nr_koszyka)
{
  sprawdz_czy_inicjowano();
  return dzieci[nr_koszyka];
}

double miejsce::ile_w_koszyku(const Long nr_koszyka)
{
  sprawdz_czy_inicjowano();
  return kosze[nr_koszyka];
}

void miejsce::dodano_do_rozwiazan(const double w_bajtach)
{
  sprawdz_czy_inicjowano();
  rozwiazania += w_bajtach;
  dostepne_miejsce -= w_bajtach;
}

void miejsce::zmiana_pokolen(const bool skasowano_dziadkow)
{
  sprawdz_czy_inicjowano();

  /*if (!skasowano_dziadkow)
    nazawsze += dziadkowie;
  else
    dostepne_miejsce += dziadkowie;*/
  if (skasowano_dziadkow)
    for (Long i = 0; i < lkoszykow; ++i)
      dostepne_miejsce += dziadkowie[i];
  /* */

  for (Long i = 0; i < lkoszykow; ++i)
  {
    dziadkowie[i] = rodzice[i];
    rodzice[i] = dzieci[i];
    dzieci[i] = 0;
  }

  stare_dostepne_miejsce = dostepne_miejsce;
}

void miejsce::skasowano_koszyk(const Long ktory)
{
  sprawdz_czy_inicjowano();
  dostepne_miejsce += kosze[ktory];
  kosze[ktory] = 0;
}

void miejsce::skasowano_rozwiazania()
{
  sprawdz_czy_inicjowano();
  dostepne_miejsce += rozwiazania;
  rozwiazania = 0;
}

double miejsce::wolne_miejsce()
{
  sprawdz_czy_inicjowano();
  return dostepne_miejsce;
}

double miejsce::do_znajdowania_potomkow()
{
  //ile potrzeba miejsca w pamieci do 'bezbolesnego' znajdowania potomkow
  double potrzeba = 0;
  for (Long i = 0; i < lkoszykow; ++i)
  {
    double pom = dziadkowie[i] + rodzice[i] + dzieci[i] + kosze[i];
    if (potrzeba < pom)
      potrzeba = pom;
  }
  return potrzeba;
}

#ifdef USE_MPI
void miejsce::synchronizuj_konf()
{
  double * dzieci_ = new double[lkoszykow];

  for (Long i = 0; i < lkoszykow; ++i)
    dzieci_[i] = dzieci[i];

  MPI_Allreduce(dzieci_, dzieci, lkoszykow, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  for (Long i = 0; i < lkoszykow; ++i)
    dostepne_miejsce -= dzieci[i] - dzieci_[i];



  /*ostringstream wyjs(ostringstream::out);
  wyjs << "Przed konf #" << mytid << ": ";
  for (Long i = 0; i < lkoszykow; ++i)
    wyjs << dzieci_[i] << " ";
  if (!mytid)
  {
    wyjs << " Po : ";
    for (Long i = 0; i < lkoszykow; ++i)
      wyjs << dzieci[i] << " ";
  }
  wyjs << "\n";
  cout << wyjs.str();*/



  delete [] dzieci_;
}

void miejsce::synchronizuj_koszyki()
{
  double * kosze_ = new double[lkoszykow];

  for (Long i = 0; i < lkoszykow; ++i)
    kosze_[i] = kosze[i];

  MPI_Allreduce(kosze_, kosze, lkoszykow, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  for (Long i = 0; i < lkoszykow; ++i)
    dostepne_miejsce -= kosze[i] - kosze_[i];

/*  ostringstream wyjs(ostringstream::out);
  wyjs << "Przed baskets #" << mytid << ": ";
  for (Long i = 0; i < lkoszykow; ++i)
    wyjs << kosze_[i] << " ";
  if (!mytid)
  {
    wyjs << " Po : ";
    for (Long i = 0; i < lkoszykow; ++i)
      wyjs << kosze[i] << " ";
  }
  wyjs << "\n";
  cout << wyjs.str();*/


  delete [] kosze_;
}
#endif

void miejsce::sprawdz_czy_inicjowano()
{
  if (!inicjowano)
  {
    cerr << "Error: Class 'miejsce' is not properly initialized!\n";
    error_quit();
  }
}

hashe::hashe(const Long max_K_, const Long nadmiar)
{
  max_K = max_K_;
  max_H = max_K + nadmiar;
  wierzch = max_H;//bo niezainicjowane
  H = new bajt*[max_H];
  H_nast = new Long[max_H];
}

hashe::~hashe()
{
  delete [] H;
  delete [] H_nast;
}

void hashe::wyczysc()
{
  wierzch = max_K;
  for (Long i = 0; i < max_K; ++i)
    H[i] = 0;
}

void hashe::dodaj_konf(bajt * konf)
{
  Long poz = wylicz_hash(konf);

  if (!H[poz])
  {
    H[poz] = konf;
    H_nast[poz] = 0;
    return;
  }

  if (wierzch == max_H)
  {
    cerr << "Error: Not enough memory for hashes!!\n";
    error_quit();
  }

  while (H_nast[poz])
    poz = H_nast[poz];
  H[wierzch] = konf;
  H_nast[wierzch] = 0;
  H_nast[poz] = wierzch;
  wierzch++;
}

bool hashe::nowa_konfiguracja(bajt * konf)
{
  Long poz = wylicz_hash(konf);

  if (!H[poz])
    return true;

  do
  {
    if (identyczne_konfiguracje(H[poz], konf))
      return false;

    poz = H_nast[poz];

  } while (poz);

  return true;
}

bajt * hashe::stara_konfiguracja(bajt * konf)
{
  Long poz = wylicz_hash(konf);

  if (!H[poz])
    return 0;

  do
  {
    if (identyczne_konfiguracje(H[poz], konf))
      return H[poz];

    poz = H_nast[poz];

  } while (poz);

  return 0;
}

double hashe::rzeczywisty_nadmiar()
{
  return (double(wierzch)-max_K)/max_K;
}

koszyki::koszyki(bajt * A, bajt * Z)
{
  poczatek = new bajt*[lkoszykow];
  pozycja = new bajt*[lkoszykow];
  ile_w_koszyku = new Long[lkoszykow];
  plik = new FILE*[lkoszykow];

  rozmiar = static_cast<Long> ( (reinterpret_cast<uLong>(Z) - reinterpret_cast<uLong>(A)) / static_cast<uLong>(N * lkoszykow) );

#ifdef BE_VERBOSE
  cerr << "Baskets size: "<<rozmiar<<" bytes\n";
#endif

  poczatek[0] = A;
  for (Long i = 1; i < lkoszykow; ++i)
    poczatek[i] = poczatek[i-1] + rozmiar * N;

  for (Long i = 0; i < lkoszykow; ++i)
    pozycja[i] = poczatek[i];

  for (Long i = 0; i < lkoszykow; ++i)
    ile_w_koszyku[i] = 0;

  for (Long i = 0; i < lkoszykow; ++i)
  {
    char nazwa[30];
    sprintf(nazwa, "temp_%ld/basket_%ld", mytid, i);
    plik[i] = fopen(nazwa, "ab");
    if (!plik[i])
    {
      cerr << "Error: I can't open file: "<<nazwa<<"\n";
      error_quit();
    }

  }
}

koszyki::~koszyki()
{
  if (poczatek) //files are not closed
    close_all();
}

void koszyki::dorzuc(const bajt * konf)
{
  const Long koszyk = wybierz_koszyk(konf);
  kopiuj_konf(konf, pozycja[koszyk]);
  ile_w_koszyku[koszyk]++;
  if (ile_w_koszyku[koszyk] == rozmiar)
  {
#ifdef BE_VERBOSE
    cerr << "Storing data from basket number "<<koszyk<<"\n";
#endif
    Long ile_zapisano = fwrite(poczatek[koszyk], N, rozmiar, plik[koszyk]);
    Na_dysku.dodano_do_koszyka(koszyk, double(ile_zapisano) * N);
    if (ile_zapisano < rozmiar)
    {
      cerr << "Error: I can't write configurations to basket number "<<koszyk<<" !\n";
      error_quit();
    }
    pozycja[koszyk] = poczatek[koszyk];
    ile_w_koszyku[koszyk] = 0;
  }
  else
    pozycja[koszyk] += N;
}

void koszyki::close_all()
{
  for (Long i = 0; i < lkoszykow; ++i)
  {
    Long ile_zapisano = fwrite(poczatek[i], N, ile_w_koszyku[i], plik[i]);
    Na_dysku.dodano_do_koszyka(i, double(ile_zapisano) * N);
    if (ile_zapisano < ile_w_koszyku[i])
    {
      cerr << "Error: I can't write configurations to basket number "<<i<<"!\n";
      error_quit();
    }
    fclose(plik[i]);
  }
  delete [] poczatek;
  delete [] pozycja;
  delete [] ile_w_koszyku;
  delete [] plik;

  poczatek = 0;
  pozycja = 0;
  ile_w_koszyku = 0;
  plik = 0;
}

wczytujemy_koszyki::wczytujemy_koszyki(const Long tid, const Long koszyk)
{
  char nazwa[30];
  sprintf(nazwa, "temp_%ld/basket_%ld", tid, koszyk);
  if (!(plik=fopen(nazwa,"rb")))
  {
    cerr << "Error: I can't open file "<<nazwa<<" for reading!\n";
    error_quit();
  }
}

wczytujemy_koszyki::~wczytujemy_koszyki()
{
  fclose(plik);
}

Long wczytujemy_koszyki::wczytaj_porcje(bajt * tablica, const Long rozmiar)
{
  Long wczytano = 0;
  if (!feof(plik)) {
    wczytano = fread(tablica, N, rozmiar, plik);
    if ((wczytano < rozmiar) && (!feof(plik)))
    {
      cerr << "Error: I can't read from file!\n";
      error_quit();
    }
  }
  return wczytano;
}

wczytujemy_konfiguracje::wczytujemy_konfiguracje(const Long pokolenie, const Long koszyk)
{
  char nazwa[30];
  sprintf(nazwa, "temp_%ld/generation_%ld_%ld", koszyk % nproc, pokolenie, koszyk);
  if (!(plik=fopen(nazwa,"rb")))
  {
    cerr << "Error: I can't open file "<<nazwa<<" for reading!\n";
    error_quit();
  }
}

wczytujemy_konfiguracje::~wczytujemy_konfiguracje()
{
  fclose(plik);
}

Long wczytujemy_konfiguracje::wczytaj_porcje(bajt * tablica, const Long rozmiar)
{
  Long wczytano = 0;
  if (!feof(plik)) {
    wczytano = fread(tablica, N, rozmiar, plik);
    if ((wczytano < rozmiar) && (!feof(plik)))
    {
      cerr << "Error: I can't read from file!\n";
      error_quit();
    }
  }
  return wczytano;
}

//based on http://www.zorc.breitbandkatze.de/crctester.c
void generate_crc_table() {
        for (uLong i=0; i<256; i++) {
                uLong crc=i << (order-8);
                for (uLong j=0; j<8; j++) {
                        uLong bit = crc & crchighbit;
                        crc<<= 1;
                        if (bit) crc^= polynom;
                }
                crctab[i]= crc;
        }
}
inline uLong compute_crc (const unsigned char* p, const Long len) {
        uLong crc = 0;
        Long len_ = len;
        while (len_--) crc = (crc << 8) ^ crctab[ (crc >> (order-8)) ^ *p++ ];
        return crc;
}


Long wylicz_hash(const bajt * konf)
{
  return (Long)(compute_crc((unsigned char *)konf, N) >> 1) % max_K;
}

void kopiuj_konf(const bajt * zrodlo, bajt * cel)
{
  for (Long i = 0; i < N; ++i)
    cel[i] = zrodlo[i];
}

bool konf_to_plansza(const bajt * konf, Long * plansza)
{
  for (Long i = 0; i < R; ++i)
    plansza[i] = pl_konc[i];

  for (Long i = 0; i < N; ++i)
  {
    Long pol_pier = konf[i];
    Long nr = i+1;

    if (pol_pier < 0) return false;
    if (pol_pier > max_przesun[i]) return false;
    if (!wazniejszy[i])
    {
      if (plansza[pol_pier]) return false;

      plansza[pol_pier] = nr;
      Long rozm = rozmiar[i];
      if (rozm)
      {
        Long * kszt = ksztalt[i];
        for (Long j = 0; j < rozm; ++j)
        {
          Long poloz = pol_pier + kszt[j];
          if (plansza[poloz]) return false;
          plansza[poloz] = nr;
        }
      }
    }
    else
    {
      if (plansza[pol_pier] > 0) return false; //czyli cokolwiek poza "-3"

      plansza[pol_pier] = nr;
      Long rozm = rozmiar[i];
      if (rozm)
      {
        Long * kszt = ksztalt[i];
        for (Long j = 0; j < rozm; ++j)
        {
          Long poloz = pol_pier + kszt[j];
          if (plansza[poloz] > 0 ) return false;
          plansza[poloz] = nr;
        }
      }
    }
  }
  return true;
}

bool poprawna_konf(const bajt * konf)
{
  Long * plansza = new Long [R];

  for (Long i = 0; i < R; ++i)
    plansza[i] = pl_konc[i];

  for (Long i = 0; i < N; ++i)
  {
    Long pol_pier = konf[i];
    Long nr = i+1;

    if (pol_pier<0)
    {
      cerr << "Q1 : "<< pol_pier<<" "<<i<<"\n";
      delete [] plansza;
      return false;
    }
    if (pol_pier > max_przesun[i])
    {
      cerr << "Q2 : "<< pol_pier<<" "<<i<<"\n";
      delete [] plansza;
      return false;
    }

    if (!wazniejszy[i])
    {

      if (plansza[pol_pier])
      {
        cerr << "Q3 : "<< pol_pier<<" "<<i<<" "<<plansza[pol_pier]<<"\n";
        delete [] plansza;
        return false;
      }

      plansza[pol_pier] = nr;
      Long rozm = rozmiar[i];
      if (rozm)
      {
        Long * kszt = ksztalt[i];
        for (Long j = 0; j < rozm; ++j)
        {
          Long poloz = pol_pier + kszt[j];
          if(plansza[poloz])
          {
            cerr << "Q4 : "<<j<<" "<< pol_pier<<" "<<poloz<<" "<<i<<" "<<plansza[poloz]<<"\n";
            delete [] plansza;
            return false;
          }
          plansza[poloz] = nr;
        }
      }

    }
    else
    {

      if (plansza[pol_pier] > 0)
      {
        cerr << "Q3 : "<< pol_pier<<" "<<i<<" "<<plansza[pol_pier]<<"\n";
        delete [] plansza;
        return false;
      }

      plansza[pol_pier] = nr;
      Long rozm = rozmiar[i];
      if (rozm)
      {
        Long * kszt = ksztalt[i];
        for (Long j = 0; j < rozm; ++j)
        {
          Long poloz = pol_pier + kszt[j];
          if(plansza[poloz] > 0)
          {
            cerr << "Q4 : "<<j<<" "<< pol_pier<<" "<<poloz<<" "<<i<<" "<<plansza[poloz]<<"\n";
            delete [] plansza;
            return false;
          }
          plansza[poloz] = nr;
        }
      }


    }

  }
  delete [] plansza;
  return true;
}

bool identyczne_konfiguracje(const bajt * pier, const bajt * drug)
{
  for (Long i = 0; i < N; ++i)
    if (pier[i] != drug[i])
      return false;
  return true;
}

bool sorted_conf(bajt * konf, const Long nr)
{
  //checks if piece number "nr" is on right position within its piece group

  const Long nrgrupy = nr_grupy[nr];

  if (licznosc_grupy[nrgrupy] < 2)
    return true;

  if (pier_w_grupie[nrgrupy] < nr)
    if (konf[nr-1] >= konf[nr])
      return false;

  if (nr < ost_w_grupie[nrgrupy])
    if (konf[nr] >= konf[nr+1])
      return false;

  return true;
}

bool try_rescue(bajt * konf, const Long desired_bucket)
{
  //note: if you don't know to which bucket "konf" should belong, set "desired_bucket" to value less than 0
  Long * board = new Long[R];
  Long success = 0;
  Long lucky_bit = -1;
  Long lucky_bajt = -1;

  for (uLong j = 0; j < 8*sizeof(bajt); ++j)
  {
    const bajt bit = (((bajt)1) << j);
    for (Long i = 0; i < N; ++i)
    {
      konf[i] ^= bit;
      if ((desired_bucket < 0) || (wybierz_koszyk(konf) == desired_bucket))
        if (konf_to_plansza(konf, board))
          if (sorted_conf(konf, i))
          {
            if (success == 0)
              cerr << "\"Rescue\" procedure:\n";
            cerr << "- corrected configuration: ";wypisz_konf_literalnie(konf);cerr<<"\n";
            success++;
            lucky_bit = bit;
            lucky_bajt = i;
          }
      konf[i] ^= bit;
    }
  }

  delete [] board;

  if (!success)
  {
    cerr << "Error: \"Rescue\" procedure didn't help here. This configuration is too much blurred!\n";
    return false;
  } else if (success > 1) {
    cerr << "Error: \"Rescue\" procedure found " << success << " correct configurations. There is no easy way to check which one is really correct.\n";
    return false;
  }

  konf[lucky_bajt] ^= lucky_bit;
  return true;
}

void wypisz_konf(bajt * konf)
{
  Long * plansza = new Long[R];

  if (!konf_to_plansza(konf, plansza))
    {
      cerr << "I can't print a configuration, it is blurred!! Probably you have a hardware problem or it is a bug...\n";
      cerr << "Blurred configuration: ";wypisz_konf_literalnie(konf);cerr<<"\n";
      if (!try_rescue(konf,-1))
      {
        delete [] plansza;
        return;
      }
      else
        konf_to_plansza(konf, plansza);
    }

  for (Long iy = 0; iy < Ry; ++iy)
  {
    for (Long ix = 0; ix < Rx; ++ix)
      if ( plansza[iy * Rx + ix] == 1000000000)
        cout << "X ";
      else if ( plansza[iy * Rx + ix] == -3)
        cout << "Y ";
      else if ( plansza[iy * Rx + ix] == 0)
        cout << ". ";
      else
        cout << plansza[iy * Rx + ix] << " ";
    cout << "\n";
  }

  delete [] plansza;
}

void sortuj_konf(bajt * konf, const Long nr)
{
  //sortuje klocek "nr" w danej "konf", niczym innym sie nie przejmuje

  Long nrgrupy = nr_grupy[nr];
  if (licznosc_grupy[nrgrupy] < 2)
    return;

  Long poz = konf[nr];

  Long lewy = pier_w_grupie[nrgrupy];
  Long nowa_lewa_poz = lewy;

  for (Long i = nr-1; i >= lewy; --i)
    if (konf[i] < poz)
    {
      nowa_lewa_poz = i + 1;
      break;
    }

  if (nowa_lewa_poz < nr)
  {
    for (Long i = nr; i > nowa_lewa_poz; --i)
      konf[i] = konf[i-1];
    konf[nowa_lewa_poz] = poz;
    return;
  }

  Long prawy = ost_w_grupie[nrgrupy];
  Long nowa_prawa_poz = prawy;

  for (Long i = nr+1; i <= prawy; ++i)
    if (poz < konf[i])
    {
      nowa_prawa_poz = i - 1;
      break;
    }

  if (nr < nowa_prawa_poz)
  {
    for (Long i = nr; i < nowa_prawa_poz; ++i)
      konf[i] = konf[i+1];
    konf[nowa_prawa_poz] = poz;
  }

}

bool moze_byc(const Long * plansza, const Long polozenie, const Long nr_klocka)
{
  //sprawdza, czy na danej planszy da sie zmiescic bezkolizyjnie klocka
  if (!wazniejszy[nr_klocka])
  {
    if (plansza[polozenie])
      return false;
    Long * kszt = ksztalt[nr_klocka];
    for (Long i = 0; i < rozmiar[nr_klocka]; ++i)
      if (plansza[ polozenie +kszt[i] ])
        return false;
    return true;
  }
  else
  {
    if (plansza[polozenie] > 0)
      return false;
    Long * kszt = ksztalt[nr_klocka];
    for (Long i = 0; i < rozmiar[nr_klocka]; ++i)
      if (plansza[ polozenie +kszt[i] ] > 0)
        return false;
    return true;
  }
}

void wczytanie_danych(const char * filename)
{
  ifstream inputfile;
  inputfile.open(filename, ifstream::in);
  if (!inputfile.is_open())
  {
    cerr << "I can't open file " << filename << " for reading!\n";
    error_quit();
  }

  inputfile >> Rx;
  inputfile >> Ry;
  R = Rx *Ry;

  //check columns
  if (Rx < 1)
  {
    if (!mytid) cout << "Error: Not enough columns, there should be at least one!\n";
    error_quit();
  }

  //check rows
  if (Ry < 1)
  {
    if (!mytid) cout << "Error: Not enough rows, there should be at least one!\n";
    error_quit();
  }

  //check size of board
  if (R > 127)
  {
    if (!mytid) cout << "Error: Too big board, there should be at most 127 squares!\n";
    error_quit();
  }

  pl_pocz = new Long[R];
  for (Long i = 0; i < R; ++i)
    inputfile >> pl_pocz[i];

  pl_konc = new Long[R];
  for (Long i = 0; i < R; ++i)
    inputfile >> pl_konc[i];

  N = 0;
  for (Long i = 0; i < R; ++i)
    N = max(N, pl_pocz[i]);

  //check if all 'negative' squares are proper
  for (Long i = 0; i < R; ++i)
    if (pl_pocz[i] < -3)
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board has improper value \""
          << pl_pocz[i] << "\"\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if (pl_konc[i] < -3)
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at ending board has improper value \""
          << pl_konc[i] << "\"\n";
      error_quit();
    }

  //check if additional column is neccessary (neccessary condition)
/*  for (Long i = Rx-1; i < R; i += Rx)
    if ( ((pl_pocz[i] >= 0) || (pl_pocz[i] == -2)) && ((pl_konc[i] >= 0) || (pl_konc[i] == -2)) )
    {
      if (!mytid) cout << "Error: Please add column with values: \"-1\" (see manual for details)\n";
      error_quit();
    }
*/
  //check for impossible combinations of blocks
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -1) && (pl_konc[i] > 0) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board is not accesible for any piece but at ending board is occuped by piece #"
          << pl_konc[i] << "\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] > 0) && (pl_konc[i] == -1) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at ending board is not accesible for any piece but at starting board is occuped by piece #"
          << pl_pocz[i] << "\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -1) && (pl_konc[i] == 0) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board is not accesible for any piece but at ending board is empty\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -3) && (pl_konc[i] == 0) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board is not accesible for ordinary pieces but at ending board is empty\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -1) && (pl_konc[i] == -2) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board is not accesible for any piece but at ending board should be empty\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -3) && (pl_konc[i] == -2) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board is not accesible for ordinary pieces but at ending board should be empty\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -3) && (pl_konc[i] == -1) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board is not accesible for ordinary pieces but at ending board is not accesible by any piece\n";
      error_quit();
    }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -1) && (pl_konc[i] == -3) )
    {
      if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
          << " row #" << ((i / Rx)+1) << " at starting board is not accesible for any piece but at ending board is not accesible only by ordinary piece\n";
      error_quit();
    }

  //check for missing pieces 1...N
  for (Long k = 1; k <= N; ++k)
  {
    bool znalazlem = false;
    for (Long i = 0; i < R; ++i)
      if (pl_pocz[i] == k)
        znalazlem = true;
    if (!znalazlem)
    {
      if (!mytid) cout << "Error: I can't find piece #" << k << " at starting board! I am stopping to not waist time\n";
      error_quit();
    }
  }
  //check for additional pieces
  for (Long i = 0; i < R; ++i)
    if (pl_konc[i] > N)
    {
      if (!mytid) cout << "Error: At ending board there is a piece #" << pl_konc[i] << ", but at starting board there is only "
           << N << " pieces! I am stopping to not waist time\n";
      error_quit();
    }

  //check if shapes of pieces are the same at boards
  for (Long k = 1; k <= N; ++k)
  {
    Long positionE = -1;
    for (Long i = 0; i < R; ++i)
      if (pl_konc[i] == k)
      {
        positionE = i;
        break;
      }
    if (positionE >= 0)
    {
      Long sizeS = 0;
      for (Long i = 0; i < R; ++i)
        if (pl_pocz[i] == k)
          sizeS++;
      Long sizeE = 0;
      for (Long i = 0; i < R; ++i)
        if (pl_pocz[i] == k)
          sizeE++;
      if (sizeS != sizeE)
      {
        if (!mytid) cout << "Error: Piece #" << k << " has different shapes at boards!\n";
        error_quit();
      }
      Long positionS = -100000000;
      for (Long i = 0; i < R; ++i)
        if (pl_pocz[i] == k)
        {
          positionS = i;
          break;
        }
      for (Long i = positionS + 1; i < R; ++i)
        if (pl_pocz[i] == k)
        {
          if (i - positionS + positionE < 0)
          {
            if (!mytid) cout << "Error: Piece #" << k << " has different shapes at boards!\n";
            error_quit();
          }
          if (i - positionS + positionE >= R)
          {
            if (!mytid) cout << "Error: Piece #" << k << " has different shapes at boards!\n";
            error_quit();
          }
          if (pl_konc[i - positionS + positionE] != k)
          {
            if (!mytid) cout << "Error: Piece #" << k << " has different shapes at boards!\n";
            error_quit();
          }
        }
    }
  }


  Long * war_temp = new Long[2*R];//z nadmiarem, ze hoho :)
  Long ile_wymuszonych_pustych = 0;
  for (Long i = 0; i < 2*R; ++i)
    war_temp[i] = 0;
  for (Long i = 0; i < R; ++i)
    if (pl_konc[i] > 0)
    {
      if (!war_temp[ pl_konc[i] ])
        war_temp[ pl_konc[i] ] = i+1;//jakos trzeba zobaczyc pole o numerze 0
    }
    else if ( (pl_konc[i] == -2) || (pl_pocz[i] == -2) )
    {
      war_temp[R+ile_wymuszonych_pustych] = i+1;//jakos trzeba zobaczyc, ze pole o numerze 0 ma byc puste
      ile_wymuszonych_pustych++;
    }

  lwar=0;
  for (Long i = 0; i < 2*R; ++i)
    if (war_temp[i])
      lwar++;

  war_nr = new Long[lwar];
  war_pol = new Long[lwar];

  Long it=0;
  for (Long i = 0; i < R; ++i)
    if (war_temp[i])
    {
      war_nr[it] = i-1;//numeracja klockow od zera !
      war_pol[it] = war_temp[i]-1;//numeracja pol od zera
      it++;
    }
  for (Long i = R; i < 2*R; ++i)
    if (war_temp[i])
    {
      war_nr[it] = -1;//wymuszone puste pole
      war_pol[it] = war_temp[i]-1;//numeracja pol od zera
      it++;
    }

  delete [] war_temp;

  wazniejszy = new bool[N];

  for (Long i = 0; i < N; ++i)
    wazniejszy[i] = false;
  for (Long i = 0; i < lwar; ++i)
    if (war_nr[i] >= 0) //dany klocek musi pojawic sie na jakims polu
      wazniejszy[ war_nr[i] ] = true; //tzn. jest wazniejszy od innych, moze chodzic po polach "-3"


   //check for impossible combinations of blocks
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] == -3) && (pl_konc[i] > 0) )
      if ( !wazniejszy[ pl_konc[i]-1 ] )
      {
        if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
            << " row #" << ((i / Rx)+1) << " at starting board is not accesible for ordinary piece but at ending board is occuped by piece #"
            << pl_konc[i] << "\n";
        error_quit();
      }
  for (Long i = 0; i < R; ++i)
    if ( (pl_pocz[i] > 0) && (pl_konc[i] == -3) )
      if ( !wazniejszy[ pl_pocz[i]-1 ] )
      {
        if (!mytid) cout << "Error: Square in column #" << ((i % Rx) + 1)
            << " row #" << ((i / Rx)+1) << " at ending board is not accesible for ordinary piece but at starting board is occuped by piece #"
            << pl_pocz[i] << "\n";
        error_quit();
      }


  for (Long i = 0; i < R; ++i)//zostawiamy tylko obwodke planszy
    if ((pl_konc[i] != -1) && (pl_konc[i] != -3) && (pl_pocz[i] != -1) && (pl_pocz[i] != -3))
      pl_konc[i] = 0;
  for (Long i = 0; i < R; ++i)
    if (pl_pocz[i] == -3)
      pl_konc[i] = -3;
  for (Long i = 0; i < R; ++i)// zmieniamy numery dla pol z obwodki planszy, przydatne dla wazniejszych klockow
    if ( (pl_konc[i] == -1) || (pl_pocz[i] == -1) )
      pl_konc[i] = 1000000000;//tyle klockow na pewno nie ma

  ile_long = (N + sizeof(Long)/sizeof(bajt) - 1) / (sizeof(Long)/sizeof(bajt));

  Long lgrupowan;//liczba grup identycznych klockow
  inputfile >> lgrupowan;

  Long * pier_grup = new Long[lgrupowan];
  Long * ost_grup = new Long[lgrupowan];

  for (Long i = 0; i < lgrupowan; ++i)
  {
      inputfile >> pier_grup[i];
      inputfile >> ost_grup[i];
      pier_grup[i]--;
      ost_grup[i]--;
  }

  M = N;
  for (Long i = 0; i < lgrupowan; ++i)
    M -= ost_grup[i] - pier_grup[i];

  nr_grupy = new Long[N];
  pier_w_grupie = new Long[M];
  ost_w_grupie = new Long[M];
  licznosc_grupy = new Long[M];

  for (Long i = 0; i < N; ++i)
    nr_grupy[i] = i;

  for (Long i = 0; i < lgrupowan; ++i)
  {
    Long nrpier = pier_grup[i];
    Long nrost = ost_grup[i];
    Long nrgrupy = nr_grupy[nrpier];
    for (Long j = nrpier+1; j <= nrost; ++j)
      nr_grupy[j] = nrgrupy;
    for (Long j = nrost+1; j < N; ++j)
      nr_grupy[j] -= nrost - nrpier;
  }

  delete [] pier_grup;
  delete [] ost_grup;

  for (Long i = 0; i < M; ++i)
    for (Long j = 0; j < N; ++j)
      if (nr_grupy[j] == i)
        ost_w_grupie[i] = j;

  for (Long i = 0; i < M; ++i)
    for (Long j = N-1; j >= 0; --j)
      if (nr_grupy[j] == i)
        pier_w_grupie[i] = j;

  for (Long i = 0; i < M; ++i)
    licznosc_grupy[i] = ost_w_grupie[i] - pier_w_grupie[i] + 1;

  rozmiar = new Long[N];
  ksztalt = new Long*[N];

  for (Long i = 0; i < N; ++i)
    rozmiar[i] = -1;
  for (Long i = 0; i < R; ++i)
    if (pl_pocz[i] > 0)
      rozmiar[ pl_pocz[i]-1 ]++;

  for (Long i = 0; i < N; ++i)
    if (rozmiar[i] > 0)
      ksztalt[i] = new Long[ rozmiar[i] ];

  konf_pocz = new bajt[N];

  for (Long i = 0; i < N; ++i)
    rozmiar[i] = -1;
  for (Long i = 0; i < R; ++i)
    if (pl_pocz[i] > 0)
    {
      Long nr = pl_pocz[i] - 1;
      rozmiar[nr]++;
      if (rozmiar[nr] == 0)
        konf_pocz[nr] = i;
      else
        ksztalt[nr][ rozmiar[nr]-1 ] = i - konf_pocz[nr];
    }

  max_przesun = new Long[N];
  for (Long i = 0; i < N; ++i)
    if (rozmiar[i] == 0)//kwadrat 1x1
      max_przesun[i] = R - 1;
    else //cos wiekszego
      max_przesun[i] = R - 1 - ksztalt[i][ rozmiar[i]-1];

  for (Long i = 0; i < N; ++i)//wiele razy, dla pewnosci i by sie nie klopocic
    for (Long j = 0; j < N; ++j)
      sortuj_konf(konf_pocz, j);

  //check if additional column is neccessary (sufficient condition)
  bool sa_3 = false;
  for (Long i = 0; i < R; ++i)
    if (pl_konc[i] == -3)
      sa_3 = true;
  if (sa_3) for (Long i = 0; i < N; ++i) if (wazniejszy[i])
  {
    if (rozmiar[i])
    {
      Long maxx, maxy, minx, miny;

      maxx = konf_pocz[i] % Rx;
      maxy = konf_pocz[i] / Rx;
      minx = maxx;
      miny = maxy;

      for (Long j = 0; j < rozmiar[i]; ++j)
      {
        if ( maxx < ((konf_pocz[i]+ksztalt[i][j]) % Rx) )
          maxx = (konf_pocz[i]+ksztalt[i][j]) % Rx;
        if ( minx > ((konf_pocz[i]+ksztalt[i][j]) % Rx) )
          minx = (konf_pocz[i]+ksztalt[i][j]) % Rx;
        if ( maxy < ((konf_pocz[i]+ksztalt[i][j]) / Rx) )
          maxy = (konf_pocz[i]+ksztalt[i][j]) / Rx;
        if ( miny > ((konf_pocz[i]+ksztalt[i][j]) / Rx) )
          miny = (konf_pocz[i]+ksztalt[i][j]) / Rx;
      }

      maxx -= minx;
      maxy -= miny;
      Long shift = (konf_pocz[i] % Rx) - minx + (konf_pocz[i] / Rx) - miny;
      //assert((konf_pocz[i] / Rx) - miny == 0);

      bool przejdzie = false;
      for (Long y = 0; y < Ry - maxy; ++y)
      {
        Long x = Rx - maxx - 1;
        if (x + y * Rx + 1 + shift > max_przesun[i])
          continue;
        if (!moze_byc(pl_konc, x + y * Rx + shift, i))
          continue;
        if (!moze_byc(pl_konc, x + y * Rx + 1 + shift, i))
          continue;
        przejdzie = true;
        break;
      }
      for (Long y = 1; y < Ry - maxy; ++y)
      {
        Long x = -1;
        if (x + y * Rx + 1 + shift > max_przesun[i])
          continue;
        if (!moze_byc(pl_konc, x + y * Rx + shift, i))
          continue;
        if (!moze_byc(pl_konc, x + y * Rx + 1 + shift, i))
          continue;
        przejdzie = true;
        break;
      }
      if (przejdzie)
      {
        if (!mytid) cout << "Error: Please add column with values: \"-1\" (see manual for details)!\n";
        error_quit();
      }
    }
    else // !rozmiar[i]
    {
      if (!mytid) cout << "Error: Please add column with values: \"-1\" (see manual for details)!\n";
      error_quit();
    }
  }

  inputfile.close();

}

Long wybierz_koszyk(const bajt * konf)
{
  long_bajt unia;
  for (Long i = 0; i < ile_long; ++i)
    unia.longi[i] = 0;
  for (Long i = 0; i < N; ++i)
    unia.bajty[i] = konf[i];

  Long hash = unia.longi[0];
  for (Long i = 1; i < ile_long; ++i)
    hash = hash xor (unia.longi[i] << (2*i+1));

  return abs(hash) % lkoszykow;

}

Long wczytaj_konf(const Long pokolenie, const Long koszyk, bajt * tablica, const Long rozmiar)
{
  FILE * plik;
  char nazwa[30];
  sprintf(nazwa, "temp_%ld/generation_%ld_%ld", koszyk % nproc, pokolenie, koszyk);
  if (!(plik=fopen(nazwa,"rb")))
  {
    cerr << "Error: I can't open file "<<nazwa<<" for reading!\n";
    error_quit();
  }
  Long wczytano = fread(tablica, N, rozmiar, plik);
  if (!feof(plik))
  {
    cerr << "Error: Not enough memory for data from file "<<nazwa<<" !\n";
    error_quit();
  }
  fclose(plik);
  return wczytano;
}

bool rozwiazanie(bajt * konf)
{
  bool rozpakowano = false;
  Long * plansza = 0;

  for (Long i = 0; i < lwar; ++i)
  {
    if (war_nr[i] >= 0)
    {
      if (konf[war_nr[i]] != war_pol[i])
        return false;
    }
    else
    {
      if (!rozpakowano)
      {
        plansza = new Long[R];
        if (! konf_to_plansza(konf, plansza))
        {
          cerr << "I can't check if configuration is a solution, because it's blurred!! Probably you have a hardware problem or this is a bug...\n";
          cerr << "Blurred configuration: ";wypisz_konf_literalnie(konf);cerr<<"\n";
          if (!try_rescue(konf,-1))
          {
            delete [] plansza;
            return false;
          }
          else
            konf_to_plansza(konf, plansza);
        }
        rozpakowano = true;
      }
      if (plansza[war_pol[i]])
      {
        delete [] plansza;
        return false;
      }
    }
  }

  if (rozpakowano)
    delete [] plansza;
  return true;

}

void wypisz_konf_literalnie(const bajt * konf)
{
  for (Long i = 0; i < N; ++i)
    cerr << static_cast<uLong> (konf[i]) << " ";
}

dlugi znajdz_potencjalnych_potomkow(const Long pokolenie, const Long kosz)
{
  max_K = max_k1;
  bajt * K = new bajt[N * max_K];

  Long lrodzicow = wczytaj_konf(pokolenie, kosz, K, max_K);

  if (lrodzicow < 1)
  {
    delete [] K;
    return 0;
  }

  dlugi lpolaczen = 0;

  bajt * potomek = new bajt[N];
  Long * pl_rodzica = new Long[R];
  Long * pl_bez1 = new Long[R];
  bool * pl_zazn = new bool[R];
  Long * stos = new Long[R];

  Long miejsceA = lrodzicow * N;//miejsca dla potomkow
  Long miejsceZ = (max_K - 1) * N;

  koszyki Koszyk(K+miejsceA, K+miejsceZ);

  for (Long i = 0; i < lrodzicow; ++i)
  {
    bajt * rodzic = K + i * N;//konfiguracja rodzica

    if (!konf_to_plansza(rodzic, pl_rodzica))
    {
      cerr << "I can't consider moves from configuration (" << i << " of " << lrodzicow << ") which is blurred!! Probably you have a hardware problem or this is a bug...\n";
      cerr << "Blurred configuration: ";wypisz_konf_literalnie(rodzic);cerr<<"\n";
      if (!try_rescue(rodzic, kosz))
        continue;
      else
        konf_to_plansza(rodzic, pl_rodzica);
    }

    if (wybierz_koszyk(rodzic) != kosz)
    {
      cerr << "Read configuration (" << i << " of " << lrodzicow << ") has wrong basket number. It has: " << wybierz_koszyk(rodzic) << ", but should be " << kosz << "\n";
      cerr << "Blurred configuration: ";wypisz_konf_literalnie(rodzic);cerr<<"\n";
      if (!try_rescue(rodzic, kosz))
        continue;
      else
        konf_to_plansza(rodzic, pl_rodzica);
    }

    for (Long j = 0; j < N; ++j)
    {

      Long nr_klocka = j+1;
      Long pol_oryg = rodzic[j];
      Long max_pol = max_przesun[j];

      for (Long k = 0; k < R; ++k)
        if (pl_rodzica[k] != nr_klocka)
          pl_bez1[k] = pl_rodzica[k];
        else
          pl_bez1[k] = 0;

      for (Long k = 0; k < R; ++k)
        pl_zazn[k] = false;
      pl_zazn[ pol_oryg ] = true;

      Long na_stosie = 0;

      Long pol = pol_oryg-1;
      if (pol >= 0)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      pol = pol_oryg+1;
      if (pol <= max_pol)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      pol = pol_oryg-Rx;
      if (pol >= 0)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      pol = pol_oryg+Rx;
      if (pol <= max_pol)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      while (na_stosie)
      {
        na_stosie--;
        Long pol_akt = stos[na_stosie];

        if (moze_byc(pl_bez1, pol_akt, j))
        {
          kopiuj_konf(rodzic, potomek);
          potomek[j] = pol_akt;
          sortuj_konf(potomek, j);
          Koszyk.dorzuc(potomek);
          lpolaczen++;

          Long pol = pol_akt-1;
          if ( (pol >= 0) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_akt+1;
          if ( (pol <= max_pol) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_akt-Rx;
          if ( (pol >= 0) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_akt+Rx;
          if ( (pol <= max_pol) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }
        }

      }

    }

  }

  Koszyk.close_all();

  delete [] potomek;
  delete [] pl_rodzica;
  delete [] pl_bez1;
  delete [] pl_zazn;
  delete [] stos;

  delete [] K;

  return lpolaczen;
}

dlugi znajdz_potencjalnych_potomkow_Climb24Pro(const Long pokolenie, const Long kosz, const bajt cut)
{
  max_K = max_k1;
  bajt * K = new bajt[N * max_K];

  Long lrodzicow = wczytaj_konf(pokolenie, kosz, K, max_K);

  if (lrodzicow < 1)
  {
    delete [] K;
    return 0;
  }

  dlugi lpolaczen = 0;

  bajt * potomek = new bajt[N];
  Long * pl_rodzica = new Long[R];
  Long * pl_bez1 = new Long[R];
  bool * pl_zazn = new bool[R];
  Long * stos = new Long[R];

  Long miejsceA = lrodzicow * N;//miejsca dla potomkow
  Long miejsceZ = (max_K - 1) * N;

  koszyki Koszyk(K+miejsceA, K+miejsceZ);

  for (Long i = 0; i < lrodzicow; ++i)
  {
    bajt * rodzic = K + i * N;//konfiguracja rodzica

    if (!konf_to_plansza(rodzic, pl_rodzica))
    {
      cerr << "I can't consider moves from configuration (" << i << " of " << lrodzicow << ") which is blurred!! Probably you have a hardware problem or this is a bug...\n";
      cerr << "Blurred configuration: ";wypisz_konf_literalnie(rodzic);cerr<<"\n";
      if (!try_rescue(rodzic, kosz))
        continue;
      else
        konf_to_plansza(rodzic, pl_rodzica);
    }

    if (wybierz_koszyk(rodzic) != kosz)
    {
      cerr << "Read configuration (" << i << " of " << lrodzicow << ") has wrong basket number. It has: " << wybierz_koszyk(rodzic) << ", but should be " << kosz << "\n";
      cerr << "Blurred configuration: ";wypisz_konf_literalnie(rodzic);cerr<<"\n";
      if (!try_rescue(rodzic, kosz))
        continue;
      else
        konf_to_plansza(rodzic, pl_rodzica);
    }

    for (Long j = 0; j < N; ++j)
    {
      /* special code for Climb 24 Pro */
      if ( ((rodzic[4] % 11) < cut) && (j != 4) )
        break;


      Long nr_klocka = j+1;
      Long pol_oryg = rodzic[j];
      Long max_pol = max_przesun[j];

      for (Long k = 0; k < R; ++k)
        if (pl_rodzica[k] != nr_klocka)
          pl_bez1[k] = pl_rodzica[k];
        else
          pl_bez1[k] = 0;

      for (Long k = 0; k < R; ++k)
        pl_zazn[k] = false;
      pl_zazn[ pol_oryg ] = true;

      Long na_stosie = 0;

      Long pol = pol_oryg-1;
      if (pol >= 0)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      pol = pol_oryg+1;
      if (pol <= max_pol)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      pol = pol_oryg-Rx;
      if (pol >= 0)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      pol = pol_oryg+Rx;
      if (pol <= max_pol)
      {
        pl_zazn[pol] = 1;
        stos[na_stosie++] = pol;
      }

      while (na_stosie)
      {
        na_stosie--;
        Long pol_akt = stos[na_stosie];

        if (moze_byc(pl_bez1, pol_akt, j))
        {

          kopiuj_konf(rodzic, potomek);
          potomek[j] = pol_akt;
          sortuj_konf(potomek, j);

          /* special code for Climb 24 Pro */
          if ( (potomek[4] % 11) >= cut) {


          Koszyk.dorzuc(potomek);
          lpolaczen++;


          /* special code for Climb 24 Pro */
          }


          Long pol = pol_akt-1;
          if ( (pol >= 0) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_akt+1;
          if ( (pol <= max_pol) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_akt-Rx;
          if ( (pol >= 0) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_akt+Rx;
          if ( (pol <= max_pol) && (!pl_zazn[pol]))
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }
        }

      }

    }

  }

  Koszyk.close_all();

  delete [] potomek;
  delete [] pl_rodzica;
  delete [] pl_bez1;
  delete [] pl_zazn;
  delete [] stos;

  delete [] K;

  return lpolaczen;
}
void zapisz_konf_pocz()
{
  for (Long i = 0; i < lkoszykow; ++i)
  {
    char nazwa[30];
    sprintf(nazwa, "temp_%ld/generation_0_%ld", i % nproc, i);
    FILE * plik = fopen(nazwa, "wb");
    if (!plik)
    {
      cerr << "Error: I can't create file: "<<nazwa<<" !\n";
      error_quit();
    }
    fclose(plik);
  }

  Long nrkoszyka = wybierz_koszyk(konf_pocz);
  char nazwa[30];
  sprintf(nazwa, "temp_%ld/generation_0_%ld", nrkoszyka % nproc, nrkoszyka);
  FILE * plik = fopen(nazwa, "wb");
  if (!plik)
  {
    cerr << "Error: I can't create file: "<<nazwa<<" !\n";
    error_quit();
  }
  Long ile_zapisano = fwrite(konf_pocz, N, 1, plik);
  Na_dysku.dodano_konf(nrkoszyka, double(ile_zapisano) * N);
  if (ile_zapisano < 1)
  {
    cerr << "Error: I can't write starting configuration to the file!\n";
    error_quit();
  }
  fclose(plik);
  Na_dysku.zmiana_pokolen(false);
  cout << "\nStatistics for every generations: configurations, moves. Total: configurations, moves.\n\n";
  cout << "[0s] Generation = 0 : 1 0 Total: 1 0\n";
}

void utworz_pliki_konf(const Long pokolenie)
{
  for (Long i = 0; i < lkoszykow; ++i)
  {
    char nazwa[30];
    sprintf(nazwa, "temp_%ld/generation_%ld_%ld", i % nproc, pokolenie, i);
    FILE * plik = fopen(nazwa, "wb");
    if (!plik)
    {
      cerr << "Error: I can't create file: "<<nazwa<<" !\n";
      error_quit();
    }
    fclose(plik);
  }
}

void zapisz_konf(const Long pokolenie, const Long koszyk,
                 const bajt * tablica, const Long ilosc)
{
  char nazwa[30];
  sprintf(nazwa, "temp_%ld/generation_%ld_%ld", koszyk % nproc, pokolenie, koszyk);
  FILE * plik = fopen(nazwa, "ab");
  if (!plik)
  {
    cerr << "Error: I can't open file: "<<nazwa<<" !\n";
    error_quit();
  }
  Long ile_zapisano = fwrite(tablica, N, ilosc, plik);
  Na_dysku.dodano_konf(koszyk, double(ile_zapisano) * N);
  if (ile_zapisano < ilosc)
  {
    cerr << "Error: I can't write configurations to file: "<<nazwa<<" !\n";
    error_quit();
  }
  fclose(plik);
}

void zapisz_rozw(const bajt * konf)
{
  FILE * plik = fopen("temp_0/solutions","ab");
  if (!plik)
  {
    cerr << "Error: I can't open file: temp_0/solutions !\n";
    error_quit();
  }
  Long zapisano = fwrite(konf, N, 1, plik);
  Na_dysku.dodano_do_rozwiazan(double(zapisano) * N);
  if (zapisano < 1)
  {
    cerr << "Error: I can't write configurations to file: temp_0/solutions !\n";
    error_quit();
  }
  fclose(plik);
}


dlugi znajdz_potomkow(const Long pokolenie)
{
  dlugi lkonfiguracji = 0;

  max_K = max( min(max_k2, Long(Na_dysku.do_znajdowania_potomkow() / N + 3)), Long(10) );
  bajt * K = new bajt[N * max_K];

  hashe Hash(max_K, min(nadmiar2, max_K));
  for (Long i = mytid; i < lkoszykow; i+=nproc)
  {
    Long lrodzicow = 0;
    if (pokolenie > 0)
      lrodzicow = wczytaj_konf(pokolenie-1, i, K, max_K);
    lrodzicow += wczytaj_konf(pokolenie  , i, K + lrodzicow * N, max_K - lrodzicow);
    lrodzicow += wczytaj_konf(pokolenie+1, i, K + lrodzicow * N, max_K - lrodzicow);

    Hash.wyczysc();
    for (Long j = 0; j < lrodzicow; ++j)
      Hash.dodaj_konf(K + j * N);

    bajt * Kpoz = K + lrodzicow * N;//pozycja pierwszego potencjalnego potomka
    Long wolne_miejsce = max_K - lrodzicow;//ile miejsca dla potomkow
    if (wolne_miejsce <= 0)
    {
      cerr << "Error: Not enough memory for new configurations!\n";
      error_quit();
    }

    Long lkonf = 0;
    for (Long tid = 0; tid < nproc; tid++)
    {
      Long porcja;
      wczytujemy_koszyki Wczytaj_koszyk(tid, i);

      do
      {
        porcja = Wczytaj_koszyk.wczytaj_porcje(Kpoz, wolne_miejsce);
#ifdef BE_VERBOSE
        if (porcja)
          cerr << "I have read "<<porcja<<" potentialy new configurations from file\n";
#endif
        bajt * potomek = Kpoz;
        for (Long j = 0; j < porcja; ++j)
        {
          if (wybierz_koszyk(potomek) != i)
          {
            cerr << "Read configuration (" << j << " of " << porcja << ") has wrong basket number. It has: " << wybierz_koszyk(potomek) << ", but should be " << i << "\n";
            cerr << "Blurred configuration: ";wypisz_konf_literalnie(potomek);cerr<<"\n";
            if (!try_rescue(potomek, i))
            {
              potomek += N;
              continue;
            }
          }
          if (Hash.nowa_konfiguracja(potomek))
          {

            if (odl_rozw > pokolenie)
              if (rozwiazanie(potomek))
              {
                odl_rozw = pokolenie+1;
                cout << "I have found a solution (" << odl_rozw << " moves):\n";
                wypisz_konf(potomek);
                zapisz_rozw(potomek);
              }

            kopiuj_konf(potomek, Kpoz);
            Hash.dodaj_konf(Kpoz);
            Kpoz += N;
            lkonf++;
            wolne_miejsce--;
            if (wolne_miejsce == 0)
            {
              cerr << "Error: Not enough memory for new configurations!\n";
              error_quit();
            }
          }
          potomek += N;
        }
      } while (porcja);

    }

    for (Long tid = 0; tid < nproc; ++tid)
      wyczysc_plik_koszyka(tid, i);


    zapisz_konf(pokolenie+1, i, K + lrodzicow * N, lkonf);
    lkonfiguracji += lkonf;

#ifndef NO_STATISTICS
    stringstream outstr;
    if (nproc>1)
      outstr << "Process #" << mytid << " ";
    outstr << "Basket #" << i << ": Hash table: total space: " << max_K
           << " filled in "<< (100.0 * (lrodzicow+lkonf))/max_K
           << "%, way out: " << Hash.rzeczywisty_nadmiar()*100 << "%\n";
    cerr << outstr.str();
#endif

  }

  delete [] K;

  for (Long i = 0; i < lkoszykow; ++i)
    Na_dysku.skasowano_koszyk(i);

  return lkonfiguracji;

}

dlugi znajdz_potomkow_Climb24Pro (const Long pokolenie, const bajt cut)
{
  dlugi lkonfiguracji = 0;

  max_K = max( min(max_k2, Long(Na_dysku.do_znajdowania_potomkow() / N + 3)), Long(10) );
  bajt * K = new bajt[N * max_K];

  hashe Hash(max_K, min(nadmiar2, max_K));
  for (Long i = mytid; i < lkoszykow; i+=nproc)
  {
    Long lrodzicow = 0;
    if (pokolenie > 0)
      lrodzicow = wczytaj_konf(pokolenie-1, i, K, max_K);
    lrodzicow += wczytaj_konf(pokolenie  , i, K + lrodzicow * N, max_K - lrodzicow);
    lrodzicow += wczytaj_konf(pokolenie+1, i, K + lrodzicow * N, max_K - lrodzicow);

    Hash.wyczysc();
    for (Long j = 0; j < lrodzicow; ++j)
      /* special code for Climb 24 Pro */
      if ( (K[j * N + 4] % 11) >= cut )


      Hash.dodaj_konf(K + j * N);

    bajt * Kpoz = K + lrodzicow * N;//pozycja pierwszego potencjalnego potomka
    Long wolne_miejsce = max_K - lrodzicow;//ile miejsca dla potomkow
    if (wolne_miejsce <= 0)
    {
      cerr << "Error: Not enough memory for new configurations!\n";
      error_quit();
    }

    Long lkonf = 0;
    for (Long tid = 0; tid < nproc; tid++)
    {
      Long porcja;
      wczytujemy_koszyki Wczytaj_koszyk(tid, i);

      do
      {
        porcja = Wczytaj_koszyk.wczytaj_porcje(Kpoz, wolne_miejsce);
#ifdef BE_VERBOSE
        if (porcja)
          cerr << "I have read "<<porcja<<" potentialy new configurations from file\n";
#endif
        bajt * potomek = Kpoz;
        for (Long j = 0; j < porcja; ++j)
        {
          if (wybierz_koszyk(potomek) != i)
          {
            cerr << "Read configuration (" << j << " of " << porcja << ") has wrong basket number. It has: " << wybierz_koszyk(potomek) << ", but should be " << i << "\n";
            cerr << "Blurred configuration: ";wypisz_konf_literalnie(potomek);cerr<<"\n";
            if (!try_rescue(potomek, i))
            {
              potomek += N;
              continue;
            }
          }
          if (Hash.nowa_konfiguracja(potomek))
          {

            if (odl_rozw > pokolenie)
              if (rozwiazanie(potomek))
              {
                odl_rozw = pokolenie+1;
                cout << "I have found a solution (" << odl_rozw << " moves):\n";
                wypisz_konf(potomek);
                zapisz_rozw(potomek);
              }

            kopiuj_konf(potomek, Kpoz);
            Hash.dodaj_konf(Kpoz);
            Kpoz += N;
            lkonf++;
            wolne_miejsce--;
            if (wolne_miejsce == 0)
            {
              cerr << "Error: Not enough memory for new configurations!\n";
              error_quit();
            }
          }
          potomek += N;
        }
      } while (porcja);

    }

    for (Long tid = 0; tid < nproc; ++tid)
      wyczysc_plik_koszyka(tid, i);


    zapisz_konf(pokolenie+1, i, K + lrodzicow * N, lkonf);
    lkonfiguracji += lkonf;

#ifndef NO_STATISTICS
    stringstream outstr;
    if (nproc>1)
      outstr << "Process #" << mytid << " ";
    outstr << "Basket #" << i << ": Hash table: total space: " << max_K
           << " filled in "<< (100.0 * (lrodzicow+lkonf))/max_K
           << "%, way out: " << Hash.rzeczywisty_nadmiar()*100 << "%\n";
    cerr << outstr.str();
#endif

  }

  delete [] K;

  for (Long i = 0; i < lkoszykow; ++i)
    Na_dysku.skasowano_koszyk(i);

  return lkonfiguracji;

}


void sciezka_rozw(const bajt * konf, const Long pokolenie_)
{
  const Long co_i_z = co_ile_zostaw;
  if (co_ile_zostaw < 5)
    co_ile_zostaw = 12;

  Long pokolenie = pokolenie_;
  Long szukaj_do_pokolenia = co_ile_zostaw * ((pokolenie-1) / co_ile_zostaw);

  max_K = max_k3;
  bajt * K = new bajt[N * max_K];
  Long * nr_rodzica = new Long[max_K];
  bajt * sciezka = new bajt[(pokolenie_+1) * N];//kolejne konfiguracje na sciezce rozwiazania

  bajt * od_niej_zaczniemy = new bajt[N];
  kopiuj_konf(konf, od_niej_zaczniemy);

  while (pokolenie > 0)
  {

    kopiuj_konf(od_niej_zaczniemy, K);

    Long nr_pierwszego;
    Long nr_wierzch = 1;
    Long nr_pierwszego_wolnego = 0;

    nr_rodzica[0] = -1;

    hashe Hash(max_K, min(nadmiar3, max_K));
    Hash.wyczysc();
    Hash.dodaj_konf(K);

    for (Long pok = pokolenie; pok > szukaj_do_pokolenia; pok--)
    {
      nr_pierwszego = nr_pierwszego_wolnego;
      nr_pierwszego_wolnego = nr_wierzch;

      bajt * potomek = new bajt[N];
      Long * pl_rodzica = new Long[R];
      Long * pl_bez1 = new Long[R];
      bool * pl_zazn = new bool[R];
      Long * stos = new Long[R];

      //znajdujemy potomkow
      for (Long nr = nr_pierwszego; nr < nr_pierwszego_wolnego; nr++)
      {

        bajt * rodzic = K + nr * N;//konfiguracja rodzica
        if (!konf_to_plansza(rodzic, pl_rodzica))
        {
          cerr << "I can't consider moves from configuration which is blurred!! Probably you have a hardware problem or this is a bug...\n";
          cerr << "Blurred configuration: ";wypisz_konf_literalnie(rodzic);cerr<<"\n";
          if (!try_rescue(rodzic, -1))
            continue;
          else
            konf_to_plansza(rodzic, pl_rodzica);
        }

        for (Long j = 0; j < N; ++j)
        {

          Long nr_klocka = j+1;
          Long pol_oryg = rodzic[j];
          Long max_pol = max_przesun[j];

          for (Long k = 0; k < R; ++k)
            if (pl_rodzica[k] != nr_klocka)
              pl_bez1[k] = pl_rodzica[k];
            else
              pl_bez1[k] = 0;

          for (Long k = 0; k < R; ++k)
            pl_zazn[k] = false;
          pl_zazn[ pol_oryg ] = true;

          Long na_stosie = 0;

          Long pol = pol_oryg-1;
          if (pol >= 0)
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_oryg+1;
          if (pol <= max_pol)
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_oryg-Rx;
          if (pol >= 0)
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          pol = pol_oryg+Rx;
          if (pol <= max_pol)
          {
            pl_zazn[pol] = 1;
            stos[na_stosie++] = pol;
          }

          while (na_stosie)
          {
            na_stosie--;
            Long pol_akt = stos[na_stosie];

            if (moze_byc(pl_bez1, pol_akt, j))
            {
              kopiuj_konf(rodzic, potomek);
              potomek[j] = pol_akt;
              sortuj_konf(potomek, j);
              Long pol = pol_akt-1;
              if ( (pol >= 0) && (!pl_zazn[pol]))
              {
                pl_zazn[pol] = 1;
                stos[na_stosie++] = pol;
              }

              pol = pol_akt+1;
              if ( (pol <= max_pol) && (!pl_zazn[pol]))
              {
                pl_zazn[pol] = 1;
                stos[na_stosie++] = pol;
              }

              pol = pol_akt-Rx;
              if ( (pol >= 0) && (!pl_zazn[pol]))
              {
                pl_zazn[pol] = 1;
                stos[na_stosie++] = pol;
              }

              pol = pol_akt+Rx;
              if ( (pol <= max_pol) && (!pl_zazn[pol]))
              {
                pl_zazn[pol] = 1;
                stos[na_stosie++] = pol;
              }

              if (Hash.nowa_konfiguracja(potomek))
              {
                kopiuj_konf(potomek, K + nr_wierzch * N);
                Hash.dodaj_konf(K + nr_wierzch * N);
                nr_rodzica[nr_wierzch] = nr;
                nr_wierzch++;
                if (nr_wierzch == max_K)
                {
                  cerr << "Error: Not enough memory for new configurations. Please increase number of generations stored on disc, or maybe increase available RAM!\n";
                  error_quit();
                }
              }
            }
          }
        }
      }
      delete [] potomek;
      delete [] pl_rodzica;
      delete [] pl_bez1;
      delete [] pl_zazn;
      delete [] stos;
      //koniec znajdowania potomkow

    }

    //wsrod otrzymanych konf. szukamy ktorejkolwiek, ktora jest na dysku
    bool znalazlem = false;
    for (Long k = 0; k < lkoszykow; ++k)
    {
      if (znalazlem)
        break;
      Long porcja;
      wczytujemy_konfiguracje Wczytaj_konf(szukaj_do_pokolenia,k);
      bajt * Kpoz = K + nr_wierzch * N;//pozycja pierwszego potencjalnego potomka
      Long wolne_miejsce = max_K - nr_wierzch;//ile miejsca dla potomkow
      if (wolne_miejsce <= 0)
      {
        cerr << "Error: Not enough memory for comparing configurations with stored on disc. Please increase number of generations stored on disc, or maybe increase available RAM!\n";
        error_quit();
      }

      do
      {
        porcja = Wczytaj_konf.wczytaj_porcje(Kpoz, wolne_miejsce);
#ifdef BE_VERBOSE
        if (porcja)
          cerr << "I have read "<<porcja<<" potentialy new configurations\n";
#endif
        bajt * rodzic = Kpoz;
        for (Long j = 0; j < porcja; ++j)
        {

          bajt * stara_konf;

          if ((stara_konf = Hash.stara_konfiguracja(rodzic)))
          {
            znalazlem = true;
            Long nr_konf = static_cast<Long>( (reinterpret_cast<uLong>(stara_konf) - reinterpret_cast<uLong>(K)) / static_cast<uLong>(N) );

            kopiuj_konf(stara_konf, od_niej_zaczniemy);

            Long aktualne_pokolenie = szukaj_do_pokolenia;
            while (nr_konf >=0)
            {
              kopiuj_konf(K + nr_konf * N, sciezka + aktualne_pokolenie * N);
              nr_konf = nr_rodzica[nr_konf];
              aktualne_pokolenie++;
            }

            break;
          }

          rodzic += N;

        }
      } while (porcja && (!znalazlem));
    }

    pokolenie = szukaj_do_pokolenia;
    szukaj_do_pokolenia -= co_ile_zostaw;
  }

  //wypisujemy sciezke rozwiazania
  for (Long i = 0; i <= pokolenie_; ++i)
  {
    cout << "Generation number "<<i<<"\n";
    wypisz_konf(sciezka+i*N);
  }

  delete [] nr_rodzica;
  delete [] sciezka;
  delete [] od_niej_zaczniemy;
  delete [] K;

  co_ile_zostaw = co_i_z;
}

void wypisz_rozwiazania(){
  FILE * plik = fopen("temp_0/solutions","rb");
  if (!plik)
  {
    cerr << "Error: I can't open file temp_0/solutions for reading!\n";
    error_quit();
  }

  bajt * konf = new bajt[N];

  Long wczytano = fread(konf, N, 1, plik);
  while (wczytano == 1)
  {
    cout << "\nPath to the shortest solution:\n\n";
    sciezka_rozw(konf, odl_rozw);
    wczytano = fread(konf, N, 1, plik);
  }
  delete [] konf;

  fclose(plik);
}

void wypisz_konf_koncowe(const Long pokolenie){
  for (Long nr_koszyka = 0; nr_koszyka < lkoszykow; ++nr_koszyka)
  {
    char nazwa[30];
    sprintf(nazwa, "temp_%ld/generation_%ld_%ld", nr_koszyka % nproc, pokolenie, nr_koszyka);
    FILE * plik = fopen(nazwa,"rb");
    if (!plik)
    {
      cerr << "Error: I can't open file "<<nazwa<<" for reading!\n";
      error_quit();
    }

    bajt * konf = new bajt[N];

    Long wczytano = fread(konf, N, 1, plik);
    while (wczytano == 1)
    {
      cout << "\nPaths to the furthest configurations:\n\n";
      sciezka_rozw(konf, pokolenie);
      wczytano = fread(konf, N, 1, plik);
    }
    delete [] konf;

    fclose(plik);
  }
}


void usun_pliki_konf(const Long pokolenie)
{
  for (Long i = 0; i < lkoszykow; ++i)
  {
    char nazwa[30];
    sprintf(nazwa, "temp_%ld/generation_%ld_%ld", i % nproc, pokolenie, i);
    remove(nazwa);
  }
}

void usun_pliki_koszykow()
{
  for (Long tid = 0; tid < nproc; ++tid)
    for (Long i = 0; i < lkoszykow; ++i)
    {
      char nazwa[30];
      sprintf(nazwa, "temp_%ld/basket_%ld", mytid, i);
      remove(nazwa);
    }
}

void make_directories()
{
    char command[30];
    for (Long i = 0; i < nproc; ++i)
    {
      sprintf(command, "mkdir temp_%ld", i);
      int result = system(command);
      if (!result)
      { } // it is almost impossible to say if something went wrong...
    }
}

void remove_directories()
{
    char command[30];
    for (Long i = 0; i < nproc; ++i)
    {
      sprintf(command, "rmdir temp_%ld", i);
      int result = system(command);
      if (!result)
      { } // it is almost impossible to say if something went wrong...
    }
}

void wyczysc_plik_koszyka(const Long tid, const Long ktory)
{
    char nazwa[30];
    sprintf(nazwa, "temp_%ld/basket_%ld", tid, ktory);
    FILE * plik = fopen(nazwa, "wb");
    if (!plik)
    {
      cerr << "Error: I can't erase file: " << nazwa << " !\n";
      error_quit();
    }
    fclose(plik);
}

void wyczysc_pliki_koszykow(const Long tid)
{
  for (Long i = 0; i < lkoszykow; ++i)
    wyczysc_plik_koszyka(tid, i);
}

bool lpierwsza(const Long liczba){
  if (liczba < 2) return false;
  if (liczba < 4) return true;
  if (!(liczba % 2)) return false;
  for (Long i = 3; i <= Long(sqrt(double(liczba))); i+=2)
    if (!(liczba % i)) return false;
  return true;
}

Long obok_lpierwsza(const Long liczba)
{
  if (liczba < 2)
    return 1;
  Long i = liczba;
  while (! lpierwsza(i++)) ;
  return --i;
}

Long rozmiar_pliku_konf(const Long pokolenie, const Long nr_koszyka)
{
  if (pokolenie < 0)
    return 0;

  FILE * plik;
  char nazwa[30];
  sprintf(nazwa, "temp_%ld/generation_%ld_%ld", nr_koszyka % nproc, pokolenie, nr_koszyka);

  plik = fopen (nazwa,"rb");

  if (!plik)
  {
    cerr <<  "Error: I can't open file "<<nazwa<<" in order to check its size!\n";
    error_quit();
  }

  fseek(plik, 0, SEEK_END);
  Long rozmiar = ftell(plik);
  fclose(plik);

  return rozmiar;
}

void ustal_prawdziwe_miejsce_na_dysku(const Long pokolenie_do_restartu)
{
  for (Long pok = pokolenie_do_restartu-2; pok < pokolenie_do_restartu; ++pok)
  {
    for (Long i = 0; i < lkoszykow; i++)
      Na_dysku.dodano_konf(i, double(rozmiar_pliku_konf(pok, i)));
    Na_dysku.zmiana_pokolen(false);
  }
}

void write_end_conditions()
{
  Long * plansza = new Long[R];
  for (Long i = 0; i < R; ++i)
    plansza[i] = pl_konc[i];

  for (Long i = 0; i < lwar; ++i)
  {
    Long number = war_nr[i];
    if (number >= 0)
    {
      Long position = war_pol[i];
      plansza[ position ] = number + 1;
      for (Long j = 0; j < rozmiar[number]; ++j)
        plansza[ position + ksztalt[number][j] ] = number + 1;
    }
    else
      plansza[war_pol[i]] = -2;
  }

  for (Long iy = 0; iy < Ry; ++iy)
  {
    for (Long ix = 0; ix < Rx; ++ix)
      if ( plansza[iy * Rx + ix] == 1000000000)
        cout << "X ";
      else if ( plansza[iy * Rx + ix] == -3)
        cout << "Y ";
      else if ( plansza[iy * Rx + ix] == -2)
        cout << "Z ";
      else if ( plansza[iy * Rx + ix] == 0)
        cout << ". ";
      else
        cout << plansza[iy * Rx + ix] << " ";
    cout << "\n";
  }

  cout << "\nWhere:\n";
  cout << "X - field not accesible for any piece\n";
  cout << "Y - field not accesible for ordinary piece ('ordinary' means 'not listed in ending conditions')\n";
  cout << "Z - field must be empty at ending position\n\n";

  delete [] plansza;
}

int main(int argc, char *argv[])
{

#ifdef USE_MPI
  MPI_Init(&argc, &argv);
  int mytid_, nproc_;
  MPI_Comm_rank(MPI_COMM_WORLD, &mytid_);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc_);
  mytid = mytid_;
  nproc = nproc_;

/*trap for MPI debugging
    Long ijkl = 0;
    printf("MyTaskID %ld: PID %d ready for attach\n", mytid, getpid());
    fflush(stdout);
    while (!ijkl)
      sleep(5);
end of trap*/

#endif

  if ( (sizeof(Long) < sizeof(char *)) || (sizeof(char) > 1) )
  {
    if (!mytid) cout << "Critical error: Wrong compilation!!\n";
    return 1;
  }

  time_t rawtime_0;
  time ( &rawtime_0 );

  if (!mytid) cout << "Klotski Solver v" << version << " Copyright (C) 2009 Daniel Blazewicz\n";
  if (!mytid) cout << "This program comes with ABSOLUTELY NO WARRANTY. This is free software,\n";
  if (!mytid) cout << "and you are welcome to redistribute it under certain conditions.\n";
  if (!mytid) cout << "Please see license.txt for details.\n\n";

  generate_crc_table();

  if (argc < 7) {
    if (!mytid) cout << "Obligatory parameters (in order):\n";
    if (!mytid) cout << " - Available RAM memory in MiB\n";
    if (!mytid) cout << " - Available disc space in GiB\n";
    if (!mytid) cout << " - Number of baskets (configurations are splited into baskets to save memory)\n";
    if (!mytid) cout << " - Excess of hashes in range [0.0, 1.0] (recommended value: 0.38)\n";
    if (!mytid) cout << " - How seldom hold generations on disc? (it means: only one per ... generation)\n";
    if (!mytid) cout << " - File with input data\n";
    if (!mytid) cout << "\nAdditional parameters for restart of calculations:\n";
    if (!mytid) cout << " - Non-calculated generation\n";
    if (!mytid) cout << " - Optional: 1 - Solutions were found previously, 0 - Search solutions (default)\n";
    if (!mytid) cout << "\nRemarks:\n";
    if (!mytid) cout << " - Results are written to standard output\n";
    if (!mytid) cout << " - Statistics are written to standard error\n\n";
    return 0;
  }

  if (!mytid) cout << "Parameters of program execution:\n";
  if (!mytid) cout << " - Available RAM memory in megabytes: "<<argv[1]<<"\n";
  if (!mytid) cout << " - Available disc space in gigabytes: "<<argv[2]<<"\n";
  if (!mytid) cout << " - Number of baskets: "<<argv[3]<<"\n";
  if (!mytid) cout << " - Excess of hashes: "<<argv[4]<<"\n";
  if (!mytid) cout << " - How seldom hold generations on disc: "<<argv[5]<<"\n";
  if (!mytid) cout << " - File with input data: "<<argv[6]<<"\n";

  Long restart = false, nr_do_restartu = -1;
  if (argc >= 8)
  {
    nr_do_restartu = atol(argv[7]);
    if (nr_do_restartu > 0)
    {
      restart = true;
      if (!mytid) cout << " - Restart from generation number: "<<nr_do_restartu<<"\n";
    }
  }

  if (argc >= 9)
  {
     if (atol(argv[8]) == 1) {
       odl_rozw = -2;
       if (!mytid) cout << " - Don't search solutions, they were found earlier\n";
     }
  }

  if (!mytid) cout << "\n";

  wczytanie_danych(argv[6]);

  if (!mytid) cout << "Starting configuration:\n";
  if (!mytid) wypisz_konf(konf_pocz);
  if (!mytid) cout << "\nEnding conditions:\n";
  if (!mytid) write_end_conditions();

  if (!mytid) make_directories();
#ifdef USE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif

  double pamiec_wMB = atof(argv[1]);

  if (pamiec_wMB <= 2.0 * nproc)
  {
    if (!mytid) cout << "To small memory declaration! Program takes about 2 MiB RAM by itself\n";
    if ( (!mytid) && (nproc > 1) ) cout << "You have started "<< nproc << " processes in parallel, so you need to declare at least "<<2*nproc<< " megabytes of RAM\n";
    return 1;
  }

  if ( (1024.0 * 1024.0 * (pamiec_wMB - 2.0 * nproc)) > Long_MAX )
  {
    if (!mytid) cout << "To big memory declaration!\n";
    return 1;
  }
  Long pamiec_wB = Long (1024.0 * 1024.0 * (pamiec_wMB - 2.0 * nproc));

  max_k1 = obok_lpierwsza(pamiec_wB / N / nproc); //for finding potentially new configurations
  max_k2 = obok_lpierwsza(Long( pamiec_wB / (N + (sizeof(bajt*)+sizeof(Long)) * (1.0+atof(argv[4]))) ) / nproc); //for finding really new configurations
  //important for 32-bits operating systems, max allocatable RAM is 4GiB
  if ( (sizeof(Long) <= 4) && (pamiec_wMB >= 4.0 * 1024.0) )
    max_k3 = obok_lpierwsza(Long( 4.0 * 1024.0 * 1024.0 * 1024.0 / (N + (sizeof(bajt*)+sizeof(Long)) * (1.0+atof(argv[4])) + sizeof(Long)) )); //for finding path to solutions
  else
    max_k3 = obok_lpierwsza(Long( pamiec_wB / (N + (sizeof(bajt*)+sizeof(Long)) * (1.0+atof(argv[4])) + sizeof(Long)) )); //for finding path to solutions

  if ( (max_k3 < 100) || (max_k2 < 100) )
  {
    if (!mytid) cerr << "Error: To small memory declaration!!\n";
    error_quit();
  }

  nadmiar2 = Long(atof(argv[4]) * max_k2);
  nadmiar3 = Long(atof(argv[4]) * max_k3);

  if ( (!mytid) && (nproc > 1) ) cout << "Run in parallel: "<< nproc << " processes\n";
  if (!mytid) cout << "I will reserve memory for max. " << max_k1 << " configurations,\n";
  if (!mytid) cout << "or for max. " << max_k2 << ", if table of hashes is for " << nadmiar2
       << " (" << 100*atof(argv[4]) << "%) of additional configurations\n";

  if (!mytid) cout << "I will use max. " << atof(argv[2]) << " GiB of disc space\n";

  lkoszykow = obok_lpierwsza(atol(argv[3]));
  if (lkoszykow < nproc)
    lkoszykow = obok_lpierwsza(nproc);

  if (!mytid) cout << "I will split configurations into " << lkoszykow << " baskets\n";

  co_ile_zostaw = atol(argv[5]);
  if (!mytid) cout << "I will store on disc only one per "<< co_ile_zostaw << " generations of configurations\n";

  if (restart)
  {
    if (!mytid) cout << "Restart from "<<nr_do_restartu<<". generation\n";
#ifndef NO_STATISTICS
    if (!mytid) cerr << "Restart from "<<nr_do_restartu<<". generation\n";
#endif
  }

  Na_dysku.inicjuj(lkoszykow, atof(argv[2]) * 1024.0 * 1024.0 * 1024.0);

  Long pokolenie;

  if (!restart)
  {
    if (!mytid) zapisz_konf_pocz();
    pokolenie = 0;
  }
  else
  {
    ustal_prawdziwe_miejsce_na_dysku(nr_do_restartu);
#ifndef NO_STATISTICS
    if (!mytid) cerr << "It is restart, real free disc space: "<<
            Na_dysku.wolne_miejsce()<<" bytes\n";;
#endif
    pokolenie = nr_do_restartu-1;
  }

  dlugi lpolaczen = 0;
  dlugi liczba_starych_polaczen = 0;
  dlugi lkonfiguracji = 1;
  dlugi liczba_nowych_potomkow = 0;
  dlugi liczba_starych_potomkow = 1;
  wyczysc_pliki_koszykow(mytid);
  do
  {
    liczba_nowych_potomkow = 0;
    utworz_pliki_konf(pokolenie+1);

    Long nr_koszyka = mytid;//each process takes own baskets
    Long l_koszykow_na_dysku = 0;
    double miejsce_na_dysku_przed = Na_dysku.wolne_miejsce();
    //if (!mytid) cout << "Miejsce na dysku przed: " << miejsce_na_dysku_przed << "\n";

#ifdef USE_MPI
    if (nproc == 1)
    {
#endif
      while (nr_koszyka < lkoszykow)
      {
#ifndef NO_STATISTICS
          cerr << "I am considering basket #"<<nr_koszyka<<"\n";
#endif
          lpolaczen += znajdz_potencjalnych_potomkow(pokolenie, nr_koszyka);
          nr_koszyka += nproc;
          l_koszykow_na_dysku += nproc;

          double miejsce_na_dysku_po = Na_dysku.wolne_miejsce();

          double ile_mozna = (l_koszykow_na_dysku * miejsce_na_dysku_po) / (miejsce_na_dysku_przed - miejsce_na_dysku_po + 1) - 1;
          if (ile_mozna > lkoszykow)
            ile_mozna = lkoszykow;
          Long ile_mozna_koszykow = Long(ile_mozna);
          if ( (ile_mozna_koszykow < 0) && (l_koszykow_na_dysku == 1) )
          {
            if (!mytid) cerr << "Not enough disc space! Please increase disc space or decrease number of stored generations!\n";
            error_quit();
          }

#ifndef NO_STATISTICS
        if (ile_mozna_koszykow < lkoszykow-nr_koszyka)
        {
          cerr << "I need to write potentialy new configurations derived from " << lkoszykow-nr_koszyka
                 << " baskets, but on disc there is a place for only about " << ile_mozna_koszykow
                 << ". You should increase disc space or decrease number of stored generations!\n";
        }
#endif

        if ((ile_mozna_koszykow < 1) || (nr_koszyka == lkoszykow))
        {
          liczba_nowych_potomkow += znajdz_potomkow(pokolenie);
          l_koszykow_na_dysku = 0;
          miejsce_na_dysku_przed = Na_dysku.wolne_miejsce();
        }

      }
      lkonfiguracji += liczba_nowych_potomkow;
#ifdef USE_MPI
  }
  else // it means: nproc > 1
  {

    MPI_Barrier(MPI_COMM_WORLD);
    dlugi liczba_nowych_polaczen = 0;

    while (nr_koszyka < lkoszykow)
    {

#ifndef NO_STATISTICS
      stringstream outstr;
      outstr << "Process #"<< mytid << ": I am considering basket number: "<<nr_koszyka<<"\n";
      cerr << outstr.str();
#endif
      liczba_nowych_polaczen += znajdz_potencjalnych_potomkow(pokolenie, nr_koszyka);
      nr_koszyka += nproc;

      Long ile_mozna_koszykow;
      if (!mytid)
      {
        double miejsce_na_dysku_po = Na_dysku.wolne_miejsce();
        //if (!mytid) cout << "Miejsce na dysku po: " << miejsce_na_dysku_po << "\n";

        double ile_mozna = miejsce_na_dysku_po / (miejsce_na_dysku_przed - miejsce_na_dysku_po + 1.0) - 1.0 - (nproc-1.0);// "- 1.0" should be: - nproc * size(potentially new confs.)/size(really new confs.)
        if (ile_mozna > lkoszykow)
          ile_mozna = lkoszykow;
        if (ile_mozna < 0)
          ile_mozna = 0;
        ile_mozna_koszykow = Long(ile_mozna);
        if ( (ile_mozna_koszykow < lkoszykow) && (ile_mozna_koszykow > 0) )
          ile_mozna_koszykow = (ile_mozna_koszykow / nproc) * nproc;
      }
      MPI_Bcast(&ile_mozna_koszykow, 1, MPI_LONG, 0, MPI_COMM_WORLD);
      if (ile_mozna_koszykow == 0)
      {
        if (!mytid) cerr << "Not enough disc space! Please increase disc space or decrease number of stored generations!\n";
        error_quit();
      }

#ifndef NO_STATISTICS
      if (lkoszykow-nr_koszyka > 0)
        if (!mytid) if (ile_mozna_koszykow < lkoszykow-nr_koszyka)
        {
          stringstream outstr;
          outstr << "I need to write potentialy new configurations derived from " << lkoszykow-nr_koszyka
                 << " baskets, but on disc there is a place for only about " << ile_mozna_koszykow
                 << ". You should increase disc space or decrease number of stored generations!\n";
          cerr << outstr.str();
        }
#endif

      Long ost_nr_koszyka = nr_koszyka+ile_mozna_koszykow-mytid;
      if (ost_nr_koszyka > lkoszykow)
        ost_nr_koszyka = lkoszykow;
      while(nr_koszyka<ost_nr_koszyka)
      {
#ifndef NO_STATISTICS
        stringstream outstr;
        outstr << "Process #"<< mytid << ": I am considering basket number: "<<nr_koszyka<<"\n";
        cerr << outstr.str();
#endif
        liczba_nowych_polaczen += znajdz_potencjalnych_potomkow(pokolenie, nr_koszyka);
        nr_koszyka+=nproc;
      }

      Na_dysku.synchronizuj_koszyki();
      liczba_nowych_potomkow += znajdz_potomkow(pokolenie);
      Na_dysku.synchronizuj_konf();

      miejsce_na_dysku_przed = Na_dysku.wolne_miejsce();

      if ( (nr_koszyka >= lkoszykow) && (ost_nr_koszyka < lkoszykow) )
      {
#ifndef NO_STATISTICS
        stringstream outstr;
        outstr << "Process #"<< mytid << ": I am waiting in silence\n";
        cerr << outstr.str();
#endif
        MPI_Bcast(&ile_mozna_koszykow, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        Na_dysku.synchronizuj_koszyki();
        liczba_nowych_potomkow += znajdz_potomkow(pokolenie);
        Na_dysku.synchronizuj_konf();
      }
    }

    dlugi temp = 0;
    MPI_Reduce(&liczba_nowych_polaczen, &temp, 1, MPI_DLUGI, MPI_SUM, 0, MPI_COMM_WORLD);
    lpolaczen += temp;

    MPI_Allreduce(&liczba_nowych_potomkow, &temp, 1, MPI_DLUGI, MPI_SUM, MPI_COMM_WORLD);
    liczba_nowych_potomkow = temp;
    lkonfiguracji += temp;

  }
#endif  //USE_MPI

    pokolenie++;

    if (!mytid)
    {
      time_t rawtime;
      time ( &rawtime );
      cout << "[" << rawtime - rawtime_0 << "s] ";
      cout <<  "Generation = " << pokolenie << " : " << liczba_nowych_potomkow << " "
           << lpolaczen-liczba_starych_polaczen
           << " Total: " << lkonfiguracji << " " << lpolaczen << "\n";
    }


#ifdef USE_MPI
    if (odl_rozw >= pokolenie)
    {
      Long znalazlem =  (odl_rozw == pokolenie) ? 1 : 0;
      Long ktos_znalazl;
      MPI_Allreduce(&znalazlem, &ktos_znalazl, 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);
      if (ktos_znalazl)
      {
        odl_rozw = pokolenie;
        if (!mytid) wypisz_rozwiazania();
        if (!mytid) remove("temp_0/solutions");
        Na_dysku.skasowano_rozwiazania();
      }
    }
#else
    if (odl_rozw == pokolenie)
    {
      wypisz_rozwiazania();
      remove("temp_0/solutions");
      Na_dysku.skasowano_rozwiazania();
    }
#endif


    if (pokolenie > 2)
    {
      if ((pokolenie-2) % co_ile_zostaw)
      {
        if (!mytid) usun_pliki_konf(pokolenie-2);
        Na_dysku.zmiana_pokolen(true);
      }
      else
      {
        Na_dysku.zmiana_pokolen(false);
      }
    }

    liczba_starych_polaczen = lpolaczen;
    liczba_starych_potomkow = lkonfiguracji;
  } while (liczba_nowych_potomkow > 0);

  if (!mytid) wypisz_konf_koncowe(pokolenie-1);

  if (!mytid) cout << "Furthest configurations are accessible in "<<pokolenie-1<<" moves\n";
  if (!mytid) cout << "Total number of configurations: "<<lkonfiguracji<<"\n";
  if (!mytid) cout << "Total number of moves: "<<lpolaczen<<"\n";
  if (!mytid) cout << "Please, delete manually all directories: temp_*\nThey aren't automatically deleted because you may want to use these temporary data for further research.\n";

/*  if (!mytid) for (Long i = 0; i <= pokolenie; ++i)
    usun_pliki_konf(i);*/
  usun_pliki_koszykow();
/*  if (!mytid)
    remove_directories();*/

#ifdef USE_MPI
  MPI_Finalize();
#endif

  return 0;
}
