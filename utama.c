/* ================================================================
 * utama.c - Titik Masuk Utama dan Utilitas Bot
 * Fungsi main, proses pipeline, log, konfigurasi
 * ================================================================ */

#include "bot.h"

/* ================================================================
 * VARIABEL GLOBAL - LOG
 * ================================================================ */

static FILE *berkas_log = NULL;
static int log_aktif = SALAH;

/* ================================================================
 * FUNGSI UTILITAS
 * ================================================================ */

/* ---------------------------------------------------------------
 * huruf_kecil - Konversi seluruh karakter ke huruf kecil
 * (lowercase). Aman untuk karakter ASCII dan Latin-1 dasar.
 * --------------------------------------------------------------- */
void huruf_kecil(char *teks)
{
    int i;
    for (i = 0; teks[i] != '\0'; i++) {
        teks[i] = (char)tolower((unsigned char)teks[i]);
    }
}

/* ---------------------------------------------------------------
 * hapus_tanda_baca - Hapus tanda baca dari teks
 * Mempertahankan huruf, angka, dan spasi saja.
 * --------------------------------------------------------------- */
void hapus_tanda_baca(char *teks)
{
    int i, j = 0;
    for (i = 0; teks[i] != '\0'; i++) {
        if (isalnum((unsigned char)teks[i]) ||
            teks[i] == ' ' || teks[i] == '-') {
            teks[j++] = teks[i];
        }
    }
    teks[j] = '\0';
}

/* ---------------------------------------------------------------
 * bersihkan_teks - Normalisasi teks: hapus spasi berlebih
 * di awal/akhir dan spasi ganda di tengah
 * --------------------------------------------------------------- */
void bersihkan_teks(char *teks)
{
    int i, j = 0;
    int spasi_sebelumnya = BENAR;

    /* hapus spasi di awal */
    for (i = 0; teks[i] == ' '; i++) {
        /* kosong */
    }

    /* proses sisa teks */
    for (; teks[i] != '\0'; i++) {
        if (teks[i] == ' ') {
            if (!spasi_sebelumnya) {
                teks[j++] = ' ';
                spasi_sebelumnya = BENAR;
            }
        } else {
            teks[j++] = teks[i];
            spasi_sebelumnya = SALAH;
        }
    }

    /* hapus spasi di akhir */
    if (j > 0 && teks[j - 1] == ' ') {
        j--;
    }
    teks[j] = '\0';
}

/* ---------------------------------------------------------------
 * teks_kosong - Cek apakah teks kosong atau hanya spasi
 * Mengembalikan BENAR jika kosong.
 * --------------------------------------------------------------- */
int teks_kosong(const char *teks)
{
    int i;
    if (teks == NULL) return BENAR;
    for (i = 0; teks[i] != '\0'; i++) {
        if (teks[i] != ' ' && teks[i] != '\t' &&
            teks[i] != '\n' && teks[i] != '\r') {
            return SALAH;
        }
    }
    return BENAR;
}

/* ================================================================
 * FUNGSI LOG
 * ================================================================ */

/* ---------------------------------------------------------------
 * buka_log - Buka berkas log untuk pencatatan kinerja
 * Mengembalikan BENAR jika berhasil.
 * --------------------------------------------------------------- */
static int buka_log(const char *nama_file)
{
    berkas_log = fopen(nama_file, "a");
    if (berkas_log == NULL) {
        fprintf(stderr,
                "gagal membuka log: %s\n", nama_file);
        return SALAH;
    }
    log_aktif = BENAR;
    return BENAR;
}

/* ---------------------------------------------------------------
 * tutup_log - Tutup berkas log dengan aman
 * --------------------------------------------------------------- */
static void tutup_log(void)
{
    if (berkas_log != NULL) {
        fclose(berkas_log);
        berkas_log = NULL;
    }
    log_aktif = SALAH;
}

/* ---------------------------------------------------------------
 * catat_log - Tulis entri log dengan format standar
 * Format: [WAKTU] [FASE] pesan
 * --------------------------------------------------------------- */
static void catat_log(const char *fase, const char *pesan)
{
    char waktu[MAKS_KARAKTER];
    time_t kini;
    struct tm *info;

    if (!log_aktif || berkas_log == NULL) return;

    time(&kini);
    info = localtime(&kini);
    strftime(waktu, MAKS_KARAKTER, "%Y-%m-%d %H:%M:%S",
             info);

    fprintf(berkas_log, "[%s] [%s] %s\n",
            waktu, fase, pesan);
    fflush(berkas_log);
}

/* ================================================================
 * FUNGSI KONFIGURASI
 * ================================================================ */

/* ---------------------------------------------------------------
 * muat_konfigurasi - Baca berkas konfigurasi .txt
 * Format per baris: kunci=nilai
 * Mengembalikan BENAR jika berhasil.
 * --------------------------------------------------------------- */
typedef struct {
    char nama_db[MAKS_NAMA_DB];
    char nama_log[MAKS_NAMA_LOG];
    int log_level;
} Konfigurasi;

static int muat_konfigurasi(const char *nama_file,
                             Konfigurasi *cfg)
{
    FILE *f;
    char baris[MAKS_BARIS_LOG];
    char kunci[MAKS_KARAKTER];
    char nilai[MAKS_BARIS_LOG];
    int baris_no = 0;

    /* default */
    SALIN(cfg->nama_db, "bot.db", MAKS_NAMA_DB);
    SALIN(cfg->nama_log, "bot.log", MAKS_NAMA_LOG);
    cfg->log_level = 0;

    f = fopen(nama_file, "r");
    if (f == NULL) {
        /* konfigurasi default, bukan error */
        return BENAR;
    }

    while (fgets(baris, MAKS_BARIS_LOG, f) != NULL) {
        char *tanda_sama;
        baris_no++;

        /* hapus newline */
        {
            int panjang = (int)strlen(baris);
            if (panjang > 0 && baris[panjang - 1] == '\n') {
                baris[panjang - 1] = '\0';
            }
        }

        /* lewati baris kosong dan komentar */
        if (teks_kosong(baris)) continue;
        if (baris[0] == '#' || baris[0] == ';') continue;

        /* cari tanda = */
        tanda_sama = strchr(baris, '=');
        if (tanda_sama == NULL) continue;

        /* pecah kunci dan nilai */
        {
            int panjang_kunci = (int)(tanda_sama - baris);
            if (panjang_kunci >= MAKS_KARAKTER)
                panjang_kunci = MAKS_KARAKTER - 1;
            memcpy(kunci, baris, panjang_kunci);
            kunci[panjang_kunci] = '\0';

            SALIN(nilai, tanda_sama + 1, MAKS_BARIS_LOG);
            bersihkan_teks(nilai);
        }

        /* terapkan konfigurasi */
        if (strcmp(kunci, "database") == 0) {
            SALIN(cfg->nama_db, nilai, MAKS_NAMA_DB);
        } else if (strcmp(kunci, "log") == 0) {
            SALIN(cfg->nama_log, nilai, MAKS_NAMA_LOG);
        } else if (strcmp(kunci, "log_level") == 0) {
            cfg->log_level = atoi(nilai);
        }
    }

    fclose(f);
    return BENAR;
}

/* ================================================================
 * proses - Pipeline utama pemrosesan satu input
 * Menjalankan fase 1 sampai 10 secara berurutan.
 * Mengisi jawaban dengan respons bot.
 * ================================================================ */
void proses(const char *input, char *jawaban, int ukuran,
            KonteksSesi *ks)
{
    DaftarToken dt;
    Spok spok;
    Intent intent;
    char isi[MAKS_HASIL];
    int ada_hasil;
    char input_bersih[MAKS_KALIMAT];
    clock_t mulai, selesai;
    double durasi_ms;

    /* inisialisasi semua struktur */
    memset(&dt, 0, sizeof(dt));
    memset(&spok, 0, sizeof(spok));
    memset(&intent, 0, sizeof(intent));
    memset(isi, 0, sizeof(isi));

    /* salin dan bersihkan input */
    SALIN(input_bersih, input, MAKS_KALIMAT);
    huruf_kecil(input_bersih);
    bersihkan_teks(input_bersih);

    mulai = clock();
    catat_log("MULAI", input_bersih);

    /* ===== FASE 1: TOKENISASI ===== */
    fase1_tokenisasi(input_bersih, &dt);
    if (log_aktif && berkas_log != NULL) {
        char buf[MAKS_BARIS_LOG];
        snprintf(buf, MAKS_BARIS_LOG,
                 "token: %d token terbentuk", dt.jumlah);
        catat_log("F1_TOKENISASI", buf);
    }

    /* ===== FASE 2: MORFOLOGI ===== */
    fase2_morfologi(&dt);

    /* ===== FASE 3: POS TAGGING ===== */
    fase3_pos(&dt, ks);

    /* ===== FASE 4: ENTITY RECOGNITION ===== */
    fase4_entitas(&dt);

    /* ===== FASE 5: SPOK PARSING ===== */
    fase5_spok(&dt, &spok);

    /* ===== FASE 6: INTENT CLASSIFICATION ===== */
    fase6_intent(&dt, &intent);
    if (log_aktif && berkas_log != NULL) {
        char buf[MAKS_BARIS_LOG];
        snprintf(buf, MAKS_BARIS_LOG,
                 "intent=%s entitas1=%s entitas2=%s "
                 "aspek=%s prioritas=%d",
                 intent.nama, intent.entitas1,
                 intent.entitas2, intent.aspek,
                 intent.prioritas);
        catat_log("F6_INTENT", buf);
    }

    /* ===== FASE 7: CONTEXT RESOLUTION ===== */
    fase7_konteks(&dt, &intent, ks);

    /* ===== FASE 8: PENALARAN (RULE ENGINE) ===== */
    ada_hasil = fase8_penalaran(&dt, &spok, &intent, ks,
                                isi, MAKS_HASIL);
    if (log_aktif && berkas_log != NULL) {
        char buf[MAKS_BARIS_LOG];
        snprintf(buf, MAKS_BARIS_LOG,
                 "penalaran: ada_hasil=%d panjang_isi=%d",
                 ada_hasil, (int)strlen(isi));
        catat_log("F8_PENALARAN", buf);
    }

    /* ===== FASE 9: RESPONSE GENERATION ===== */
    fase9_respons(&intent, ks,
                  ada_hasil ? isi : NULL,
                  jawaban, ukuran);

    /* ===== FASE 10: SIMPAN RIWAYAT ===== */
    fase10_riwayat(input_bersih, jawaban, &intent, ks);

    selesai = clock();
    durasi_ms = ((double)(selesai - mulai) /
                 CLOCKS_PER_SEC) * 1000.0;

    if (log_aktif && berkas_log != NULL) {
        char buf[MAKS_BARIS_LOG];
        snprintf(buf, MAKS_BARIS_LOG,
                 "SELESAI durasi=%.1fms respons=%d karakter",
                 durasi_ms, (int)strlen(jawaban));
        catat_log("SELESAI", buf);
    }

    (void)ukuran;
}

/* ================================================================
 * tampilkan_cara_pakai - Tampilkan bantuan penggunaan
 * --------------------------------------------------------------- */
static void tampilkan_cara_pakai(const char *nama_program)
{
    printf("Penggunaan: %s [pilihan]\n\n", nama_program);
    printf("Pilihan:\n");
    printf("  -c <file>    berkas konfigurasi "
           "(default: konfigurasi.txt)\n");
    printf("  -d <file>    berkas database "
           "(default: bot.db)\n");
    printf("  -l <file>    aktifkan log ke berkas\n");
    printf("  -h           tampilkan bantuan ini\n");
    printf("\nContoh:\n");
    printf("  %s                  # default\n",
           nama_program);
    printf("  %s -l bot.log       # dengan log\n",
           nama_program);
    printf("  %s -d data.db -l bot.log\n",
           nama_program);
}

/* ================================================================
 * main - Titik Masuk Utama Aplikasi Bot
 * Membuka database, memulai sesi, loop percakapan.
 * ================================================================ */
int main(int argc, char *argv[])
{
    char *baris = NULL;
    size_t ukuran_baris = 0;
    ssize_t panjang;
    char jawaban[MAKS_JAWABAN];
    KonteksSesi ks;
    Konfigurasi cfg;
    int sesi_aktif;
    int gunakan_log_arg = SALAH;
    char nama_log_arg[MAKS_NAMA_LOG];
    int gunakan_db_arg = SALAH;
    char nama_db_arg[MAKS_NAMA_DB];
    char nama_konfig[MAKS_NAMA_DB];
    int i;

    /* default konfigurasi */
    SALIN(nama_konfig, "konfigurasi.txt", MAKS_NAMA_DB);
    nama_log_arg[0] = '\0';
    nama_db_arg[0] = '\0';

    /* parse argumen baris perintah */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 ||
            strcmp(argv[i], "--help") == 0) {
            tampilkan_cara_pakai(argv[0]);
            return KELUAR_BERHASIL;
        } else if (strcmp(argv[i], "-l") == 0 ||
                   strcmp(argv[i], "--log") == 0) {
            if (i + 1 < argc) {
                i++;
                SALIN(nama_log_arg, argv[i], MAKS_NAMA_LOG);
                gunakan_log_arg = BENAR;
            }
        } else if (strcmp(argv[i], "-d") == 0 ||
                   strcmp(argv[i], "--db") == 0) {
            if (i + 1 < argc) {
                i++;
                SALIN(nama_db_arg, argv[i], MAKS_NAMA_DB);
                gunakan_db_arg = BENAR;
            }
        } else if (strcmp(argv[i], "-c") == 0 ||
                   strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                i++;
                SALIN(nama_konfig, argv[i], MAKS_NAMA_DB);
            }
        }
    }

    /* muat konfigurasi dari berkas */
    muat_konfigurasi(nama_konfig, &cfg);

    /* argumen CLI lebih prioritas dari konfigurasi */
    if (gunakan_db_arg) {
        SALIN(cfg.nama_db, nama_db_arg, MAKS_NAMA_DB);
    }
    if (gunakan_log_arg) {
        SALIN(cfg.nama_log, nama_log_arg, MAKS_NAMA_LOG);
        cfg.log_level = 1;
    }

    /* buka log jika diminta */
    if (cfg.log_level > 0 && cfg.nama_log[0] != '\0') {
        buka_log(cfg.nama_log);
    }

    catat_log("SISTEM", "bot dimulai");

    /* buka database */
    if (!buka_database(cfg.nama_db)) {
        fprintf(stderr, "gagal membuka database: %s\n",
                cfg.nama_db);
        tutup_log();
        return KELUAR_GAGAL;
    }

    catat_log("SISTEM", "database berhasil dibuka");

    /* buat skema tabel jika belum ada */
    if (!buat_skema()) {
        fprintf(stderr,
                "gagal membuat skema database.\n");
        tutup_database();
        tutup_log();
        return KELUAR_GAGAL;
    }

    catat_log("SISTEM", "skema database siap");

    /* impor datasheet otomatis jika database masih kosong */
    if (!import_datasheet_jika_kosong()) {
        fprintf(stderr,
                "peringatan: gagal mengimpor datasheet.\n");
        /* tidak fatal, bot tetap jalan tanpa data */
    }

    /* muat data konfigurasi ke cache memori */
    if (!muat_cache_db()) {
        fprintf(stderr,
                "gagal memuat cache database.\n");
        tutup_database();
        tutup_log();
        return KELUAR_GAGAL;
    }

    catat_log("SISTEM", "cache database dimuat");

    /* mulai sesi percakapan */
    sesi_aktif = mulai_sesi(&ks);
    if (!sesi_aktif) {
        fprintf(stderr,
                "gagal memulai sesi percakapan.\n");
        tutup_database();
        tutup_log();
        return KELUAR_GAGAL;
    }

    catat_log("SISTEM", "sesi percakapan dimulai");

    /* loop utama percakapan */
    for (;;) {
        /* tampilkan prompt */
        printf("User: ");
        fflush(stdout);

        /* baca input dari stdin */
        panjang = getline(&baris, &ukuran_baris, stdin);
        if (panjang == -1) {
            /* EOF (Ctrl+D) */
            printf("\n");
            break;
        }

        /* hapus newline di akhir */
        if (panjang > 0 && baris[panjang - 1] == '\n') {
            baris[panjang - 1] = '\0';
            panjang--;
        }

        /* bersihkan input */
        bersihkan_teks(baris);

        /* lewati input kosong */
        if (teks_kosong(baris)) {
            continue;
        }

        /* proses input melalui pipeline */
        jawaban[0] = '\0';
        proses(baris, jawaban, sizeof(jawaban), &ks);

        /* cek perintah ganti sesi */
        {
            const char *sinyal_ganti =
                ambil_konfigurasi("ganti_sesi");
            if (sinyal_ganti != NULL &&
                strcmp(jawaban, sinyal_ganti) == 0) {
                ganti_sesi(&ks);
                catat_log("SISTEM", "sesi diganti");
                printf("Bot: %s\n\n",
                       ambil_konfigurasi(
                           "pesan_ganti_sesi") != NULL
                       ? ambil_konfigurasi(
                             "pesan_ganti_sesi")
                       : "");
                continue;
            }
        }

        /* cek perintah akhiri sesi */
        {
            const char *sinyal_akhiri =
                ambil_konfigurasi("akhiri_sesi");
            if (sinyal_akhiri != NULL &&
                strcmp(jawaban, sinyal_akhiri) == 0) {
                akhiri_sesi();
                catat_log("SISTEM", "sesi diakhiri");
                printf("Bot: %s\n\n",
                       ambil_konfigurasi(
                           "pesan_akhiri_sesi") != NULL
                       ? ambil_konfigurasi(
                             "pesan_akhiri_sesi")
                       : "");
                break;
            }
        }

        /* tampilkan jawaban bot */
        printf("\nBot: ");
        if (jawaban[0] != '\0') {
            printf("%s", jawaban);
        } else {
            const char *pk =
                ambil_konfigurasi("jawaban_kosong");
            if (pk != NULL) {
                printf("%s", pk);
            }
        }
        printf("\n\n");
    }

    /* bersihkan */
    if (baris != NULL) {
        free(baris);
    }

    /* tutup database */
    tutup_database();
    catat_log("SISTEM", "bot dihentikan");
    tutup_log();

    return KELUAR_BERHASIL;
}
