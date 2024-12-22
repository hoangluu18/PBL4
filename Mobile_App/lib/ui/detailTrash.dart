import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:pbl4/model/TrashInformation.dart';
import 'package:intl/intl.dart';
// class Detailtrash extends StatelessWidget {
//   TrashInformation trashInformation;
//   Detailtrash({super.key, required this.trashInformation});
//
//   @override
//   Widget build(BuildContext context) {
//     String time = DateFormat('yyyy-MM-dd – HH:mm').format(trashInformation.timestamp).toString();
//     String type = trashInformation.type;
//     String content = 'The ' "$type " ' has been sorted at ' "$time" '.';
//     return Scaffold(
//       appBar: AppBar(
//         title: Text("Detail Trash Information"),
//         backgroundColor: Colors.green,
//       ),
//       body: Center(
//         child: Column (
//           mainAxisAlignment: MainAxisAlignment.center,
//           children: [
//             Text(
//               trashInformation.type.toUpperCase(),
//               style: GoogleFonts.roboto(
//                   textStyle: const TextStyle(
//                     fontSize: 30,
//                     fontWeight: FontWeight.bold,)
//                 ),
//               ),
//             const SizedBox(height: 100),
//             Container(
//               width: 450,
//               height: 340,
//               decoration: BoxDecoration(
//                 borderRadius: BorderRadius.circular(12.0),
//                 border: Border.all(color: Colors.grey.shade300, width: 2),
//                 color: Colors.grey.shade200,
//               ),
//               child: ClipRRect(
//                 borderRadius: BorderRadius.circular(8.0),
//                 child: Image.network(
//                   trashInformation.imageURL,
//                   width: 450,
//                   height: 340,
//                   fit: BoxFit.cover,
//                   errorBuilder: (context, error, stackTrace) {
//                     return Image.asset(
//                       'assets/placeholder.png',
//                       width: 450,
//                       height: 340,
//                       fit: BoxFit.cover,
//                     );
//                   },
//                 ),
//               ),
//             ),
//             const SizedBox(height: 16),
//             Column(
//               mainAxisAlignment: MainAxisAlignment.center,
//               crossAxisAlignment: CrossAxisAlignment.center,
//               children: [
//                 Text(
//                   content,
//                   style: GoogleFonts.roboto(
//                     textStyle: const TextStyle(
//                       fontSize: 24,
//                       fontWeight: FontWeight.bold,)
//                   ),
//                 ),
//               ]
//             ),
//           ],
//         ),
//       )
//     );
//   }
// }

class DetailTrash extends StatelessWidget {
  Color color;
  final TrashInformation trashInformation;
  DetailTrash({super.key, required this.trashInformation, required this.color});

  @override
  Widget build(BuildContext context) {
    String time = DateFormat('yyyy-MM-dd – HH:mm').format(trashInformation.timestamp).toString();
    String type = trashInformation.type;
    String content = 'The ' "$type " ' has been sorted at ' "$time" '.';
    return Scaffold(
      appBar: AppBar(
        title: const Text("Detail Trash Information"),
        backgroundColor: Colors.green,
      ),
      body: SingleChildScrollView(
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            // Loại rác
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 16.0),
              child: Text(
                trashInformation.type.toUpperCase(),
                style: GoogleFonts.roboto(
                  textStyle: TextStyle(
                    fontSize: 28,
                    fontWeight: FontWeight.bold,
                    color: color,
                  ),
                ),
              ),
            ),
            // Card chứa hình ảnh
            Card(
              elevation: 6,
              margin: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(12),
              ),
              child: ClipRRect(
                borderRadius: BorderRadius.circular(12),
                child: Image.network(
                  trashInformation.imageURL,
                  height: 300,
                  width: double.infinity,
                  fit: BoxFit.cover,
                  errorBuilder: (context, error, stackTrace) {
                    return Image.asset(
                      'assets/placeholder.png',
                      height: 300,
                      width: double.infinity,
                      fit: BoxFit.cover,
                    );
                  },
                ),
              ),
            ),
            // Thông tin thời gian
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 20.0, vertical: 16),
              child: Text(
                content,
                textAlign: TextAlign.center,
                style: GoogleFonts.roboto(
                  textStyle: const TextStyle(
                    fontSize: 20,
                    color: Colors.black87,
                    fontWeight: FontWeight.w500,
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
