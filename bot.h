/* ================================================================
 * bot.h - Header Utama Bot Bahasa Indonesia Berbasis Aturan
 * Teknologi: C89 + POSIX + SQLite3 | 37 Tabel | April 2026
 * ================================================================
 * SELURUH elemen berbahasa Indonesia: fungsi, variabel, makro,
 * struktur data, dan komentar.
 * ================================================================ */

#ifndef BOT_H
#define BOT_H

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <sqlite3.h>

/* ================================================================
 * KONSTANTA SISTEM
 * ================================================================ */

#define MAKS_TOKEN        256     /* maks token per kalimat        */
#define MAKS_KARAKTER     256     /* maks karakter per token        */
#define MAKS_KALIMAT      2048    /* maks karakter per kalimat       */
#define MAKS_JAWABAN      8192    /* maks karakter respons bot       */
#define MAKS_NAMA_DB      512     /* maks karakter nama database     */
#define MAKS_QUERY        8192    /* maks karakter query SQL         */
#define MAKS_ENTITAS      1024    /* maks karakter nama entitas      */
#define MAKS_ELEMEN       32      /* maks elemen pola intent        */
#define MAKS_HASIL        8192    /* maks karakter hasil penalaran  */

#define MAKS_NAMA_LOG     512     /* maks karakter nama file log   */
#define MAKS_BARIS_LOG    2048    /* maks karakter per baris log   */

#define MAKS_KELAS_KATEGORI  64     /* maks entri cache kelas kategori */
#define MAKS_INTENT_HANDLER  64     /* maks entri cache intent handler */
#define MAKS_KONFIGURASI     64     /* maks entri cache konfigurasi   */

#define BENAR             1
#define SALAH             0

/* kode keluaran proses */
#define KELUAR_BERHASIL   0
#define KELUAR_GAGAL      1

/* ================================================================
 * MAKRO UTILITAS
 * ================================================================ */

#define AMAN(p) do { \
    if ((p) != NULL) { free((void *)(p)); (p) = NULL; } \
} while (0)

#define SALIN(dst, src, n) do { \
    strncpy((dst), (src), (n) - 1); \
    (dst)[(n) - 1] = '\0'; \
} while (0)

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

/* panjang aman string */
#define PANJANG(s) ((int)strlen((s)))

/* ================================================================
 * STRUKTUR DATA - TOKEN
 * ================================================================ */

typedef struct {
    char teks[MAKS_KARAKTER];       /* teks asli dari input           */
    char normal[MAKS_KARAKTER];     /* bentuk ternormalisasi           */
    char id_kata[MAKS_KARAKTER];    /* ID di leksikon (ma.1)          */
    char kata_dasar[MAKS_KARAKTER]; /* kata dasar setelah stemming     */
    char kelas[MAKS_KARAKTER];      /* kelas kata (kata benda, dll)   */
    char persona[MAKS_KARAKTER];    /* jenis persona jika kata ganti   */
    int  is_kiasan;                 /* penanda kata kiasan             */
    int  is_frasa;                  /* penanda bagian frasa           */
    int  is_ordinal;                /* penanda bilangan ordinal        */
    int  punya_angka;               /* penanda ada nilai angka         */
    double nilai_angka;             /* nilai numerik token            */
} Token;

/* ================================================================
 * STRUKTUR DATA - DAFTAR TOKEN
 * ================================================================ */

typedef struct {
    Token token[MAKS_TOKEN];
    int   jumlah;                   /* jumlah token valid             */
} DaftarToken;

/* ================================================================
 * STRUKTUR DATA - SPOK
 * ================================================================ */

typedef struct {
    char subjek[MAKS_KARAKTER];     /* subjek kalimat                 */
    char predikat[MAKS_KARAKTER];   /* predikat kalimat               */
    char objek[MAKS_KARAKTER];      /* objek kalimat                  */
    char pelengkap[MAKS_KARAKTER];  /* pelengkap kalimat              */
    char keterangan[MAKS_KARAKTER]; /* keterangan kalimat             */
    char jenis_kalimat[MAKS_KARAKTER]; /* jenis kalimat              */
    char struktur[MAKS_KARAKTER];   /* pola struktur SPOK             */
} Spok;

/* ================================================================
 * STRUKTUR DATA - INTENT
 * ================================================================ */

typedef struct {
    char nama[MAKS_KARAKTER];       /* nama intent (TANYA_ARTI, dll)  */
    char entitas1[MAKS_KARAKTER];   /* entitas pertama                */
    char entitas2[MAKS_KARAKTER];   /* entitas kedua                  */
    char aspek[MAKS_KARAKTER];      /* aspek yang diminta             */
    int  prioritas;                 /* prioritas intent               */
    int  id;                        /* ID pola intent dari DB         */
} Intent;

/* ================================================================
 * STRUKTUR DATA - KONTEKS SESI
 * ================================================================ */

typedef struct {
    int  id;                        /* ID sesi aktif                  */
    char topik[MAKS_KARAKTER];      /* topik pembicaraan              */
    char entitas_aktif[MAKS_ENTITAS]; /* entitas yang sedang dibahas */
    char subjek_terakhir[MAKS_KARAKTER]; /* subjek kalimat sebelumnya */
    char objek_terakhir[MAKS_KARAKTER];  /* objek kalimat sebelumnya  */
    char intent_terakhir[MAKS_KARAKTER]; /* intent sebelumnya         */
    char gaya_bahasa[MAKS_KARAKTER];   /* gaya bahasa sesi            */
    int  percakapan_ke;             /* urutan percakapan              */
} KonteksSesi;

/* ================================================================
 * VARIABEL GLOBAL - HANDLE DATABASE
 * ================================================================ */

extern sqlite3 *db;

/* ================================================================
 * DEKLARASI FUNGSI - basisdata.c
 * Buka, tutup, dan buat skema database
 * ================================================================ */

int  buka_database(const char *nama_file);
void tutup_database(void);
int  import_datasheet_jika_kosong(void);
int  buat_skema(void);
int  muat_cache_db(void);
int  cek_kategori(const char *kelas, const char *kategori);
const char *cari_handler(const char *intent);
const char *cari_tipe_pembuka_intent(const char *intent);
const char *cari_tipe_jawaban_intent(const char *intent);
const char *cari_aksi_handler(const char *intent);
int  ambil_tampilkan_langsung(const char *intent);
const char *ambil_konfigurasi(const char *kunci);
int  cek_konektor(const char *kata);

/* ================================================================
 * DEKLARASI FUNGSI - bahasa.c
 * Fase 1-5: tokenisasi, morfologi, POS, entity, SPOK
 * ================================================================ */

void fase1_tokenisasi(const char *input, DaftarToken *dt);
void fase2_morfologi(DaftarToken *dt);
void fase3_pos(DaftarToken *dt, KonteksSesi *ks);
void fase4_entitas(DaftarToken *dt);
void fase5_spok(DaftarToken *dt, Spok *spok);

/* ================================================================
 * DEKLARASI FUNGSI - logika.c
 * Fase 6-8: intent, konteks, penalaran
 * ================================================================ */

void fase6_intent(DaftarToken *dt, Intent *intent);
void fase7_konteks(DaftarToken *dt, Intent *intent, KonteksSesi *ks);
int  fase8_penalaran(DaftarToken *dt, Spok *spok, Intent *intent,
                     KonteksSesi *ks, char *hasil, int ukuran);

/* ================================================================
 * DEKLARASI FUNGSI - keluaran.c
 * Fase 9-10: respons, riwayat, manajemen sesi
 * ================================================================ */

void fase9_respons(Intent *intent, KonteksSesi *ks,
                   const char *isi, char *jawaban, int ukuran);
void fase10_riwayat(const char *input, const char *jawaban,
                    const Intent *intent, const KonteksSesi *ks);
int  mulai_sesi(KonteksSesi *ks);
void ganti_sesi(KonteksSesi *ks);
void akhiri_sesi(void);

/* ================================================================
 * DEKLARASI FUNGSI - utama.c
 * Fungsi utilitas umum dan main loop
 * ================================================================ */

void bersihkan_teks(char *teks);
int  teks_kosong(const char *teks);
void huruf_kecil(char *teks);
void hapus_tanda_baca(char *teks);
void proses(const char *input, char *jawaban, int ukuran,
            KonteksSesi *ks);

/* ================================================================
 * FUNGSI HELPER - basisdata.c
 * cek_konektor: cek apakah kata ada dalam daftar konektor
 * ================================================================ */
int  cek_konektor(const char *kata);

/* ================================================================
 * STRUKTUR CACHE - Kelas Kategori
 * ================================================================ */

typedef struct {
    char kelas[MAKS_KARAKTER];
    char kategori[MAKS_KARAKTER];
} KelasKategori;

/* ================================================================
 * STRUKTUR CACHE - Intent Handler
 * ================================================================ */

typedef struct {
    char intent[MAKS_KARAKTER];
    char handler[MAKS_KARAKTER];
    char tipe_pembuka[MAKS_KARAKTER];
    char tipe_jawaban[MAKS_KARAKTER];
    int  tampilkan_langsung;
    char aksi[MAKS_KARAKTER];
} IntentHandler;

/* ================================================================
 * STRUKTUR CACHE - Konfigurasi Sistem
 * ================================================================ */

typedef struct {
    char kunci[MAKS_KARAKTER];
    char nilai[MAKS_KARAKTER];
} KonfigurasiSistem;

#endif /* BOT_H */
