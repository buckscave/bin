/* ================================================================
 * bahasa.c - Fase 1 sampai 5: Pemrosesan Bahasa
 * Tokenisasi, Morfologi, POS Tagging, Entity, SPOK
 *
 * Versi database-driven: semua string kelas/kategori hardcode
 * diganti dengan lookup ke basisdata (cek_kategori, ambil_konfigurasi).
 * Kompatibel C89: variabel dideklarasikan di awal blok.
 * ================================================================ */

#include "bot.h"

/* ================================================================
 * FUNGSI BANTUAN INTERNAL
 * ================================================================ */

/* ---------------------------------------------------------------
 * cek_nama_satuan - Cek apakah token adalah singkatan satuan
 * Contoh: "km" -> "kilometer", "kg" -> "kilogram"
 * Mengembalikan BENAR jika ditemukan.
 * --------------------------------------------------------------- */
static int cek_nama_satuan(const char *token, char *hasil,
                           int ukuran_hasil)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT satuan FROM nama_satuan "
                      "WHERE kata = ? LIMIT 1;";
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *satuan;
        satuan = (const char *)sqlite3_column_text(stmt, 0);
        if (satuan != NULL) {
            SALIN(hasil, satuan, ukuran_hasil);
            sqlite3_finalize(stmt);
            return BENAR;
        }
    }

    sqlite3_finalize(stmt);
    return SALAH;
}

/* ---------------------------------------------------------------
 * cari_kata_dasar - Cek kata turunan, cari kata dasar
 * Mengisi kata_dasar, id_kata, dan kelas jika ditemukan.
 * --------------------------------------------------------------- */
static void cari_kata_dasar(const char *kata, char *kata_dasar,
                            int uk_dasar, char *id_kata,
                            int uk_id, char *kelas, int uk_kelas)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id_kata_dasar, kata_dasar, kelas "
                      "FROM kata_turunan "
                      "WHERE kata_turunan = ? "
                      "ORDER BY frekuensi DESC LIMIT 1;";
    const char *dasar, *id, *kls;
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id = (const char *)sqlite3_column_text(stmt, 0);
        dasar = (const char *)sqlite3_column_text(stmt, 1);
        kls = (const char *)sqlite3_column_text(stmt, 2);

        if (id != NULL)   SALIN(id_kata, id, uk_id);
        if (dasar != NULL) SALIN(kata_dasar, dasar, uk_dasar);
        if (kls != NULL)   SALIN(kelas, kls, uk_kelas);
    }

    sqlite3_finalize(stmt);
}

/* ---------------------------------------------------------------
 * cek_kata_kiasan - Cek pasangan 2 kata sebagai kata kiasan
 * (pencarian N-gram). Mengembalikan BENAR jika cocok.
 * --------------------------------------------------------------- */
static int cek_kata_kiasan(const char *kata1, const char *kata2)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM kata_kiasan "
                      "WHERE (kata1 = ? AND kata2 = ?) "
                      "   OR (kata1 = ? AND kata2 = ?) "
                      "LIMIT 1;";
    int rc, hasil = SALAH;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, kata1, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, kata2, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, kata2, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, kata1, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        hasil = BENAR;
    }

    sqlite3_finalize(stmt);
    return hasil;
}

/* ---------------------------------------------------------------
 * cek_komponen_makna - Cek apakah token adalah komponen frasa
 * Mengembalikan BENAR jika ditemukan di komponen_makna.
 * --------------------------------------------------------------- */
static int cek_komponen_makna(const char *kata)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM komponen_makna "
                      "WHERE kata_komponen = ? LIMIT 1;";
    int rc, hasil = SALAH;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        hasil = BENAR;
    }

    sqlite3_finalize(stmt);
    return hasil;
}

/* ---------------------------------------------------------------
 * ambil_kelas_leksikon - Ambil kelas kata dari leksikon
 * Mengembalikan BENAR jika ditemukan.
 * --------------------------------------------------------------- */
static int ambil_kelas_leksikon(const char *kata, char *kelas,
                               char *id_kata, int uk_id)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT kelas, id_kata FROM leksikon "
                      "WHERE kata = ? LIMIT 1;";
    const char *k, *id;
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        k = (const char *)sqlite3_column_text(stmt, 0);
        id = (const char *)sqlite3_column_text(stmt, 1);
        if (k != NULL)  SALIN(kelas, k, MAKS_KARAKTER);
        if (id != NULL) SALIN(id_kata, id, uk_id);
        sqlite3_finalize(stmt);
        return BENAR;
    }

    sqlite3_finalize(stmt);
    return SALAH;
}

/* ---------------------------------------------------------------
 * cek_kata_fungsional - Cek apakah kata adalah kata fungsional
 * Mengembalikan BENAR jika ditemukan.
 * --------------------------------------------------------------- */
static int cek_kata_fungsional(const char *kata, char *kelas)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT kelas FROM kata_fungsional "
                      "WHERE kata = ? LIMIT 1;";
    const char *k;
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        k = (const char *)sqlite3_column_text(stmt, 0);
        if (k != NULL) SALIN(kelas, k, MAKS_KARAKTER);
        sqlite3_finalize(stmt);
        return BENAR;
    }

    sqlite3_finalize(stmt);
    return SALAH;
}

/* ---------------------------------------------------------------
 * deteksi_persona - Deteksi persona dari kata ganti
 * Mengisi persona dan gaya bahasa dari referensi_konteks.
 * Mengembalikan BENAR jika ditemukan.
 * --------------------------------------------------------------- */
static int deteksi_persona(const char *kata, char *persona,
                           char *gaya, int uk_gaya)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT jenis_persona, gaya "
                      "FROM referensi_konteks "
                      "WHERE kata = ? AND jenis_persona IS NOT NULL "
                      "LIMIT 1;";
    const char *jp, *g;
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        jp = (const char *)sqlite3_column_text(stmt, 0);
        g = (const char *)sqlite3_column_text(stmt, 1);
        if (jp != NULL) SALIN(persona, jp, MAKS_KARAKTER);
        if (g != NULL)  SALIN(gaya, g, uk_gaya);
        sqlite3_finalize(stmt);
        return BENAR;
    }

    sqlite3_finalize(stmt);
    return SALAH;
}

/* ---------------------------------------------------------------
 * konversi_kata_angka - Konversi kata angka ke nilai numerik
 * Mengembalikan nilai numerik. Jika ordinal, *ordinal = BENAR.
 *
 * Deteksi ordinal: hanya berdasarkan prefiks "ke" pada kata
 * (contoh: "kedua", "ketiga", "kesepuluh").
 * Tidak ada hardcode kata ordinal tertentu.
 * --------------------------------------------------------------- */
static double konversi_kata_angka(const char *kata, int *ordinal)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nilai FROM kata_angka "
                      "WHERE kata = ? LIMIT 1;";
    int rc;
    double nilai = 0.0;

    *ordinal = SALAH;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 0.0;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        nilai = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);

    /* deteksi ordinal berdasarkan prefiks "ke" */
    if (strlen(kata) > 3) {
        if (strncmp(kata, "ke", 2) == 0) {
            *ordinal = BENAR;
        }
    }

    return nilai;
}

/* ================================================================
 * FASE 1: TOKENISASI
 * Normalisasi teks, pisahkan per kata, cek singkatan satuan
 * ================================================================ */
void fase1_tokenisasi(const char *input, DaftarToken *dt)
{
    char salinan[MAKS_KALIMAT];
    char *tok;
    int i = 0;

    memset(dt, 0, sizeof(*dt));
    SALIN(salinan, input, MAKS_KALIMAT);

    huruf_kecil(salinan);
    hapus_tanda_baca(salinan);

    tok = strtok(salinan, " \t\n\r");
    while (tok != NULL && i < MAKS_TOKEN) {
        SALIN(dt->token[i].teks, tok, MAKS_KARAKTER);
        SALIN(dt->token[i].normal, tok, MAKS_KARAKTER);

        /* cek apakah token adalah singkatan satuan */
        if (!cek_nama_satuan(tok, dt->token[i].normal,
                             MAKS_KARAKTER)) {
            SALIN(dt->token[i].normal, tok, MAKS_KARAKTER);
        }

        i++;
        tok = strtok(NULL, " \t\n\r");
    }

    dt->jumlah = i;
}

/* ================================================================
 * FASE 2: MORFOLOGI (Stemming)
 * 2a: Cari kata dasar dari kata_turunan
 * 2b: Deteksi kiasan via N-gram 2 kata
 * 2c: Deteksi frasa/idiom via komponen_makna
 * ================================================================ */
void fase2_morfologi(DaftarToken *dt)
{
    int i;

    /* 2a: cari kata dasar */
    for (i = 0; i < dt->jumlah; i++) {
        if (dt->token[i].kata_dasar[0] == '\0') {
            cari_kata_dasar(dt->token[i].normal,
                            dt->token[i].kata_dasar,
                            MAKS_KARAKTER,
                            dt->token[i].id_kata,
                            MAKS_KARAKTER,
                            dt->token[i].kelas,
                            MAKS_KARAKTER);
        }
    }

    /* 2b: deteksi kiasan (N-gram 2 kata berurutan) */
    for (i = 0; i < dt->jumlah - 1; i++) {
        if (cek_kata_kiasan(dt->token[i].normal,
                            dt->token[i + 1].normal)) {
            dt->token[i].is_kiasan = BENAR;
            dt->token[i + 1].is_kiasan = BENAR;
        }
    }

    /* 2c: deteksi frasa/idiom via komponen_makna */
    for (i = 0; i < dt->jumlah; i++) {
        if (!dt->token[i].is_kiasan) {
            dt->token[i].is_frasa = cek_komponen_makna(
                dt->token[i].normal);
        }
    }
}

/* ================================================================
 * FASE 3: POS TAGGING (Klasifikasi Per Kata)
 * Tentukan kelas kata, deteksi persona dan gaya bahasa
 * ================================================================ */
void fase3_pos(DaftarToken *dt, KonteksSesi *ks)
{
    int i;

    for (i = 0; i < dt->jumlah; i++) {
        char gaya[MAKS_KARAKTER];

        /* lewati jika kelas sudah terisi dari morfologi */
        if (dt->token[i].kelas[0] != '\0') {
            goto cek_persona;
        }

        /* cek leksikon (kata dasar dan turunan) */
        if (ambil_kelas_leksikon(dt->token[i].normal,
                                dt->token[i].kelas,
                                dt->token[i].id_kata,
                                MAKS_KARAKTER)) {
            goto cek_persona;
        }

        /* cek kata dasar jika belum ada */
        if (dt->token[i].kata_dasar[0] != '\0') {
            ambil_kelas_leksikon(dt->token[i].kata_dasar,
                                dt->token[i].kelas,
                                dt->token[i].id_kata,
                                MAKS_KARAKTER);
            goto cek_persona;
        }

        /* cek kata fungsional */
        cek_kata_fungsional(dt->token[i].normal,
                           dt->token[i].kelas);

cek_persona:
        /* deteksi persona dari kata ganti */
        memset(gaya, 0, sizeof(gaya));
        if (deteksi_persona(dt->token[i].normal,
                           dt->token[i].persona,
                           gaya, MAKS_KARAKTER)) {
            /* update gaya bahasa sesi jika terdeteksi */
            if (gaya[0] != '\0' && ks != NULL) {
                SALIN(ks->gaya_bahasa, gaya, MAKS_KARAKTER);
            }
        }
    }
}

/* ================================================================
 * FASE 4: ENTITY RECOGNITION
 * Kenali entitas: angka, waktu, bulan, hari
 *
 * Database-driven: kelas "bilangan" diperiksa via cek_kategori,
 * keberadaan kata di kata_angka dicek langsung tanpa hardcode
 * "nol"/"se", deteksi ordinal murni prefiks "ke".
 * ================================================================ */
void fase4_entitas(DaftarToken *dt)
{
    int i;

    for (i = 0; i < dt->jumlah; i++) {
        /* deteksi angka dari kata_angka */
        if (dt->token[i].kelas[0] == '\0' ||
            cek_kategori(dt->token[i].kelas, "bilangan")) {
            int ord;
            int ada_di_kata_angka;
            double nilai;
            sqlite3_stmt *stmt_ka;
            const char *sql_ka;
            int rc_ka;

            ord = SALAH;
            ada_di_kata_angka = SALAH;
            nilai = konversi_kata_angka(
                dt->token[i].normal, &ord);

            /*
             * Cek keberadaan kata di tabel kata_angka,
             * menangani kata dengan nilai 0 (misalnya "nol",
             * "se") tanpa perlu hardcode string tertentu.
             */
            sql_ka = "SELECT 1 FROM kata_angka "
                     "WHERE kata = ? LIMIT 1;";
            rc_ka = sqlite3_prepare_v2(db, sql_ka, -1,
                                       &stmt_ka, NULL);
            if (rc_ka == SQLITE_OK) {
                sqlite3_bind_text(stmt_ka, 1,
                                  dt->token[i].normal, -1,
                                  SQLITE_STATIC);
                if (sqlite3_step(stmt_ka) == SQLITE_ROW) {
                    ada_di_kata_angka = BENAR;
                }
                sqlite3_finalize(stmt_ka);
            }

            if (nilai != 0.0 || ada_di_kata_angka) {
                dt->token[i].nilai_angka = nilai;
                dt->token[i].punya_angka = BENAR;
                dt->token[i].is_ordinal = ord;
                if (dt->token[i].kelas[0] == '\0') {
                    const char *kb = ambil_konfigurasi("kelas_bilangan");
                    if (kb != NULL) {
                        SALIN(dt->token[i].kelas, kb,
                              MAKS_KARAKTER);
                    }
                }
            }
        }

        /* cek angka digit langsung (contoh: "123") */
        {
            char *akhir = NULL;
            double d = strtod(dt->token[i].normal, &akhir);
            if (akhir != NULL && *akhir == '\0' &&
                akhir != dt->token[i].normal) {
                dt->token[i].nilai_angka = d;
                dt->token[i].punya_angka = BENAR;
                if (dt->token[i].kelas[0] == '\0') {
                    const char *kb = ambil_konfigurasi("kelas_bilangan");
                    if (kb != NULL) {
                        SALIN(dt->token[i].kelas, kb,
                              MAKS_KARAKTER);
                    }
                }
            }
        }
    }
}

/* ================================================================
 * FASE 5: SPOK PARSING
 * Deteksi struktur kalimat: Subjek, Predikat, Objek,
 * Pelengkap, Keterangan. Juga jenis kalimat.
 *
 * Database-driven: semua kelas kata diperiksa via cek_kategori,
 * jenis kalimat diambil via ambil_konfigurasi, dan struktur
 * SPOK dibangun secara dinamis.
 * ================================================================ */
void fase5_spok(DaftarToken *dt, Spok *spok)
{
    int i;
    int idx_subjek = -1;
    int idx_predikat = -1;
    int idx_objek = -1;
    const char *jenis_val;

    memset(spok, 0, sizeof(*spok));

    if (dt->jumlah == 0) return;

    /* cari predikat (kata kerja) */
    for (i = 0; i < dt->jumlah; i++) {
        if (cek_kategori(dt->token[i].kelas, "kerja")) {
            idx_predikat = i;
            break;
        }
    }

    /* cari subjek (kata benda atau kata ganti sebelum predikat) */
    if (idx_predikat > 0) {
        for (i = 0; i < idx_predikat; i++) {
            if (cek_kategori(dt->token[i].kelas, "benda") ||
                cek_kategori(dt->token[i].kelas, "ganti")) {
                idx_subjek = i;
                break;
            }
        }
        /* jika tidak ada benda, ambil token pertama */
        if (idx_subjek == -1) {
            idx_subjek = 0;
        }
    }

    /* cari objek (kata benda setelah predikat) */
    if (idx_predikat >= 0) {
        for (i = idx_predikat + 1; i < dt->jumlah; i++) {
            if (cek_kategori(dt->token[i].kelas, "benda")) {
                idx_objek = i;
                break;
            }
        }
    }

    /* isi struktur SPOK */
    if (idx_subjek >= 0) {
        SALIN(spok->subjek, dt->token[idx_subjek].normal,
              MAKS_KARAKTER);
    }
    if (idx_predikat >= 0) {
        SALIN(spok->predikat, dt->token[idx_predikat].normal,
              MAKS_KARAKTER);
    }
    if (idx_objek >= 0) {
        SALIN(spok->objek, dt->token[idx_objek].normal,
              MAKS_KARAKTER);
    }

    /* deteksi keterangan (kata keterangan) */
    for (i = 0; i < dt->jumlah; i++) {
        if (cek_kategori(dt->token[i].kelas, "keterangan")) {
            SALIN(spok->keterangan, dt->token[i].normal,
                  MAKS_KARAKTER);
            break;
        }
    }

    /* tentukan jenis kalimat (database-driven) */
    {
        /* cek kata tanya */
        for (i = 0; i < dt->jumlah; i++) {
            if (cek_kategori(dt->token[i].kelas, "tanya")) {
                jenis_val = ambil_konfigurasi(
                    "jenis_kalimat_tanya");
                if (jenis_val != NULL && jenis_val[0] != '\0') {
                    SALIN(spok->jenis_kalimat, jenis_val,
                          MAKS_KARAKTER);
                }
                /* jika konfigurasi tidak tersedia, jenis_kalimat tetap kosong */
                SALIN(spok->struktur, spok->jenis_kalimat,
                      MAKS_KARAKTER);
                return;
            }
        }

        /* cek kata seru */
        for (i = 0; i < dt->jumlah; i++) {
            if (cek_kategori(dt->token[i].kelas, "seru")) {
                jenis_val = ambil_konfigurasi(
                    "jenis_kalimat_seruan");
                if (jenis_val != NULL && jenis_val[0] != '\0') {
                    SALIN(spok->jenis_kalimat, jenis_val,
                          MAKS_KARAKTER);
                }
                /* jika konfigurasi tidak tersedia, jenis_kalimat tetap kosong */
                SALIN(spok->struktur, spok->jenis_kalimat,
                      MAKS_KARAKTER);
                return;
            }
        }

        /* cek kata perintah (predikat di posisi pertama) */
        if (idx_predikat == 0) {
            jenis_val = ambil_konfigurasi(
                "jenis_kalimat_perintah");
            if (jenis_val != NULL && jenis_val[0] != '\0') {
                SALIN(spok->jenis_kalimat, jenis_val,
                      MAKS_KARAKTER);
            }
            /* jika konfigurasi tidak tersedia, jenis_kalimat tetap kosong */
            SALIN(spok->struktur, spok->jenis_kalimat,
                  MAKS_KARAKTER);
            return;
        }

        /* default: kalimat berita */
        jenis_val = ambil_konfigurasi(
            "jenis_kalimat_default");
        if (jenis_val != NULL && jenis_val[0] != '\0') {
            SALIN(spok->jenis_kalimat, jenis_val,
                  MAKS_KARAKTER);
        }
        /* jika konfigurasi tidak tersedia, jenis_kalimat tetap kosong */

        /* bangun struktur SPOK secara dinamis */
        {
            const char *lbl_s = ambil_konfigurasi("spok_label_subjek");
            const char *lbl_p = ambil_konfigurasi("spok_label_predikat");
            const char *lbl_o = ambil_konfigurasi("spok_label_objek");
            const char *lbl_k = ambil_konfigurasi("spok_label_keterangan");
            const char *sep = ambil_konfigurasi("spok_separator");
            char struktur[MAKS_KARAKTER];

            struktur[0] = '\0';
            if (lbl_s != NULL && lbl_p != NULL) {
                snprintf(struktur, MAKS_KARAKTER, "%s%s%s",
                         lbl_s, sep ? sep : "-", lbl_p);
            } else {
                SALIN(struktur, "S-P", MAKS_KARAKTER);
            }
            if (spok->objek[0] != '\0' && lbl_o != NULL) {
                strncat(struktur, sep ? sep : "-",
                        MAKS_KARAKTER - strlen(struktur) - 1);
                strncat(struktur, lbl_o,
                        MAKS_KARAKTER - strlen(struktur) - 1);
            }
            if (spok->keterangan[0] != '\0' && lbl_k != NULL) {
                strncat(struktur, sep ? sep : "-",
                        MAKS_KARAKTER - strlen(struktur) - 1);
                strncat(struktur, lbl_k,
                        MAKS_KARAKTER - strlen(struktur) - 1);
            }
            SALIN(spok->struktur, struktur, MAKS_KARAKTER);
        }
    }
}
