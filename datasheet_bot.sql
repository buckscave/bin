-- ================================================================
-- datasheet_bot.sql - Datasheet Lengkap Bot Bahasa Indonesia
-- Berbasis Aturan (Rule-Based)
-- Teknologi: SQLite3 | 39 Tabel | April 2026
-- ================================================================
-- Penggunaan: sqlite3 bot.db < datasheet_bot.sql
-- ================================================================

BEGIN TRANSACTION;

-- ================================================================
-- KELOMPOK A: BAHASA (14 Tabel)
-- ================================================================

-- T1: kelas_kata (termasuk kolom kategori untuk pemetaan singkat)
CREATE TABLE IF NOT EXISTS kelas_kata (
    id INTEGER PRIMARY KEY,
    kelas TEXT NOT NULL UNIQUE,
    kategori TEXT NOT NULL
);
INSERT INTO kelas_kata VALUES (1, 'kata benda', 'benda');
INSERT INTO kelas_kata VALUES (2, 'kata kerja', 'kerja');
INSERT INTO kelas_kata VALUES (3, 'kata kerja aktif', 'kerja');
INSERT INTO kelas_kata VALUES (4, 'kata kerja pasif', 'kerja');
INSERT INTO kelas_kata VALUES (5, 'kata sifat', 'sifat');
INSERT INTO kelas_kata VALUES (6, 'kata keterangan', 'keterangan');
INSERT INTO kelas_kata VALUES (7, 'kata ganti', 'ganti');
INSERT INTO kelas_kata VALUES (8, 'kata bilangan', 'bilangan');
INSERT INTO kelas_kata VALUES (9, 'kata depan', 'depan');
INSERT INTO kelas_kata VALUES (10, 'kata hubung', 'hubung');
INSERT INTO kelas_kata VALUES (11, 'kata seru', 'seru');
INSERT INTO kelas_kata VALUES (12, 'kata sandang', 'sandang');
INSERT INTO kelas_kata VALUES (13, 'kata penegas', 'penegas');
INSERT INTO kelas_kata VALUES (14, 'kata tanya', 'tanya');
INSERT INTO kelas_kata VALUES (15, 'kata tunjuk', 'tunjuk');
INSERT INTO kelas_kata VALUES (16, 'kata ulang', 'ulang');

-- T2: leksikon (kata dasar)
CREATE TABLE IF NOT EXISTS leksikon (
    id INTEGER PRIMARY KEY,
    id_kata TEXT NOT NULL UNIQUE,
    kata TEXT NOT NULL,
    kelas TEXT NOT NULL,
    asal_kata TEXT,
    frekuensi REAL DEFAULT 0.0,
    berpengetahuan INTEGER DEFAULT 0
);
INSERT INTO leksikon VALUES (1,'ma.1','makan','kata kerja','Sanskrit',1.0,1);
INSERT INTO leksikon VALUES (2,'ma.2','masak','kata kerja','Sanskrit',0.95,1);
INSERT INTO leksikon VALUES (3,'ma.3','malam','kata benda','Sanskrit',0.90,0);
INSERT INTO leksikon VALUES (4,'ma.4','manusia','kata benda','Sanskrit',0.95,1);
INSERT INTO leksikon VALUES (5,'ma.5','main','kata kerja','Melayu',0.85,0);
INSERT INTO leksikon VALUES (6,'be.1','benci','kata kerja','Sanskrit',0.60,0);
INSERT INTO leksikon VALUES (7,'be.2','besar','kata sifat','Sanskrit',0.95,0);
INSERT INTO leksikon VALUES (8,'bi.26','bisa','kata kerja','Portugis',1.0,0);
INSERT INTO leksikon VALUES (9,'ha.1','hati','kata benda','Sanskrit',0.90,0);
INSERT INTO leksikon VALUES (10,'ha.2','hari','kata benda','Sanskrit',1.0,0);

-- T3: kata_turunan (morfologi)
CREATE TABLE IF NOT EXISTS kata_turunan (
    id INTEGER PRIMARY KEY,
    id_kata_dasar TEXT NOT NULL,
    kata_dasar TEXT NOT NULL,
    kata_turunan TEXT NOT NULL,
    kelas TEXT,
    frekuensi REAL DEFAULT 0.0
);
INSERT INTO kata_turunan VALUES (1,'ma.1','makan','makanan','kata benda',0.95);
INSERT INTO kata_turunan VALUES (2,'ma.1','makan','dimakan','kata kerja',0.70);
INSERT INTO kata_turunan VALUES (3,'ma.1','makan','termakan','kata kerja',0.50);
INSERT INTO kata_turunan VALUES (4,'ma.1','makan','pemakan','kata benda',0.40);
INSERT INTO kata_turunan VALUES (5,'ma.1','makan','makan-makan','kata kerja',0.55);
INSERT INTO kata_turunan VALUES (6,'ma.2','masak','masakan','kata benda',0.90);
INSERT INTO kata_turunan VALUES (7,'ma.2','masak','dimasak','kata kerja',0.75);
INSERT INTO kata_turunan VALUES (8,'ma.2','masak','memasak','kata kerja',0.90);
INSERT INTO kata_turunan VALUES (9,'be.2','besar','memperbesar','kata kerja',0.65);
INSERT INTO kata_turunan VALUES (10,'be.2','besar','kebesaran','kata benda',0.55);

-- T4: kata_fungsional
CREATE TABLE IF NOT EXISTS kata_fungsional (
    id INTEGER PRIMARY KEY,
    kata TEXT NOT NULL UNIQUE,
    kelas TEXT NOT NULL,
    fungsi TEXT
);
INSERT INTO kata_fungsional VALUES (1,'yang','kata hubung','penghubung kalimat');
INSERT INTO kata_fungsional VALUES (2,'dan','kata hubung','menghubungkan dua klausa setara');
INSERT INTO kata_fungsional VALUES (3,'atau','kata hubung','menunjukkan pilihan');
INSERT INTO kata_fungsional VALUES (4,'di','kata depan','menunjukkan tempat');
INSERT INTO kata_fungsional VALUES (5,'ke','kata depan','menunjukkan arah');
INSERT INTO kata_fungsional VALUES (6,'dari','kata depan','menunjukkan asal');
INSERT INTO kata_fungsional VALUES (7,'pada','kata depan','menunjukkan tempat atau waktu');
INSERT INTO kata_fungsional VALUES (8,'dengan','kata depan','menunjukkan cara atau pendamping');
INSERT INTO kata_fungsional VALUES (9,'untuk','kata depan','menunjukkan tujuan');
INSERT INTO kata_fungsional VALUES (10,'oleh','kata depan','menunjukkan pelaku dalam kalimat pasif');

-- T5: sinonim
CREATE TABLE IF NOT EXISTS sinonim (
    id INTEGER PRIMARY KEY,
    kata1 TEXT NOT NULL,
    kata2 TEXT NOT NULL,
    relasi TEXT NOT NULL,
    arti_kata TEXT
);
INSERT INTO sinonim VALUES (1,'pandai','pintar','sinonim','memiliki kecakapan');
INSERT INTO sinonim VALUES (2,'besar','raksasa','sinonim','ukuran yang melebihi biasa');
INSERT INTO sinonim VALUES (3,'kecil','mungil','sinonim','ukuran yang lebih sedikit dari biasa');
INSERT INTO sinonim VALUES (4,'cepat','sigap','sinonim','bergerak dengan kecepatan tinggi');
INSERT INTO sinonim VALUES (5,'cantik','indah','sinonim','memiliki daya tarik visual');
INSERT INTO sinonim VALUES (6,'baik','bagus','sinonim','memiliki kualitas yang positif');
INSERT INTO sinonim VALUES (7,'sedih','dukacita','sinonim','perasaan tidak senang');
INSERT INTO sinonim VALUES (8,'marah','geram','sinonim','perasaan tidak terima');
INSERT INTO sinonim VALUES (9,'senang','gembira','sinonim','perasaan puas dan bahagia');
INSERT INTO sinonim VALUES (10,'kaya','hartawan','sinonim','memiliki banyak harta');

-- T6: antonim
CREATE TABLE IF NOT EXISTS antonim (
    id INTEGER PRIMARY KEY,
    kata1 TEXT NOT NULL,
    kata2 TEXT NOT NULL,
    relasi TEXT NOT NULL,
    arti_kata TEXT
);
INSERT INTO antonim VALUES (1,'besar','kecil','antonim','ukuran');
INSERT INTO antonim VALUES (2,'tinggi','rendah','antonim','ketinggian');
INSERT INTO antonim VALUES (3,'berat','ringan','antonim','massa');
INSERT INTO antonim VALUES (4,'panas','dingin','antonim','suhu');
INSERT INTO antonim VALUES (5,'cepat','lambat','antonim','kecepatan');
INSERT INTO antonim VALUES (6,'baik','buruk','antonim','kualitas');
INSERT INTO antonim VALUES (7,'senang','sedih','antonim','perasaan');
INSERT INTO antonim VALUES (8,'rajin','malas','antonim','semangat kerja');
INSERT INTO antonim VALUES (9,'pintar','bodoh','antonim','kecerdasan');
INSERT INTO antonim VALUES (10,'kaya','miskin','antonim','kekayaan');

-- T7: arti_kata
CREATE TABLE IF NOT EXISTS arti_kata (
    id INTEGER PRIMARY KEY,
    id_kata TEXT NOT NULL UNIQUE,
    kata TEXT NOT NULL,
    arti TEXT NOT NULL,
    contoh_kalimat TEXT,
    frekuensi REAL DEFAULT 1.0,
    tipe TEXT DEFAULT 'harfiah'
);
INSERT INTO arti_kata VALUES (1,'ma.1.1','makan','suatu kegiatan memasukkan sesuatu ke dalam mulut, mengunyah lalu menelannya','Ibu sedang makan nasi di ruang makan.',1.0,'harfiah');
INSERT INTO arti_kata VALUES (2,'ma.1.2','makan','mengambil sebagian atau lebih hak milik orang lain secara tidak sah','Koruptor itu memakan uang rakyat.',0.85,'harfiah');
INSERT INTO arti_kata VALUES (3,'ma.1.3','makan','membuat jengkel atau marah','Perkataannya benar-benar membuatku makannya.',0.50,'kiasan');
INSERT INTO arti_kata VALUES (4,'ma.2.1','masak','mengolah bahan makanan agar menjadi layak dimakan dengan cara dipanaskan','Ibu sedang masak sayur di dapur.',1.0,'harfiah');
INSERT INTO arti_kata VALUES (5,'ma.2.2','masak','telah direncanakan atau dipersiapkan dengan matang','Rencana itu sudah masak sejak lama.',0.60,'kiasan');
INSERT INTO arti_kata VALUES (6,'bi.26.1','bisa','mampu atau memiliki kemampuan untuk melakukan sesuatu','Saya bisa berenang sejak kecil.',1.0,'harfiah');
INSERT INTO arti_kata VALUES (7,'bi.26.2','bisa','boleh atau diizinkan untuk melakukan sesuatu','Bisa saya pinjam bukumu?',0.80,'harfiah');
INSERT INTO arti_kata VALUES (8,'bi.26.3','bisa','racun atau bisa-bisa yang biasa digunakan pada senjata tradisional','Pedang itu diolesi bisa.',0.35,'harfiah');
INSERT INTO arti_kata VALUES (9,'ha.1.1','hati','organ dalam tubuh yang berfungsi memompa darah ke seluruh tubuh','Hati manusia berdetak sekitar 100000 kali sehari.',1.0,'harfiah');
INSERT INTO arti_kata VALUES (10,'ha.1.2','hati','perasaan atau emosi yang dirasakan oleh seseorang','Hati saya sangat sedih mendengar berita itu.',0.90,'kiasan');

-- T8: makna_kata
CREATE TABLE IF NOT EXISTS makna_kata (
    id INTEGER PRIMARY KEY,
    id_kata TEXT NOT NULL UNIQUE,
    kata TEXT NOT NULL,
    kelas TEXT,
    makna TEXT NOT NULL,
    contoh_kalimat TEXT NOT NULL,
    frekuensi REAL DEFAULT 1.0,
    jenis_kata TEXT
);
INSERT INTO makna_kata VALUES (1,'mk.ma.1.1','makan hati','ungkapan','sangat sedih atau sakit hati','Jangan hanya makan hati saja karena masalah kecil.',0.90,'idiom');
INSERT INTO makna_kata VALUES (2,'mk.ma.2.1','makan gaji buta','ungkapan','menerima gaji tanpa bekerja dengan baik','Dia dipecat karena makan gaji buta selama bertahun-tahun.',0.80,'idiom');
INSERT INTO makna_kata VALUES (3,'mk.ma.3.1','makan teman sendiri','ungkapan','berkhianat kepada teman','Tidak kusangka dia akan makan teman sendiri.',0.55,'idiom');
INSERT INTO makna_kata VALUES (4,'mk.ha.1.1','patah hati','ungkapan','sangat sedih atau kecewa karena cinta','Setelah ditinggal, ia patah hati selama berminggu-minggu.',0.90,'idiom');
INSERT INTO makna_kata VALUES (5,'mk.ha.2.1','hati-hati','frasa','berhati-hati atau waspada','Hati-hati di jalan, hujan deras.',0.95,'frasa');
INSERT INTO makna_kata VALUES (6,'mk.ha.3.1','buka hati','ungkapan','menceritakan isi hati atau perasaan','Akhirnya ia mau membuka hati kepada saya.',0.50,'idiom');
INSERT INTO makna_kata VALUES (7,'mk.ma.4.1','makan waktu','ungkapan','memerlukan waktu yang lama','Proses ini makan waktu berhari-hari.',0.70,'idiom');
INSERT INTO makna_kata VALUES (8,'mk.ma.5.1','makan malam','frasa','kegiatan makan pada sore atau malam hari','Kita makan malam bersama keluarga pukul tujuh.',0.85,'frasa');

-- T9: kata_kiasan
CREATE TABLE IF NOT EXISTS kata_kiasan (
    id INTEGER PRIMARY KEY,
    id_kiasan TEXT NOT NULL UNIQUE,
    kata1 TEXT NOT NULL,
    kata2 TEXT NOT NULL,
    kata TEXT NOT NULL,
    jenis TEXT NOT NULL,
    makna TEXT NOT NULL,
    contoh TEXT
);
INSERT INTO kata_kiasan VALUES (1,'kk.1','gaji','buta','makan gaji buta','eufemisme','menerima gaji tanpa bekerja dengan baik','Dia dipecat karena makan gaji buta selama bertahun-tahun.');
INSERT INTO kata_kiasan VALUES (2,'kk.2','patah','hati','patah hati','idiom','sangat sedih atau kecewa karena cinta','Setelah ditinggal, ia patah hati selama berminggu-minggu.');
INSERT INTO kata_kiasan VALUES (3,'kk.3','hati','batu','hati batu','simile','sangat keras hatinya tidak mau mengalah','Hatinya keras seperti batu.');
INSERT INTO kata_kiasan VALUES (4,'kk.4','tangan','besi','tangan besi','sinekdok','pemerintahan yang tegas dan keras','Rezim tangan besi itu berakhir setelah puluhan tahun.');
INSERT INTO kata_kiasan VALUES (5,'kk.5','tutup','mulut','tutup mulut','eufemisme','diam atau tidak mau berkomentar','Lebih baik tutup mulut daripada berkata kasar.');
INSERT INTO kata_kiasan VALUES (6,'kk.6','kuda','kayu','kuda kayu','idiom','tipu daya yang terungkap','Strategi kuda kudanya akhirnya terbongkar juga.');
INSERT INTO kata_kiasan VALUES (7,'kk.7','buka','hati','buka hati','ungkapan','menceritakan isi hati atau perasaan','Akhirnya ia mau membuka hati kepada saya.');
INSERT INTO kata_kiasan VALUES (8,'kk.8','matahari','pagi','matahari pagi','metafora','perbandingan untuk ketenaran atau kebahagiaan','Anak itu adalah matahari pagi bagi keluarganya.');
INSERT INTO kata_kiasan VALUES (9,'kk.9','angin','bertiup','angin bertiup','personifikasi','angin seolah-olah bernafas','Angin bertiup seolah menghembuskan kesedihan.');
INSERT INTO kata_kiasan VALUES (10,'kk.10','lautan','pasir','lautan pasir','hiperbola','padang pasir yang sangat luas','Luasnya lautan pasir membuat mata tidak berkedip.');

-- T10: contoh_sastra
CREATE TABLE IF NOT EXISTS contoh_sastra (
    id INTEGER PRIMARY KEY,
    jenis TEXT NOT NULL,
    judul TEXT NOT NULL,
    isi TEXT NOT NULL,
    arti TEXT
);
INSERT INTO contoh_sastra VALUES (1,'peribahasa','rajin lebih baik dari','Rajin lebih baik dari pandai','Bekerja keras lebih bernilai daripada hanya pintar tapi malas.');
INSERT INTO contoh_sastra VALUES (2,'peribahasa','air mengalir ke','Air mengalir ke tempat yang rendah','Orang bijak selalu merendah diri.');
INSERT INTO contoh_sastra VALUES (3,'peribahasa','berat sama dipikul','Berat sama dipikul ringan sama dijinjing','Saling tolong-menolong dalam suka dan duka.');
INSERT INTO contoh_sastra VALUES (4,'pantun','pantun nasihat','Durian runtuh di jalan mati, Anak durian durian duri, Bersedekah jangan ditanya, Pahala dari Tuhan yang memberi.','Pantun yang menasihatkan untuk gemar bersedekah.');
INSERT INTO contoh_sastra VALUES (5,'pantun','pantun cinta','Bunga melati harum baunya, Ditanam di halaman rumah, Kalau rindu datang menghampiri, Jangan lupa bertanya kabar.','Pantun yang menyatakan kerinduan.');
INSERT INTO contoh_sastra VALUES (6,'puisi','puisi pendek','Aku ini binatang jalang, Dari kumpulan itu terbuang, Biar peluru menembus kulitku, Aku tetap panjang dan keras.','Puisi karya Chairil Anwar tentang semangat juang.');
INSERT INTO contoh_sastra VALUES (7,'prosa','cerita rakyat','Malin Kundang adalah kisah seorang anak yang durhaka kepada ibunya dan dikutuk menjadi batu.','Cerita rakyat Sumatera Barat yang mengajarkan tentang bakti kepada orang tua.');

-- T11: imbuhan (afiksasi)
CREATE TABLE IF NOT EXISTS imbuhan (
    id INTEGER PRIMARY KEY,
    jenis TEXT NOT NULL,
    imbuhan TEXT NOT NULL,
    fungsi TEXT
);
INSERT INTO imbuhan VALUES (1,'awalan','ber-','membentuk kata kerja aktif tidak transitif');
INSERT INTO imbuhan VALUES (2,'awalan','meN-','membentuk kata kerja aktif transitif');
INSERT INTO imbuhan VALUES (3,'awalan','di-','membentuk kata kerja pasif');
INSERT INTO imbuhan VALUES (4,'awalan','ter-','membentuk kata kerja tanpa sengaja atau sudah selesai');
INSERT INTO imbuhan VALUES (5,'awalan','peN-','membentuk kata benda dari kata kerja (pelaku)');
INSERT INTO imbuhan VALUES (6,'akhiran','-kan','membentuk kata kerja dengan makna menyebabkan atau untuk');
INSERT INTO imbuhan VALUES (7,'akhiran','-an','membentuk kata benda dari kata kerja');
INSERT INTO imbuhan VALUES (8,'akhiran','-i','membentuk kata kerja dengan makna memberikan sesuatu');
INSERT INTO imbuhan VALUES (9,'konfiks','ber-...-an','membentuk kata kerja dengan makna saling atau berulang');
INSERT INTO imbuhan VALUES (10,'konfiks','peN-...-an','membentuk kata benda tempat atau hasil');

-- T12: majas
CREATE TABLE IF NOT EXISTS majas (
    id INTEGER PRIMARY KEY,
    jenis TEXT NOT NULL UNIQUE,
    definisi TEXT NOT NULL,
    contoh TEXT
);
INSERT INTO majas VALUES (1,'metafora','perbandingan langsung tanpa kata pembanding','Waktu adalah uang.');
INSERT INTO majas VALUES (2,'simile','perbandingan dengan kata seperti bagaikan laksana','Gagah seperti singa.');
INSERT INTO majas VALUES (3,'personifikasi','penyifatan benda seolah-olah bernyawa','Angin bertiup membawa kabar.');
INSERT INTO majas VALUES (4,'hiperbola','pernyataan yang berlebihan untuk menekankan','Darahnya mengalir sungai.');
INSERT INTO majas VALUES (5,'eufemisme','penghalusan kata agar tidak kasar','Beliau telah tiada.');
INSERT INTO majas VALUES (6,'litotes','pernyataan merendah sebenarnya bermakna sebaliknya','Rumahnya tidak besar memang.');
INSERT INTO majas VALUES (7,'paradoks','pernyataan yang bertentangan tapi benar','Semakin banyak tahu semakin merasa bodoh.');
INSERT INTO majas VALUES (8,'antitesis','pertentangan dua hal yang berlawanan','Muda di usia tua di hati.');
INSERT INTO majas VALUES (9,'ironi','sindiran halus yang bertolak belakang dengan fakta','Bagus sekali pekerjaanmu ini.');
INSERT INTO majas VALUES (10,'sarkasme','sindiran kasar dan menyakitkan','Kau pikir kau siapa sampai berani seperti itu.');

-- T13: jenis_kalimat
CREATE TABLE IF NOT EXISTS jenis_kalimat (
    id INTEGER PRIMARY KEY,
    jenis TEXT NOT NULL UNIQUE,
    kelas_kata TEXT
);
INSERT INTO jenis_kalimat VALUES (1,'kalimat berita (deklaratif)','S[kata_benda/kata_ganti]+P[kata_kerja]');
INSERT INTO jenis_kalimat VALUES (2,'kalimat tanya (interogatif)','kata_tanya+S[kata_benda]+P[kata_kerja]');
INSERT INTO jenis_kalimat VALUES (3,'kalimat perintah (imperatif)','P[kata_kerja]+O[kata_benda]');
INSERT INTO jenis_kalimat VALUES (4,'kalimat seruan (interjektif)','kata_seru+klausa');
INSERT INTO jenis_kalimat VALUES (5,'kalimat tunggal','satu klausa utama');
INSERT INTO jenis_kalimat VALUES (6,'kalimat majemuk','klausa utama+klausa anak (dipisah kata_hubung)');
INSERT INTO jenis_kalimat VALUES (7,'kalimat inversif','P[kata_kerja]+S[kata_benda]');
INSERT INTO jenis_kalimat VALUES (8,'kalimat langsung','diawali/diakhiri tanda petik');
INSERT INTO jenis_kalimat VALUES (9,'kalimat tidak langsung','diawali kata_penghubung (bahwa, agar)');
INSERT INTO jenis_kalimat VALUES (10,'kalimat aktif','S[kata_benda]+P[kata_kerja_aktif]+O[kata_benda]');
INSERT INTO jenis_kalimat VALUES (11,'kalimat pasif','O[kata_benda]+P[kata_kerja_pasif]+oleh+S[kata_benda]');
INSERT INTO jenis_kalimat VALUES (12,'kalimat medial','S[kata_benda]+P[kata_kerja]');
INSERT INTO jenis_kalimat VALUES (13,'kalimat resiprokal','S[kata_benda jamak]+P[kata_kerja saling]');
INSERT INTO jenis_kalimat VALUES (14,'kalimat introgatif','kata_tanya+klausa');

-- T14: struktur_kalimat
CREATE TABLE IF NOT EXISTS struktur_kalimat (
    id INTEGER PRIMARY KEY,
    struktur TEXT NOT NULL,
    kelas TEXT,
    jenis_kalimat TEXT,
    contoh_kalimat TEXT
);
INSERT INTO struktur_kalimat VALUES (1,'S-P','S[kata_benda/kata_ganti]+P[kata_kerja]','kalimat berita','Anak itu menangis.');
INSERT INTO struktur_kalimat VALUES (2,'S-P-O','S[kata_benda]+P[kata_kerja]+O[kata_benda]','kalimat berita','Ibu memasak nasi.');
INSERT INTO struktur_kalimat VALUES (3,'S-P-O-K','S[kata_benda]+P[kata_kerja]+O[kata_benda]+K[kata_keterangan]','kalimat berita','Budi membaca buku di perpustakaan.');
INSERT INTO struktur_kalimat VALUES (4,'S-Pel-P','S[kata_benda]+Pel[kata_benda]+P[kata_kerja]','kalimat berita','Ibukota Jepang adalah Tokyo.');
INSERT INTO struktur_kalimat VALUES (5,'S-P-K','S[kata_benda]+P[kata_kerja]+K[kata_keterangan]','kalimat berita','Hujan turun dengan deras.');
INSERT INTO struktur_kalimat VALUES (6,'P-S','P[kata_kerja]+S[kata_benda]','kalimat inversif','Datanglah seorang tamu.');
INSERT INTO struktur_kalimat VALUES (7,'S-P','S[kata_benda]+P[kata_kerja]+P[kata_kerja]','kalimat majemuk setara','Ayah pergi ke pasar dan ibu memasak di dapur.');
INSERT INTO struktur_kalimat VALUES (8,'S-P','S[kata_benda]+P[kata_kerja]+kata_hubung+S[kata_benda]+P[kata_kerja]','kalimat majemuk bertingkat','Kucing itu lari ketika melihat anjing.');
INSERT INTO struktur_kalimat VALUES (9,'S-P-O','O[kata_benda]+P[kata_kerja_pasif]+oleh+S[kata_benda]','kalimat pasif','Nasi itu dimasak oleh ibu.');
INSERT INTO struktur_kalimat VALUES (10,'S-P-O','kata_tanya+S[kata_benda]+P[kata_kerja]+O[kata_benda]','kalimat tanya','Siapa nama ayahmu?');

-- ================================================================
-- KELOMPOK B: PENGETAHUAN (5 Tabel)
-- ================================================================

-- T15: pengetahuan_umum
CREATE TABLE IF NOT EXISTS pengetahuan_umum (
    id INTEGER PRIMARY KEY,
    entitas TEXT NOT NULL,
    aspek TEXT NOT NULL,
    judul TEXT,
    penjelasan TEXT,
    nomor_item INTEGER,
    label_item TEXT,
    urutan INTEGER DEFAULT 1,
    tipe TEXT DEFAULT 'fakta',
    rujukan_tabel TEXT,
    rujukan_id INTEGER,
    kategori TEXT,
    domain TEXT,
    id_kata TEXT
);
INSERT INTO pengetahuan_umum VALUES (1,'pancasila','sila','Ketuhanan Yang Maha Esa','Mengakui adanya Tuhan Yang Maha Esa sebagai pencipta alam semesta',1,'Sila 1',1,'fakta',NULL,NULL,'politik','ideologi_negara',NULL);
INSERT INTO pengetahuan_umum VALUES (2,'pancasila','sila','Kemanusiaan Yang Adil dan Beradab','Mengakui bahwa setiap manusia memiliki derdekat dan martabat yang sama',2,'Sila 2',2,'fakta',NULL,NULL,'politik','ideologi_negara',NULL);
INSERT INTO pengetahuan_umum VALUES (3,'pancasila','sila','Persatuan Indonesia','Mempersatukan seluruh bangsa Indonesia',3,'Sila 3',3,'fakta',NULL,NULL,'politik','ideologi_negara',NULL);
INSERT INTO pengetahuan_umum VALUES (4,'pancasila','sila','Kerakyatan yang Dipimpin oleh Hikmat Kebijaksanaan','Kedaulatan ada di tangan rakyat dengan musyawarah dan perwakilan',4,'Sila 4',4,'fakta',NULL,NULL,'politik','ideologi_negara',NULL);
INSERT INTO pengetahuan_umum VALUES (5,'pancasila','sila','Keadilan Sosial bagi Seluruh Rakyat Indonesia','Keadilan yang merata bagi seluruh rakyat Indonesia',5,'Sila 5',5,'fakta',NULL,NULL,'politik','ideologi_negara',NULL);
INSERT INTO pengetahuan_umum VALUES (6,'komputer','definisi','Komputer','Mesin elektronik yang memproses data menjadi informasi',NULL,NULL,1,'konsep',NULL,NULL,'teknologi','teknologi_komputer',NULL);
INSERT INTO pengetahuan_umum VALUES (7,'komputer','komponen','CPU','Otak komputer yang memproses seluruh instruksi program',NULL,NULL,1,'konsep',NULL,NULL,'teknologi','teknologi_komputer',NULL);
INSERT INTO pengetahuan_umum VALUES (8,'komputer','komponen','RAM','Memori sementara untuk menyimpan data yang sedang diproses',NULL,NULL,2,'konsep',NULL,NULL,'teknologi','teknologi_komputer',NULL);
INSERT INTO pengetahuan_umum VALUES (9,'komputer','komponen','VGA','Memproses dan mengirim gambar ke layar monitor',NULL,NULL,3,'konsep',NULL,NULL,'teknologi','teknologi_komputer',NULL);
INSERT INTO pengetahuan_umum VALUES (10,'komputer','komponen','Motherboard','Papan sirkuit utama yang menghubungkan semua komponen',NULL,NULL,4,'konsep',NULL,NULL,'teknologi','teknologi_komputer',NULL);
INSERT INTO pengetahuan_umum VALUES (11,'makan','waktu_makan','Sarapan','Makan pagi hari sebelum beraktivitas, biasanya pukul 06.00-09.00',NULL,NULL,1,'budaya',NULL,NULL,'kuliner','kuliner',NULL);
INSERT INTO pengetahuan_umum VALUES (12,'makan','waktu_makan','Makan siang','Makan pada tengah hari, biasanya pukul 12.00-13.00',NULL,NULL,2,'budaya',NULL,NULL,'kuliner','kuliner',NULL);
INSERT INTO pengetahuan_umum VALUES (13,'makan','waktu_makan','Makan malam','Makan pada sore atau malam hari, biasanya pukul 18.00-20.00',NULL,NULL,3,'budaya',NULL,NULL,'kuliner','kuliner',NULL);
INSERT INTO pengetahuan_umum VALUES (14,'makan','jenis_makan','Makanan pembuka','Hidangan pertama dalam suatu acara makan, biasanya berupa sup atau salad',NULL,NULL,1,'budaya',NULL,NULL,'kuliner','kuliner',NULL);
INSERT INTO pengetahuan_umum VALUES (15,'makan','jenis_makan','Makanan penutup','Hidangan terakhir, biasanya berupa hidangan manis atau buah',NULL,NULL,2,'budaya',NULL,NULL,'kuliner','kuliner',NULL);
INSERT INTO pengetahuan_umum VALUES (16,'bot','kemampuan','pengetahuan_umum','menjawab pertanyaan umum berdasarkan data pengetahuan',NULL,NULL,1,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (17,'bot','kemampuan','perbandingan','membandingkan dua entitas dari domain yang sama',NULL,NULL,2,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (18,'bot','kemampuan','matematika','menghitung angka dan menyelesaikan soal cerita',NULL,NULL,3,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (19,'bot','kemampuan','konversi','mengkonversi satuan panjang berat volume suhu uang',NULL,NULL,4,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (20,'bot','kemampuan','waktu','mengidentifikasi waktu tanggal dan durasi',NULL,NULL,5,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (21,'bot','kemampuan','definisi_kata','memberikan definisi arti kata turunan dan frasa',NULL,NULL,6,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (22,'bot','keterbatasan','menggambar','tidak bisa menghasilkan gambar atau visual',NULL,NULL,1,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (23,'bot','keterbatasan','internet','tidak bisa mengakses internet atau data real-time',NULL,NULL,2,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (24,'bot','keterbatasan','multimedia','tidak bisa memutar musik video atau audio',NULL,NULL,3,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (25,'bot','keterbatasan','data_realtime','tidak memiliki data yang selalu diperbarui secara otomatis',NULL,NULL,4,'fakta',NULL,NULL,'sistem','sistem',NULL);
INSERT INTO pengetahuan_umum VALUES (26,'kucing dan anjing','perbedaan_utama','Kemandirian','Kucing bersifat mandiri sementara anjing lebih bergantung pada manusia',NULL,NULL,1,'analisis',NULL,NULL,'hewan','hewan',NULL);
INSERT INTO pengetahuan_umum VALUES (27,'kucing dan anjing','persamaan_utama','Mamalia Karnivora','Keduanya merupakan mamalia karnivora yang dipelihara sebagai hewan kesayangan',NULL,NULL,1,'analisis',NULL,NULL,'hewan','hewan',NULL);

-- T16: daftar_aspek
CREATE TABLE IF NOT EXISTS daftar_aspek (
    id INTEGER PRIMARY KEY,
    aspek TEXT NOT NULL UNIQUE,
    tampilan TEXT,
    kategori TEXT DEFAULT 'NETRAL'
);
INSERT INTO daftar_aspek VALUES (1,'definisi','pengertian','NETRAL');
INSERT INTO daftar_aspek VALUES (2,'fungsi','fungsi utama','POSITIF');
INSERT INTO daftar_aspek VALUES (3,'komponen','komponen utama','NETRAL');
INSERT INTO daftar_aspek VALUES (4,'jenis','jenis-jenis','NETRAL');
INSERT INTO daftar_aspek VALUES (5,'sejarah','sejarah','NETRAL');
INSERT INTO daftar_aspek VALUES (6,'cara_kerja','cara kerja','NETRAL');
INSERT INTO daftar_aspek VALUES (7,'manfaat','manfaat','POSITIF');
INSERT INTO daftar_aspek VALUES (8,'waktu_makan','waktu makan','NETRAL');
INSERT INTO daftar_aspek VALUES (9,'kelebihan','keunggulan','POSITIF');
INSERT INTO daftar_aspek VALUES (10,'kekurangan','kelemahan','NEGATIF');
INSERT INTO daftar_aspek VALUES (11,'kelemahan','kekurangan','NEGATIF');
INSERT INTO daftar_aspek VALUES (12,'keunggulan','kelebihan','POSITIF');
INSERT INTO daftar_aspek VALUES (13,'risiko','potensi bahaya','NEGATIF');
INSERT INTO daftar_aspek VALUES (14,'dampak_negatif','efek buruk','NEGATIF');

-- T17: kata_kunci_aspek (sinonim resolver)
CREATE TABLE IF NOT EXISTS kata_kunci_aspek (
    id INTEGER PRIMARY KEY,
    kata_kunci TEXT NOT NULL,
    aspek TEXT NOT NULL,
    tipe TEXT DEFAULT 'sinonim'
);
INSERT INTO kata_kunci_aspek VALUES (1,'komponen','komponen','exak');
INSERT INTO kata_kunci_aspek VALUES (2,'bagian','komponen','sinonim');
INSERT INTO kata_kunci_aspek VALUES (3,'unsur','komponen','sinonim');
INSERT INTO kata_kunci_aspek VALUES (4,'elemen','komponen','sinonim');
INSERT INTO kata_kunci_aspek VALUES (5,'faktor','komponen','sinonim');
INSERT INTO kata_kunci_aspek VALUES (6,'sub','komponen','sinonim');
INSERT INTO kata_kunci_aspek VALUES (7,'sejarah','sejarah','exak');
INSERT INTO kata_kunci_aspek VALUES (8,'asal-usul','sejarah','sinonim');
INSERT INTO kata_kunci_aspek VALUES (9,'awal mula','sejarah','sinonim');
INSERT INTO kata_kunci_aspek VALUES (10,'latar belakang','sejarah','sinonim');
INSERT INTO kata_kunci_aspek VALUES (11,'jenis','jenis','exak');
INSERT INTO kata_kunci_aspek VALUES (12,'macam-macam','jenis','sinonim');
INSERT INTO kata_kunci_aspek VALUES (13,'ragam','jenis','sinonim');
INSERT INTO kata_kunci_aspek VALUES (14,'tipe','jenis','sinonim');
INSERT INTO kata_kunci_aspek VALUES (15,'fungsi','fungsi','exak');
INSERT INTO kata_kunci_aspek VALUES (16,'kegunaan','fungsi','sinonim');
INSERT INTO kata_kunci_aspek VALUES (17,'manfaat','manfaat','exak');
INSERT INTO kata_kunci_aspek VALUES (18,'kelebihan','kelebihan','exak');
INSERT INTO kata_kunci_aspek VALUES (19,'keunggulan','kelebihan','sinonim');
INSERT INTO kata_kunci_aspek VALUES (20,'kekurangan','kekurangan','exak');
INSERT INTO kata_kunci_aspek VALUES (21,'kelemahan','kekurangan','sinonim');
INSERT INTO kata_kunci_aspek VALUES (22,'risiko','risiko','exak');
INSERT INTO kata_kunci_aspek VALUES (23,'bahaya','risiko','sinonim');
INSERT INTO kata_kunci_aspek VALUES (24,'dampak negatif','risiko','sinonim');
INSERT INTO kata_kunci_aspek VALUES (25,'definisi','definisi','exak');
INSERT INTO kata_kunci_aspek VALUES (26,'pengertian','definisi','sinonim');
INSERT INTO kata_kunci_aspek VALUES (27,'penjelasan','definisi','sinonim');
INSERT INTO kata_kunci_aspek VALUES (28,'penjabaran','definisi','sinonim');
INSERT INTO kata_kunci_aspek VALUES (29,'rincian','definisi','sinonim');
INSERT INTO kata_kunci_aspek VALUES (30,'uraian','definisi','sinonim');
INSERT INTO kata_kunci_aspek VALUES (31,'detail','definisi','sinonim');
INSERT INTO kata_kunci_aspek VALUES (32,'cara_kerja','cara_kerja','exak');
INSERT INTO kata_kunci_aspek VALUES (33,'cara','cara_kerja','sinonim');
INSERT INTO kata_kunci_aspek VALUES (34,'langkah','cara_kerja','sinonim');
INSERT INTO kata_kunci_aspek VALUES (35,'fase','cara_kerja','sinonim');
INSERT INTO kata_kunci_aspek VALUES (36,'tahap','cara_kerja','sinonim');
INSERT INTO kata_kunci_aspek VALUES (37,'proses','cara_kerja','sinonim');

-- T18: relasi_entitas
CREATE TABLE IF NOT EXISTS relasi_entitas (
    id INTEGER PRIMARY KEY,
    entitas_a TEXT NOT NULL,
    entitas_b TEXT NOT NULL,
    jenis_relasi TEXT NOT NULL
);
INSERT INTO relasi_entitas VALUES (1,'komputer','cpu','komponen');
INSERT INTO relasi_entitas VALUES (2,'komputer','ram','komponen');
INSERT INTO relasi_entitas VALUES (3,'komputer','vga','komponen');
INSERT INTO relasi_entitas VALUES (4,'komputer','motherboard','komponen');
INSERT INTO relasi_entitas VALUES (5,'komputer','storage','komponen');
INSERT INTO relasi_entitas VALUES (6,'komputer','laptop','jenis');
INSERT INTO relasi_entitas VALUES (7,'komputer','desktop','jenis');
INSERT INTO relasi_entitas VALUES (8,'komputer','server','jenis');
INSERT INTO relasi_entitas VALUES (9,'makan','minum','kaitan');
INSERT INTO relasi_entitas VALUES (10,'makan','restoran','kaitan');

-- T19: kata_angka (24 baris)
CREATE TABLE IF NOT EXISTS kata_angka (
    id INTEGER PRIMARY KEY,
    kata TEXT NOT NULL UNIQUE,
    nilai REAL NOT NULL,
    jenis TEXT NOT NULL
);
INSERT INTO kata_angka VALUES (1,'nol',0.0,'dasar');
INSERT INTO kata_angka VALUES (2,'se',1.0,'pengganti');
INSERT INTO kata_angka VALUES (3,'satu',1.0,'dasar');
INSERT INTO kata_angka VALUES (4,'dua',2.0,'dasar');
INSERT INTO kata_angka VALUES (5,'tiga',3.0,'dasar');
INSERT INTO kata_angka VALUES (6,'empat',4.0,'dasar');
INSERT INTO kata_angka VALUES (7,'lima',5.0,'dasar');
INSERT INTO kata_angka VALUES (8,'enam',6.0,'dasar');
INSERT INTO kata_angka VALUES (9,'tujuh',7.0,'dasar');
INSERT INTO kata_angka VALUES (10,'delapan',8.0,'dasar');
INSERT INTO kata_angka VALUES (11,'sembilan',9.0,'dasar');
INSERT INTO kata_angka VALUES (12,'belas',10.0,'belas');
INSERT INTO kata_angka VALUES (13,'puluh',10.0,'puluh');
INSERT INTO kata_angka VALUES (14,'ratus',100.0,'kelipatan');
INSERT INTO kata_angka VALUES (15,'ribu',1000.0,'kelipatan');
INSERT INTO kata_angka VALUES (16,'juta',1000000.0,'kelipatan');
INSERT INTO kata_angka VALUES (17,'miliar',1000000000.0,'kelipatan');
INSERT INTO kata_angka VALUES (18,'triliun',1000000000000.0,'kelipatan');
INSERT INTO kata_angka VALUES (19,'setengah',0.5,'pecahan');
INSERT INTO kata_angka VALUES (20,'seperempat',0.25,'pecahan');
INSERT INTO kata_angka VALUES (21,'sepersepuluh',0.1,'pecahan');
INSERT INTO kata_angka VALUES (22,'sepertiga',0.333,'pecahan');
INSERT INTO kata_angka VALUES (23,'tiga perempat',0.75,'pecahan_kompleks');
INSERT INTO kata_angka VALUES (24,'minus',-1.0,'negatif');

-- ================================================================
-- KELOMPOK C: WAKTU (4 Tabel)
-- ================================================================

-- T20: waktu_ekspresi
CREATE TABLE IF NOT EXISTS waktu_ekspresi (
    id INTEGER PRIMARY KEY,
    kata TEXT NOT NULL UNIQUE,
    jenis TEXT NOT NULL,
    offset INTEGER NOT NULL
);
INSERT INTO waktu_ekspresi VALUES (1,'hari ini','relatif_hari',0);
INSERT INTO waktu_ekspresi VALUES (2,'kemarin','relatif_hari',-1);
INSERT INTO waktu_ekspresi VALUES (3,'besok','relatif_hari',1);
INSERT INTO waktu_ekspresi VALUES (4,'lusa','relatif_hari',2);
INSERT INTO waktu_ekspresi VALUES (5,'kemarin lusa','relatif_hari',-2);
INSERT INTO waktu_ekspresi VALUES (6,'minggu ini','relatif_minggu',0);
INSERT INTO waktu_ekspresi VALUES (7,'minggu lalu','relatif_minggu',-7);
INSERT INTO waktu_ekspresi VALUES (8,'minggu depan','relatif_minggu',7);
INSERT INTO waktu_ekspresi VALUES (9,'bulan ini','relatif_bulan',0);
INSERT INTO waktu_ekspresi VALUES (10,'bulan lalu','relatif_bulan',-30);

-- T21: waktu_unit
CREATE TABLE IF NOT EXISTS waktu_unit (
    id INTEGER PRIMARY KEY,
    kata TEXT NOT NULL UNIQUE,
    detik INTEGER NOT NULL
);
INSERT INTO waktu_unit VALUES (1,'detik',1);
INSERT INTO waktu_unit VALUES (2,'menit',60);
INSERT INTO waktu_unit VALUES (3,'jam',3600);
INSERT INTO waktu_unit VALUES (4,'hari',86400);
INSERT INTO waktu_unit VALUES (5,'minggu',604800);
INSERT INTO waktu_unit VALUES (6,'bulan',2592000);

-- T22: nama_bulan
CREATE TABLE IF NOT EXISTS nama_bulan (
    id INTEGER PRIMARY KEY,
    nama TEXT NOT NULL UNIQUE,
    nomor INTEGER NOT NULL
);
INSERT INTO nama_bulan VALUES (1,'januari',1);
INSERT INTO nama_bulan VALUES (2,'februari',2);
INSERT INTO nama_bulan VALUES (3,'maret',3);
INSERT INTO nama_bulan VALUES (4,'april',4);
INSERT INTO nama_bulan VALUES (5,'mei',5);
INSERT INTO nama_bulan VALUES (6,'juni',6);
INSERT INTO nama_bulan VALUES (7,'juli',7);
INSERT INTO nama_bulan VALUES (8,'agustus',8);
INSERT INTO nama_bulan VALUES (9,'september',9);
INSERT INTO nama_bulan VALUES (10,'oktober',10);
INSERT INTO nama_bulan VALUES (11,'november',11);
INSERT INTO nama_bulan VALUES (12,'desember',12);

-- T23: nama_hari
CREATE TABLE IF NOT EXISTS nama_hari (
    id INTEGER PRIMARY KEY,
    nama TEXT NOT NULL UNIQUE,
    nomor INTEGER NOT NULL
);
INSERT INTO nama_hari VALUES (1,'minggu',0);
INSERT INTO nama_hari VALUES (2,'senin',1);
INSERT INTO nama_hari VALUES (3,'selasa',2);
INSERT INTO nama_hari VALUES (4,'rabu',3);
INSERT INTO nama_hari VALUES (5,'kamis',4);
INSERT INTO nama_hari VALUES (6,'jumat',5);
INSERT INTO nama_hari VALUES (7,'sabtu',6);

-- ================================================================
-- KELOMPOK D: KONVERSI (2 Tabel)
-- ================================================================

-- T24: konversi_satuan
CREATE TABLE IF NOT EXISTS konversi_satuan (
    id INTEGER PRIMARY KEY,
    dari TEXT NOT NULL,
    ke TEXT NOT NULL,
    faktor REAL NOT NULL,
    kategori TEXT NOT NULL,
    rumus_khusus TEXT
);
INSERT INTO konversi_satuan VALUES (1,'kilometer','meter',1000.0,'panjang',NULL);
INSERT INTO konversi_satuan VALUES (2,'meter','centimeter',100.0,'panjang',NULL);
INSERT INTO konversi_satuan VALUES (3,'kilometer','mil',0.621371,'panjang',NULL);
INSERT INTO konversi_satuan VALUES (4,'kilogram','gram',1000.0,'berat',NULL);
INSERT INTO konversi_satuan VALUES (5,'kilogram','pon',2.20462,'berat',NULL);
INSERT INTO konversi_satuan VALUES (6,'liter','mililiter',1000.0,'volume',NULL);
INSERT INTO konversi_satuan VALUES (7,'dolar','rupiah',15800.0,'uang',NULL);
INSERT INTO konversi_satuan VALUES (8,'euro','rupiah',17200.0,'uang',NULL);
INSERT INTO konversi_satuan VALUES (9,'fahrenheit','celsius',0.0,'suhu','f_to_c');
INSERT INTO konversi_satuan VALUES (10,'celsius','fahrenheit',0.0,'suhu','c_to_f');

-- T25: nama_satuan
CREATE TABLE IF NOT EXISTS nama_satuan (
    id INTEGER PRIMARY KEY,
    kata TEXT NOT NULL UNIQUE,
    satuan TEXT NOT NULL
);
INSERT INTO nama_satuan VALUES (1,'km','kilometer');
INSERT INTO nama_satuan VALUES (2,'m','meter');
INSERT INTO nama_satuan VALUES (3,'cm','centimeter');
INSERT INTO nama_satuan VALUES (4,'kg','kilogram');
INSERT INTO nama_satuan VALUES (5,'g','gram');
INSERT INTO nama_satuan VALUES (6,'l','liter');
INSERT INTO nama_satuan VALUES (7,'ml','mililiter');
INSERT INTO nama_satuan VALUES (8,'usd','dolar');
INSERT INTO nama_satuan VALUES (9,'$','dolar');
INSERT INTO nama_satuan VALUES (10,'idr','rupiah');

-- ================================================================
-- KELOMPOK E: MATEMATIKA (2 Tabel)
-- ================================================================

-- T26: operator_matematika
CREATE TABLE IF NOT EXISTS operator_matematika (
    id INTEGER PRIMARY KEY,
    kata TEXT NOT NULL UNIQUE,
    operator TEXT NOT NULL,
    prioritas INTEGER NOT NULL
);
INSERT INTO operator_matematika VALUES (1,'tambah','+',2);
INSERT INTO operator_matematika VALUES (2,'ditambah','+',2);
INSERT INTO operator_matematika VALUES (3,'plus','+',2);
INSERT INTO operator_matematika VALUES (4,'kurang','-',2);
INSERT INTO operator_matematika VALUES (5,'dikurangi','-',2);
INSERT INTO operator_matematika VALUES (6,'kali','*',3);
INSERT INTO operator_matematika VALUES (7,'dikali','*',3);
INSERT INTO operator_matematika VALUES (8,'bagi','/',3);
INSERT INTO operator_matematika VALUES (9,'dibagi','/',3);
INSERT INTO operator_matematika VALUES (10,'sisa','%',3);
INSERT INTO operator_matematika VALUES (11,'+','+',1);
INSERT INTO operator_matematika VALUES (12,'-','-',1);
INSERT INTO operator_matematika VALUES (13,'*','*',1);
INSERT INTO operator_matematika VALUES (14,'/','/',1);
INSERT INTO operator_matematika VALUES (15,'%','%',1);

-- T27: relasi_perhitungan
CREATE TABLE IF NOT EXISTS relasi_perhitungan (
    id INTEGER PRIMARY KEY,
    variabel_a TEXT NOT NULL,
    variabel_b TEXT NOT NULL,
    operator TEXT NOT NULL,
    variabel_c TEXT NOT NULL,
    rumus TEXT NOT NULL
);
INSERT INTO relasi_perhitungan VALUES (1,'harga_total','harga_per_item','*','jumlah_item','harga_total = harga_per_item * jumlah_item');
INSERT INTO relasi_perhitungan VALUES (2,'jumlah_item','total_berat','*','item_per_satuan','jumlah_item = total_berat * item_per_satuan');
INSERT INTO relasi_perhitungan VALUES (3,'berat_per_item','total_berat','/','jumlah_item','berat_per_item = total_berat / jumlah_item');
INSERT INTO relasi_perhitungan VALUES (4,'harga_per_kg','harga_per_item','*','item_per_kg','harga_per_kg = harga_per_item * item_per_kg');
INSERT INTO relasi_perhitungan VALUES (5,'persen','bagian','/','total','persen = (bagian / total) * 100');
INSERT INTO relasi_perhitungan VALUES (6,'selisih','nilai_akhir','-','nilai_awal','selisih = nilai_akhir - nilai_awal');
INSERT INTO relasi_perhitungan VALUES (7,'rata_rata','total','/','jumlah','rata_rata = total / jumlah');

-- ================================================================
-- KELOMPOK F: SISTEM & RESPONS (6 Tabel)
-- ================================================================

-- T28: pola_pembuka
CREATE TABLE IF NOT EXISTS pola_pembuka (
    id INTEGER PRIMARY KEY,
    pola TEXT NOT NULL,
    tipe TEXT NOT NULL
);
INSERT INTO pola_pembuka VALUES (1,'Ya, saya tahu.','afirmasi');
INSERT INTO pola_pembuka VALUES (2,'Tentu, berikut informasi yang saya miliki.','afirmasi');
INSERT INTO pola_pembuka VALUES (3,'Berdasarkan data saya,','informatif');
INSERT INTO pola_pembuka VALUES (4,'Oh, itu topik yang menarik.','antusias');
INSERT INTO pola_pembuka VALUES (5,'Baik, saya coba jelaskan.','transisi');
INSERT INTO pola_pembuka VALUES (6,'Saya paham soal itu.','afirmasi');
INSERT INTO pola_pembuka VALUES (7,'Tentu saja.','afirmasi');
INSERT INTO pola_pembuka VALUES (8,'Mari saya jelaskan.','transisi');
INSERT INTO pola_pembuka VALUES (9,'Halo!','sapaan');
INSERT INTO pola_pembuka VALUES (10,'Hai! Ada yang bisa saya bantu?','sapaan');

-- T29: pola_penutup
CREATE TABLE IF NOT EXISTS pola_penutup (
    id INTEGER PRIMARY KEY,
    pola TEXT NOT NULL,
    kondisi TEXT NOT NULL
);
INSERT INTO pola_penutup VALUES (1,'Apakah kamu ingin tahu lebih lanjut tentang [topik] ini?','ada_data_lanjut');
INSERT INTO pola_penutup VALUES (2,'Menurut data saya, [kata] mempunyai beberapa arti lain. Apakah kamu tertarik untuk mengetahuinya?','ada_arti_lain');
INSERT INTO pola_penutup VALUES (3,'Ada hal lain tentang [topik] yang ingin kamu bahas?','ada_data_terkait');
INSERT INTO pola_penutup VALUES (4,'Maaf, hanya itu yang saya ketahui tentang [topik]. Mau coba bahas topik lain?','data_habis');
INSERT INTO pola_penutup VALUES (5,'Saya tidak mempunyai data lebih lanjut soal [topik]. Ada hal lain yang ingin kamu tanyakan?','data_habis');
INSERT INTO pola_penutup VALUES (6,'Apakah kamu ingin tahu tentang [aspek] dari [topik]?','ada_aspek_lain');

-- T30: pola_respons
CREATE TABLE IF NOT EXISTS pola_respons (
    id INTEGER PRIMARY KEY,
    pola TEXT NOT NULL,
    tipe_jawaban TEXT NOT NULL
);
INSERT INTO pola_respons VALUES (1,'[S] [P] [O]','deklaratif');
INSERT INTO pola_respons VALUES (2,'[O] [P] oleh [S]','pasif');
INSERT INTO pola_respons VALUES (3,'[S] merupakan [O]','deklaratif');
INSERT INTO pola_respons VALUES (4,'Yang dimaksud [S] [P] [O]','informatif');
INSERT INTO pola_respons VALUES (5,'[S], yaitu [O], [P] [K]','deskriptif');
INSERT INTO pola_respons VALUES (6,'[S] dapat diartikan sebagai [O]','informatif');
INSERT INTO pola_respons VALUES (7,'[S] memiliki [O]','definisi');
INSERT INTO pola_respons VALUES (8,'[K], [S] [P] [O]','keterangan');
INSERT INTO pola_respons VALUES (9,'[S], [P] [O]','deklaratif_sederhana');
INSERT INTO pola_respons VALUES (10,'Jadi, [S] [P] [O]','penegasan');
INSERT INTO pola_respons VALUES (11,'Bukan, [S] [P] [O]','bantahan');
INSERT INTO pola_respons VALUES (12,'Tidak, [S] tidak [P] [O]','penolakan');
INSERT INTO pola_respons VALUES (13,'Menurut yang saya ketahui, [S] [P] [O]','koreksi_halus');
INSERT INTO pola_respons VALUES (14,'Sebagian benar, [S] [P] [O]','konfirmasi_parsial');
INSERT INTO pola_respons VALUES (15,'Ya, benar. [S] [P] [O]','konfirmasi');

-- T31: intent_pola (61 pola)
CREATE TABLE IF NOT EXISTS intent_pola (
    id INTEGER PRIMARY KEY,
    pola TEXT NOT NULL,
    intent TEXT NOT NULL,
    prioritas INTEGER DEFAULT 8
);
-- INTENT AWAL
INSERT INTO intent_pola VALUES (1,'halo','SAPAAN',10);
INSERT INTO intent_pola VALUES (2,'S + siapa','TANYA_ORANG',8);
INSERT INTO intent_pola VALUES (3,'S + apa + arti','TANYA_ARTI',8);
INSERT INTO intent_pola VALUES (4,'S + apa','TANYA_FAKTA',7);
INSERT INTO intent_pola VALUES (5,'S + di mana','TANYA_LOKASI',8);
INSERT INTO intent_pola VALUES (6,'S + kapan','TANYA_WAKTU',8);
INSERT INTO intent_pola VALUES (7,'S + berapa','TANYA_ANGKA',8);
INSERT INTO intent_pola VALUES (8,'mengapa + S','TANYA_ALASAN',7);
INSERT INTO intent_pola VALUES (9,'bagaimana + S','TANYA_CARA',7);
INSERT INTO intent_pola VALUES (10,'tolong + P','PERINTAH',9);
-- INTENT PERBANDINGAN
INSERT INTO intent_pola VALUES (11,'bandingkan + E1 + dan + E2','BANDINGKAN',8);
INSERT INTO intent_pola VALUES (12,'apa perbandingan + E1 + dengan + E2','BANDINGKAN',8);
INSERT INTO intent_pola VALUES (13,'perbandingan antara + E1 + dan + E2','BANDINGKAN',8);
INSERT INTO intent_pola VALUES (14,'apa persamaan + E1 + dan + E2','CARI_PERSAMAAN',8);
INSERT INTO intent_pola VALUES (15,'persamaan antara + E1 + dengan + E2','CARI_PERSAMAAN',8);
INSERT INTO intent_pola VALUES (16,'kesamaan + E1 + dan + E2','CARI_PERSAMAAN',8);
INSERT INTO intent_pola VALUES (17,'apa perbedaan + E1 + dan + E2','CARI_PERBEDAAN',8);
INSERT INTO intent_pola VALUES (18,'perbedaan antara + E1 + dengan + E2','CARI_PERBEDAAN',8);
INSERT INTO intent_pola VALUES (19,'bedanya + E1 + dan + E2','CARI_PERBEDAAN',8);
INSERT INTO intent_pola VALUES (20,'apa kelebihan + E1','CARI_KELEBIHAN',8);
INSERT INTO intent_pola VALUES (21,'keunggulan + E1','CARI_KELEBIHAN',8);
INSERT INTO intent_pola VALUES (22,'kelebihan + E1 + dibanding + E2','CARI_KELEBIHAN',8);
INSERT INTO intent_pola VALUES (23,'apa kekurangan + E1','CARI_KEKURANGAN',8);
INSERT INTO intent_pola VALUES (24,'kelemahan + E1','CARI_KEKURANGAN',8);
INSERT INTO intent_pola VALUES (25,'kekurangan + E1 + dibanding + E2','CARI_KEKURANGAN',8);
-- INTENT NEGASI & ARGUMENTASI
INSERT INTO intent_pola VALUES (26,'bukankan + S + P + O','VERIFIKASI_KLAIM',8);
INSERT INTO intent_pola VALUES (27,'S + itu + O + kan','VERIFIKASI_KLAIM',8);
INSERT INTO intent_pola VALUES (28,'menurut saya + S + adalah + O','VERIFIKASI_KLAIM',8);
INSERT INTO intent_pola VALUES (29,'S + pasti + O','VERIFIKASI_KLAIM',8);
INSERT INTO intent_pola VALUES (30,'apakah benar + S + itu + O','VERIFIKASI_KLAIM',8);
INSERT INTO intent_pola VALUES (31,'benar tidak + S + itu + O','VERIFIKASI_KLAIM',8);
INSERT INTO intent_pola VALUES (32,'apakah kamu bisa + P','TANYA_KEMAMPUAN',8);
INSERT INTO intent_pola VALUES (33,'kamu bisa + apa saja','TANYA_KEMAMPUAN',8);
INSERT INTO intent_pola VALUES (34,'bisakah kamu + P','TAYA_KEMAMPUAN',8);
INSERT INTO intent_pola VALUES (35,'salah + S + itu + O','TANTANGAN',8);
INSERT INTO intent_pola VALUES (36,'kamu salah + yang benar adalah + S','TANTANGAN',8);
-- INTENT PERSETUJUAN
INSERT INTO intent_pola VALUES (37,'ya','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (38,'oke','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (39,'betul','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (40,'bener','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (41,'setuju','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (42,'sip','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (43,'mantap','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (44,'iya','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (45,'paham','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (46,'mengerti','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (47,'oh begitu ya','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (48,'terima kasih','PERSETUJUAN',9);
INSERT INTO intent_pola VALUES (49,'makasih','PERSETUJUAN',9);
-- INTENT PENOLAKAN
INSERT INTO intent_pola VALUES (50,'tidak','PENOLAKAN',9);
INSERT INTO intent_pola VALUES (51,'salah','PENOLAKAN',9);
INSERT INTO intent_pola VALUES (52,'nggak','PENOLAKAN',9);
INSERT INTO intent_pola VALUES (53,'bukan','PENOLAKAN',9);
INSERT INTO intent_pola VALUES (54,'kurang tepat','PENOLAKAN',9);
INSERT INTO intent_pola VALUES (55,'tidak setuju','PENOLAKAN',9);
-- INTENT KONFIRMASI
INSERT INTO intent_pola VALUES (56,'benar kan','KONFIRMASI',9);
INSERT INTO intent_pola VALUES (57,'betul tidak','KONFIRMASI',9);
INSERT INTO intent_pola VALUES (58,'gitu kan','KONFIRMASI',9);
INSERT INTO intent_pola VALUES (59,'benar','KONFIRMASI',8);
INSERT INTO intent_pola VALUES (60,'bener kan','KONFIRMASI',9);
INSERT INTO intent_pola VALUES (61,'kan','KONFIRMASI',7);
-- INTENT TAMBAHAN: sapaan aktual
INSERT INTO intent_pola VALUES (62,'halo','SAPAAN',10);
INSERT INTO intent_pola VALUES (63,'hai','SAPAAN',10);
INSERT INTO intent_pola VALUES (64,'apa kabar','SAPAAN',10);
INSERT INTO intent_pola VALUES (65,'selamat pagi','SAPAAN',10);
INSERT INTO intent_pola VALUES (66,'selamat siang','SAPAAN',10);
INSERT INTO intent_pola VALUES (67,'selamat sore','SAPAAN',10);
INSERT INTO intent_pola VALUES (68,'selamat malam','SAPAAN',10);
INSERT INTO intent_pola VALUES (69,'assalamualaikum','SAPAAN',10);
-- INTENT TAMBAHAN: konversi dan hitung
INSERT INTO intent_pola VALUES (70,'berapa + angka + satuan','KONVERSI',8);
INSERT INTO intent_pola VALUES (71,'ubah + angka + satuan','KONVERSI',8);
INSERT INTO intent_pola VALUES (72,'konversi + satuan','KONVERSI',8);
INSERT INTO intent_pola VALUES (73,'hitung','HITUNG_SOAL',8);
INSERT INTO intent_pola VALUES (74,'berapa + hasil + dari','HITUNG_SOAL',8);
INSERT INTO intent_pola VALUES (75,'berapa + sama dengan','HITUNG_SOAL',8);
-- INTENT TAMBAHAN: definisi via "apa itu"
INSERT INTO intent_pola VALUES (76,'apa + itu + E1','TANYA_ARTI',9);
INSERT INTO intent_pola VALUES (77,'definisi + E1','TANYA_ARTI',9);
INSERT INTO intent_pola VALUES (78,'pengertian + E1','TANYA_ARTI',9);
INSERT INTO intent_pola VALUES (79,'artinya + apa','TANYA_ARTI',7);
-- INTENT: pengetahuan umum
INSERT INTO intent_pola VALUES (80,'ceritakan + E1','TANYA_FAKTA',8);
INSERT INTO intent_pola VALUES (81,'jelaskan + E1','TANYA_FAKTA',8);
INSERT INTO intent_pola VALUES (82,'tentang + E1','TANYA_FAKTA',7);

-- T32: referensi_konteks (persona mapping)
CREATE TABLE IF NOT EXISTS referensi_konteks (
    id INTEGER PRIMARY KEY,
    kata TEXT NOT NULL UNIQUE,
    merujuk_ke TEXT,
    jenis_persona TEXT,
    gaya TEXT,
    ganti_dengan TEXT
);
-- ORANG PERTAMA (user)
INSERT INTO referensi_konteks VALUES (1,'saya','pengguna','orang_pertama','netral','kamu');
INSERT INTO referensi_konteks VALUES (2,'aku','pengguna','orang_pertama','netral','kamu');
INSERT INTO referensi_konteks VALUES (3,'gue','pengguna','orang_pertama','slang','kamu');
INSERT INTO referensi_konteks VALUES (4,'gw','pengguna','orang_pertama','slang','kamu');
INSERT INTO referensi_konteks VALUES (5,'aing','pengguna','orang_pertama','slang','kamu');
INSERT INTO referensi_konteks VALUES (6,'beta','pengguna','orang_pertama','formal','anda');
-- ORANG KEDUA (bot)
INSERT INTO referensi_konteks VALUES (7,'kamu','bot','orang_kedua','netral','saya');
INSERT INTO referensi_konteks VALUES (8,'kau','bot','orang_kedua','netral','saya');
INSERT INTO referensi_konteks VALUES (9,'lo','bot','orang_kedua','slang','aku');
INSERT INTO referensi_konteks VALUES (10,'lu','bot','orang_kedua','slang','aku');
INSERT INTO referensi_konteks VALUES (11,'anda','bot','orang_kedua','formal','saya');
INSERT INTO referensi_konteks VALUES (12,'sia','bot','orang_kedua','formal','saya');
INSERT INTO referensi_konteks VALUES (13,'kowe','bot','orang_kedua','formal','saya');
INSERT INTO referensi_konteks VALUES (14,'engkau','bot','orang_kedua','formal','saya');
INSERT INTO referensi_konteks VALUES (15,'sampeyan','bot','orang_kedua','formal','saya');
INSERT INTO referensi_konteks VALUES (16,'kalian','bot','orang_kedua','netral','kami');
-- ORANG KETIGA
INSERT INTO referensi_konteks VALUES (17,'dia','subjek_terakhir','orang_ketiga',NULL,NULL);
INSERT INTO referensi_konteks VALUES (18,'ia','subjek_terakhir','orang_ketiga',NULL,NULL);
INSERT INTO referensi_konteks VALUES (19,'beliau','subjek_terakhir','orang_ketiga',NULL,NULL);
INSERT INTO referensi_konteks VALUES (20,'mereka','subjek_terakhir','orang_ketiga',NULL,NULL);
INSERT INTO referensi_konteks VALUES (21,'nya','objek_terakhir','orang_ketiga',NULL,NULL);
-- KATA TUNJUK
INSERT INTO referensi_konteks VALUES (22,'ini','entitas_aktif',NULL,NULL,NULL);
INSERT INTO referensi_konteks VALUES (23,'itu','topik',NULL,NULL,NULL);

-- T33: perintah_sistem
CREATE TABLE IF NOT EXISTS perintah_sistem (
    id INTEGER PRIMARY KEY,
    pola TEXT NOT NULL UNIQUE,
    aksi TEXT NOT NULL
);
INSERT INTO perintah_sistem VALUES (1,'ganti topik','BUAT_SESI_BARU');
INSERT INTO perintah_sistem VALUES (2,'keluar','AKHIRI_SESI');
INSERT INTO perintah_sistem VALUES (3,'stop','AKHIRI_SESI');
INSERT INTO perintah_sistem VALUES (4,'selesai','AKHIRI_SESI');

-- ================================================================
-- KELOMPOK G: SESI & PERCAKAPAN (4 Tabel)
-- ================================================================

-- T34: sesi
CREATE TABLE IF NOT EXISTS sesi (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nomor_sesi INTEGER,
    status TEXT DEFAULT 'aktif',
    waktu_mulai TEXT,
    waktu_akhir TEXT
);

-- T35: konteks_sesi
CREATE TABLE IF NOT EXISTS konteks_sesi (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sesi_id INTEGER NOT NULL,
    topik TEXT,
    tipe_topik TEXT,
    intent_terakhir TEXT,
    entitas_aktif TEXT,
    subjek_terakhir TEXT,
    objek_terakhir TEXT,
    percakapan_ke INTEGER DEFAULT 0,
    timestamp TEXT,
    gaya_bahasa TEXT
);

-- T36: riwayat_chat
CREATE TABLE IF NOT EXISTS riwayat_chat (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sesi_id INTEGER NOT NULL,
    pengguna TEXT,
    bot_respon TEXT,
    topik TEXT,
    intent TEXT,
    timestamp TEXT
);

-- T37: komponen_makna
CREATE TABLE IF NOT EXISTS komponen_makna (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    id_makna TEXT NOT NULL,
    kata_komponen TEXT NOT NULL
);
INSERT INTO komponen_makna VALUES (1,'mk.ma.1.1','makan');
INSERT INTO komponen_makna VALUES (2,'mk.ma.1.1','hati');
INSERT INTO komponen_makna VALUES (3,'mk.ma.2.1','makan');
INSERT INTO komponen_makna VALUES (4,'mk.ma.2.1','gaji');
INSERT INTO komponen_makna VALUES (5,'mk.ma.2.1','buta');
INSERT INTO komponen_makna VALUES (6,'mk.ma.3.1','makan');
INSERT INTO komponen_makna VALUES (7,'mk.ma.3.1','teman');
INSERT INTO komponen_makna VALUES (8,'mk.ma.3.1','sendiri');
INSERT INTO komponen_makna VALUES (9,'mk.ha.1.1','patah');
INSERT INTO komponen_makna VALUES (10,'mk.ha.1.1','hati');
INSERT INTO komponen_makna VALUES (11,'mk.ha.2.1','hati');
INSERT INTO komponen_makna VALUES (12,'mk.ha.3.1','buka');
INSERT INTO komponen_makna VALUES (13,'mk.ha.3.1','hati');
INSERT INTO komponen_makna VALUES (14,'mk.ma.4.1','makan');
INSERT INTO komponen_makna VALUES (15,'mk.ma.4.1','waktu');
INSERT INTO komponen_makna VALUES (16,'mk.ma.5.1','makan');
INSERT INTO komponen_makna VALUES (17,'mk.ma.5.1','malam');

-- ================================================================
-- INDEX untuk optimasi
-- ================================================================
CREATE INDEX IF NOT EXISTS idx_leksikon_kata ON leksikon(kata);
CREATE UNIQUE INDEX IF NOT EXISTS idx_arti_kata_id ON arti_kata(id_kata);
CREATE INDEX IF NOT EXISTS idx_arti_kata_kata ON arti_kata(kata);
CREATE UNIQUE INDEX IF NOT EXISTS idx_makna_kata_id ON makna_kata(id_kata);
CREATE INDEX IF NOT EXISTS idx_makna_kata_kata ON makna_kata(kata);
CREATE INDEX IF NOT EXISTS idx_komponen_kata ON komponen_makna(kata_komponen);
CREATE INDEX IF NOT EXISTS idx_komponen_makna ON komponen_makna(id_makna);
CREATE INDEX IF NOT EXISTS idx_pu_entitas_aspek ON pengetahuan_umum(entitas, aspek);
CREATE INDEX IF NOT EXISTS idx_pu_nomor ON pengetahuan_umum(entitas, aspek, nomor_item);
CREATE INDEX IF NOT EXISTS idx_pu_urutan ON pengetahuan_umum(entitas, aspek, urutan);
CREATE INDEX IF NOT EXISTS idx_pu_domain ON pengetahuan_umum(entitas, domain);
CREATE INDEX IF NOT EXISTS idx_pu_entitas ON pengetahuan_umum(entitas);
CREATE INDEX IF NOT EXISTS idx_relasi_a ON relasi_entitas(entitas_a);
CREATE INDEX IF NOT EXISTS idx_relasi_b ON relasi_entitas(entitas_b);
CREATE INDEX IF NOT EXISTS idx_konteks_sesi ON konteks_sesi(sesi_id);
CREATE INDEX IF NOT EXISTS idx_riwayat_sesi ON riwayat_chat(sesi_id);
CREATE INDEX IF NOT EXISTS idx_referensi_persona ON referensi_konteks(jenis_persona, gaya);
CREATE INDEX IF NOT EXISTS idx_aspek_kategori ON daftar_aspek(aspek, kategori);
CREATE INDEX IF NOT EXISTS idx_kunci_aspek ON kata_kunci_aspek(kata_kunci);
CREATE INDEX IF NOT EXISTS idx_turunan_kata ON kata_turunan(kata_turunan);
CREATE INDEX IF NOT EXISTS idx_turunan_dasar ON kata_turunan(kata_dasar);
CREATE INDEX IF NOT EXISTS idx_kiasan_kata1 ON kata_kiasan(kata1, kata2);
CREATE INDEX IF NOT EXISTS idx_intent_prioritas ON intent_pola(intent, prioritas);


-- ================================================================
-- KELOMPOK H: KONFIGURASI & ROUTING (2 Tabel)
-- ================================================================

-- T38: intent_handler (routing configuration for each intent)
CREATE TABLE IF NOT EXISTS intent_handler (
    id INTEGER PRIMARY KEY,
    intent TEXT NOT NULL UNIQUE,
    handler TEXT NOT NULL,
    tipe_pembuka TEXT,
    tipe_jawaban TEXT,
    tampilkan_langsung INTEGER DEFAULT 0,
    aksi TEXT
);
INSERT INTO intent_handler VALUES (1, 'SAPAAN', 'sapaan', 'sapaan', NULL, 1, NULL);
INSERT INTO intent_handler VALUES (2, 'PERSETUJUAN', 'persetujuan', 'afirmasi', 'konfirmasi', 1, NULL);
INSERT INTO intent_handler VALUES (3, 'PENOLAKAN', 'penolakan', 'transisi', 'penolakan', 1, NULL);
INSERT INTO intent_handler VALUES (4, 'KONFIRMASI', 'konfirmasi', 'afirmasi', 'konfirmasi', 1, NULL);
INSERT INTO intent_handler VALUES (5, 'TANYA_ARTI', 'definisi', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (6, 'TANYA_ARTI_LAIN', 'arti_lain', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (7, 'BANDINGKAN', 'bandingkan', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (8, 'CARI_PERSAMAAN', 'bandingkan', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (9, 'CARI_PERBEDAAN', 'bandingkan', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (10, 'CARI_KELEBIHAN', 'bandingkan', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (11, 'CARI_KEKURANGAN', 'bandingkan', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (12, 'TANYA_KEMAMPUAN', 'kemampuan', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (13, 'VERIFIKASI_KLAIM', 'verifikasi', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (14, 'TANTANGAN', 'verifikasi', 'informatif', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (15, 'KONVERSI', 'konversi', 'afirmasi', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (16, 'HITUNG_SOAL', 'matematika', 'afirmasi', NULL, 0, NULL);
INSERT INTO intent_handler VALUES (17, 'BUAT_SESI_BARU', 'sistem', NULL, NULL, 1, 'ganti_sesi');
INSERT INTO intent_handler VALUES (18, 'AKHIRI_SESI', 'sistem', NULL, NULL, 1, 'akhiri_sesi');

-- T39: konfigurasi_sistem (system configuration key-value pairs)
CREATE TABLE IF NOT EXISTS konfigurasi_sistem (
    id INTEGER PRIMARY KEY,
    kunci TEXT NOT NULL UNIQUE,
    nilai TEXT NOT NULL
);
INSERT INTO konfigurasi_sistem VALUES (1, 'ganti_sesi', '__GANTI_SESI__');
INSERT INTO konfigurasi_sistem VALUES (2, 'akhiri_sesi', '__AKHIRI_SESI__');
INSERT INTO konfigurasi_sistem VALUES (3, 'gaya_default', 'netral');
INSERT INTO konfigurasi_sistem VALUES (4, 'aspek_default', 'definisi');
INSERT INTO konfigurasi_sistem VALUES (5, 'status_aktif', 'aktif');
INSERT INTO konfigurasi_sistem VALUES (6, 'status_arsip', 'archived');
INSERT INTO konfigurasi_sistem VALUES (7, 'placeholder_user', '[USER]');
INSERT INTO konfigurasi_sistem VALUES (8, 'placeholder_bot', '[BOT]');
INSERT INTO konfigurasi_sistem VALUES (9, 'placeholder_topik', '[topik]');
INSERT INTO konfigurasi_sistem VALUES (10, 'placeholder_aspek', '[aspek]');
INSERT INTO konfigurasi_sistem VALUES (11, 'fallback_topik', 'topik');
INSERT INTO konfigurasi_sistem VALUES (12, 'fallback_aspek', 'aspek lain');
INSERT INTO konfigurasi_sistem VALUES (13, 'jawaban_kosong', 'Maaf, saya tidak memiliki data untuk itu.');
INSERT INTO konfigurasi_sistem VALUES (14, 'pesan_ganti_sesi', 'Sesi percakapan diganti.');
INSERT INTO konfigurasi_sistem VALUES (15, 'pesan_akhiri_sesi', 'Sesi percakapan diakhiri.');
INSERT INTO konfigurasi_sistem VALUES (16, 'tipe_pembuka_tanpa_isi', 'transisi');
INSERT INTO konfigurasi_sistem VALUES (17, 'tipe_pembuka_default', 'afirmasi');
INSERT INTO konfigurasi_sistem VALUES (18, 'tipe_jawaban_default', 'netral');
INSERT INTO konfigurasi_sistem VALUES (19, 'kondisi_penutup_sapaan', 'ada_data_lanjut');
INSERT INTO konfigurasi_sistem VALUES (20, 'handler_default', 'pengetahuan');
INSERT INTO konfigurasi_sistem VALUES (21, 'jenis_kalimat_default', 'kalimat berita');
INSERT INTO konfigurasi_sistem VALUES (22, 'jenis_kalimat_tanya', 'kalimat tanya');
INSERT INTO konfigurasi_sistem VALUES (23, 'jenis_kalimat_seruan', 'kalimat seruan');
INSERT INTO konfigurasi_sistem VALUES (24, 'jenis_kalimat_perintah', 'kalimat perintah');
INSERT INTO konfigurasi_sistem VALUES (25, 'persona_orang_pertama', 'orang_pertama');
INSERT INTO konfigurasi_sistem VALUES (26, 'persona_orang_kedua', 'orang_kedua');
INSERT INTO konfigurasi_sistem VALUES (27, 'rumus_f_to_c', 'f_to_c');
INSERT INTO konfigurasi_sistem VALUES (28, 'rumus_c_to_f', 'c_to_f');
INSERT INTO konfigurasi_sistem VALUES (29, 'pesan_konversi_gagal', 'Konversi dari %s ke %s tidak tersedia.');
INSERT INTO konfigurasi_sistem VALUES (30, 'kelas_bilangan', 'kata bilangan');
INSERT INTO konfigurasi_sistem VALUES (31, 'spok_label_subjek', 'S');
INSERT INTO konfigurasi_sistem VALUES (32, 'spok_label_predikat', 'P');
INSERT INTO konfigurasi_sistem VALUES (33, 'spok_label_objek', 'O');
INSERT INTO konfigurasi_sistem VALUES (34, 'spok_label_keterangan', 'K');
INSERT INTO konfigurasi_sistem VALUES (35, 'spok_separator', '-');
INSERT INTO konfigurasi_sistem VALUES (36, 'default_db', 'bot.db');
INSERT INTO konfigurasi_sistem VALUES (37, 'default_log', 'bot.log');
INSERT INTO konfigurasi_sistem VALUES (38, 'default_konfigurasi', 'konfigurasi.txt');
INSERT INTO konfigurasi_sistem VALUES (39, 'konektor_entitas', 'dan,dengan,dibanding,sama');
INSERT INTO konfigurasi_sistem VALUES (40, 'default_operator', '+');
INSERT INTO konfigurasi_sistem VALUES (41, 'kemampuan_entitas', 'bot');
INSERT INTO konfigurasi_sistem VALUES (42, 'kemampuan_aspek_1', 'kemampuan');
INSERT INTO konfigurasi_sistem VALUES (43, 'kemampuan_aspek_2', 'keterbatasan');
INSERT INTO konfigurasi_sistem VALUES (44, 'kondisi_penutup_data_habis', 'data_habis');
INSERT INTO konfigurasi_sistem VALUES (45, 'kondisi_penutup_arti_lain', 'ada_arti_lain');
INSERT INTO konfigurasi_sistem VALUES (46, 'kondisi_penutup_data_lanjut', 'ada_data_lanjut');
INSERT INTO konfigurasi_sistem VALUES (47, 'definisi_template', '%s adalah %s');
INSERT INTO konfigurasi_sistem VALUES (48, 'bandingkan_domain_berbeda', '%s dan %s termasuk kategori yang berbeda sehingga tidak dapat diperbandingkan secara langsung.');
INSERT INTO konfigurasi_sistem VALUES (49, 'konteks_referensi_subjek', 'subjek_terakhir');
INSERT INTO konfigurasi_sistem VALUES (50, 'konteks_referensi_objek', 'objek_terakhir');
INSERT INTO konfigurasi_sistem VALUES (51, 'konteks_referensi_topik', 'topik');
INSERT INTO konfigurasi_sistem VALUES (52, 'konteks_referensi_entitas', 'entitas_aktif');
INSERT INTO konfigurasi_sistem VALUES (53, 'format_konversi', '%.4g %s = %.4g %s');
INSERT INTO konfigurasi_sistem VALUES (54, 'format_bilangan_bulat', '%.0f');
INSERT INTO konfigurasi_sistem VALUES (55, 'format_bilangan_desimal', '%.4f');
INSERT INTO konfigurasi_sistem VALUES (56, 'label_contoh', '\n  Contoh: %s');
INSERT INTO konfigurasi_sistem VALUES (57, 'format_aspek_judul', '%s %d: ');
INSERT INTO konfigurasi_sistem VALUES (58, 'format_aspek_judul_penjelasan', '%s: %s');
INSERT INTO konfigurasi_sistem VALUES (59, 'format_aspek_pemisah', '- %s: %s');
INSERT INTO konfigurasi_sistem VALUES (60, 'format_bandingkan_entitas', '%s:\n');
INSERT INTO konfigurasi_sistem VALUES (61, 'format_bandingkan_baris', '- %s: %s\n');
INSERT INTO konfigurasi_sistem VALUES (63, 'tipe_pembuka_sapaan', 'sapaan');
INSERT INTO konfigurasi_sistem VALUES (64, 'tipe_jawaban_netral', 'netral');
INSERT INTO konfigurasi_sistem VALUES (65, 'prompt_user', 'User: ');
INSERT INTO konfigurasi_sistem VALUES (66, 'prompt_bot', '\nBot: ');
INSERT INTO konfigurasi_sistem VALUES (67, 'prompt_baris_baru', '\n\n');
INSERT INTO konfigurasi_sistem VALUES (68, 'gagal_buka_db', 'gagal membuka database: %s\n');
INSERT INTO konfigurasi_sistem VALUES (69, 'gagal_buat_skema', 'gagal membuat skema database.\n');
INSERT INTO konfigurasi_sistem VALUES (70, 'gagal_mulai_sesi', 'gagal memulai sesi percakapan.\n');
INSERT INTO konfigurasi_sistem VALUES (71, 'bot_dimulai', 'bot dimulai');
INSERT INTO konfigurasi_sistem VALUES (72, 'db_berhasil', 'database berhasil dibuka');
INSERT INTO konfigurasi_sistem VALUES (73, 'skema_siap', 'skema database siap');
INSERT INTO konfigurasi_sistem VALUES (74, 'sesi_dimulai', 'sesi percakapan dimulai');
INSERT INTO konfigurasi_sistem VALUES (75, 'sesi_diganti', 'sesi diganti');
INSERT INTO konfigurasi_sistem VALUES (76, 'sesi_dihakhiri', 'sesi diakhiri');
INSERT INTO konfigurasi_sistem VALUES (77, 'log_mulai', 'MULAI');
INSERT INTO konfigurasi_sistem VALUES (78, 'log_f1', 'F1_TOKENISASI');
INSERT INTO konfigurasi_sistem VALUES (79, 'log_f6', 'F6_INTENT');
INSERT INTO konfigurasi_sistem VALUES (80, 'log_f8', 'F8_PENALARAN');
INSERT INTO konfigurasi_sistem VALUES (81, 'log_selesai', 'SELESAI');

COMMIT;
