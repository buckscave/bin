/* ================================================================
 * logika.c - Fase 6 sampai 8: Intent, Konteks, Penalaran
 * Klasifikasi intent, resolusi konteks, rule engine
 *
 * MODIFIKASI: Semua nama intent, kelas, aspek default, dan
 * identifier rumus kini diambil dari basisdata via fungsi
 * lookup di basisdata.c (cek_kategori, cari_handler, dll).
 * ================================================================ */

#include "bot.h"

/* ================================================================
 * FUNGSI BANTUAN INTERNAL
 * ================================================================ */

/* ---------------------------------------------------------------
 * apakah_var_pola - Cek apakah elemen adalah variabel pola
 * Mengembalikan BENAR jika elemen adalah variabel pola SPOK.
 * --------------------------------------------------------------- */
static int apakah_var_pola(const char *elemen)
{
    const char *vs = ambil_konfigurasi("spok_label_subjek");
    const char *vp = ambil_konfigurasi("spok_label_predikat");
    const char *vo = ambil_konfigurasi("spok_label_objek");
    const char *vk = ambil_konfigurasi("spok_label_keterangan");

    if (vs != NULL && strcmp(elemen, vs) == 0) return BENAR;
    if (vp != NULL && strcmp(elemen, vp) == 0) return BENAR;
    if (vo != NULL && strcmp(elemen, vo) == 0) return BENAR;
    if (vk != NULL && strcmp(elemen, vk) == 0) return BENAR;
    return SALAH;
}

/* ---------------------------------------------------------------
 * apakah_keyword_intent - Cek apakah kata adalah keyword
 * khusus yang bukan entitas (kata hubung, tanya, dll)
 * --------------------------------------------------------------- */
static int apakah_keyword(const char *kata)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM kata_fungsional "
                      "WHERE kata = ? UNION "
                      "SELECT 1 FROM kata_angka "
                      "WHERE kata = ? LIMIT 1;";
    int rc, hasil = SALAH;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        hasil = BENAR;
    }

    sqlite3_finalize(stmt);
    return hasil;
}

/* ---------------------------------------------------------------
 * gabungkan_token - Gabungkan semua token menjadi satu string
 * --------------------------------------------------------------- */
static void gabungkan_token(const DaftarToken *dt, char *hasil,
                            int ukuran)
{
    int i;
    hasil[0] = '\0';
    for (i = 0; i < dt->jumlah; i++) {
        if (i > 0) strncat(hasil, " ", ukuran - strlen(hasil) - 1);
        strncat(hasil, dt->token[i].normal,
                ukuran - strlen(hasil) - 1);
    }
}

/* ---------------------------------------------------------------
 * cari_entitas_dari_input - Cari kata benda non-keyword
 * yang merupakan entitas dalam input
 * --------------------------------------------------------------- */
static void cari_entitas(const DaftarToken *dt, Intent *intent)
{
    int i;
    int idx_e1 = -1, idx_e2 = -1;

    /* cari entitas pertama: gunakan cek_kategori untuk
       mengecek apakah kelas token termasuk "benda" atau "ganti"
       tanpa menyebut nama kelas lengkap ("kata benda", dll) */
    for (i = 0; i < dt->jumlah; i++) {
        if (cek_kategori(dt->token[i].kelas, "benda") ||
            cek_kategori(dt->token[i].kelas, "ganti")) {
            if (!apakah_keyword(dt->token[i].normal)) {
                idx_e1 = i;
                break;
            }
        }
    }

    /* cari entitas kedua (setelah konektor) */
    for (i = 0; i < dt->jumlah; i++) {
        if (cek_konektor(dt->token[i].normal)) {
            if (i + 1 < dt->jumlah) {
                idx_e2 = i + 1;
            }
            break;
        }
    }

    /* isi entitas ke intent */
    if (idx_e1 >= 0) {
        SALIN(intent->entitas1, dt->token[idx_e1].normal,
              MAKS_KARAKTER);
    }
    if (idx_e2 >= 0) {
        SALIN(intent->entitas2, dt->token[idx_e2].normal,
              MAKS_KARAKTER);
    }

    /* fallback: jika entitas1 belum terisi, ambil kata dasar */
    if (intent->entitas1[0] == '\0') {
        for (i = 0; i < dt->jumlah; i++) {
            if (dt->token[i].kata_dasar[0] != '\0' &&
                !apakah_keyword(dt->token[i].normal) &&
                !apakah_keyword(dt->token[i].kata_dasar)) {
                SALIN(intent->entitas1,
                      dt->token[i].kata_dasar, MAKS_KARAKTER);
                break;
            }
        }
    }

    /* fallback terakhir: ambil token non-keyword terakhir
       (berguna untuk "apa itu X" dimana X belum terdaftar) */
    if (intent->entitas1[0] == '\0') {
        for (i = dt->jumlah - 1; i >= 0; i--) {
            if (!apakah_keyword(dt->token[i].normal) &&
                !dt->token[i].punya_angka) {
                SALIN(intent->entitas1,
                      dt->token[i].normal, MAKS_KARAKTER);
                break;
            }
        }
    }
}

/* ---------------------------------------------------------------
 * cari_aspek_dari_input - Cari kata kunci aspek dalam input
 * --------------------------------------------------------------- */
static void cari_aspek(const DaftarToken *dt, Intent *intent)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT aspek FROM kata_kunci_aspek "
                      "WHERE kata_kunci = ? LIMIT 1;";
    int i, rc;
    const char *aspek;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return;

    for (i = 0; i < dt->jumlah && intent->aspek[0] == '\0'; i++) {
        sqlite3_bind_text(stmt, 1, dt->token[i].normal,
                          -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            aspek = (const char *)sqlite3_column_text(stmt, 0);
            if (aspek != NULL) {
                SALIN(intent->aspek, aspek, MAKS_KARAKTER);
            }
        }
        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
}

/* ---------------------------------------------------------------
 * cek_perintah_sistem - Cek apakah input adalah perintah sistem
 * Mengembalikan nama aksi atau NULL jika bukan perintah.
 * --------------------------------------------------------------- */
static const char *cek_perintah_sistem(const char *input_full)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT aksi FROM perintah_sistem "
                      "WHERE ? LIKE '%' || pola || '%';";
    static char aksi[MAKS_KARAKTER];
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return NULL;

    sqlite3_bind_text(stmt, 1, input_full, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *a = (const char *)sqlite3_column_text(stmt, 0);
        if (a != NULL) {
            SALIN(aksi, a, MAKS_KARAKTER);
            sqlite3_finalize(stmt);
            return aksi;
        }
    }

    sqlite3_finalize(stmt);
    return NULL;
}

/* ================================================================
 * FASE 6: INTENT CLASSIFICATION
 * Memetakan pola kalimat ke aksi/intent.
 * Mengembalikan intent terbaik berdasarkan prioritas.
 * ================================================================ */
void fase6_intent(DaftarToken *dt, Intent *intent)
{
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;
    char input_full[MAKS_KALIMAT];
    int prioritas_terbaik = -1;

    memset(intent, 0, sizeof(*intent));
    intent->id = -1;

    gabungkan_token(dt, input_full, MAKS_KALIMAT);

    /* cek perintah sistem terlebih dahulu */
    {
        const char *aksi = cek_perintah_sistem(input_full);
        if (aksi != NULL) {
            SALIN(intent->nama, aksi, MAKS_KARAKTER);
            intent->prioritas = 99;
            return;
        }
    }

    /* muat semua pola intent, diurutkan prioritas menurun */
    sql = "SELECT id, pola, intent, prioritas "
          "FROM intent_pola ORDER BY prioritas DESC";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *pola = (const char *)
            sqlite3_column_text(stmt, 1);
        const char *nama = (const char *)
            sqlite3_column_text(stmt, 2);
        int pri = sqlite3_column_int(stmt, 3);
        int cocok = SALAH;
        char pola_copy[MAKS_KALIMAT];
        char *elemen[MAKS_ELEMEN];
        int jml_elemen = 0;
        char *t;
        int i, j;

        if (pola == NULL || nama == NULL) continue;

        /* salin pola untuk dipecah */
        SALIN(pola_copy, pola, MAKS_KALIMAT);

        /* pecah pola berdasarkan " + " */
        t = strtok(pola_copy, " + ");
        jml_elemen = 0;
        while (t != NULL && jml_elemen < MAKS_ELEMEN) {
            elemen[jml_elemen++] = t;
            t = strtok(NULL, " + ");
        }

        /* pola satu kata: cocok langsung */
        if (jml_elemen == 1) {
            /* cocok terhadap seluruh input */
            if (strcmp(pola, input_full) == 0) {
                cocok = BENAR;
            }
            /* cocok terhadap token tunggal */
            if (dt->jumlah == 1 &&
                strcmp(pola, dt->token[0].normal) == 0) {
                cocok = BENAR;
            }
        }
        /* pola multi-kata: cek semua keyword */
        else {
            int semua_keyword = BENAR;
            for (i = 0; i < jml_elemen && semua_keyword; i++) {
                int adalah_var = SALAH;
                int ditemukan = SALAH;

                /* cek apakah variabel pola */
                if (apakah_var_pola(elemen[i])) {
                    adalah_var = BENAR;
                }

                if (!adalah_var) {
                    /* cari keyword di token input */
                    for (j = 0; j < dt->jumlah; j++) {
                        if (strcmp(elemen[i],
                                  dt->token[j].normal) == 0) {
                            ditemukan = BENAR;
                            break;
                        }
                    }
                    if (!ditemukan) {
                        semua_keyword = SALAH;
                    }
                }
            }
            if (semua_keyword) {
                cocok = BENAR;
            }
        }

        /* jika cocok dan prioritas lebih tinggi */
        if (cocok && pri > prioritas_terbaik) {
            SALIN(intent->nama, nama, MAKS_KARAKTER);
            intent->id = sqlite3_column_int(stmt, 0);
            intent->prioritas = pri;
            prioritas_terbaik = pri;
        }
    }

    sqlite3_finalize(stmt);

    /* cari entitas dan aspek dari input */
    cari_entitas(dt, intent);
    cari_aspek(dt, intent);
}

/* ================================================================
 * FASE 7: CONTEXT RESOLUTION
 * Resolusi referensi kata ganti dan update konteks sesi
 * ================================================================ */
void fase7_konteks(DaftarToken *dt, Intent *intent,
                   KonteksSesi *ks)
{
    int i;
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;

    /* resolusi kata ganti ke referensi aktual */
    sql = "SELECT merujuk_ke FROM referensi_konteks "
          "WHERE kata = ? AND merujuk_ke IS NOT NULL "
          "LIMIT 1;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return;

    for (i = 0; i < dt->jumlah; i++) {
        const char *merujuk_ke = NULL;
        const char *nilai_referen = NULL;
        char referen[MAKS_KARAKTER];

        memset(referen, 0, sizeof(referen));

        sqlite3_bind_text(stmt, 1, dt->token[i].normal,
                          -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            merujuk_ke = (const char *)
                sqlite3_column_text(stmt, 0);
        }

        if (merujuk_ke == NULL) {
            sqlite3_reset(stmt);
            continue;
        }

        /*
         * Catatan: String-string di bawah ini adalah nilai
         * yang berasal dari kolom referensi_konteks.merujuk_ke
         * di database. Mereka adalah "routing key" internal
         * yang memetakan referensi ke field struct KonteksSesi.
         * Karena C89 tidak mendukung akses field struct secara
         * dinamis berdasarkan nama, pemetaan ini harus tetap
         * eksplisit di kode. Nilai-nilai ini HARUS konsisten
         * dengan data di tabel referensi_konteks.
         */
        if (strcmp(merujuk_ke, "subjek_terakhir") == 0) {
            nilai_referen = ks->subjek_terakhir;
        } else if (strcmp(merujuk_ke, "objek_terakhir") == 0) {
            nilai_referen = ks->objek_terakhir;
        } else if (strcmp(merujuk_ke, "topik") == 0) {
            nilai_referen = ks->topik;
        } else if (strcmp(merujuk_ke, "entitas_aktif") == 0) {
            nilai_referen = ks->entitas_aktif;
        }

        /* ganti token dengan referensi yang ter-resolve */
        if (nilai_referen != NULL && nilai_referen[0] != '\0') {
            SALIN(referen, nilai_referen, MAKS_KARAKTER);
            SALIN(dt->token[i].normal, referen, MAKS_KARAKTER);
            SALIN(dt->token[i].kata_dasar, referen,
                  MAKS_KARAKTER);
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    /* update konteks sesi */
    if (intent->entitas1[0] != '\0') {
        SALIN(ks->entitas_aktif, intent->entitas1,
              MAKS_ENTITAS);
        SALIN(ks->topik, intent->entitas1, MAKS_KARAKTER);
    }
    SALIN(ks->subjek_terakhir, intent->entitas1,
          MAKS_KARAKTER);
    SALIN(ks->objek_terakhir, intent->entitas2,
          MAKS_KARAKTER);
    SALIN(ks->intent_terakhir, intent->nama,
          MAKS_KARAKTER);
}

/* ================================================================
 * FUNGSI PENALARAN - Sub-fungsi Fase 8
 * ================================================================ */

/* ---------------------------------------------------------------
 * penalaran_pengetahuan - Query pengetahuan umum berdasarkan
 * entitas dan aspek
 * --------------------------------------------------------------- */
static void penalaran_pengetahuan(const Intent *intent, char *hasil,
                                  int ukuran)
{
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;
    int baris_pertama = BENAR;
    int ada_aspek_lain = SALAH;

    if (intent->entitas1[0] == '\0') {
        SALIN(hasil, "", ukuran);
        return;
    }

    /* resolve aspek via kata_kunci_aspek */
    {
        char aspek_kanonik[MAKS_KARAKTER];
        sqlite3_stmt *stmt2;
        const char *sql2 = "SELECT aspek FROM kata_kunci_aspek "
                           "WHERE kata_kunci = ? LIMIT 1;";
        const char *ak;
        const char *aspek_default;

        if (intent->aspek[0] != '\0') {
            SALIN(aspek_kanonik, intent->aspek, MAKS_KARAKTER);
        } else {
            aspek_kanonik[0] = '\0';
        }

        /* jika aspek masih kosong, ambil aspek default
           dari konfigurasi_sistem (kunci: aspek_default) */
        if (aspek_kanonik[0] == '\0') {
            aspek_default = ambil_konfigurasi("aspek_default");
            if (aspek_default != NULL) {
                SALIN(aspek_kanonik, aspek_default,
                      MAKS_KARAKTER);
            } else {
                aspek_kanonik[0] = '\0';
            }
        }

        /* jika tetap kosong setelah lookup, skip query */
        if (aspek_kanonik[0] == '\0') {
            SALIN(hasil, "", ukuran);
            return;
        }

        /* query pengetahuan_umum */
        sql = "SELECT judul, penjelasan, aspek, nomor_item, "
              "label_item FROM pengetahuan_umum "
              "WHERE entitas = ? AND aspek = ? "
              "ORDER BY urutan ASC";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            SALIN(hasil, "", ukuran);
            return;
        }

        sqlite3_bind_text(stmt, 1, intent->entitas1,
                          -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, aspek_kanonik,
                          -1, SQLITE_STATIC);

        hasil[0] = '\0';

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *judul = (const char *)
                sqlite3_column_text(stmt, 0);
            const char *penjelasan = (const char *)
                sqlite3_column_text(stmt, 1);
            const char *aspek_row = (const char *)
                sqlite3_column_text(stmt, 2);
            int nomor = sqlite3_column_int(stmt, 3);
            const char *label = (const char *)
                sqlite3_column_text(stmt, 4);
            char buf[MAKS_KARAKTER];

            if (!baris_pertama) {
                strncat(hasil, "\n",
                        ukuran - strlen(hasil) - 1);
            }

            /* format: label/nomor judul - penjelasan */
            if (label != NULL && nomor > 0) {
                snprintf(buf, MAKS_KARAKTER, "%s %d: ",
                         label, nomor);
                strncat(hasil, buf,
                        ukuran - strlen(hasil) - 1);
            } else if (judul != NULL) {
                strncat(hasil, judul,
                        ukuran - strlen(hasil) - 1);
                strncat(hasil, ": ",
                        ukuran - strlen(hasil) - 1);
            }

            if (penjelasan != NULL) {
                strncat(hasil, penjelasan,
                        ukuran - strlen(hasil) - 1);
            }

            baris_pertama = SALAH;
        }

        sqlite3_finalize(stmt);

        /* jika aspek yang diminta kosong tapi ada data lain */
        if (hasil[0] == '\0') {
            sql = "SELECT DISTINCT aspek FROM pengetahuan_umum "
                  "WHERE entitas = ?";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt2, NULL);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt2, 1,
                                  intent->entitas1,
                                  -1, SQLITE_STATIC);
                while (sqlite3_step(stmt2) == SQLITE_ROW) {
                    ak = (const char *)
                        sqlite3_column_text(stmt2, 0);
                    if (ak != NULL &&
                        strcmp(ak, aspek_kanonik) != 0) {
                        ada_aspek_lain = BENAR;
                    }
                }
                sqlite3_finalize(stmt2);
            }
        }
    }

    (void)ada_aspek_lain;
}

/* ---------------------------------------------------------------
 * penalaran_definisi - Cari definisi kata dari arti_kata
 * Hanya menampilkan 1 arti default (frekuensi tertinggi).
 * Format: "topik adalah definisi" (tanpa tipe, tanpa contoh)
 * --------------------------------------------------------------- */
static void penalaran_definisi(const Intent *intent, char *hasil,
                               int ukuran)
{
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;
    const char *kata = intent->entitas1;
    const char *aspek_def;
    char buf[MAKS_KARAKTER * 4];

    if (kata[0] == '\0') {
        SALIN(hasil, "", ukuran);
        return;
    }

    /* ambil aspek default dari konfigurasi_sistem */
    aspek_def = ambil_konfigurasi("aspek_default");
    /* aspek_def sudah NULL jika tidak ada di DB */

    /* ambil 1 arti default dari arti_kata (frekuensi DESC) */
    sql = "SELECT arti FROM arti_kata "
          "WHERE kata = ? ORDER BY frekuensi DESC LIMIT 1;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        SALIN(hasil, "", ukuran);
        return;
    }

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *arti = (const char *)
            sqlite3_column_text(stmt, 0);

        if (arti != NULL) {
            /* format: "topik adalah definisi" */
            snprintf(hasil, ukuran, "%s adalah %s",
                     kata, arti);
        }

        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    /* fallback: cek pengetahuan_umum dengan aspek default */
    sql = "SELECT penjelasan FROM pengetahuan_umum "
          "WHERE entitas = ? AND aspek = ? "
          "LIMIT 1;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        SALIN(hasil, "", ukuran);
        return;
    }

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, aspek_def, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *p = (const char *)
            sqlite3_column_text(stmt, 0);

        if (p != NULL) {
            snprintf(hasil, ukuran, "%s adalah %s",
                     kata, p);
        }

        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    /* fallback: cek judul + penjelasan dari pengetahuan_umum */
    sql = "SELECT judul, penjelasan FROM pengetahuan_umum "
          "WHERE entitas = ? ORDER BY urutan ASC LIMIT 1;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        SALIN(hasil, "", ukuran);
        return;
    }

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *judul = (const char *)
            sqlite3_column_text(stmt, 0);
        const char *pjl = (const char *)
            sqlite3_column_text(stmt, 1);

        if (judul != NULL && pjl != NULL) {
            snprintf(hasil, ukuran, "%s adalah %s",
                     kata, pjl);
        } else if (pjl != NULL) {
            snprintf(hasil, ukuran, "%s adalah %s",
                     kata, pjl);
        }
    }

    sqlite3_finalize(stmt);

    (void)buf;
}

/* ---------------------------------------------------------------
 * penalaran_arti_lain - Tampilkan semua arti/makna kata
 * Digunakan saat user meminta arti lain dari kata.
 * Menampilkan semua arti dengan nomor, tipe, dan contoh.
 * --------------------------------------------------------------- */
static void penalaran_arti_lain(const Intent *intent, char *hasil,
                                  int ukuran)
{
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;
    int baris_pertama = BENAR;
    int nomor = 1;
    const char *kata = intent->entitas1;

    if (kata[0] == '\0') {
        SALIN(hasil, "", ukuran);
        return;
    }

    hasil[0] = '\0';

    /* query semua arti dari arti_kata */
    sql = "SELECT arti, tipe, contoh_kalimat FROM arti_kata "
          "WHERE kata = ? ORDER BY frekuensi DESC;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        SALIN(hasil, "", ukuran);
        return;
    }

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *arti = (const char *)
            sqlite3_column_text(stmt, 0);
        const char *tipe = (const char *)
            sqlite3_column_text(stmt, 1);
        const char *contoh = (const char *)
            sqlite3_column_text(stmt, 2);
        char buf[MAKS_KARAKTER * 2];

        if (!baris_pertama) {
            strncat(hasil, "\n",
                    ukuran - strlen(hasil) - 1);
        }

        snprintf(buf, sizeof(buf), "%d. ", nomor);
        strncat(hasil, buf,
                ukuran - strlen(hasil) - 1);

        if (arti != NULL) {
            strncat(hasil, arti,
                    ukuran - strlen(hasil) - 1);
        }

        if (contoh != NULL) {
            snprintf(buf, sizeof(buf),
                     "\n  Contoh: %s", contoh);
            strncat(hasil, buf,
                    ukuran - strlen(hasil) - 1);
        }

        nomor++;
        baris_pertama = SALAH;
    }

    sqlite3_finalize(stmt);

    /* juga tampilkan makna_kata (frasa/idiom) */
    sql = "SELECT kata, makna, jenis_kata, contoh_kalimat "
          "FROM makna_kata WHERE kata = ? "
          "ORDER BY frekuensi DESC;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *makna = (const char *)
            sqlite3_column_text(stmt, 1);
        const char *jenis = (const char *)
            sqlite3_column_text(stmt, 2);
        const char *contoh = (const char *)
            sqlite3_column_text(stmt, 3);
        char buf[MAKS_KARAKTER * 2];

        if (!baris_pertama) {
            strncat(hasil, "\n",
                    ukuran - strlen(hasil) - 1);
        }

        snprintf(buf, sizeof(buf), "%d. ", nomor);
        strncat(hasil, buf,
                ukuran - strlen(hasil) - 1);

        if (makna != NULL) {
            strncat(hasil, makna,
                    ukuran - strlen(hasil) - 1);
        }

        if (contoh != NULL) {
            snprintf(buf, sizeof(buf),
                     "\n  Contoh: %s", contoh);
            strncat(hasil, buf,
                    ukuran - strlen(hasil) - 1);
        }

        nomor++;
        baris_pertama = SALAH;
    }

    sqlite3_finalize(stmt);
}

/* ---------------------------------------------------------------
 * penalaran_bandingkan - Bandingkan dua entitas
 * --------------------------------------------------------------- */
static void penalaran_bandingkan(const Intent *intent, char *hasil,
                                 int ukuran)
{
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;

    if (intent->entitas1[0] == '\0' ||
        intent->entitas2[0] == '\0') {
        SALIN(hasil, "", ukuran);
        return;
    }

    /* validasi domain */
    sql = "SELECT domain FROM pengetahuan_umum "
          "WHERE entitas = ? AND domain IS NOT NULL "
          "GROUP BY domain LIMIT 1;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        SALIN(hasil, "", ukuran);
        return;
    }

    sqlite3_bind_text(stmt, 1, intent->entitas1,
                      -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        /* cek entitas gabungan langsung */
        sql = "SELECT penjelasan FROM pengetahuan_umum "
              "WHERE entitas = ?";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            SALIN(hasil, "", ukuran);
            return;
        }
        sqlite3_bind_text(stmt, 1, intent->entitas1,
                          -1, SQLITE_STATIC);
        hasil[0] = '\0';
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *p = (const char *)
                sqlite3_column_text(stmt, 0);
            if (p != NULL) {
                if (hasil[0] != '\0') {
                    strncat(hasil, "\n",
                            ukuran - strlen(hasil) - 1);
                }
                strncat(hasil, p,
                        ukuran - strlen(hasil) - 1);
            }
        }
        sqlite3_finalize(stmt);
        return;
    }

    {
        char domain1[MAKS_KARAKTER];
        const char *d1 = (const char *)
            sqlite3_column_text(stmt, 0);
        if (d1 != NULL) SALIN(domain1, d1, MAKS_KARAKTER);
        else domain1[0] = '\0';
        sqlite3_finalize(stmt);

        /* cek domain entitas2 */
        sql = "SELECT domain FROM pengetahuan_umum "
              "WHERE entitas = ? AND domain IS NOT NULL "
              "GROUP BY domain LIMIT 1;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            SALIN(hasil, "", ukuran);
            return;
        }
        sqlite3_bind_text(stmt, 1, intent->entitas2,
                          -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *d2 = (const char *)
                sqlite3_column_text(stmt, 0);
            if (d2 != NULL && domain1[0] != '\0' &&
                strcmp(domain1, d2) != 0) {
                /* domain berbeda */
                char buf[MAKS_KARAKTER * 4];
                snprintf(buf, sizeof(buf),
                         "%s dan %s termasuk kategori yang "
                         "berbeda sehingga tidak dapat "
                         "diperbandingkan secara langsung.",
                         intent->entitas1, intent->entitas2);
                SALIN(hasil, buf, ukuran);
                sqlite3_finalize(stmt);
                return;
            }
        }
        sqlite3_finalize(stmt);
    }

    /* query aspek entitas1 */
    sql = "SELECT aspek, judul, penjelasan "
          "FROM pengetahuan_umum "
          "WHERE entitas = ? ORDER BY aspek, urutan";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        SALIN(hasil, "", ukuran);
        return;
    }

    hasil[0] = '\0';
    snprintf(hasil, ukuran, "%s:\n", intent->entitas1);

    sqlite3_bind_text(stmt, 1, intent->entitas1,
                      -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *asp = (const char *)
            sqlite3_column_text(stmt, 0);
        const char *jdl = (const char *)
            sqlite3_column_text(stmt, 1);
        const char *pjl = (const char *)
            sqlite3_column_text(stmt, 2);
        char buf[MAKS_KARAKTER * 2];

        if (jdl != NULL && pjl != NULL) {
            snprintf(buf, sizeof(buf), "- %s: %s\n", jdl, pjl);
            strncat(hasil, buf,
                    ukuran - strlen(hasil) - 1);
        }
        (void)asp;
    }
    sqlite3_finalize(stmt);

    /* query aspek entitas2 */
    {
        char buf2[MAKS_KARAKTER * 2];
        snprintf(buf2, sizeof(buf2), "\n%s:\n",
                 intent->entitas2);
        strncat(hasil, buf2,
                ukuran - strlen(hasil) - 1);
    }

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, intent->entitas2,
                          -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *asp = (const char *)
                sqlite3_column_text(stmt, 0);
            const char *jdl = (const char *)
                sqlite3_column_text(stmt, 1);
            const char *pjl = (const char *)
                sqlite3_column_text(stmt, 2);
            char buf[MAKS_KARAKTER * 2];

            if (jdl != NULL && pjl != NULL) {
                snprintf(buf, sizeof(buf), "- %s: %s\n",
                         jdl, pjl);
                strncat(hasil, buf,
                        ukuran - strlen(hasil) - 1);
            }
            (void)asp;
        }
        sqlite3_finalize(stmt);
    }
}

/* ---------------------------------------------------------------
 * penalaran_matematika - Evaluasi ekspresi matematika sederhana
 * dari token angka dan operator
 * --------------------------------------------------------------- */
static void penalaran_matematika(const DaftarToken *dt, char *hasil,
                                 int ukuran)
{
    int i;
    double nilai = 0.0;
    char op[4] = "+";
    int punya_operator = SALAH;
    int punya_angka = SALAH;

    hasil[0] = '\0';

    for (i = 0; i < dt->jumlah; i++) {
        if (dt->token[i].punya_angka) {
            double n = dt->token[i].nilai_angka;

            if (!punya_angka) {
                nilai = n;
                punya_angka = BENAR;
            } else if (punya_operator) {
                if (strcmp(op, "+") == 0) {
                    nilai = nilai + n;
                } else if (strcmp(op, "-") == 0) {
                    nilai = nilai - n;
                } else if (strcmp(op, "*") == 0) {
                    nilai = nilai * n;
                } else if (strcmp(op, "/") == 0) {
                    if (n != 0.0) nilai = nilai / n;
                } else if (strcmp(op, "%") == 0) {
                    if (n != 0.0) nilai = fmod(nilai, n);
                }
                punya_operator = SALAH;
            }
        } else {
            /* cek apakah ini operator matematika */
            sqlite3_stmt *stmt;
            const char *sql = "SELECT operator "
                             "FROM operator_matematika "
                             "WHERE kata = ? LIMIT 1;";
            int rc;
            const char *opr;

            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1,
                                  dt->token[i].normal,
                                  -1, SQLITE_STATIC);
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    opr = (const char *)
                        sqlite3_column_text(stmt, 0);
                    if (opr != NULL) {
                        SALIN(op, opr, 4);
                        punya_operator = BENAR;
                    }
                }
                sqlite3_finalize(stmt);
            }
        }
    }

    if (punya_angka) {
        /* format hasil: bulat jika tidak ada desimal */
        if (nilai == (double)(long long)nilai) {
            snprintf(hasil, ukuran, "%.0f", nilai);
        } else {
            snprintf(hasil, ukuran, "%.4f", nilai);
        }
    }
}

/* ---------------------------------------------------------------
 * penalaran_konversi - Konversi satuan
 * --------------------------------------------------------------- */
static void penalaran_konversi(const DaftarToken *dt, char *hasil,
                                int ukuran)
{
    int i;
    double nilai = 0.0;
    char satuan_dari[MAKS_KARAKTER];
    char satuan_ke[MAKS_KARAKTER];
    int dari_set = SALAH;

    satuan_dari[0] = '\0';
    satuan_ke[0] = '\0';
    hasil[0] = '\0';

    /* cari angka dan dua satuan */
    for (i = 0; i < dt->jumlah; i++) {
        if (dt->token[i].punya_angka) {
            nilai = dt->token[i].nilai_angka;
            dari_set = BENAR;
        } else if (dt->token[i].kata_dasar[0] != '\0' &&
                   satuan_dari[0] == '\0') {
            SALIN(satuan_dari, dt->token[i].kata_dasar,
                  MAKS_KARAKTER);
        } else if (dt->token[i].kata_dasar[0] != '\0' &&
                   satuan_ke[0] == '\0' &&
                   satuan_dari[0] != '\0') {
            SALIN(satuan_ke, dt->token[i].kata_dasar,
                  MAKS_KARAKTER);
        }
    }

    /* cek nama_satuan untuk resolve singkatan */
    if (satuan_dari[0] != '\0') {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT satuan FROM nama_satuan "
                          "WHERE kata = ? LIMIT 1;";
        int rc;

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, satuan_dari,
                              -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *s = (const char *)
                    sqlite3_column_text(stmt, 0);
                if (s != NULL) {
                    SALIN(satuan_dari, s, MAKS_KARAKTER);
                }
            }
            sqlite3_finalize(stmt);
        }
    }
    if (satuan_ke[0] != '\0') {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT satuan FROM nama_satuan "
                          "WHERE kata = ? LIMIT 1;";
        int rc;

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, satuan_ke,
                              -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *s = (const char *)
                    sqlite3_column_text(stmt, 0);
                if (s != NULL) {
                    SALIN(satuan_ke, s, MAKS_KARAKTER);
                }
            }
            sqlite3_finalize(stmt);
        }
    }

    if (!dari_set || satuan_dari[0] == '\0' ||
        satuan_ke[0] == '\0') {
        return;
    }

    /* query konversi_satuan */
    {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT faktor, rumus_khusus "
                          "FROM konversi_satuan "
                          "WHERE dari = ? AND ke = ? LIMIT 1;";
        int rc;
        const char *rumus = NULL;
        const char *rumus_ftoc = NULL;
        const char *rumus_ctof = NULL;
        const char *pesan_gagal = NULL;

        /*
         * Ambil identifier rumus dari konfigurasi_sistem.
         * Nilai-nilai ini HARUS konsisten dengan kolom
         * rumus_khusus di tabel konversi_satuan.
         * Contoh konfigurasi:
         *   rumus_f_to_c -> "f_to_c"
         *   rumus_c_to_f -> "c_to_f"
         *   pesan_konversi_gagal -> "Konversi dari %s ke %s tidak tersedia."
         */
        rumus_ftoc = ambil_konfigurasi("rumus_f_to_c");
        rumus_ctof = ambil_konfigurasi("rumus_c_to_f");
        pesan_gagal = ambil_konfigurasi("pesan_konversi_gagal");

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) return;

        sqlite3_bind_text(stmt, 1, satuan_dari,
                          -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, satuan_ke,
                          -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            double faktor = sqlite3_column_double(stmt, 0);
            double hasil_konversi;

            if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
                rumus = (const char *)
                    sqlite3_column_text(stmt, 1);
            }

            if (rumus != NULL && rumus_ftoc != NULL &&
                strcmp(rumus, rumus_ftoc) == 0) {
                hasil_konversi = (nilai - 32.0) * 5.0 / 9.0;
            } else if (rumus != NULL && rumus_ctof != NULL &&
                       strcmp(rumus, rumus_ctof) == 0) {
                hasil_konversi = (nilai * 9.0 / 5.0) + 32.0;
            } else {
                hasil_konversi = nilai * faktor;
            }

            snprintf(hasil, ukuran,
                     "%.4g %s = %.4g %s",
                     nilai, satuan_dari,
                     hasil_konversi, satuan_ke);
        } else {
            /* pesan gagal: ambil dari konfigurasi_sistem,
               atau kosongkan jika tidak tersedia */
            if (pesan_gagal != NULL) {
                snprintf(hasil, ukuran, pesan_gagal,
                         satuan_dari, satuan_ke);
            } else {
                /* tidak ada konversi yang tersedia */
                hasil[0] = '\0';
            }
        }

        sqlite3_finalize(stmt);
    }
}

/* ================================================================
 * FASE 8: PENALARAN (RULE ENGINE) - HANDLER-BASED DISPATCH
 *
 * Berdasarkan intent, cari handler yang sesuai via cari_handler()
 * dari basisdata.c, lalu lakukan penalaran yang sesuai.
 * Mengembalikan BENAR jika ada hasil.
 *
 * Handler yang tersedia (didapat dari konfigurasi intent_handler):
 *   sistem     - perintah sistem (sinyal internal)
 *   sapaan     - pembuka + penutup percakapan
 *   persetujuan - respons konfirmasi positif
 *   penolakan  - respons penolakan
 *   konfirmasi - respons konfirmasi netral
 *   arti_lain  - semua arti/makna kata
 *   definisi   - definisi kata (1 arti default)
 *   bandingkan - perbandingan dua entitas
 *   kemampuan  - daftar kemampuan bot
 *   verifikasi - verifikasi klaim
 *   konversi   - konversi satuan
 *   matematika - evaluasi ekspresi matematika
 * ================================================================ */
int fase8_penalaran(DaftarToken *dt, Spok *spok, Intent *intent,
                    KonteksSesi *ks, char *hasil, int ukuran)
{
    const char *handler;
    int panjang;

    (void)spok;
    (void)ks;

    hasil[0] = '\0';

    /* cari handler dari cache (basisdata.c) */
    handler = cari_handler(intent->nama);
    if (handler == NULL) {
        handler = ambil_konfigurasi("handler_default");
    }
    /* handler = NULL berarti tidak ada handler yang tersedia */

    /* jika handler NULL, langsung ke fallback pengetahuan umum */
    if (handler == NULL) {
        goto fallback_pengetahuan;
    }

    /* handler: sistem */
    if (strcmp(handler, "sistem") == 0) {
        const char *aksi = cari_aksi_handler(intent->nama);
        if (aksi != NULL) {
            const char *sinyal = ambil_konfigurasi(aksi);
            if (sinyal != NULL) {
                SALIN(hasil, sinyal, ukuran);
                return BENAR;
            }
        }
        return SALAH;
    }

    /* handler: sapaan */
    if (strcmp(handler, "sapaan") == 0) {
        const char *tipe_pb = NULL;
        sqlite3_stmt *stmt;
        const char *sql;
        char sql_buf[MAKS_QUERY];
        int rc;

        tipe_pb = cari_tipe_pembuka_intent(intent->nama);
        if (tipe_pb == NULL) tipe_pb = ambil_konfigurasi("tipe_pembuka_sapaan");

        /* jika tipe_pembuka tidak tersedia, skip pembuka */
        if (tipe_pb == NULL) {
            hasil[0] = '\0';
        } else {
        /* ambil pola pembuka */
        snprintf(sql_buf, MAKS_QUERY,
                 "SELECT pola FROM pola_pembuka "
                 "WHERE tipe = '%s' ORDER BY RANDOM() LIMIT 1;",
                 tipe_pb);
        sql = sql_buf;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *p = (const char *)
                    sqlite3_column_text(stmt, 0);
                if (p != NULL) SALIN(hasil, p, ukuran);
            }
            sqlite3_finalize(stmt);
        }
        }

        strncat(hasil, " ", ukuran - strlen(hasil) - 1);

        /* ambil pola penutup */
        sql = "SELECT pola FROM pola_penutup WHERE kondisi = ? "
              "ORDER BY RANDOM() LIMIT 1;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            const char *kondisi = ambil_konfigurasi(
                "kondisi_penutup_sapaan");
            /* kondisi sudah NULL jika tidak ada di DB */
            if (kondisi != NULL) {
                sqlite3_bind_text(stmt, 1, kondisi,
                                  -1, SQLITE_STATIC);
            }
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *p = (const char *)
                    sqlite3_column_text(stmt, 0);
                if (p != NULL) {
                    strncat(hasil, p,
                            ukuran - strlen(hasil) - 1);
                }
            }
            sqlite3_finalize(stmt);
        }
        return (hasil[0] != '\0') ? BENAR : SALAH;
    }

    /* handler: persetujuan, penolakan, konfirmasi */
    if (strcmp(handler, "persetujuan") == 0 ||
        strcmp(handler, "penolakan") == 0 ||
        strcmp(handler, "konfirmasi") == 0) {
        const char *tipe_jw = NULL;
        sqlite3_stmt *stmt;
        const char *sql;
        char sql_buf[MAKS_QUERY];
        int rc;

        tipe_jw = cari_tipe_jawaban_intent(intent->nama);
        if (tipe_jw == NULL) {
            tipe_jw = ambil_konfigurasi(
                "tipe_jawaban_default");
        }
        if (tipe_jw == NULL) tipe_jw = ambil_konfigurasi("tipe_jawaban_netral");

        /* jika tipe_jawaban tidak tersedia, skip query */
        if (tipe_jw == NULL) {
            return SALAH;
        }

        snprintf(sql_buf, MAKS_QUERY,
                 "SELECT pola FROM pola_respons "
                 "WHERE tipe_jawaban = '%s' "
                 "ORDER BY RANDOM() LIMIT 1;", tipe_jw);
        sql = sql_buf;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *p = (const char *)
                    sqlite3_column_text(stmt, 0);
                if (p != NULL) SALIN(hasil, p, ukuran);
            }
            sqlite3_finalize(stmt);
        }
        return (hasil[0] != '\0') ? BENAR : SALAH;
    }

    /* handler: arti_lain */
    if (strcmp(handler, "arti_lain") == 0) {
        penalaran_arti_lain(intent, hasil, ukuran);
        panjang = (int)strlen(hasil);
        return (panjang > 0) ? BENAR : SALAH;
    }

    /* handler: definisi */
    if (strcmp(handler, "definisi") == 0) {
        penalaran_definisi(intent, hasil, ukuran);
        panjang = (int)strlen(hasil);
        return (panjang > 0) ? BENAR : SALAH;
    }

    /* handler: bandingkan */
    if (strcmp(handler, "bandingkan") == 0) {
        penalaran_bandingkan(intent, hasil, ukuran);
        panjang = (int)strlen(hasil);
        return (panjang > 0) ? BENAR : SALAH;
    }

    /* handler: kemampuan */
    if (strcmp(handler, "kemampuan") == 0) {
        sqlite3_stmt *stmt;
        const char *ent_bot = ambil_konfigurasi("kemampuan_entitas");
        const char *asp1 = ambil_konfigurasi("kemampuan_aspek_1");
        const char *asp2 = ambil_konfigurasi("kemampuan_aspek_2");
        char sql_buf[MAKS_QUERY];
        int rc;

        if (ent_bot == NULL) ent_bot = "bot";
        if (asp1 == NULL) asp1 = "kemampuan";
        if (asp2 == NULL) asp2 = "keterbatasan";
        snprintf(sql_buf, MAKS_QUERY,
                 "SELECT judul, penjelasan "
                 "FROM pengetahuan_umum "
                 "WHERE entitas = '%s' "
                 "AND (aspek = '%s' "
                 "  OR aspek = '%s') "
                 "ORDER BY urutan",
                 ent_bot, asp1, asp2);
        rc = sqlite3_prepare_v2(db, sql_buf, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *j = (const char *)
                    sqlite3_column_text(stmt, 0);
                const char *p = (const char *)
                    sqlite3_column_text(stmt, 1);
                char buf[MAKS_KARAKTER * 2];
                if (j != NULL && p != NULL) {
                    snprintf(buf, sizeof(buf), "- %s: %s\n",
                             j, p);
                    strncat(hasil, buf,
                            ukuran - strlen(hasil) - 1);
                }
            }
            sqlite3_finalize(stmt);
        }
        panjang = (int)strlen(hasil);
        return (panjang > 0) ? BENAR : SALAH;
    }

    /* handler: verifikasi */
    if (strcmp(handler, "verifikasi") == 0) {
        const char *aspek_def = NULL;
        sqlite3_stmt *stmt;
        const char *sql;

        aspek_def = ambil_konfigurasi("aspek_default");
        /* aspek_def sudah NULL jika tidak ada di DB */

        sql = "SELECT judul, penjelasan "
              "FROM pengetahuan_umum "
              "WHERE entitas = ? AND aspek = ? "
              "ORDER BY urutan LIMIT 1;";
        {
            int rc = sqlite3_prepare_v2(db, sql, -1, &stmt,
                                        NULL);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, intent->entitas1,
                                  -1, SQLITE_STATIC);
                if (intent->aspek[0] != '\0') {
                    sqlite3_bind_text(stmt, 2, intent->aspek,
                                      -1, SQLITE_STATIC);
                } else if (aspek_def != NULL) {
                    sqlite3_bind_text(stmt, 2, aspek_def,
                                      -1, SQLITE_STATIC);
                }
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    const char *j = (const char *)
                        sqlite3_column_text(stmt, 0);
                    const char *p = (const char *)
                        sqlite3_column_text(stmt, 1);
                    if (j != NULL && p != NULL) {
                        snprintf(hasil, ukuran,
                                 "%s: %s", j, p);
                    }
                }
                sqlite3_finalize(stmt);
            }
        }
        panjang = (int)strlen(hasil);
        return (panjang > 0) ? BENAR : SALAH;
    }

    /* handler: konversi */
    if (strcmp(handler, "konversi") == 0) {
        penalaran_konversi(dt, hasil, ukuran);
        panjang = (int)strlen(hasil);
        return (panjang > 0) ? BENAR : SALAH;
    }

    /* handler: matematika */
    if (strcmp(handler, "matematika") == 0) {
        penalaran_matematika(dt, hasil, ukuran);
        panjang = (int)strlen(hasil);
        return (panjang > 0) ? BENAR : SALAH;
    }

    /* fallback: coba pengetahuan umum untuk handler lain */
fallback_pengetahuan:
    if (intent->entitas1[0] != '\0') {
        penalaran_pengetahuan(intent, hasil, ukuran);
        panjang = (int)strlen(hasil);
        if (panjang > 0) return BENAR;

        /* coba definisi kata */
        penalaran_definisi(intent, hasil, ukuran);
        panjang = (int)strlen(hasil);
        if (panjang > 0) return BENAR;
    }

    return SALAH;
}
