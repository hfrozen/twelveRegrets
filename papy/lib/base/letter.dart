import 'package:drift/native.dart';
import 'package:path_provider/path_provider.dart';
import 'package:path/path.dart' as p;
import 'package:drift/drift.dart';
import 'dart:io';

part 'letter.g.dart';

class Letters extends Table {
  IntColumn get id => integer().autoIncrement()();
  IntColumn get drawId => integer().nullable()();
  IntColumn get view => integer()();
  TextColumn get story => text()();
  TextColumn get plainStory => text()();
  DateTimeColumn get date => dateTime()();
  TextColumn get how => text()();
}

class Drawings extends Table {
  IntColumn get id => integer().autoIncrement()();
  TextColumn get paths => text().nullable()();
  TextColumn get figures => text().nullable()();
  TextColumn get preview => text().nullable()();
}

LazyDatabase _openConnection() {
  return LazyDatabase(() async {
    final dbFolder = await getApplicationDocumentsDirectory();
    final file = File(p.join(dbFolder.path, 'db.sqlite'));
    return NativeDatabase(file);
  });
}

@DriftDatabase(tables: [Letters, Drawings])
class LetterDatabase extends _$LetterDatabase {
  LetterDatabase() : super(_openConnection());

  @override
  int get schemaVersion => 3;

  Future<int> insertLetter(LettersCompanion letter) =>
      into(letters).insert(letter);
  Future<List<Letter>> getAllLetters() => select(letters).get();
  Stream<List<Letter>> watchAllLetters() => select(letters).watch();
  Future updateLetter(Letter letter) => update(letters).replace(letter);
  Future deleteLetter(Letter letter) => delete(letters).delete(letter);
  Future<Letter> getLetter(int id) {
    var query = select(letters);
    query.where((tbl) => tbl.id.equals(id));
    return query.getSingle();
  }
  Stream<List<Letter>> watchEntriesWithText(String searchContent) {
    var query = select(letters);
    query.where((t) => t.plainStory.like('%$searchContent%'));
    return query.watch();
  }

  Future<int> insertDrawing(DrawingsCompanion drawing) =>
      into(drawings).insert(drawing);
  Future updateDrawing(Drawing drawing) => update(drawings).replace(drawing);
  Future deleteDrawing(Drawing drawing) => delete(drawings).delete(drawing);
  Future<Drawing> getDrawing(int drawId) {
    var query = select(drawings);
    query.where((tbd) => tbd.id.equals(drawId));
    return query.getSingle();
  }
}
