/* ================================================================
 * basisdata.c - Operasi Database Bot
 * Membuka, menutup, membuat skema tabel, memuat cache,
 * dan menyediakan fungsi pencarian untuk bot Bahasa Indonesia.
 *
 * File ini berisi:
 *   - buka_database()      : membuka file database SQLite3
 *   - tutup_database()     : menutup koneksi database
 *   - buat_skema()         : membuat seluruh tabel dan index
 *   - muat_cache_db()      : memuat data konfigurasi ke memori
 *   - fungsi pencarian     : cek_kategori, cari_handler, dll.
 *
 * Kompatibel C89: variabel dideklarasikan di awal blok.
 * ================================================================ */

#include "bot.h"

/* handle database global */
sqlite3 *db = NULL;

/* ================================================================
 * STRUKTUR CACHE - untuk menyimpan data tabel tambahan di memori
 * agar pencarian lebih cepat tanpa kueri ulang ke database.
 * ================================================================ */

/* Struktur cache untuk tabel kelas_kata (termasuk kategori) */
typedef struct {
    char kelas[128];
    char kategori[128];
} CacheKelasKata;

/* Struktur cache untuk tabel intent_handler */
typedef struct {
    char intent[128];
    char handler[128];
    char tipe_pembuka[64];
    char tipe_jawaban[64];
    int tampilkan_langsung;
    char aksi[256];
} CacheIntentHandler;

/* Struktur cache untuk tabel konfigurasi_sistem */
typedef struct {
    char kunci[128];
    char nilai[512];
} CacheKonfigurasi;

/* Array cache statis dan penghitung jumlah entri */
static CacheKelasKata cache_kelas_kata[512];
static int jumlah_kelas_kata = 0;

static CacheIntentHandler cache_intent_handler[256];
static int jumlah_intent_handler = 0;

static CacheKonfigurasi cache_konfigurasi[256];
static int jumlah_konfigurasi = 0;

/* ---------------------------------------------------------------
 * buka_database - Buka atau buat file database SQLite3
 * mengembalikan BENAR jika berhasil, SALAH jika gagal
 * --------------------------------------------------------------- */
int buka_database(const char *nama_file)
{
    int rc;

    rc = sqlite3_open(nama_file, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "gagal membuka database: %s\n",
                sqlite3_errmsg(db));
        db = NULL;
        return SALAH;
    }

    /* aktifkan foreign key (opsional, untuk konsistensi) */
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    /* gunakan journal mode DELETE (tanpa file -wal/-shm) */
    sqlite3_exec(db, "PRAGMA journal_mode = DELETE;",
                 NULL, NULL, NULL);

    return BENAR;
}

/* ---------------------------------------------------------------
 * tutup_database - Tutup koneksi database dengan aman
 * --------------------------------------------------------------- */
void tutup_database(void)
{
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}

/* ---------------------------------------------------------------
 * import_datasheet_jika_kosong - Mengecek apakah database masih
 * kosong (belum ada data konfigurasi). Jika kosong, otomatis
 * mengimpor datasheet_bot.sql dari direktori kerja.
 *
 * Juga mendeteksi schema lama yang kadaluarsa dengan mengecek
 * keberadaan kolom kategori di tabel kelas_kata. Jika kolom
 * tidak ada, seluruh database di-drop dan dibuat ulang.
 *
 * Mengembalikan BENAR jika berhasil atau sudah ada data,
 * SALAH jika gagal mengimpor.
 * --------------------------------------------------------------- */
int import_datasheet_jika_kosong(void)
{
    sqlite3_stmt *stmt;
    const char *sql_cek;
    int rc;
    int jumlah;
    FILE *f;
    long ukuran;
    char *isi;
    char *pesan_error = NULL;
    const char *nama_file_sql;
    static const char *coba_path[] = {
        "datasheet_bot.sql",
        "./datasheet_bot.sql",
        NULL
    };
    int idx_path = 0;

    /* ---- cek apakah konfigurasi_sistem sudah punya data ---- */
    sql_cek = "SELECT COUNT(*) FROM konfigurasi_sistem;";
    rc = sqlite3_prepare_v2(db, sql_cek, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        goto lakukan_import;
    }

    jumlah = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        jumlah = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (jumlah > 0) {
        return BENAR;
    }

lakukan_import:
    /* ---- cek schema lama ---- */
    sql_cek = "SELECT kategori FROM kelas_kata LIMIT 0;";
    rc = sqlite3_prepare_v2(db, sql_cek, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "mendeteksi schema lama, "
                "membuat ulang database...\n");
        sqlite3_exec(db,
            "DROP TABLE IF EXISTS komponen_makna;"
            "DROP TABLE IF EXISTS riwayat_chat;"
            "DROP TABLE IF EXISTS konteks_sesi;"
            "DROP TABLE IF EXISTS sesi;"
            "DROP TABLE IF EXISTS perintah_sistem;"
            "DROP TABLE IF EXISTS referensi_konteks;"
            "DROP TABLE IF EXISTS intent_pola;"
            "DROP TABLE IF EXISTS pola_respons;"
            "DROP TABLE IF EXISTS pola_penutup;"
            "DROP TABLE IF EXISTS pola_pembuka;"
            "DROP TABLE IF EXISTS relasi_perhitungan;"
            "DROP TABLE IF EXISTS operator_matematika;"
            "DROP TABLE IF EXISTS nama_satuan;"
            "DROP TABLE IF EXISTS konversi_satuan;"
            "DROP TABLE IF EXISTS nama_hari;"
            "DROP TABLE IF EXISTS nama_bulan;"
            "DROP TABLE IF EXISTS waktu_unit;"
            "DROP TABLE IF EXISTS waktu_ekspresi;"
            "DROP TABLE IF EXISTS kata_angka;"
            "DROP TABLE IF EXISTS relasi_entitas;"
            "DROP TABLE IF EXISTS kata_kunci_aspek;"
            "DROP TABLE IF EXISTS daftar_aspek;"
            "DROP TABLE IF EXISTS pengetahuan_umum;"
            "DROP TABLE IF EXISTS struktur_kalimat;"
            "DROP TABLE IF EXISTS jenis_kalimat;"
            "DROP TABLE IF EXISTS majas;"
            "DROP TABLE IF EXISTS imbuhan;"
            "DROP TABLE IF EXISTS contoh_sastra;"
            "DROP TABLE IF EXISTS kata_kiasan;"
            "DROP TABLE IF EXISTS makna_kata;"
            "DROP TABLE IF EXISTS arti_kata;"
            "DROP TABLE IF EXISTS antonim;"
            "DROP TABLE IF EXISTS sinonim;"
            "DROP TABLE IF EXISTS kata_fungsional;"
            "DROP TABLE IF EXISTS kata_turunan;"
            "DROP TABLE IF EXISTS leksikon;"
            "DROP TABLE IF EXISTS kelas_kata;"
            "DROP TABLE IF EXISTS kelas_kategori;"
            "DROP TABLE IF EXISTS intent_handler;"
            "DROP TABLE IF EXISTS konfigurasi_sistem;",
            NULL, NULL, NULL);
    } else {
        sqlite3_finalize(stmt);
    }

    /* ---- cari dan baca file datasheet_bot.sql ---- */
    f = NULL;
    while (coba_path[idx_path] != NULL) {
        f = fopen(coba_path[idx_path], "r");
        if (f != NULL) {
            nama_file_sql = coba_path[idx_path];
            break;
        }
        idx_path++;
    }
    if (f == NULL) {
        fprintf(stderr,
            "peringatan: datasheet_bot.sql tidak ditemukan.\n"
            "pastikan file datasheet_bot.sql ada di "
            "direktori yang sama dengan bot.\n");
        return SALAH;
    }

    fseek(f, 0, SEEK_END);
    ukuran = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (ukuran <= 0) {
        fclose(f);
        fprintf(stderr,
            "peringatan: datasheet_bot.sql kosong.\n");
        return SALAH;
    }

    isi = (char *)malloc((size_t)ukuran + 1);
    if (isi == NULL) {
        fclose(f);
        fprintf(stderr,
            "peringatan: gagal alokasi memori.\n");
        return SALAH;
    }

    if (fread(isi, 1, (size_t)ukuran, f) != (size_t)ukuran) {
        free(isi);
        fclose(f);
        fprintf(stderr,
            "peringatan: gagal membaca datasheet_bot.sql.\n");
        return SALAH;
    }
    isi[ukuran] = '\0';
    fclose(f);

    /* ---- eksekusi SQL ---- */
    rc = sqlite3_exec(db, isi, NULL, NULL, &pesan_error);
    free(isi);

    if (rc != SQLITE_OK) {
        fprintf(stderr,
            "gagal mengimpor %s: %s\n",
            nama_file_sql, pesan_error);
        sqlite3_free(pesan_error);
        return SALAH;
    }

    fprintf(stderr, "datasheet berhasil diimpor.\n");
    return BENAR;
}

/* ---------------------------------------------------------------
 * buat_skema - Membuat seluruh 39 tabel dan index
 * Menggunakan CREATE TABLE IF NOT EXISTS agar aman dipanggil
 * berulang kali. Mengembalikan BENAR jika berhasil.
 * --------------------------------------------------------------- */
int buat_skema(void)
{
    int rc;
    char *pesan_error = NULL;
    const char *sql_tabel_tambahan;

    /* ============================================================
     * SQL pembuatan seluruh tabel (39 tabel)
     * Dikelompokkan sesuai desain: A(Bahasa), B(Pengetahuan),
     * C(Waktu), D(Konversi), E(Matematika), F(Sistem), G(Sesi)
     * ============================================================ */
    const char *sql_tabel =
    /* === KELOMPOK A: BAHASA (14 Tabel) ======================== */
    "CREATE TABLE IF NOT EXISTS kelas_kata ("
    "  id INTEGER PRIMARY KEY,"
    "  kelas TEXT NOT NULL UNIQUE,"
    "  kategori TEXT NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS leksikon ("
    "  id INTEGER PRIMARY KEY,"
    "  id_kata TEXT NOT NULL UNIQUE,"
    "  kata TEXT NOT NULL,"
    "  kelas TEXT NOT NULL,"
    "  asal_kata TEXT,"
    "  frekuensi REAL DEFAULT 0.0,"
    "  berpengetahuan INTEGER DEFAULT 0"
    ");"

    "CREATE TABLE IF NOT EXISTS kata_turunan ("
    "  id INTEGER PRIMARY KEY,"
    "  id_kata_dasar TEXT NOT NULL,"
    "  kata_dasar TEXT NOT NULL,"
    "  kata_turunan TEXT NOT NULL,"
    "  kelas TEXT,"
    "  frekuensi REAL DEFAULT 0.0"
    ");"

    "CREATE TABLE IF NOT EXISTS kata_fungsional ("
    "  id INTEGER PRIMARY KEY,"
    "  kata TEXT NOT NULL UNIQUE,"
    "  kelas TEXT NOT NULL,"
    "  fungsi TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS sinonim ("
    "  id INTEGER PRIMARY KEY,"
    "  kata1 TEXT NOT NULL,"
    "  kata2 TEXT NOT NULL,"
    "  relasi TEXT NOT NULL,"
    "  arti_kata TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS antonim ("
    "  id INTEGER PRIMARY KEY,"
    "  kata1 TEXT NOT NULL,"
    "  kata2 TEXT NOT NULL,"
    "  relasi TEXT NOT NULL,"
    "  arti_kata TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS arti_kata ("
    "  id INTEGER PRIMARY KEY,"
    "  id_kata TEXT NOT NULL UNIQUE,"
    "  kata TEXT NOT NULL,"
    "  arti TEXT NOT NULL,"
    "  contoh_kalimat TEXT,"
    "  frekuensi REAL DEFAULT 1.0,"
    "  tipe TEXT DEFAULT 'harfiah'"
    ");"

    "CREATE TABLE IF NOT EXISTS makna_kata ("
    "  id INTEGER PRIMARY KEY,"
    "  id_kata TEXT NOT NULL UNIQUE,"
    "  kata TEXT NOT NULL,"
    "  kelas TEXT,"
    "  makna TEXT NOT NULL,"
    "  contoh_kalimat TEXT NOT NULL,"
    "  frekuensi REAL DEFAULT 1.0,"
    "  jenis_kata TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS kata_kiasan ("
    "  id INTEGER PRIMARY KEY,"
    "  id_kiasan TEXT NOT NULL UNIQUE,"
    "  kata1 TEXT NOT NULL,"
    "  kata2 TEXT NOT NULL,"
    "  kata TEXT NOT NULL,"
    "  jenis TEXT NOT NULL,"
    "  makna TEXT NOT NULL,"
    "  contoh TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS contoh_sastra ("
    "  id INTEGER PRIMARY KEY,"
    "  jenis TEXT NOT NULL,"
    "  judul TEXT NOT NULL,"
    "  isi TEXT NOT NULL,"
    "  arti TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS imbuhan ("
    "  id INTEGER PRIMARY KEY,"
    "  jenis TEXT NOT NULL,"
    "  imbuhan TEXT NOT NULL,"
    "  fungsi TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS majas ("
    "  id INTEGER PRIMARY KEY,"
    "  jenis TEXT NOT NULL UNIQUE,"
    "  definisi TEXT NOT NULL,"
    "  contoh TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS jenis_kalimat ("
    "  id INTEGER PRIMARY KEY,"
    "  jenis TEXT NOT NULL UNIQUE,"
    "  kelas_kata TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS struktur_kalimat ("
    "  id INTEGER PRIMARY KEY,"
    "  struktur TEXT NOT NULL,"
    "  kelas TEXT,"
    "  jenis_kalimat TEXT,"
    "  contoh_kalimat TEXT"
    ");"

    /* === KELOMPOK B: PENGETAHUAN (5 Tabel) =================== */
    "CREATE TABLE IF NOT EXISTS pengetahuan_umum ("
    "  id INTEGER PRIMARY KEY,"
    "  entitas TEXT NOT NULL,"
    "  aspek TEXT NOT NULL,"
    "  judul TEXT,"
    "  penjelasan TEXT,"
    "  nomor_item INTEGER,"
    "  label_item TEXT,"
    "  urutan INTEGER DEFAULT 1,"
    "  tipe TEXT DEFAULT 'fakta',"
    "  rujukan_tabel TEXT,"
    "  rujukan_id INTEGER,"
    "  kategori TEXT,"
    "  domain TEXT,"
    "  id_kata TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS daftar_aspek ("
    "  id INTEGER PRIMARY KEY,"
    "  aspek TEXT NOT NULL UNIQUE,"
    "  tampilan TEXT,"
    "  kategori TEXT DEFAULT 'NETRAL'"
    ");"

    "CREATE TABLE IF NOT EXISTS kata_kunci_aspek ("
    "  id INTEGER PRIMARY KEY,"
    "  kata_kunci TEXT NOT NULL,"
    "  aspek TEXT NOT NULL,"
    "  tipe TEXT DEFAULT 'sinonim'"
    ");"

    "CREATE TABLE IF NOT EXISTS relasi_entitas ("
    "  id INTEGER PRIMARY KEY,"
    "  entitas_a TEXT NOT NULL,"
    "  entitas_b TEXT NOT NULL,"
    "  jenis_relasi TEXT NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS kata_angka ("
    "  id INTEGER PRIMARY KEY,"
    "  kata TEXT NOT NULL UNIQUE,"
    "  nilai REAL NOT NULL,"
    "  jenis TEXT NOT NULL"
    ");"

    /* === KELOMPOK C: WAKTU (4 Tabel) ========================= */
    "CREATE TABLE IF NOT EXISTS waktu_ekspresi ("
    "  id INTEGER PRIMARY KEY,"
    "  kata TEXT NOT NULL UNIQUE,"
    "  jenis TEXT NOT NULL,"
    "  offset INTEGER NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS waktu_unit ("
    "  id INTEGER PRIMARY KEY,"
    "  kata TEXT NOT NULL UNIQUE,"
    "  detik INTEGER NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS nama_bulan ("
    "  id INTEGER PRIMARY KEY,"
    "  nama TEXT NOT NULL UNIQUE,"
    "  nomor INTEGER NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS nama_hari ("
    "  id INTEGER PRIMARY KEY,"
    "  nama TEXT NOT NULL UNIQUE,"
    "  nomor INTEGER NOT NULL"
    ");"

    /* === KELOMPOK D: KONVERSI (2 Tabel) ====================== */
    "CREATE TABLE IF NOT EXISTS konversi_satuan ("
    "  id INTEGER PRIMARY KEY,"
    "  dari TEXT NOT NULL,"
    "  ke TEXT NOT NULL,"
    "  faktor REAL NOT NULL,"
    "  kategori TEXT NOT NULL,"
    "  rumus_khusus TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS nama_satuan ("
    "  id INTEGER PRIMARY KEY,"
    "  kata TEXT NOT NULL UNIQUE,"
    "  satuan TEXT NOT NULL"
    ");"

    /* === KELOMPOK E: MATEMATIKA (2 Tabel) =================== */
    "CREATE TABLE IF NOT EXISTS operator_matematika ("
    "  id INTEGER PRIMARY KEY,"
    "  kata TEXT NOT NULL UNIQUE,"
    "  operator TEXT NOT NULL,"
    "  prioritas INTEGER NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS relasi_perhitungan ("
    "  id INTEGER PRIMARY KEY,"
    "  variabel_a TEXT NOT NULL,"
    "  variabel_b TEXT NOT NULL,"
    "  operator TEXT NOT NULL,"
    "  variabel_c TEXT NOT NULL,"
    "  rumus TEXT NOT NULL"
    ");"

    /* === KELOMPOK F: SISTEM & RESPONS (6 Tabel) ============= */
    "CREATE TABLE IF NOT EXISTS pola_pembuka ("
    "  id INTEGER PRIMARY KEY,"
    "  pola TEXT NOT NULL,"
    "  tipe TEXT NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS pola_penutup ("
    "  id INTEGER PRIMARY KEY,"
    "  pola TEXT NOT NULL,"
    "  kondisi TEXT NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS pola_respons ("
    "  id INTEGER PRIMARY KEY,"
    "  pola TEXT NOT NULL,"
    "  tipe_jawaban TEXT NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS intent_pola ("
    "  id INTEGER PRIMARY KEY,"
    "  pola TEXT NOT NULL,"
    "  intent TEXT NOT NULL,"
    "  prioritas INTEGER DEFAULT 8"
    ");"

    "CREATE TABLE IF NOT EXISTS referensi_konteks ("
    "  id INTEGER PRIMARY KEY,"
    "  kata TEXT NOT NULL UNIQUE,"
    "  merujuk_ke TEXT,"
    "  jenis_persona TEXT,"
    "  gaya TEXT,"
    "  ganti_dengan TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS perintah_sistem ("
    "  id INTEGER PRIMARY KEY,"
    "  pola TEXT NOT NULL UNIQUE,"
    "  aksi TEXT NOT NULL"
    ");"

    /* === KELOMPOK G: SESI & PERCAKAPAN (4 Tabel) =========== */
    "CREATE TABLE IF NOT EXISTS sesi ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  nomor_sesi INTEGER,"
    "  status TEXT DEFAULT 'aktif',"
    "  waktu_mulai TEXT,"
    "  waktu_akhir TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS konteks_sesi ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  sesi_id INTEGER NOT NULL,"
    "  topik TEXT,"
    "  tipe_topik TEXT,"
    "  intent_terakhir TEXT,"
    "  entitas_aktif TEXT,"
    "  subjek_terakhir TEXT,"
    "  objek_terakhir TEXT,"
    "  percakapan_ke INTEGER DEFAULT 0,"
    "  timestamp TEXT,"
    "  gaya_bahasa TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS riwayat_chat ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  sesi_id INTEGER NOT NULL,"
    "  pengguna TEXT,"
    "  bot_respon TEXT,"
    "  topik TEXT,"
    "  intent TEXT,"
    "  timestamp TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS komponen_makna ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  id_makna TEXT NOT NULL,"
    "  kata_komponen TEXT NOT NULL"
    ");";

    rc = sqlite3_exec(db, sql_tabel, NULL, NULL, &pesan_error);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "gagal buat tabel: %s\n", pesan_error);
        sqlite3_free(pesan_error);
        return SALAH;
    }

    /* ============================================================
     * SQL pembuatan 2 tabel tambahan untuk sistem konfigurasi
     * dan handler intent. kelas_kategori telah digabung ke
     * kelas_kata (kolom kategori).
     * ============================================================ */
    sql_tabel_tambahan =
    /* tabel pemetaan intent ke handler dan konfigurasi respons */
    "CREATE TABLE IF NOT EXISTS intent_handler ("
    "  id INTEGER PRIMARY KEY,"
    "  intent TEXT UNIQUE,"
    "  handler TEXT,"
    "  tipe_pembuka TEXT,"
    "  tipe_jawaban TEXT,"
    "  tampilkan_langsung INTEGER DEFAULT 0,"
    "  aksi TEXT"
    ");"

    /* tabel konfigurasi sistem (pasangan kunci-nilai) */
    "CREATE TABLE IF NOT EXISTS konfigurasi_sistem ("
    "  id INTEGER PRIMARY KEY,"
    "  kunci TEXT UNIQUE,"
    "  nilai TEXT"
    ");";

    rc = sqlite3_exec(db, sql_tabel_tambahan, NULL, NULL, &pesan_error);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "gagal buat tabel tambahan: %s\n", pesan_error);
        sqlite3_free(pesan_error);
        return SALAH;
    }

    /* ============================================================
     * INDEX untuk optimasi kueri
     * ============================================================ */
    const char *sql_index =
    "CREATE INDEX IF NOT EXISTS idx_leksikon_kata "
    "ON leksikon(kata);"

    "CREATE UNIQUE INDEX IF NOT EXISTS idx_arti_kata_id "
    "ON arti_kata(id_kata);"

    "CREATE INDEX IF NOT EXISTS idx_arti_kata_kata "
    "ON arti_kata(kata);"

    "CREATE UNIQUE INDEX IF NOT EXISTS idx_makna_kata_id "
    "ON makna_kata(id_kata);"

    "CREATE INDEX IF NOT EXISTS idx_makna_kata_kata "
    "ON makna_kata(kata);"

    "CREATE INDEX IF NOT EXISTS idx_komponen_kata "
    "ON komponen_makna(kata_komponen);"

    "CREATE INDEX IF NOT EXISTS idx_komponen_makna "
    "ON komponen_makna(id_makna);"

    "CREATE INDEX IF NOT EXISTS idx_pu_entitas_aspek "
    "ON pengetahuan_umum(entitas, aspek);"

    "CREATE INDEX IF NOT EXISTS idx_pu_nomor "
    "ON pengetahuan_umum(entitas, aspek, nomor_item);"

    "CREATE INDEX IF NOT EXISTS idx_pu_urutan "
    "ON pengetahuan_umum(entitas, aspek, urutan);"

    "CREATE INDEX IF NOT EXISTS idx_pu_domain "
    "ON pengetahuan_umum(entitas, domain);"

    "CREATE INDEX IF NOT EXISTS idx_pu_entitas "
    "ON pengetahuan_umum(entitas);"

    "CREATE INDEX IF NOT EXISTS idx_relasi_a "
    "ON relasi_entitas(entitas_a);"

    "CREATE INDEX IF NOT EXISTS idx_relasi_b "
    "ON relasi_entitas(entitas_b);"

    "CREATE INDEX IF NOT EXISTS idx_konteks_sesi "
    "ON konteks_sesi(sesi_id);"

    "CREATE INDEX IF NOT EXISTS idx_riwayat_sesi "
    "ON riwayat_chat(sesi_id);"

    "CREATE INDEX IF NOT EXISTS idx_referensi_persona "
    "ON referensi_konteks(jenis_persona, gaya);"

    "CREATE INDEX IF NOT EXISTS idx_aspek_kategori "
    "ON daftar_aspek(aspek, kategori);"

    "CREATE INDEX IF NOT EXISTS idx_kunci_aspek "
    "ON kata_kunci_aspek(kata_kunci);"

    "CREATE INDEX IF NOT EXISTS idx_turunan_kata "
    "ON kata_turunan(kata_turunan);"

    "CREATE INDEX IF NOT EXISTS idx_turunan_dasar "
    "ON kata_turunan(kata_dasar);"

    "CREATE INDEX IF NOT EXISTS idx_kiasan_kata1 "
    "ON kata_kiasan(kata1, kata2);"

    "CREATE INDEX IF NOT EXISTS idx_intent_prioritas "
    "ON intent_pola(intent, prioritas);"

    "CREATE INDEX IF NOT EXISTS idx_sesi_status "
    "ON sesi(status);";

    rc = sqlite3_exec(db, sql_index, NULL, NULL, &pesan_error);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "gagal buat index: %s\n", pesan_error);
        sqlite3_free(pesan_error);
        return SALAH;
    }

    return BENAR;
}

/* ================================================================
 * muat_cache_db - Memuat data dari kelas_kata (kategori),
 * intent_handler, dan konfigurasi_sistem ke dalam
 * array cache di memori. Harus dipanggil SETELAH buat_skema().
 *
 * Mengembalikan BENAR jika berhasil, SALAH jika gagal.
 * ================================================================ */
int muat_cache_db(void)
{
    int rc;
    sqlite3_stmt *stmt;
    const char *sql;
    const char *val;

    /* ---- Muat cache kelas_kata (kelas + kategori) ---- */
    jumlah_kelas_kata = 0;
    sql = "SELECT kelas, kategori FROM kelas_kata;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (jumlah_kelas_kata < 512) {
                val = (const char *)sqlite3_column_text(stmt, 0);
                SALIN(cache_kelas_kata[jumlah_kelas_kata].kelas,
                      val != NULL ? val : "", 128);
                val = (const char *)sqlite3_column_text(stmt, 1);
                SALIN(cache_kelas_kata[jumlah_kelas_kata].kategori,
                      val != NULL ? val : "", 128);
                jumlah_kelas_kata++;
            }
        }
        sqlite3_finalize(stmt);
    }

    /* ---- Muat cache intent_handler ---- */
    jumlah_intent_handler = 0;
    sql = "SELECT intent, handler, tipe_pembuka, tipe_jawaban, "
          "tampilkan_langsung, aksi FROM intent_handler;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (jumlah_intent_handler < 256) {
                val = (const char *)sqlite3_column_text(stmt, 0);
                SALIN(cache_intent_handler[jumlah_intent_handler].intent,
                      val != NULL ? val : "", 128);
                val = (const char *)sqlite3_column_text(stmt, 1);
                SALIN(cache_intent_handler[jumlah_intent_handler].handler,
                      val != NULL ? val : "", 128);
                val = (const char *)sqlite3_column_text(stmt, 2);
                SALIN(cache_intent_handler[jumlah_intent_handler].tipe_pembuka,
                      val != NULL ? val : "", 64);
                val = (const char *)sqlite3_column_text(stmt, 3);
                SALIN(cache_intent_handler[jumlah_intent_handler].tipe_jawaban,
                      val != NULL ? val : "", 64);
                cache_intent_handler[jumlah_intent_handler].tampilkan_langsung =
                    sqlite3_column_int(stmt, 4);
                val = (const char *)sqlite3_column_text(stmt, 5);
                SALIN(cache_intent_handler[jumlah_intent_handler].aksi,
                      val != NULL ? val : "", 256);
                jumlah_intent_handler++;
            }
        }
        sqlite3_finalize(stmt);
    }

    /* ---- Muat cache konfigurasi_sistem ---- */
    jumlah_konfigurasi = 0;
    sql = "SELECT kunci, nilai FROM konfigurasi_sistem;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (jumlah_konfigurasi < 256) {
                val = (const char *)sqlite3_column_text(stmt, 0);
                SALIN(cache_konfigurasi[jumlah_konfigurasi].kunci,
                      val != NULL ? val : "", 128);
                val = (const char *)sqlite3_column_text(stmt, 1);
                SALIN(cache_konfigurasi[jumlah_konfigurasi].nilai,
                      val != NULL ? val : "", 512);
                jumlah_konfigurasi++;
            }
        }
        sqlite3_finalize(stmt);
    }

    return BENAR;
}

/* ---------------------------------------------------------------
 * cek_konektor - Cek apakah kata ada dalam daftar konektor
 * yang tersimpan di konfigurasi_sistem (kunci: konektor_entitas)
 * Daftar berupa kata-kata yang dipisahkan koma.
 * Mengembalikan BENAR jika ditemukan.
 * --------------------------------------------------------------- */
int cek_konektor(const char *kata)
{
    const char *daftar;
    char buf[MAKS_KALIMAT];
    char *t;

    daftar = ambil_konfigurasi("konektor_entitas");
    if (daftar == NULL || daftar[0] == '\0') return SALAH;

    /* salin untuk aman (strtok memodifikasi string) */
    SALIN(buf, daftar, MAKS_KALIMAT);

    t = strtok(buf, ",");
    while (t != NULL) {
        /* trim spasi */
        while (*t == ' ') t++;
        if (strcmp(t, kata) == 0) {
            return BENAR;
        }
        t = strtok(NULL, ",");
    }

    return SALAH;
}

/* ================================================================
 * FUNGSI PENCARIAN CACHE
 * Semua fungsi di bawah ini mencari data dari array cache
 * yang telah dimuat oleh muat_cache_db().
 * ================================================================ */

/* ---------------------------------------------------------------
 * cek_kategori - Memeriksa apakah suatu kelas termasuk dalam
 * kategori tertentu. Mengembalikan BENAR (1) jika cocok,
 * SALAH (0) jika tidak ditemukan.
 * --------------------------------------------------------------- */
int cek_kategori(const char *kelas, const char *kategori)
{
    int i;

    for (i = 0; i < jumlah_kelas_kata; i++) {
        if (strcmp(cache_kelas_kata[i].kelas, kelas) == 0 &&
            strcmp(cache_kelas_kata[i].kategori, kategori) == 0) {
            return BENAR;
        }
    }
    return SALAH;
}

/* ---------------------------------------------------------------
 * cari_handler - Mencari nama handler berdasarkan intent.
 * Mengembalikan pointer ke string handler, atau NULL jika
 * intent tidak ditemukan.
 * --------------------------------------------------------------- */
const char *cari_handler(const char *intent)
{
    int i;

    for (i = 0; i < jumlah_intent_handler; i++) {
        if (strcmp(cache_intent_handler[i].intent, intent) == 0) {
            return (const char *)cache_intent_handler[i].handler;
        }
    }
    return NULL;
}

/* ---------------------------------------------------------------
 * cari_tipe_pembuka_intent - Mencari tipe_pembuka berdasarkan
 * intent. Mengembalikan pointer ke string tipe_pembuka,
 * atau NULL jika intent tidak ditemukan.
 * --------------------------------------------------------------- */
const char *cari_tipe_pembuka_intent(const char *intent)
{
    int i;

    for (i = 0; i < jumlah_intent_handler; i++) {
        if (strcmp(cache_intent_handler[i].intent, intent) == 0) {
            return (const char *)cache_intent_handler[i].tipe_pembuka;
        }
    }
    return NULL;
}

/* ---------------------------------------------------------------
 * cari_tipe_jawaban_intent - Mencari tipe_jawaban berdasarkan
 * intent. Mengembalikan pointer ke string tipe_jawaban,
 * atau NULL jika intent tidak ditemukan.
 * --------------------------------------------------------------- */
const char *cari_tipe_jawaban_intent(const char *intent)
{
    int i;

    for (i = 0; i < jumlah_intent_handler; i++) {
        if (strcmp(cache_intent_handler[i].intent, intent) == 0) {
            return (const char *)cache_intent_handler[i].tipe_jawaban;
        }
    }
    return NULL;
}

/* ---------------------------------------------------------------
 * cari_aksi_handler - Mencari aksi berdasarkan intent.
 * Mengembalikan pointer ke string aksi, atau NULL jika
 * intent tidak ditemukan.
 * --------------------------------------------------------------- */
const char *cari_aksi_handler(const char *intent)
{
    int i;

    for (i = 0; i < jumlah_intent_handler; i++) {
        if (strcmp(cache_intent_handler[i].intent, intent) == 0) {
            return (const char *)cache_intent_handler[i].aksi;
        }
    }
    return NULL;
}

/* ---------------------------------------------------------------
 * ambil_tampilkan_langsung - Mengambil nilai tampilkan_langsung
 * berdasarkan intent. Mengembalikan 1 jika langsung ditampilkan,
 * atau 0 jika tidak (default).
 * --------------------------------------------------------------- */
int ambil_tampilkan_langsung(const char *intent)
{
    int i;

    for (i = 0; i < jumlah_intent_handler; i++) {
        if (strcmp(cache_intent_handler[i].intent, intent) == 0) {
            return cache_intent_handler[i].tampilkan_langsung;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------
 * ambil_konfigurasi - Mencari nilai konfigurasi berdasarkan kunci.
 * Mengembalikan pointer ke string nilai, atau NULL jika
 * kunci tidak ditemukan dalam cache.
 * --------------------------------------------------------------- */
const char *ambil_konfigurasi(const char *kunci)
{
    int i;

    for (i = 0; i < jumlah_konfigurasi; i++) {
        if (strcmp(cache_konfigurasi[i].kunci, kunci) == 0) {
            return (const char *)cache_konfigurasi[i].nilai;
        }
    }
    return NULL;
}

/* ================================================================
 * Catatan: fungsi main() terletak di utama.c
 * File ini hanya berisi fungsi-fungsi operasional database.
 * ================================================================ */
