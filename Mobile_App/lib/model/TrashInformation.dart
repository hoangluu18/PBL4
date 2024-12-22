import 'package:cloud_firestore/cloud_firestore.dart';

class TrashInformation {
  final String type;
  final DateTime timestamp;
  final String imageURL;
  TrashInformation({required this.type, required this.timestamp, required this.imageURL});

  // Hàm chuyển đổi từ Firestore document sang model TrashInformation
  factory TrashInformation.fromFirestore(DocumentSnapshot doc) {
    final data = doc.data() as Map<String, dynamic>;
    return TrashInformation(
      type: (data?['Type'] ?? 'Unknown') as String,
      timestamp: DateTime.parse(data['Time'] as String),
      imageURL: (data?['imageURL'] ?? '') as String,
    );
  }
}

Stream<List<TrashInformation>> fetchTrashInformation()  {
  try {
    return FirebaseFirestore.instance
        .collection('Trash_Information')
        .orderBy('Time', descending: true)
        .snapshots()
        .map((querySnapshot) {
      return querySnapshot.docs
          .map((doc) => TrashInformation.fromFirestore(doc))
          .toList();
    });
  } catch (e) {
    print('Error fetching data: $e');
    return Stream.value([]); // Trả về một stream rỗng khi có lỗi
  }

}


