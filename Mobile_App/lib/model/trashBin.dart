import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/widgets.dart';

class Trashbin {
  late String imageURL;
  late bool emptiness;
  late int count;
  late String bin;
  final DatabaseReference _databaseReference;

  Trashbin(
      {required this.imageURL,
      required this.emptiness,
      required this.count,
      required this.bin})
      : _databaseReference = FirebaseDatabase.instance.ref("$bin");

  void listenToEmptinessUpdates(void Function() onUpdate) {
    _databaseReference.onValue.listen((DatabaseEvent event) {
      final data = event.snapshot.value;
      if (data != null && data is Map) {
        if (data['state'] is bool) {
          emptiness = data['state'];
          onUpdate();
        } else {
          emptiness = false;
          onUpdate();
          debugPrint('Data k phải kiểu bool');
        }

        if (data['count'] is int) {
          count = data['count'];
          onUpdate();
        }
      }
    });
  }

  Future<void> updateEmptiness() async {
    int newper = DateTime.now().second;
    await _databaseReference.set(newper);
  }
}
