/* ================================================================
 * keluaran.c - Fase 9 sampai 10: Respons, Riwayat, Sesi
 * Pembuka, isi, penutup respons. Manajemen sesi percakapan.
 * ================================================================ */

#include "bot.h"

/* ================================================================
 * FUNGSI BANTUAN INTERNAL
 * ================================================================ */

/* ---------------------------------------------------------------
 * ambil_string_acak - Ambil satu baris acak dari query SQL
 * Mengembalikan BENAR jika berhasil.
 * --------------------------------------------------------------- */
static int ambil_string_acak(const char *sql, char *hasil,
                             int ukuran)
{
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *t = (const char *)sqlite3_column_text(stmt, 0);
        if (t != NULL) {
            SALIN(hasil, t, ukuran);
            sqlite3_finalize(stmt);
            return BENAR;
        }
    }

    sqlite3_finalize(stmt);
    return SALAH;
}

/* ---------------------------------------------------------------
 * ambil_ganti_persona - Ambil kata ganti pengganti berdasarkan
 * jenis_persona dan gaya bahasa dari database
 * --------------------------------------------------------------- */
static void ambil_ganti_persona(const char *jenis_persona,
                                const char *gaya,
                                char *ganti, int ukuran)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT ganti_dengan FROM referensi_konteks "
                      "WHERE jenis_persona = ? "
                      "AND (gaya = ? OR gaya IS NULL) "
                      "AND ganti_dengan IS NOT NULL "
                      "ORDER BY CASE WHEN gaya = ? THEN 0 "
                      "ELSE 1 END LIMIT 1;";
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, jenis_persona,
                      -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, gaya, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, gaya, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *g = (const char *)sqlite3_column_text(stmt, 0);
        if (g != NULL) {
            SALIN(ganti, g, ukuran);
        }
    }

    sqlite3_finalize(stmt);
}

/* ---------------------------------------------------------------
 * ganti_teks_placeholder - Ganti satu placeholder dalam teks
 * dengan string pengganti. Aman terhadap buffer overflow.
 * --------------------------------------------------------------- */
static void ganti_teks_placeholder(char *teks, int ukuran,
                                    const char *placeholder,
                                    const char *pengganti)
{
    char *pos;
    int panjang_ph = (int)strlen(placeholder);
    int panjang_pg = (int)strlen(pengganti);

    while ((pos = strstr(teks, placeholder)) != NULL) {
        char baru[MAKS_JAWABAN];
        int panjang_kiri = (int)(pos - teks);
        int panjang_sisa;

        if (panjang_kiri >= MAKS_JAWABAN) break;

        memcpy(baru, teks, panjang_kiri);
        baru[panjang_kiri] = '\0';

        if (panjang_pg > 0) {
            strncat(baru, pengganti,
                    MAKS_JAWABAN - strlen(baru) - 1);
        }

        panjang_sisa = (int)strlen(pos + panjang_ph);
        if (panjang_kiri + strlen(baru) + panjang_sisa
            < MAKS_JAWABAN) {
            strncat(baru, pos + panjang_ph,
                    MAKS_JAWABAN - strlen(baru) - 1);
        }

        SALIN(teks, baru, ukuran);
    }
}

/* ---------------------------------------------------------------
 * ganti_persona_dalam_teks - Replace placeholder dengan nilai
 * aktual menggunakan konfigurasi dari basisdata
 * --------------------------------------------------------------- */
static void ganti_persona_dalam_teks(const char *gaya_bahasa,
                                     const Intent *intent,
                                     char *teks, int ukuran)
{
    const char *jenis_orang_pertama;
    const char *jenis_orang_kedua;
    char ganti_user[MAKS_KARAKTER];
    char ganti_bot[MAKS_KARAKTER];

    ganti_user[0] = '\0';
    ganti_bot[0] = '\0';

    jenis_orang_pertama = ambil_konfigurasi("persona_orang_pertama");
    jenis_orang_kedua = ambil_konfigurasi("persona_orang_kedua");

    /* jenis_orang_pertama/kedua = NULL jika tidak ada di DB,
       ambil_ganti_persona akan bind NULL (aman untuk sqlite3) */

    ambil_ganti_persona(jenis_orang_pertama, gaya_bahasa,
                        ganti_user, MAKS_KARAKTER);
    ambil_ganti_persona(jenis_orang_kedua, gaya_bahasa,
                        ganti_bot, MAKS_KARAKTER);

    {
        const char *ph_user;
        const char *ph_bot;
        const char *ph_topik;
        const char *ph_aspek;
        const char *fb_topik;
        const char *fb_aspek;

        ph_user = ambil_konfigurasi("placeholder_user");
        ph_bot = ambil_konfigurasi("placeholder_bot");
        ph_topik = ambil_konfigurasi("placeholder_topik");
        ph_aspek = ambil_konfigurasi("placeholder_aspek");
        fb_topik = ambil_konfigurasi("fallback_topik");
        fb_aspek = ambil_konfigurasi("fallback_aspek");

        /* ph_user/ph_bot/ph_topik/ph_aspek/fb_topik/fb_aspek
           = NULL jika tidak ada di DB,
           placeholder tidak di-replace */

        /* replace semua placeholder */
        if (ph_user != NULL) {
            ganti_teks_placeholder(teks, ukuran,
                                  ph_user, ganti_user);
        }
        if (ph_bot != NULL) {
            ganti_teks_placeholder(teks, ukuran,
                                  ph_bot, ganti_bot);
        }

        /* replace placeholder topik dengan entitas_aktif dari intent */
        if (ph_topik != NULL) {
            if (intent->entitas1[0] != '\0') {
                ganti_teks_placeholder(teks, ukuran,
                                      ph_topik, intent->entitas1);
            } else if (fb_topik != NULL) {
                ganti_teks_placeholder(teks, ukuran,
                                      ph_topik, fb_topik);
            }
        }

        /* replace placeholder aspek dengan aspek dari intent */
        if (ph_aspek != NULL) {
            if (intent->aspek[0] != '\0') {
                ganti_teks_placeholder(teks, ukuran,
                                      ph_aspek, intent->aspek);
            } else if (fb_aspek != NULL) {
                ganti_teks_placeholder(teks, ukuran,
                                      ph_aspek, fb_aspek);
            }
        }
    }
}

/* ---------------------------------------------------------------
 * cek_aspek_tersedia - Cek aspek lain yang tersedia
 * untuk entitas tertentu. Mengembalikan BENAR jika ada.
 * --------------------------------------------------------------- */
static int cek_aspek_tersedia(const char *entitas)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(DISTINCT aspek) FROM "
                      "pengetahuan_umum WHERE entitas = ?";
    int rc, jumlah = 0;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, entitas, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        jumlah = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return (jumlah > 0) ? BENAR : SALAH;
}

/* ---------------------------------------------------------------
 * cek_arti_lain - Cek apakah kata punya banyak arti
 * --------------------------------------------------------------- */
static int cek_arti_lain(const char *kata)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM arti_kata "
                      "WHERE kata = ?";
    int rc, jumlah = 0;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, kata, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        jumlah = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return (jumlah > 1) ? BENAR : SALAH;
}

/* ================================================================
 * FASE 9: RESPONSE GENERATION
 * Membangun respons lengkap: pembuka + isi + penutup
 * dengan persona mapping
 * ================================================================ */
void fase9_respons(Intent *intent, KonteksSesi *ks,
                   const char *isi, char *jawaban, int ukuran)
{
    char pembuka[MAKS_KARAKTER];
    char penutup[MAKS_JAWABAN];
    const char *gaya;
    int ada_isi;
    int ada_penutup = SALAH;

    jawaban[0] = '\0';
    pembuka[0] = '\0';
    penutup[0] = '\0';

    ada_isi = (isi != NULL && isi[0] != '\0');

    {
        const char *gaya_default;

        gaya_default = ambil_konfigurasi("gaya_default");

        gaya = (ks->gaya_bahasa[0] != '\0') ?
               ks->gaya_bahasa :
               (gaya_default != NULL ? gaya_default : "");
    }

    /* lewati jika tampilkan langsung */
    if (ada_isi && ambil_tampilkan_langsung(intent->nama)) {
        SALIN(jawaban, isi, ukuran);
        ganti_persona_dalam_teks(gaya, intent,
                                 jawaban, ukuran);
        return;
    }

    /* 9A: PEMBUKA (randomized dari pool) */
    {
        const char *tipe_pembuka;

        if (!ada_isi) {
            tipe_pembuka = ambil_konfigurasi(
                "tipe_pembuka_tanpa_isi");
        } else {
            tipe_pembuka = cari_tipe_pembuka_intent(
                intent->nama);
        }
        if (tipe_pembuka == NULL) {
            tipe_pembuka = ambil_konfigurasi(
                "tipe_pembuka_default");
        }
        /* tipe_pembuka tetap NULL jika tidak ada di DB,
           pembuka tetap kosong (tidak ada hardcoded fallback) */

        if (tipe_pembuka != NULL) {
            char sql[MAKS_QUERY];
            snprintf(sql, MAKS_QUERY,
                     "SELECT pola FROM pola_pembuka "
                     "WHERE tipe = '%s' "
                     "ORDER BY RANDOM() LIMIT 1;",
                     tipe_pembuka);
            ambil_string_acak(sql, pembuka, MAKS_KARAKTER);
        }
    }

    /* 9B: ISI (dari hasil penalaran) */
    /* sudah disiapkan oleh fase 8 */

    /* 9C: PENUTUP/TEASER (kondisional) */
    if (ada_isi) {
        /* cek apakah ada aspek lain */
        if (intent->entitas1[0] != '\0') {
            if (cek_aspek_tersedia(intent->entitas1)) {
                char sql[MAKS_QUERY];
                {
                    const char *kondisi_dl =
                        ambil_konfigurasi(
                            "kondisi_penutup_data_lanjut");
                    if (kondisi_dl == NULL)
                        kondisi_dl = "ada_data_lanjut";
                    snprintf(sql, MAKS_QUERY,
                             "SELECT pola FROM pola_penutup "
                             "WHERE kondisi = '%s' "
                             "ORDER BY RANDOM() LIMIT 1;",
                             kondisi_dl);
                }
                ada_penutup = ambil_string_acak(
                    sql, penutup, MAKS_JAWABAN);
            }
        }

        /* cek apakah ada arti lain */
        if (!ada_penutup && intent->entitas1[0] != '\0' &&
            cek_arti_lain(intent->entitas1)) {
            char sql[MAKS_QUERY];
            {
                const char *kondisi_al =
                    ambil_konfigurasi(
                        "kondisi_penutup_arti_lain");
                if (kondisi_al == NULL)
                    kondisi_al = "ada_arti_lain";
                snprintf(sql, MAKS_QUERY,
                         "SELECT pola FROM pola_penutup "
                         "WHERE kondisi = '%s' "
                         "ORDER BY RANDOM() LIMIT 1;",
                         kondisi_al);
            }
            ada_penutup = ambil_string_acak(
                sql, penutup, MAKS_JAWABAN);
        }
    }
    if (!ada_isi) {
        char sql[MAKS_QUERY];
        {
            const char *kondisi_dh =
                ambil_konfigurasi(
                    "kondisi_penutup_data_habis");
            if (kondisi_dh == NULL)
                kondisi_dh = "data_habis";
            snprintf(sql, MAKS_QUERY,
                     "SELECT pola FROM pola_penutup "
                     "WHERE kondisi = '%s' "
                     "ORDER BY RANDOM() LIMIT 1;",
                     kondisi_dh);
        }
        ada_penutup = ambil_string_acak(
            sql, penutup, MAKS_JAWABAN);
    }

    /* gabungkan: pembuka + isi + penutup */
    if (pembuka[0] != '\0') {
        strncat(jawaban, pembuka,
                ukuran - strlen(jawaban) - 1);
    }

    if (ada_isi) {
        if (pembuka[0] != '\0') {
            strncat(jawaban, " ",
                    ukuran - strlen(jawaban) - 1);
        }
        strncat(jawaban, isi,
                ukuran - strlen(jawaban) - 1);
    }

    if (ada_penutup) {
        if (jawaban[0] != '\0') {
            strncat(jawaban, "\n\n",
                    ukuran - strlen(jawaban) - 1);
        }
        strncat(jawaban, penutup,
                ukuran - strlen(jawaban) - 1);
    }

    /* persona mapping: ganti semua placeholder */
    ganti_persona_dalam_teks(gaya, intent, jawaban, ukuran);
}

/* ================================================================
 * FASE 10: SIMPAN RIWAYAT
 * Log percakapan ke riwayat_chat dan update konteks_sesi
 * ================================================================ */
void fase10_riwayat(const char *input, const char *jawaban,
                    const Intent *intent, const KonteksSesi *ks)
{
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;
    char waktu[MAKS_KARAKTER];
    time_t kini;
    struct tm *info;

    /* ambil waktu saat ini */
    time(&kini);
    info = localtime(&kini);
    strftime(waktu, MAKS_KARAKTER, "%Y-%m-%d %H:%M:%S",
             info);

    /* simpan ke riwayat_chat */
    sql = "INSERT INTO riwayat_chat "
          "(sesi_id, pengguna, bot_respon, topik, intent, "
          "timestamp) VALUES (?, ?, ?, ?, ?, ?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, ks->id);
        sqlite3_bind_text(stmt, 2, input, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, jawaban, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, ks->topik,
                          -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, intent->nama,
                          -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, waktu, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    /* update konteks_sesi: increment percakapan */
    sql = "UPDATE konteks_sesi SET percakapan_ke = ?, "
          "intent_terakhir = ?, timestamp = ? "
          "WHERE sesi_id = ? AND id = "
          "(SELECT MAX(id) FROM konteks_sesi "
          "WHERE sesi_id = ?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, ks->percakapan_ke + 1);
        sqlite3_bind_text(stmt, 2, intent->nama,
                          -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, waktu, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, ks->id);
        sqlite3_bind_int(stmt, 5, ks->id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

/* ================================================================
 * MANAJEMEN SESI
 * ================================================================ */

/* ---------------------------------------------------------------
 * mulai_sesi - Buat sesi percakapan baru
 * Mengembalikan BENAR jika berhasil.
 * --------------------------------------------------------------- */
int mulai_sesi(KonteksSesi *ks)
{
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;
    char waktu[MAKS_KARAKTER];
    time_t kini;
    struct tm *info;
    const char *status_akt;
    char sql_buf[MAKS_QUERY];

    memset(ks, 0, sizeof(*ks));
    ks->gaya_bahasa[0] = '\0';

    time(&kini);
    info = localtime(&kini);
    strftime(waktu, MAKS_KARAKTER, "%Y-%m-%d %H:%M:%S",
             info);

    /* buat sesi baru */
    status_akt = ambil_konfigurasi("status_aktif");
    snprintf(sql_buf, MAKS_QUERY,
             "INSERT INTO sesi (nomor_sesi, status, waktu_mulai) "
             "VALUES (COALESCE((SELECT MAX(nomor_sesi) "
             "FROM sesi), 0) + 1, ?, ?);");

    rc = sqlite3_prepare_v2(db, sql_buf, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_text(stmt, 1, status_akt, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, waktu, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return SALAH;

    /* ambil ID sesi yang baru dibuat */
    sql = "SELECT id FROM sesi ORDER BY id DESC LIMIT 1;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        ks->id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (ks->id <= 0) return SALAH;

    /* buat konteks sesi */
    sql = "INSERT INTO konteks_sesi "
          "(sesi_id, percakapan_ke, timestamp) "
          "VALUES (?, 0, ?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return SALAH;

    sqlite3_bind_int(stmt, 1, ks->id);
    sqlite3_bind_text(stmt, 2, waktu, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return BENAR;
}

/* ---------------------------------------------------------------
 * ganti_sesi - Archive sesi lama, buat sesi baru
 * --------------------------------------------------------------- */
void ganti_sesi(KonteksSesi *ks)
{
    char waktu[MAKS_KARAKTER];
    time_t kini;
    struct tm *info;
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;
    const char *status_arsip;

    time(&kini);
    info = localtime(&kini);
    strftime(waktu, MAKS_KARAKTER, "%Y-%m-%d %H:%M:%S",
             info);

    /* archive sesi lama */
    status_arsip = ambil_konfigurasi("status_arsip");
    sql = "UPDATE sesi SET status = ?, "
          "waktu_akhir = ? WHERE id = ?;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, status_arsip,
                          -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, waktu, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, ks->id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    /* buat sesi baru */
    mulai_sesi(ks);
}

/* ---------------------------------------------------------------
 * akhiri_sesi - Akhiri sesi aktif
 * --------------------------------------------------------------- */
void akhiri_sesi(void)
{
    /* tidak ada operasi khusus, sesi tetap tersimpan */
}
